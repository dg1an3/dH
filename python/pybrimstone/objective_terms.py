"""
Base classes for objective-function terms.

Two sibling abstractions:
  BeamletObjectiveTerm  -- evaluates in beamlet-weight space.
                           Hierarchical-Bayes CoursePriorTerm is the
                           canonical instance.
  DoseObjectiveTerm     -- evaluates in dose space, after the dose calc.
                           KLDivTerm (the brimstone data-fit term) is
                           the canonical instance.

Prescription routes gradients between the two spaces using the
dose-calc Jacobian (matrix transpose for a linear dose operator).

This split is the resolution of Open Question 4 in
HIERARCHICAL_BAYES_DESIGN.md: rather than pretending the Course prior
is a dose-space term (option b: dose-space Gaussian via Jacobian), we
admit beamlet-space terms as first-class citizens with their own base
class.
"""

from __future__ import annotations

from typing import Tuple

import numpy as np


class BeamletObjectiveTerm:
    """
    Objective term that evaluates on beamlet weights.

    Subclasses implement evaluate(beamlet_weights) -> (cost, grad_w).
    The Prescription multiplies the term's cost by self.weight when
    summing into the total objective.
    """

    weight: float = 1.0

    def evaluate(self, beamlet_weights: np.ndarray) -> Tuple[float, np.ndarray]:
        raise NotImplementedError


class DoseObjectiveTerm:
    """
    Objective term that evaluates on a per-voxel dose distribution.

    Subclasses implement evaluate(dose) -> (cost, grad_dose) where
    grad_dose has the shape of dose. Prescription back-propagates
    grad_dose to grad_beamlet via the dose-calc adjoint, then to
    grad_optimizer_params via the sigmoid transform Jacobian.
    """

    weight: float = 1.0

    def evaluate(self, dose: np.ndarray) -> Tuple[float, np.ndarray]:
        raise NotImplementedError
