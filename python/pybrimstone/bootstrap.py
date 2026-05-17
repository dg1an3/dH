"""
Voxel-subsampling bootstrap for posterior-variance estimation.

Implements option 2 of the recommendation in
HIERARCHICAL_BAYES_DESIGN.md's "The Calibration Open Question": replace
the optimizer-internal sigma_weights (= m_vAdaptVariance) with a
bootstrap variance estimate that doesn't rely on linearization or
Hessian materialization.

Method: re-run the inner CG B times, each on a random subset of the
structure voxels. The per-parameter sample variance of the converged
mu vectors is the bootstrap variance estimate. By construction this
is a genuine measure of how sensitive the converged mu is to the
data, which is exactly the posterior-variance interpretation the
hierarchical pool needs.

BootstrapPhaseOptimizer is a drop-in replacement for PhaseOptimizer
where the returned (mu_p, var_p) has mu_p from a single full-data CG
run and var_p from the bootstrap. The HierarchicalBayes driver
doesn't know the difference -- the interface contract is the same.
"""

from __future__ import annotations

from typing import Callable, Optional, Tuple

import numpy as np

from .course_prior import CoursePriorTerm
from .phase_optimizer import PhaseOptimizer
from .prescription import Prescription


PrescriptionFactory = Callable[[Optional[np.random.Generator]], Prescription]


def subsample_mask(
    mask: np.ndarray,
    fraction: float = 0.5,
    rng: Optional[np.random.Generator] = None,
) -> np.ndarray:
    """
    Random voxel subsample of a structure mask.

    Picks ceil(fraction * |mask|) voxels uniformly at random without
    replacement. Returns a new mask of the same shape with only those
    voxels active.

    Args:
        mask: shape (n_voxels,), 0/1 (or float) array; nonzero entries
            are the structure.
        fraction: in (0, 1]. fraction=1.0 returns the input mask.
        rng: optional Generator; default uses default_rng().

    Returns:
        sub: shape (n_voxels,), float array with values in {0, 1}.
    """
    if not 0.0 < fraction <= 1.0:
        raise ValueError(f"fraction must be in (0, 1], got {fraction}")
    if rng is None:
        rng = np.random.default_rng()
    mask = np.asarray(mask).astype(float)
    active_idx = np.flatnonzero(mask > 0)
    if active_idx.size == 0:
        raise ValueError("input mask has no active voxels")
    n_keep = max(1, int(np.ceil(fraction * active_idx.size)))
    keep = rng.choice(active_idx, size=n_keep, replace=False)
    sub = np.zeros_like(mask)
    sub[keep] = mask[keep]
    return sub


class BootstrapPhaseOptimizer:
    """
    Phase optimizer that returns bootstrap-estimated posterior variance.

    Drop-in replacement for PhaseOptimizer: the call signature
    (prior) -> (mu_p, var_p) is identical, and HierarchicalBayes can
    use it interchangeably.

    Args:
        prescription_factory: callable taking
            (rng: np.random.Generator | None) -> Prescription.
            Called once with rng=None to build the "full" prescription
            (used for mu_p) and B times with random rngs to build
            subsampled prescriptions (used for var_p). The factory is
            responsible for using `subsample_mask` (or equivalent) on
            each structure mask when given a non-None rng. Putting
            mask construction in user code keeps subsampling strategy
            flexible (per-structure fractions, stratified, etc.).
        n_params: optimizer parameter dimension.
        n_bootstrap: B (number of subsampled refits). Default 20.
        initial_params, max_iter, tol, adaptive_variance:
            forwarded to the inner PhaseOptimizer instances.
        bootstrap_seed: base seed for the bootstrap RNG. Sub-seeds for
            individual bootstrap runs are derived deterministically
            from this base, so results are reproducible.

    Cost: B+1 full CG runs per BootstrapPhaseOptimizer call. For the
    HierarchicalBayes outer loop with O outer iterations and P phases,
    total cost is O * P * (B+1) inner CG runs. Bootstrap should be
    cheaper than evaluating finite differences across the full
    parameter space, but it is not free.
    """

    def __init__(
        self,
        prescription_factory: PrescriptionFactory,
        n_params: int,
        n_bootstrap: int = 20,
        initial_params: Optional[np.ndarray] = None,
        max_iter: int = 200,
        tol: float = 1e-4,
        adaptive_variance: Tuple[float, float] = (0.01, 1.0),
        max_step_norm: Optional[float] = 20.0,
        bootstrap_seed: int = 0,
    ):
        if not callable(prescription_factory):
            raise TypeError("prescription_factory must be callable")
        if n_bootstrap < 2:
            raise ValueError(f"n_bootstrap must be >= 2 for sample variance, got {n_bootstrap}")

        self.prescription_factory = prescription_factory
        self.n_params = int(n_params)
        self.n_bootstrap = int(n_bootstrap)
        self.initial_params = (
            np.zeros(self.n_params)
            if initial_params is None
            else np.asarray(initial_params, dtype=np.float64)
        )
        self.max_iter = int(max_iter)
        self.tol = float(tol)
        self.adaptive_variance = adaptive_variance
        self.max_step_norm = max_step_norm
        self.bootstrap_seed = int(bootstrap_seed)

    def _build_inner(self, prescription: Prescription) -> PhaseOptimizer:
        return PhaseOptimizer(
            prescription,
            n_params=self.n_params,
            initial_params=self.initial_params.copy(),
            max_iter=self.max_iter,
            tol=self.tol,
            adaptive_variance=self.adaptive_variance,
            max_step_norm=self.max_step_norm,
        )

    def __call__(
        self,
        prior: Optional[CoursePriorTerm] = None,
    ) -> Tuple[np.ndarray, np.ndarray]:
        # Full-data run for the point estimate mu_p. Use a fresh prior
        # instance per inner optimizer to avoid the install/uninstall
        # accounting interacting across bootstrap iterations.
        full_p = self.prescription_factory(None)
        mu_p, _ = self._build_inner(full_p)(prior=prior)

        # B bootstrap refits.
        mus = np.empty((self.n_bootstrap, self.n_params))
        for b in range(self.n_bootstrap):
            sub_rng = np.random.default_rng((self.bootstrap_seed, b))
            sub_p = self.prescription_factory(sub_rng)
            mu_b, _ = self._build_inner(sub_p)(prior=prior)
            mus[b] = mu_b

        # Per-parameter sample variance (ddof=1 for unbiased).
        var_p = mus.var(axis=0, ddof=1)
        var_p = np.maximum(var_p, 1e-6)
        return mu_p, var_p
