"""
Course-level coordinate-ascent driver for hierarchical-Bayes treatment planning.

Implements Step 3 of HIERARCHICAL_BAYES_DESIGN.md:
  - pool_phases: mean-field Gaussian pooling of (mu_p, var_p) across phases
  - HierarchicalBayes: outer coordinate-ascent loop that wraps P phase-level
    optimizers, harvests (mu_p, var_p) from each, pools to (mu_eta, var_eta),
    updates each phase's CoursePriorTerm, repeats until eta stabilizes.

Naming note: the design doc writes pooling formulas with sigma-notation where
sigma means standard deviation (so sigma^-2 is precision). The Step-1 callback
contract names the surfaced quantity `sigma_weights`, but its value is actually
m_vAdaptVariance from the C++ code -- a variance, not a std. To avoid
ambiguity, this module uses `variances` as the parameter name; the math is
expressed in terms of precision (= 1 / variance), which matches what
CoursePriorTerm consumes anyway.
"""

from __future__ import annotations

from typing import Callable, List, Sequence, Tuple

import numpy as np

from .course_prior import CoursePriorTerm


PhaseOptimizer = Callable[[CoursePriorTerm], Tuple[np.ndarray, np.ndarray]]


def pool_phases(
    mus: Sequence[np.ndarray],
    variances: Sequence[np.ndarray],
    normalize: bool = False,
) -> Tuple[np.ndarray, np.ndarray]:
    """
    Mean-field Gaussian pooling across phases.

    Given P phase-level posteriors q(theta_p) = N(mu_p, var_p) (diagonal,
    per-beamlet), pool to a Course-level posterior q(eta) = N(mu_eta, var_eta)
    via the formulas from HIERARCHICAL_BAYES_DESIGN.md:

        precision_eta = (1/P) * sum_p precision_p          where precision_p = 1/var_p
        mu_eta        = (1/precision_eta) * (1/P) * sum_p precision_p * mu_p
                      = (sum_p precision_p * mu_p) / (sum_p precision_p)

    The (1/P) factor is part of the design doc's choice to model eta as a
    "typical phase" with average-precision rather than as the sum-of-evidence
    posterior; see the design doc for the rationale.

    Args:
        mus: list of P arrays, each shape (n_beamlets,), per-phase posterior means.
        variances: list of P arrays, each shape (n_beamlets,), per-phase posterior
            variances (i.e., m_vAdaptVariance from the C++ optimizer).
        normalize: if True, rescale each phase's variance to mean 1 before
            pooling. The ROW 2 prescription from
            HIERARCHICAL_BAYES_DESIGN.md "The Calibration Open Question":
            when shape is stable across phases but absolute magnitude is
            not (e.g., bootstrap variance polluted by line-search runaway
            on some phases), normalizing strips out the magnitude noise
            while preserving the relative per-beamlet uncertainty shape.

    Returns:
        (mu_eta, var_eta): each shape (n_beamlets,).
    """
    if len(mus) == 0:
        raise ValueError("pool_phases requires at least one phase")
    if len(mus) != len(variances):
        raise ValueError(
            f"mus and variances must have the same length; got {len(mus)} and {len(variances)}"
        )

    mu_arr = np.asarray(mus, dtype=np.float64)
    var_arr = np.asarray(variances, dtype=np.float64)
    if mu_arr.shape != var_arr.shape:
        raise ValueError(
            f"mus shape {mu_arr.shape} != variances shape {var_arr.shape}"
        )
    if mu_arr.ndim != 2:
        raise ValueError(
            f"expected 2-D stacks (P, n_beamlets); got mu shape {mu_arr.shape}"
        )
    if np.any(var_arr <= 0):
        raise ValueError("variances must be strictly positive")

    if normalize:
        # Per-phase rescale to mean-1. Preserves shape, strips magnitude.
        # Done as var / mean(var); equivalent to dividing precisions by their
        # per-phase mean precision (up to a constant that cancels).
        var_arr = var_arr / var_arr.mean(axis=1, keepdims=True)

    precisions = 1.0 / var_arr                          # (P, n_beamlets)
    precision_eta = precisions.mean(axis=0)              # (n_beamlets,)
    var_eta = 1.0 / precision_eta
    mu_eta = (precisions * mu_arr).sum(axis=0) / precisions.sum(axis=0)
    return mu_eta, var_eta


