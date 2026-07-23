"""
Phase-level optimizer bridging Prescription + CG to the
hierarchical-Bayes outer loop.

The HierarchicalBayes driver expects a phase_optimizer callable that
takes a CoursePriorTerm (the current Course-level prior pull) and
returns (mu_p, var_p): the converged beamlet weights and per-beamlet
posterior variance for that phase. This module implements that
interface as a class wrapping a Prescription + CG configuration.

Variance harvesting: the CG callback surfaces sigma_weights (=
m_vAdaptVariance in the C++ code). We grab the value at the last
callback invocation and return it as var_p. Per the Step-3 caveat
in HIERARCHICAL_BAYES_DESIGN.md, this is the JOINT variance (data
+ prior) and pooling it via HierarchicalBayes will exhibit O(1/n)
convergence rather than geometric. A future PhaseOptimizer extension
can compute data-only variance by toggling the prior off for a final
variance-only forward pass.
"""

from __future__ import annotations

from typing import Optional, Tuple

import numpy as np

from .course_prior import CoursePriorTerm
from .numerics.conjugate_gradient import polak_ribiere_cg
from .prescription import Prescription


class PhaseOptimizer:
    """
    Wraps a Prescription + CG configuration into a callable that
    HierarchicalBayes can invoke.

    The same PhaseOptimizer instance should be reused across outer
    iterations -- it mutates its Prescription in place to swap the
    Course prior between calls. This avoids re-binding histogram
    state and target gbins on every outer iteration.

    Args:
        prescription: Prescription configured with the data-side terms
            (KLDivTerms etc.). Beamlet-side Course prior is managed by
            this class.
        n_params: number of optimizer parameters (matches dose-operator
            beamlet count and Prescription's transform domain).
        initial_params: optional starting params; default is zeros (which
            after sigmoid maps to INPUT_SCALE / 2 per beamlet).
        max_iter, tol: inner CG controls.
        adaptive_variance: tuple (var_min, var_max) for the dynamic-
            covariance update inside CG.
        max_step_norm: optional cap on the L2 norm of the CG step in
            optimizer-parameter space. Defaults to 20.0, which is large
            enough not to bind on legitimate steps (the sigmoid maps
            |x|=20 to essentially saturated already) but small enough
            to prevent the line-search runaway documented in
            HIERARCHICAL_BAYES_DESIGN.md "The Calibration Open Question"
            -- without this cap, Brent can find lambda ~ 1e7 in flat-cost
            saturation regions and the optimizer params escape. Set to
            None to disable.
    """

    def __init__(
        self,
        prescription: Prescription,
        n_params: int,
        initial_params: Optional[np.ndarray] = None,
        max_iter: int = 200,
        tol: float = 1e-4,
        adaptive_variance: Tuple[float, float] = (0.01, 1.0),
        max_step_norm: Optional[float] = 20.0,
    ):
        self.prescription = prescription
        self.n_params = int(n_params)
        self.initial_params = (
            np.zeros(self.n_params) if initial_params is None else np.asarray(initial_params, dtype=np.float64)
        )
        if self.initial_params.shape != (self.n_params,):
            raise ValueError(
                f"initial_params shape {self.initial_params.shape} != ({self.n_params},)"
            )
        self.max_iter = int(max_iter)
        self.tol = float(tol)
        self.adaptive_variance = adaptive_variance
        self.max_step_norm = max_step_norm

        # Track the Course prior term currently installed in the prescription
        # so we can detect when HierarchicalBayes hands us a new one and
        # avoid stacking priors across outer iterations.
        self._installed_prior: Optional[CoursePriorTerm] = None

    def _install_prior(self, prior: Optional[CoursePriorTerm]) -> None:
        if self._installed_prior is not None:
            try:
                self.prescription.beamlet_terms.remove(self._installed_prior)
            except ValueError:
                pass
        if prior is not None:
            self.prescription.add_beamlet_term(prior)
        self._installed_prior = prior

    def __call__(self, prior: Optional[CoursePriorTerm] = None) -> Tuple[np.ndarray, np.ndarray]:
        """Run CG with the given Course prior, return (mu_p, var_p)."""
        self._install_prior(prior)

        def f(p):
            return self.prescription.evaluate_cost_only(p)

        def grad_f(p):
            _, g = self.prescription.evaluate(p)
            return g

        # Default variance is var_max from the AV config. CG only fires
        # the callback after the AV update each iteration; if CG converges
        # in 0..1 iterations the callback may not run at all, so the
        # fallback must be a sensible AV-initial value (matches the C++
        # InitializeDynamicCovariance behavior).
        _, var_max = self.adaptive_variance
        last_sigma = [np.full(self.n_params, var_max)]

        def cb(**kw):
            sw = kw.get("sigma_weights")
            if sw is not None:
                last_sigma[0] = sw

        x_final, _, _, _ = polak_ribiere_cg(
            f, grad_f, self.initial_params.copy(),
            callback=cb,
            adaptive_variance=self.adaptive_variance,
            max_iter=self.max_iter,
            tol=self.tol,
            max_step_norm=self.max_step_norm,
        )

        # Variance lower-bounded to keep pool_phases / Course prior precision finite.
        var_p = np.maximum(last_sigma[0], 1e-6)
        return x_final, var_p
