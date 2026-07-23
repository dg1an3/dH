"""
DVH uncertainty-band computation for hierarchical-Bayes treatment planning.

Implements Step 5 of HIERARCHICAL_BAYES_DESIGN.md (the customer-facing
deliverable): given converged per-phase variational posteriors
q(theta_p) = N(mu_p, var_p), sample N beamlet weight vectors, forward-
pass through a dose operator to get N dose samples, bin each into
per-structure DVHs, and report percentile bands (5/50/95) per structure.

Pipeline:
    (mu_p, var_p) --sample(N)--> N x n_beamlets --dose_operator--> N x n_voxels
                                                            --bin(dose, mask)--> N x n_bins (DVH per sample)
                                                            --percentile(axis=0)--> n_percentiles x n_bins

This is the deliverable that sells the MDDT story per the design doc:
DVH-with-uncertainty bands instead of point-estimate DVH curves.

The dose_operator can be either a 2-D matrix (linear dose-from-beamlets)
or a callable that maps a beamlet vector to a dose vector. In the real
brimstone pipeline this is the TERMA + spherical-kernel-convolution
forward pass; in tests it's a small dense matrix.

matplotlib is imported lazily so this module is usable in headless or
matplotlib-free environments.
"""

from __future__ import annotations

from typing import Callable, Mapping, Sequence, Tuple, Union

import numpy as np


DoseOperator = Union[np.ndarray, Callable[[np.ndarray], np.ndarray]]


def sample_phase_posterior(
    mu_p: np.ndarray,
    var_p: np.ndarray,
    n_samples: int,
    rng: np.random.Generator | None = None,
) -> np.ndarray:
    """
    Draw N beamlet-weight samples from the diagonal Gaussian q(theta_p).

    Args:
        mu_p: shape (n_beamlets,)
        var_p: shape (n_beamlets,), strictly positive
        n_samples: number of samples
        rng: optional np.random.Generator for reproducibility

    Returns:
        samples: shape (n_samples, n_beamlets)
    """
    mu_p = np.asarray(mu_p, dtype=np.float64)
    var_p = np.asarray(var_p, dtype=np.float64)
    if mu_p.shape != var_p.shape:
        raise ValueError(
            f"mu_p shape {mu_p.shape} != var_p shape {var_p.shape}"
        )
    if mu_p.ndim != 1:
        raise ValueError(f"mu_p must be 1-D, got shape {mu_p.shape}")
    if np.any(var_p <= 0):
        raise ValueError("var_p must be strictly positive")
    if n_samples < 1:
        raise ValueError(f"n_samples must be >= 1, got {n_samples}")

    if rng is None:
        rng = np.random.default_rng()
    sigma = np.sqrt(var_p)
    return mu_p[None, :] + rng.standard_normal((n_samples, mu_p.size)) * sigma[None, :]


def compute_dose(
    dose_operator: DoseOperator,
    beamlet_weights: np.ndarray,
) -> np.ndarray:
    """
    Forward-pass beamlet weights through the dose operator.

    Accepts either:
      - a 2-D matrix of shape (n_voxels, n_beamlets) -- dose = D @ w
      - a callable taking (n_beamlets,) and returning (n_voxels,)

    Args:
        dose_operator: matrix or callable
        beamlet_weights: shape (n_beamlets,) for a single sample, or
            (n_samples, n_beamlets) for batched evaluation.

    Returns:
        dose: shape (n_voxels,) or (n_samples, n_voxels) matching input.
    """
    beamlet_weights = np.asarray(beamlet_weights, dtype=np.float64)
    if isinstance(dose_operator, np.ndarray):
        if dose_operator.ndim != 2:
            raise ValueError(
                f"dose_operator matrix must be 2-D, got shape {dose_operator.shape}"
            )
        return beamlet_weights @ dose_operator.T

    if not callable(dose_operator):
        raise TypeError(
            f"dose_operator must be ndarray or callable, got {type(dose_operator).__name__}"
        )
    if beamlet_weights.ndim == 1:
        return np.asarray(dose_operator(beamlet_weights), dtype=np.float64)
    out = np.stack([dose_operator(w) for w in beamlet_weights])
    return out.astype(np.float64, copy=False)


def compute_dvh(
    dose: np.ndarray,
    structure_mask: np.ndarray,
    dose_bins: np.ndarray,
) -> np.ndarray:
    """
    Cumulative DVH for a single structure on one dose distribution.

    DVH(d) = |{v in structure : dose[v] >= d}| / |structure|

    Args:
        dose: shape (n_voxels,)
        structure_mask: shape (n_voxels,), boolean or {0, 1}
        dose_bins: shape (n_bins,), monotone non-decreasing dose levels

    Returns:
        dvh: shape (n_bins,), values in [0, 1], non-increasing in d.
    """
    dose = np.asarray(dose, dtype=np.float64)
    mask = np.asarray(structure_mask).astype(bool)
    if dose.shape != mask.shape:
        raise ValueError(
            f"dose shape {dose.shape} != mask shape {mask.shape}"
        )
    n_in_structure = int(mask.sum())
    if n_in_structure == 0:
        raise ValueError("structure_mask is empty (no voxels selected)")

    dose_in = dose[mask]
    bins = np.asarray(dose_bins, dtype=np.float64)
    # For each bin level d, count fraction of structure voxels with dose >= d.
    # Vectorized: broadcast dose_in over bins.
    counts = (dose_in[None, :] >= bins[:, None]).sum(axis=1)
    return counts / n_in_structure