class HierarchicalBayes:
    """
    Coordinate-ascent driver for Course-level hierarchical Bayes.

    Wraps P phase-level optimizers and their CoursePriorTerms. Each outer
    iteration:
      1. E-step: call each phase_optimizer with its current CoursePriorTerm
         (which encodes the pull toward the current mu_eta); the optimizer
         returns the converged (mu_p, var_p).
      2. M-step: pool (mu_p, var_p) across phases to get (mu_eta, var_eta).
      3. Update each CoursePriorTerm: target_mu := mu_eta, precision := 1/var_eta.
      4. Convergence check: stop if mu_eta stabilizes.

    The driver is optimizer-agnostic: the phase_optimizer callable abstracts
    over whether the inner solve is the real C++ PlanOptimizer (once the
    wrapper exposes both the prior input and the variance output) or the
    pure-Python polak_ribiere_cg reference from Step 1.

    Args:
        phase_optimizers: P callables. Each takes the phase's CoursePriorTerm
            and returns (mu_p, var_p) where var_p is m_vAdaptVariance shape.
        course_prior_terms: P CoursePriorTerm instances. Their target_mu and
            precision attributes are mutated by the driver after each pool.
    """

    def __init__(
        self,
        phase_optimizers: List[PhaseOptimizer],
        course_prior_terms: List[CoursePriorTerm],
        normalize_variance: bool = False,
    ):
        if len(phase_optimizers) != len(course_prior_terms):
            raise ValueError(
                f"phase_optimizers ({len(phase_optimizers)}) and "
                f"course_prior_terms ({len(course_prior_terms)}) must have equal length"
            )
        if len(phase_optimizers) == 0:
            raise ValueError("need at least one phase")

        self.phase_optimizers = list(phase_optimizers)
        self.course_prior_terms = list(course_prior_terms)
        self.normalize_variance = bool(normalize_variance)
        self.history: List[dict] = []

    def run(self, max_outer_iters: int = 20, tol: float = 1e-4) -> dict:
        """
        Run coordinate ascent until mu_eta stabilizes or max_outer_iters.

        Convergence criterion: ||mu_eta_t - mu_eta_{t-1}|| / (||mu_eta_{t-1}|| + eps)
        below tol.

        Returns:
            dict with keys mu_eta, var_eta, outer_iters, converged, history.
        """
        mu_eta_prev: np.ndarray | None = None
        mu_eta = None
        var_eta = None
        converged = False
        last_iter = 0

        for outer_iter in range(max_outer_iters):
            last_iter = outer_iter

            # E-step
            mus = []
            variances = []
            for opt, prior in zip(self.phase_optimizers, self.course_prior_terms):
                mu_p, var_p = opt(prior)
                mus.append(np.asarray(mu_p, dtype=np.float64))
                variances.append(np.asarray(var_p, dtype=np.float64))

            # M-step
            mu_eta, var_eta = pool_phases(mus, variances, normalize=self.normalize_variance)
            precision_eta = 1.0 / var_eta

            # Update each phase's prior in place. The CoursePriorTerm holds
            # the mutated arrays directly so subsequent E-steps see the new pull.
            for prior in self.course_prior_terms:
                prior.target_mu = mu_eta.copy()
                prior.precision = precision_eta.copy()

            self.history.append({
                "outer_iter": outer_iter,
                "mu_eta": mu_eta.copy(),
                "var_eta": var_eta.copy(),
                "phase_mus": [m.copy() for m in mus],
                "phase_vars": [v.copy() for v in variances],
            })

            # Convergence test on mu_eta
            if mu_eta_prev is not None:
                denom = np.linalg.norm(mu_eta_prev) + 1e-10
                rel_change = np.linalg.norm(mu_eta - mu_eta_prev) / denom
                if rel_change < tol:
                    converged = True
                    break
            mu_eta_prev = mu_eta.copy()

        return {
            "mu_eta": mu_eta,
            "var_eta": var_eta,
            "outer_iters": last_iter + 1,
            "converged": converged,
            "history": self.history,
        }
