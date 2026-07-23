"""
Simple linear dose calculator for prototyping.

This is a stand-in for the real C++ TERMA + spherical-kernel-convolution
pipeline (RtModel/BeamDoseCalc.cpp + SphereConvolve.cpp + EnergyDepKernel.cpp).
The real version ray-traces beamlets through the CT density volume to
get TERMA, then convolves with a pre-computed energy deposition kernel
that depends on beam energy. None of that is here.

What IS here: each beamlet contributes a Gaussian-shaped dose centered
at a target position in a 3D voxel grid. The result is a linear
operator from beamlet weights to per-voxel dose, materialized as an
(n_voxels, n_beamlets) matrix. Suitable for prototyping the hierarchical-
Bayes pipeline on synthetic geometry; not clinically meaningful.

The matrix form lets Prescription get its dose adjoint "for free" via
matrix transpose. When the real C++ pipeline gets wrapped, swap in a
(forward, adjoint) callable pair instead -- the Prescription API
supports both already.
"""

from __future__ import annotations

from typing import Sequence, Tuple

import numpy as np


def gaussian_bump_dose_operator(
    grid_shape: Tuple[int, int, int],
    beamlet_centers: np.ndarray,
    sigma: float = 2.0,
    voxel_spacing: float = 1.0,
) -> np.ndarray:
    """
    Build a Gaussian-bump beamlet-to-dose matrix.

    Each beamlet b contributes
        contribution[v] = exp(-||voxel_pos[v] - center[b]||^2 / (2 * sigma^2))
    to the dose, where voxel_pos[v] is the position of voxel v in mm.

    Args:
        grid_shape: (nx, ny, nz) integer voxel grid shape.
        beamlet_centers: shape (n_beamlets, 3), voxel-index coordinates
            (can be fractional) of each beamlet's center.
        sigma: Gaussian width in voxels.
        voxel_spacing: physical spacing (mm) per voxel (uniform; not
            currently used for anisotropic spacing).

    Returns:
        D: shape (n_voxels, n_beamlets), where n_voxels = nx*ny*nz.
            dose = D @ beamlet_weights.
    """
    nx, ny, nz = grid_shape
    n_voxels = nx * ny * nz
    centers = np.asarray(beamlet_centers, dtype=np.float64)
    if centers.ndim != 2 or centers.shape[1] != 3:
        raise ValueError(
            f"beamlet_centers must have shape (n_beamlets, 3), got {centers.shape}"
        )
    if sigma <= 0:
        raise ValueError(f"sigma must be positive, got {sigma}")

    n_beamlets = centers.shape[0]
    xs = np.arange(nx, dtype=np.float64) * voxel_spacing
    ys = np.arange(ny, dtype=np.float64) * voxel_spacing
    zs = np.arange(nz, dtype=np.float64) * voxel_spacing
    X, Y, Z = np.meshgrid(xs, ys, zs, indexing="ij")
    voxel_pos = np.stack([X.ravel(), Y.ravel(), Z.ravel()], axis=1)  # (n_voxels, 3)
    centers_phys = centers * voxel_spacing
    sigma_phys = sigma * voxel_spacing
    inv_two_sigma_sq = 1.0 / (2.0 * sigma_phys * sigma_phys)

    D = np.zeros((n_voxels, n_beamlets), dtype=np.float64)
    for b in range(n_beamlets):
        diff = voxel_pos - centers_phys[b]
        d2 = np.einsum("ij,ij->i", diff, diff)
        D[:, b] = np.exp(-d2 * inv_two_sigma_sq)
    return D


def grid_index_for_position(
    position: Sequence[float],
    grid_shape: Tuple[int, int, int],
) -> int:
    """Convenience: flat-array index for an integer (i, j, k) voxel coordinate."""
    nx, ny, nz = grid_shape
    i, j, k = position
    return int(i * ny * nz + j * nz + k)
