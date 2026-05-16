"""
Course-level prior term for hierarchical-Bayes treatment planning.

Implements Step 2 of HIERARCHICAL_BAYES_DESIGN.md: a pure-Python objective
term that pulls per-phase beamlet weights toward a Course-level posterior
mean with a per-beamlet precision (the inverse of the Course-level posterior
variance).

The term lives in beamlet-weight space rather than dose space -- this is the
"option (a)" resolution of Open Question 4 in HIERARCHICAL_BAYES_DESIGN.md,
which the design doc flagged as smaller-change and structure-preserving.
A new BeamletObjectiveTerm base class encodes that contract; the existing
dose-space ObjectiveTerm interface in CYTHON_WRAPPER_DESIGN.md is unaffected.
"""

from __future__ import annotations

from typing import Tuple, Union

import numpy as np


Precision = Union[float, np.ndarray]


class BeamletObjectiveTerm:
    """
    Objective term that evaluates in beamlet-weight space.

    Sibling of the dose-space ObjectiveTerm from CYTHON_WRAPPER_DESIGN.md:95-100.
    The hierarchical-Bayes Course prior is naturally a Gaussian over beamlet
    weights, so it skips the dose-calc step in its evaluation. Other beamlet-
    space regularizers (L1, smoothness, etc.) can subclass this too.
    """

    def evaluate(self, beamlet_weights: np.ndarray) -> Tuple[float, np.ndarray]:
        """Return (cost, gradient w.r.t. beamlet_weights)."""
        raise NotImplementedError


class CoursePriorTerm(BeamletObjectiveTerm):
    """
    Quadratic prior pull toward a Course-level posterior mean.

    Cost:     0.5 * sum_i precision[i] * (w[i] - target_mu[i])**2
    Gradient: precision * (w - target_mu)

    The precision corresponds to 1 / sigma_eta**2 from the hierarchical
    pooling in HIERARCHICAL_BAYES_DESIGN.md. The outer-loop driver
    (Step 3) mutates target_mu and precision between optimizer.optimize()
    calls; this term itself is stateless w.r.t. the optimizer.

    Args:
        target_mu: Course-level mean, shape (n_beamlets,). Mutable via
            the .target_mu attribute -- the hierarchical driver updates
            it after each pooling step.
        precision: 1 / sigma_eta**2. Scalar broadcasts to all beamlets;
            an array applies per-beamlet precisions.
    """

    def __init__(self, target_mu: np.ndarray, precision: Precision):
        target_mu = np.asarray(target_mu, dtype=np.float64)
        if target_mu.ndim != 1:
            raise ValueError(
                f"target_mu must be 1-D, got shape {target_mu.shape}"
            )

        precision_arr = np.asarray(precision, dtype=np.float64)
        if precision_arr.ndim == 0:
            precision_arr = np.full_like(target_mu, float(precision_arr))
        elif precision_arr.shape != target_mu.shape:
            raise ValueError(
                f"precision shape {precision_arr.shape} must match "
                f"target_mu shape {target_mu.shape} (or be a scalar)"
            )
        if np.any(precision_arr < 0):
            raise ValueError("precision must be non-negative")

        self.target_mu = target_mu
        self.precision = precision_arr

    def evaluate(self, beamlet_weights: np.ndarray) -> Tuple[float, np.ndarray]:
        beamlet_weights = np.asarray(beamlet_weights, dtype=np.float64)
        if beamlet_weights.shape != self.target_mu.shape:
            raise ValueError(
                f"beamlet_weights shape {beamlet_weights.shape} must match "
                f"target_mu shape {self.target_mu.shape}"
            )
        diff = beamlet_weights - self.target_mu
        cost = 0.5 * float(np.sum(self.precision * diff * diff))
        grad = self.precision * diff
        return cost, grad

    def __repr__(self) -> str:
        scalar_prec = (
            self.precision.flat[0]
            if np.all(self.precision == self.precision.flat[0])
            else None
        )
        if scalar_prec is not None:
            return (
                f"CoursePriorTerm(n_beamlets={self.target_mu.size}, "
                f"precision={scalar_prec:.4g})"
            )
        return (
            f"CoursePriorTerm(n_beamlets={self.target_mu.size}, "
            f"precision=array[{self.precision.min():.4g}, "
            f"{self.precision.max():.4g}])"
        )
