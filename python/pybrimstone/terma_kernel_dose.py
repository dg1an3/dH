"""
TERMA + kernel-convolution dose calculator.

Physically-motivated stand-in for the real C++ pipeline in
RtModel/BeamDoseCalc.cpp + SphereConvolve.cpp + EnergyDepKernel.cpp.
Still a matrix-representable linear operator (dose = D @ weights), so
it slots into the existing Prescription path unchanged. The
gaussian_bump_dose_operator from dose_calc.py was a geometric stand-in
for prototyping the hierarchical-Bayes pipeline; this module replaces
it with something that respects the underlying physics.

Two-step calculation:

  1. TERMA (Total Energy Released per unit MAss): trace each beamlet
     through the CT density volume. At depth d along the beam axis,
     a beam carrying unit fluence at entry deposits

         TERMA(d) = mu * rho(d) * exp(-mu * integral_0_to_d rho(d') dd')

     where mu is the linear attenuation coefficient per unit density,
     rho(d) is the mass density at depth d, and the exponential is
     the Beer-Lambert attenuation factor through everything upstream.
     Lateral profile per beamlet is a Gaussian centered on the
     beamlet position, with FWHM = beamlet_width.

  2. Kernel convolution: convolve the TERMA volume with a 3-D
     isotropic Gaussian point kernel (sigma = kernel_sigma_mm) to
     redistribute energy released at one voxel into the surrounding
     dose. Real brimstone uses a tabulated MC-computed kernel
     (Brimstone/6MV_kernel.dat etc.); the isotropic Gaussian is a
     defensible prototype substitute that captures the right
     qualitative behavior (lateral spread, no anisotropy).

Simplifications relative to a clinical TPS:
  - Single beam direction along +z. Multi-beam plans are a clean
    future extension; the current Prescription API supports adding
    several DoseObjectiveTerms, but the dose operator is built
    assuming all beamlets share a direction.
  - Parallel beam (no divergence / inverse-square fluence drop).
  - Single mu (energy-independent attenuation).
  - Isotropic Gaussian kernel (real kernels have polar-angle anisotropy).
  - No heterogeneity correction beyond what density-weighted path-
    length naturally provides (i.e., no kernel scaling at material
    boundaries).
"""

from __future__ import annotations

from typing import Tuple

import numpy as np
from scipy.ndimage import gaussian_filter


