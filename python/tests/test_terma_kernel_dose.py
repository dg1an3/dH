"""
Tests for TermaKernelDoseCalc.

The headline physical checks: uniform density gives Beer-Lambert
exponential decay along the beam axis, density slabs produce
correspondingly faster decay, kernel convolution actually widens the
dose distribution laterally relative to TERMA, and the resulting
matrix is linear in the beamlet weights (Prescription's adjoint-via-
transpose contract).
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.terma_kernel_dose import TermaKernelDoseCalc


# ---------------------------------------------------------------------------
# Construction & validation
# ---------------------------------------------------------------------------

class TestConstruction:
    def test_basic(self):
        density = np.ones((4, 4, 4))
        centers = np.array([[2.0, 2.0]])
        calc = TermaKernelDoseCalc(density, centers, beamlet_width_mm=3.0,
                                    mu=0.02, kernel_sigma_mm=2.0)
        assert calc.density_volume.shape == (4, 4, 4)

    def test_rejects_2d_density(self):
        with pytest.raises(ValueError, match="3-D"):
            TermaKernelDoseCalc(np.zeros((4, 4)), np.array([[2.0, 2.0]]))

    def test_rejects_negative_density(self):
        d = np.ones((3, 3, 3))
        d[0, 0, 0] = -1.0
        with pytest.raises(ValueError, match="non-negative"):
            TermaKernelDoseCalc(d, np.array([[1.0, 1.0]]))

    def test_rejects_bad_centers_shape(self):
        with pytest.raises(ValueError, match="\\(n_beamlets, 2\\)"):
            TermaKernelDoseCalc(np.ones((3, 3, 3)), np.array([1.0, 2.0]))

    def test_rejects_nonpositive_beamlet_width(self):
        with pytest.raises(ValueError, match="beamlet_width"):
            TermaKernelDoseCalc(
                np.ones((3, 3, 3)), np.array([[1.0, 1.0]]),
                beamlet_width_mm=0.0,
            )

    def test_rejects_nonpositive_kernel_sigma(self):
        with pytest.raises(ValueError, match="kernel_sigma"):
            TermaKernelDoseCalc(
                np.ones((3, 3, 3)), np.array([[1.0, 1.0]]),
                kernel_sigma_mm=-1.0,
            )

    def test_scalar_voxel_spacing(self):
        calc = TermaKernelDoseCalc(
            np.ones((3, 3, 3)), np.array([[1.0, 1.0]]),
            voxel_spacing_mm=2.0,
        )
        assert np.allclose(calc.voxel_spacing_mm, 2.0)

    def test_tuple_voxel_spacing(self):
        calc = TermaKernelDoseCalc(
            np.ones((3, 3, 3)), np.array([[1.0, 1.0]]),
            voxel_spacing_mm=(1.0, 2.0, 3.0),
        )
        assert np.allclose(calc.voxel_spacing_mm, [1.0, 2.0, 3.0])


# ---------------------------------------------------------------------------
# TERMA physics
# ---------------------------------------------------------------------------

class TestTermaPhysics:
    def test_beer_lambert_decay_uniform_density(self):
        """
        Uniform density, single broad beamlet. TERMA along the beam
        axis at the beamlet center should follow exp(-mu * rho * z).
        """
        rho = 1.0  # g/cm^3 (water)
        mu = 0.05  # cm^2/g, exaggerated for visibility
        dz_cm = 0.1  # voxel_spacing 1 mm
        nz = 30
        density = np.full((9, 9, nz), rho)
        # Wide beamlet so center column is essentially flat profile
        calc = TermaKernelDoseCalc(
            density, beamlet_centers_2d=np.array([[4.0, 4.0]]),
            beamlet_width_mm=20.0, mu=mu, kernel_sigma_mm=1e-3,
            voxel_spacing_mm=1.0,
        )
        terma = calc.terma_for_beamlet(0)
        # Cum density path at center column: rho * dz * (k+1) cumulative
        # but the formula uses cumsum which evaluates *at* voxel k, so
        # attenuation factor at k is exp(-mu * rho * dz * (k+1)).
        # TERMA(k) = mu * rho * exp(-mu * rho * dz * (k+1)) * lateral(0)
        center_column = terma[4, 4, :]
        # Check successive ratio matches exp(-mu * rho * dz)
        expected_ratio = float(np.exp(-mu * rho * dz_cm))
        for k in range(1, nz):
            if center_column[k] > 1e-30:
                ratio = center_column[k] / center_column[k - 1]
                assert np.isclose(ratio, expected_ratio, rtol=1e-6), \
                    f"depth {k}: ratio {ratio:.6f}, expected {expected_ratio:.6f}"

    def test_higher_density_attenuates_faster(self):
        """A high-density slab between z and the deep tissue should
        produce stronger attenuation behind it. Magnitude of the effect
        follows Beer-Lambert: extra factor exp(-mu * extra_path_density)."""
        nx, ny, nz = 5, 5, 20
        density_uniform = np.full((nx, ny, nz), 1.0)
        density_with_slab = density_uniform.copy()
        # Thick, dense slab to make the effect dramatic: 10 voxels at
        # density 5 -> extra path-density 4 * 1.0 cm = 4.0 g/cm^2.
        # With mu = 0.2, expected extra attenuation = exp(-0.8) ~ 0.45.
        density_with_slab[:, :, 5:15] = 5.0
        mu = 0.2

        centers = np.array([[2.0, 2.0]])
        kwargs = dict(beamlet_centers_2d=centers, beamlet_width_mm=10.0,
                      mu=mu, kernel_sigma_mm=1e-3, voxel_spacing_mm=1.0)
        calc_u = TermaKernelDoseCalc(density_uniform, **kwargs)
        calc_s = TermaKernelDoseCalc(density_with_slab, **kwargs)

        terma_u = calc_u.terma_for_beamlet(0)
        terma_s = calc_s.terma_for_beamlet(0)
        # Compare deep-tissue TERMA at z=17 (well past the slab) where
        # the slab volume also has density 1.0 (so mu*rho factor is
        # the same; only the attenuation differs).
        density_with_slab[:, :, 15:] = 1.0  # already true above, just to be explicit
        ratio = terma_s[2, 2, 17] / terma_u[2, 2, 17]
        # Threshold matches Beer-Lambert expectation with some slack.
        assert ratio < 0.7, f"slab attenuation ratio {ratio:.3f}, expected < 0.7"

    def test_terma_zero_in_zero_density(self):
        """Voxels with zero density have zero TERMA contribution (mu*rho=0)."""
        density = np.ones((5, 5, 10))
        density[2, 2, :] = 0.0  # air column
        calc = TermaKernelDoseCalc(
            density, beamlet_centers_2d=np.array([[2.0, 2.0]]),
            beamlet_width_mm=10.0, mu=0.05, kernel_sigma_mm=1e-3,
            voxel_spacing_mm=1.0,
        )
        terma = calc.terma_for_beamlet(0)
        # TERMA along the zero-density column should be zero
        assert np.allclose(terma[2, 2, :], 0.0)

    def test_lateral_profile_normalized(self):
        """Beamlet lateral profile integrates to 1 over voxel area."""
        calc = TermaKernelDoseCalc(
            np.zeros((20, 20, 5)),
            beamlet_centers_2d=np.array([[10.0, 10.0]]),
            beamlet_width_mm=4.0,
            voxel_spacing_mm=1.0,
        )
        profile = calc._lateral_profile[0]
        integral = profile.sum() * calc.voxel_spacing_mm[0] * calc.voxel_spacing_mm[1]
        assert np.isclose(integral, 1.0, atol=1e-2)


# ---------------------------------------------------------------------------
# Kernel convolution widens the dose
# ---------------------------------------------------------------------------

class TestKernelConvolution:
    def test_dose_wider_than_terma_lateral(self):
        """The kernel-convolved dose should be wider than the bare
        TERMA in the lateral (x-y) directions."""
        density = np.ones((20, 20, 20))
        calc = TermaKernelDoseCalc(
            density, beamlet_centers_2d=np.array([[10.0, 10.0]]),
            beamlet_width_mm=2.0,  # narrow beamlet
            mu=0.05, kernel_sigma_mm=4.0,  # broad kernel
            voxel_spacing_mm=1.0,
        )
        terma = calc.terma_for_beamlet(0)
        dose = calc.dose_for_beamlet(0)
        # At a fixed depth, measure the lateral 1-D profile and compare
        # the half-max width.
        depth = 10
        t_lat = terma[:, 10, depth]
        d_lat = dose[:, 10, depth]
        # FWHM by simple half-max threshold (using each's own max)
        def fwhm(profile):
            m = profile.max()
            if m <= 0:
                return 0.0
            above = profile >= m / 2.0
            idx = np.flatnonzero(above)
            return idx[-1] - idx[0]
        assert fwhm(d_lat) > fwhm(t_lat), \
            f"dose FWHM {fwhm(d_lat)} should exceed TERMA FWHM {fwhm(t_lat)}"

    def test_dose_preserves_total_energy_approximately(self):
        """Gaussian-filter conserves mass; sum(dose) should match sum(TERMA)
        modulo edge-truncation losses."""
        density = np.ones((20, 20, 20))
        calc = TermaKernelDoseCalc(
            density, beamlet_centers_2d=np.array([[10.0, 10.0]]),
            beamlet_width_mm=4.0, mu=0.02, kernel_sigma_mm=2.0,
            voxel_spacing_mm=1.0,
        )
        terma = calc.terma_for_beamlet(0)
        dose = calc.dose_for_beamlet(0)
        # Within a few percent given that the kernel can leak across
        # the volume boundary.
        ratio = dose.sum() / terma.sum()
        assert 0.85 < ratio < 1.05, f"energy ratio out of range: {ratio:.3f}"


# ---------------------------------------------------------------------------
# Linear operator
# ---------------------------------------------------------------------------

class TestDoseOperator:
    def _make(self, n_beamlets=3, nx=10, ny=10, nz=15):
        density = np.ones((nx, ny, nz))
        centers = np.array([
            [nx / 2.0 + 0.5 * (i - n_beamlets / 2.0), ny / 2.0]
            for i in range(n_beamlets)
        ])
        return TermaKernelDoseCalc(
            density, beamlet_centers_2d=centers,
            beamlet_width_mm=2.0, mu=0.05, kernel_sigma_mm=2.0,
            voxel_spacing_mm=1.0,
        )

    def test_operator_shape(self):
        calc = self._make(n_beamlets=4, nx=8, ny=8, nz=10)
        D = calc.build_dose_operator()
        assert D.shape == (8 * 8 * 10, 4)

    def test_operator_is_linear(self):
        calc = self._make(n_beamlets=3, nx=10, ny=10, nz=12)
        D = calc.build_dose_operator()
        rng = np.random.default_rng(0)
        w1 = rng.uniform(0, 1, size=3)
        w2 = rng.uniform(0, 1, size=3)
        a, b = 0.7, 1.3
        combined = D @ (a * w1 + b * w2)
        separate = a * (D @ w1) + b * (D @ w2)
        assert np.allclose(combined, separate)

    def test_dose_for_beamlet_matches_column(self):
        calc = self._make(n_beamlets=2)
        D = calc.build_dose_operator()
        # Column 0 of D should equal the flattened dose-for-beamlet-0
        dose_b0 = calc.dose_for_beamlet(0)
        assert np.allclose(D[:, 0], dose_b0.ravel())


# ---------------------------------------------------------------------------
# Integration with Prescription
# ---------------------------------------------------------------------------

class TestIntegrationWithPrescription:
    def test_plugs_into_prescription(self):
        from pybrimstone.kl_term import KLDivTerm
        from pybrimstone.prescription import Prescription

        nx, ny, nz = 8, 8, 10
        density = np.ones((nx, ny, nz))
        centers = np.array([[4.0, 4.0], [4.5, 4.0], [3.5, 4.0]])
        calc = TermaKernelDoseCalc(
            density, beamlet_centers_2d=centers,
            beamlet_width_mm=2.0, mu=0.05, kernel_sigma_mm=2.0,
        )
        D = calc.build_dose_operator()

        n_voxels = nx * ny * nz
        ptv_mask = np.zeros(n_voxels)
        # PTV at z=4-6, central x-y
        for k in (4, 5, 6):
            for i in (3, 4):
                for j in (3, 4):
                    ptv_mask[i * ny * nz + j * nz + k] = 1.0

        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            ptv_mask, dose_min=0.001, dose_max=0.005, weight=1.0,
            bin_width=0.001, var_min=1e-6, var_max=1e-6,
        ))
        # Just verify evaluate returns finite cost + grad without crash.
        cost, grad = p.evaluate(np.zeros(3))
        assert np.isfinite(cost)
        assert grad.shape == (3,)
        assert np.all(np.isfinite(grad))
