"""
Composite objective function over a treatment plan.

Port of RtModel/Prescription.cpp. The Prescription holds a list of
dose-space terms (KLDivTerm) and a list of beamlet-space terms
(CoursePriorTerm and friends), composes them into a single cost and
gradient, and handles two layers of gradient routing:

  optimizer params   -- d_transform -->   beamlet weights
  beamlet weights    -- dose Jacobian --> dose

Forward:
    beamlets = transform(params)      # sigmoid; non-negative
    dose     = dose_operator(beamlets)
    cost     = sum_i w_i * dose_term_i.evaluate(dose).cost
             + sum_j w_j * beamlet_term_j.evaluate(beamlets).cost

Backward:
    grad_dose    = sum_i w_i * dose_term_i.evaluate(dose).grad
    grad_beamlet = dose_adjoint(grad_dose)
                 + sum_j w_j * beamlet_term_j.evaluate(beamlets).grad
    grad_params  = grad_beamlet * d_transform(params)

The dose_operator can be either a matrix (where dose = D @ w and the
adjoint is D.T @ grad_dose) or a callable + matching adjoint callable.
"""

from __future__ import annotations

from typing import Callable, List, Optional, Tuple, Union

import numpy as np

from .numerics import d_transform, transform
from .objective_terms import BeamletObjectiveTerm, DoseObjectiveTerm


DoseFn = Callable[[np.ndarray], np.ndarray]
DoseAdjointFn = Callable[[np.ndarray], np.ndarray]
DoseOperator = Union[np.ndarray, Tuple[DoseFn, DoseAdjointFn]]


class Prescription:
    """
    Composite objective over a plan.

    Args:
        dose_operator: An (n_voxels, n_beamlets) matrix, OR a tuple
            (forward, adjoint) of callables: forward(beamlets) -> dose,
            adjoint(grad_dose) -> grad_beamlet.
        use_transform: If True (default), optimizer parameters are
            unbounded and beamlet weights are sigmoid-mapped via the
            transform from numerics.parameter_transform. If False,
            optimizer params == beamlet weights (no transform).
    """

    def __init__(
        self,
        dose_operator: DoseOperator,
        use_transform: bool = True,
    ):
        if isinstance(dose_operator, np.ndarray):
            if dose_operator.ndim != 2:
                raise ValueError(
                    f"matrix dose_operator must be 2-D, got shape {dose_operator.shape}"
                )
            self._dose_matrix: Optional[np.ndarray] = dose_operator
            self._dose_forward: Optional[DoseFn] = None
            self._dose_adjoint: Optional[DoseAdjointFn] = None
        elif isinstance(dose_operator, tuple) and len(dose_operator) == 2:
            forward, adjoint = dose_operator
            if not callable(forward) or not callable(adjoint):
                raise TypeError(
                    "dose_operator tuple must be (callable forward, callable adjoint)"
                )
            self._dose_matrix = None
            self._dose_forward = forward
            self._dose_adjoint = adjoint
        else:
            raise TypeError(
                "dose_operator must be an ndarray matrix or a "
                "(forward, adjoint) callable tuple"
            )

        self.use_transform = bool(use_transform)
        self.dose_terms: List[DoseObjectiveTerm] = []
        self.beamlet_terms: List[BeamletObjectiveTerm] = []

    # ------------------------------------------------------------------
    # Term registration
    # ------------------------------------------------------------------

    def add_dose_term(self, term: DoseObjectiveTerm) -> None:
        if not isinstance(term, DoseObjectiveTerm):
            raise TypeError(f"expected DoseObjectiveTerm, got {type(term).__name__}")
        self.dose_terms.append(term)

    def add_beamlet_term(self, term: BeamletObjectiveTerm) -> None:
        if not isinstance(term, BeamletObjectiveTerm):
            raise TypeError(f"expected BeamletObjectiveTerm, got {type(term).__name__}")
        self.beamlet_terms.append(term)

    # ------------------------------------------------------------------
    # Dose forward / adjoint
    # ------------------------------------------------------------------

    def compute_dose(self, beamlets: np.ndarray) -> np.ndarray:
        if self._dose_matrix is not None:
            return beamlets @ self._dose_matrix.T
        return np.asarray(self._dose_forward(beamlets), dtype=np.float64)

    def dose_adjoint(self, grad_dose: np.ndarray) -> np.ndarray:
        if self._dose_matrix is not None:
            return grad_dose @ self._dose_matrix
        return np.asarray(self._dose_adjoint(grad_dose), dtype=np.float64)

    # ------------------------------------------------------------------
    # Evaluation
    # ------------------------------------------------------------------

    def evaluate(self, params: np.ndarray) -> Tuple[float, np.ndarray]:
        """Total cost and gradient w.r.t. optimizer params."""
        params = np.asarray(params, dtype=np.float64)

        if self.use_transform:
            beamlets = transform(params)
        else:
            beamlets = params

        dose = self.compute_dose(beamlets)

        cost = 0.0
        grad_dose = np.zeros_like(dose)
        for term in self.dose_terms:
            c, g = term.evaluate(dose)
            cost += term.weight * c
            grad_dose += term.weight * g

        grad_beamlet = self.dose_adjoint(grad_dose)

        for term in self.beamlet_terms:
            c, g = term.evaluate(beamlets)
            cost += term.weight * c
            grad_beamlet += term.weight * g

        if self.use_transform:
            grad_params = grad_beamlet * d_transform(params)
        else:
            grad_params = grad_beamlet

        return float(cost), grad_params

    def evaluate_cost_only(self, params: np.ndarray) -> float:
        """Same as evaluate() but skips the gradient computation."""
        params = np.asarray(params, dtype=np.float64)
        beamlets = transform(params) if self.use_transform else params
        dose = self.compute_dose(beamlets)
        cost = 0.0
        for term in self.dose_terms:
            c, _ = term.evaluate(dose)
            cost += term.weight * c
        for term in self.beamlet_terms:
            c, _ = term.evaluate(beamlets)
            cost += term.weight * c
        return float(cost)