class TermaKernelDoseCalc:
    """
    Linear beamlet-to-dose operator via TERMA + Gaussian kernel.

    Args:
        density_volume: shape (nx, ny, nz), mass density (g/cm^3). z is
            the beam-propagation axis; the beam enters at z=0 and
            traverses toward z=nz-1.
        beamlet_centers_2d: shape (n_beamlets, 2), (x, y) center of each
            beamlet in voxel coordinates (can be fractional).
        beamlet_width_mm: lateral FWHM of each beamlet's entry profile.
        mu: linear attenuation coefficient per unit mass density,
            in cm^-1 / (g/cm^3) = cm^2/g. Typical megavoltage photon
            value is ~0.02 cm^2/g.
        kernel_sigma_mm: sigma of the isotropic Gaussian dose kernel.
            Larger -> more lateral dose spread.
        voxel_spacing_mm: scalar or 3-tuple of voxel spacing in mm
            along each axis.
    """

    def __init__(
        self,
        density_volume: np.ndarray,
        beamlet_centers_2d: np.ndarray,
        beamlet_width_mm: float = 5.0,
        mu: float = 0.02,
        kernel_sigma_mm: float = 5.0,
        voxel_spacing_mm: float | Tuple[float, float, float] = 1.0,
    ):
        density_volume = np.asarray(density_volume, dtype=np.float64)
        if density_volume.ndim != 3:
            raise ValueError(
                f"density_volume must be 3-D (nx, ny, nz), got shape {density_volume.shape}"
            )
        if np.any(density_volume < 0):
            raise ValueError("density_volume must be non-negative")

        centers = np.asarray(beamlet_centers_2d, dtype=np.float64)
        if centers.ndim != 2 or centers.shape[1] != 2:
            raise ValueError(
                f"beamlet_centers_2d must have shape (n_beamlets, 2), got {centers.shape}"
            )
        if beamlet_width_mm <= 0:
            raise ValueError(f"beamlet_width_mm must be positive, got {beamlet_width_mm}")
        if mu < 0:
            raise ValueError(f"mu must be non-negative, got {mu}")
        if kernel_sigma_mm <= 0:
            raise ValueError(f"kernel_sigma_mm must be positive, got {kernel_sigma_mm}")

        if np.isscalar(voxel_spacing_mm):
            self.voxel_spacing_mm = np.array([voxel_spacing_mm] * 3, dtype=np.float64)
        else:
            self.voxel_spacing_mm = np.asarray(voxel_spacing_mm, dtype=np.float64)
            if self.voxel_spacing_mm.shape != (3,):
                raise ValueError(
                    f"voxel_spacing_mm must be scalar or shape (3,), got {self.voxel_spacing_mm.shape}"
                )

        self.density_volume = density_volume
        self.beamlet_centers_2d = centers
        self.beamlet_width_mm = float(beamlet_width_mm)
        self.mu = float(mu)
        self.kernel_sigma_mm = float(kernel_sigma_mm)

        # Pre-compute the Beer-Lambert attenuation factor along z. Same
        # for every beamlet because beam direction is fixed; only the
        # lateral profile differs.
        # cum_density_path[i,j,k] = sum_{k'<=k} density[i,j,k'] * dz_cm
        # so attenuation = exp(-mu * cum_density_path).
        dz_cm = self.voxel_spacing_mm[2] / 10.0  # mm -> cm
        cum_path = np.cumsum(self.density_volume * dz_cm, axis=2)
        self._attenuation = np.exp(-self.mu * cum_path)
        # TERMA per unit fluence per voxel (before lateral beamlet profile):
        # mu * rho * attenuation. Units: cm^2/g * g/cm^3 * (dimensionless)
        # -> cm^-1, integrated over a cm gives a dimensionless TERMA. The
        # absolute units are arbitrary for an inverse-planning prototype.
        self._terma_per_unit_fluence = self.mu * self.density_volume * self._attenuation

        # Pre-compute the lateral beamlet profile: a Gaussian centered at
        # each (cx, cy) with FWHM = beamlet_width_mm.
        nx, ny, _ = self.density_volume.shape
        self._lateral_profile = self._build_lateral_profiles(nx, ny, centers)

        # Kernel sigma in voxel units (per-axis).
        self._kernel_sigma_voxels = np.array([
            self.kernel_sigma_mm / self.voxel_spacing_mm[i] for i in range(3)
        ])

    # ------------------------------------------------------------------
    # Lateral beamlet profile
    # ------------------------------------------------------------------

    def _build_lateral_profiles(
        self,
        nx: int,
        ny: int,
        centers: np.ndarray,
    ) -> np.ndarray:
        """Returns shape (n_beamlets, nx, ny) of normalized Gaussian profiles."""
        # FWHM to sigma (in voxel units, x-axis spacing).
        sigma_x_vox = self.beamlet_width_mm / self.voxel_spacing_mm[0] / (2.0 * np.sqrt(2.0 * np.log(2.0)))
        sigma_y_vox = self.beamlet_width_mm / self.voxel_spacing_mm[1] / (2.0 * np.sqrt(2.0 * np.log(2.0)))
        inv_2sx2 = 1.0 / (2.0 * sigma_x_vox ** 2)
        inv_2sy2 = 1.0 / (2.0 * sigma_y_vox ** 2)

        xs = np.arange(nx, dtype=np.float64)
        ys = np.arange(ny, dtype=np.float64)
        X, Y = np.meshgrid(xs, ys, indexing="ij")  # (nx, ny)

        profiles = np.empty((centers.shape[0], nx, ny), dtype=np.float64)
        for b, (cx, cy) in enumerate(centers):
            dx = X - cx
            dy = Y - cy
            p = np.exp(-(dx * dx) * inv_2sx2 - (dy * dy) * inv_2sy2)
            # Normalize so each beamlet's lateral profile integrates to 1
            # over voxel area (so weight=1 means "unit fluence delivered").
            total = p.sum() * self.voxel_spacing_mm[0] * self.voxel_spacing_mm[1]
            if total > 0:
                p = p / total
            profiles[b] = p
        return profiles

    # ------------------------------------------------------------------
    # Per-beamlet TERMA and dose
    # ------------------------------------------------------------------

    def terma_for_beamlet(self, b: int) -> np.ndarray:
        """3-D TERMA volume contribution from beamlet b at unit weight."""
        if not 0 <= b < self.beamlet_centers_2d.shape[0]:
            raise IndexError(f"beamlet index {b} out of range")
        # TERMA = lateral profile * depth-dependent attenuated mu*rho
        return self._lateral_profile[b][:, :, None] * self._terma_per_unit_fluence

    def dose_for_beamlet(self, b: int) -> np.ndarray:
        """3-D dose contribution = TERMA convolved with the Gaussian kernel."""
        terma = self.terma_for_beamlet(b)
        return gaussian_filter(terma, sigma=self._kernel_sigma_voxels)

    # ------------------------------------------------------------------
    # Linear dose operator
    # ------------------------------------------------------------------

    def build_dose_operator(self) -> np.ndarray:
        """
        Materialize the (n_voxels, n_beamlets) matrix D such that
        dose.ravel() = D @ beamlet_weights. Plug into Prescription
        directly: Prescription(D, use_transform=True).
        """
        nx, ny, nz = self.density_volume.shape
        n_voxels = nx * ny * nz
        n_beamlets = self.beamlet_centers_2d.shape[0]
        D = np.empty((n_voxels, n_beamlets), dtype=np.float64)
        for b in range(n_beamlets):
            D[:, b] = self.dose_for_beamlet(b).ravel()
        return D