def compute_dvh_bands(
    dose_samples: np.ndarray,
    structure_mask: np.ndarray,
    dose_bins: np.ndarray,
    percentiles: Sequence[float] = (5.0, 50.0, 95.0),
) -> np.ndarray:
    """
    Percentile DVH bands across N dose samples for one structure.

    Args:
        dose_samples: shape (n_samples, n_voxels)
        structure_mask: shape (n_voxels,)
        dose_bins: shape (n_bins,)
        percentiles: tuple/list of percentile values in [0, 100].

    Returns:
        bands: shape (len(percentiles), n_bins)
            bands[k, i] = percentiles[k]-th percentile of DVH(dose_bins[i])
            across the N samples.
    """
    dose_samples = np.asarray(dose_samples, dtype=np.float64)
    if dose_samples.ndim != 2:
        raise ValueError(
            f"dose_samples must be 2-D (n_samples, n_voxels), got {dose_samples.shape}"
        )
    pct = np.asarray(percentiles, dtype=np.float64)
    if np.any(pct < 0) or np.any(pct > 100):
        raise ValueError(f"percentiles must be in [0, 100]; got {percentiles}")

    # Stack per-sample DVHs into shape (n_samples, n_bins) then take
    # percentile along the sample axis.
    per_sample = np.stack([
        compute_dvh(d, structure_mask, dose_bins) for d in dose_samples
    ])
    return np.percentile(per_sample, pct, axis=0)


def dvh_uncertainty_bands(
    mu_p: np.ndarray,
    var_p: np.ndarray,
    dose_operator: DoseOperator,
    structures: Mapping[str, np.ndarray],
    n_samples: int = 200,
    dose_bins: np.ndarray | None = None,
    percentiles: Sequence[float] = (5.0, 50.0, 95.0),
    rng: np.random.Generator | None = None,
) -> Mapping[str, Tuple[np.ndarray, np.ndarray]]:
    """
    Full Step-5 pipeline: sample -> dose -> DVH -> percentile bands.

    Args:
        mu_p, var_p: converged per-phase posterior (or pooled mu_eta, var_eta).
        dose_operator: ndarray matrix or callable.
        structures: dict mapping structure name -> voxel mask.
        n_samples: number of Monte Carlo draws.
        dose_bins: dose levels to evaluate DVH at. If None, computed from
            the sample mean dose: linspace(0, 1.1*max(mean_dose), 100).
        percentiles: which percentiles to return.
        rng: optional np.random.Generator.

    Returns:
        dict mapping structure name -> (dose_bins, bands), where bands is
        shape (len(percentiles), len(dose_bins)).
    """
    samples = sample_phase_posterior(mu_p, var_p, n_samples, rng=rng)
    dose_samples = compute_dose(dose_operator, samples)

    if dose_bins is None:
        mean_dose = dose_samples.mean(axis=0)
        max_d = float(mean_dose.max())
        dose_bins = np.linspace(0.0, 1.1 * max_d if max_d > 0 else 1.0, 100)
    dose_bins = np.asarray(dose_bins, dtype=np.float64)

    out = {}
    for name, mask in structures.items():
        bands = compute_dvh_bands(dose_samples, mask, dose_bins, percentiles)
        out[name] = (dose_bins, bands)
    return out


def plot_dvh_bands(
    dose_bins: np.ndarray,
    bands: np.ndarray,
    structure_name: str = "",
    ax=None,
    percentiles: Sequence[float] = (5.0, 50.0, 95.0),
    color: str | None = None,
):
    """
    Plot DVH percentile bands (median + shaded uncertainty region) for one
    structure. matplotlib is imported lazily so this module loads in
    headless / matplotlib-free environments.

    Args:
        dose_bins: shape (n_bins,)
        bands: shape (len(percentiles), n_bins) -- assumed sorted low to high.
        structure_name: legend label.
        ax: optional matplotlib Axes.
        percentiles: corresponding to bands rows. Used to find median row
            for the line, and outer rows for the shaded band.
        color: optional color spec.

    Returns:
        ax: the Axes used.
    """
    import matplotlib.pyplot as plt

    if ax is None:
        _, ax = plt.subplots()
    pct = np.asarray(percentiles, dtype=np.float64)
    if bands.shape[0] != pct.size:
        raise ValueError(
            f"bands first dim ({bands.shape[0]}) must match percentiles ({pct.size})"
        )

    # Median row (closest percentile to 50).
    median_idx = int(np.argmin(np.abs(pct - 50.0)))
    line, = ax.plot(dose_bins, bands[median_idx], label=structure_name, color=color)
    color_used = line.get_color()

    # Shaded band from min to max percentile.
    lo_idx = int(np.argmin(pct))
    hi_idx = int(np.argmax(pct))
    if lo_idx != hi_idx:
        ax.fill_between(
            dose_bins,
            bands[lo_idx], bands[hi_idx],
            alpha=0.25, color=color_used,
        )

    ax.set_xlabel("Dose")
    ax.set_ylabel("Volume fraction >= dose")
    ax.set_ylim(0, 1.02)
    return ax
