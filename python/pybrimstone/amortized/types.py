"""
Dataclasses for the amortized course-prior prototype.

These mirror the existing CoursePriorTerm contract from
pybrimstone.course_prior: a diagonal Gaussian course-level prior over
a flat beamlet-weight vector.

Shape note. The CoursePriorTerm in the live codebase carries
(target_mu, precision), both (n_beamlets,). `CourseState` here uses
the same shape. `sigma_inv` is the precision vector (1 / variance),
matching what CoursePriorTerm.precision holds and what HierarchicalBayes
sets after pool_phases returns var_eta.

Open question deferred to Phase 1b: does `PhaseState.x` live in
beamlet-weight space (what the prior speaks) or in optimizer-parameter
space (what PhaseOptimizer.__call__ returns from polak_ribiere_cg)?
For Phase 0 / Phase 1a we use the same flat vector either way; the
toy data generator works entirely in one space. See
notes/coursepriorterm_audit.md for the discussion that needs Derek's
sign-off before Phase 1b.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class PhaseState:
    """One phase's current parameter vector.

    Attributes:
        x: shape (n_beamlets,). Either beamlet weights or unbounded
            optimizer parameters depending on the data-generation mode;
            consistent within a single training corpus.
        phase_id: stable integer index within the parent plan. Carried
            through so we can stratify eval metrics by phase later.
    """

    x: np.ndarray
    phase_id: int


@dataclass
class CourseState:
    """Course-level Gaussian prior parameters.

    Attributes:
        mu: shape (n_beamlets,). Course-level posterior mean.
        sigma_inv: shape (n_beamlets,). Per-beamlet precision; same
            quantity that CoursePriorTerm.precision holds.
    """

    mu: np.ndarray
    sigma_inv: np.ndarray

    @property
    def dim(self) -> int:
        return int(self.mu.shape[0])

    def flatten(self) -> np.ndarray:
        """Concatenate (mu, sigma_inv) as the network's course input.

        Length is 2 * n_beamlets. Phase 2 will likely want a fitted
        scaler in front of this — kept here unscaled for now.
        """
        return np.concatenate([self.mu, self.sigma_inv])


@dataclass
class Sample:
    """One training tuple: (current params, course state, target delta).

    delta_x is the regression target — the per-beamlet correction the
    real solver applied: x_star - x_init. Predicting the delta rather
    than the absolute x_star keeps the target mean-zero and well-scaled
    (see §6.3 of the prototype guide).
    """

    x_init: np.ndarray
    course_state: CourseState
    delta_x: np.ndarray
