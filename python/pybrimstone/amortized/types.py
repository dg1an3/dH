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

Phase 1b decision (Derek): per-sample x_init and trajectory are
recorded in beamlet-weight space (post-sigmoid). The toy generator
in data.py operates in an unbounded vector space because the toy
has no sigmoid in its physics; that mismatch is documented in the
toy's docstring. The real generator applies `transform` to the
CG-trajectory optimizer params before storage so everything in
Sample lives in the same beamlet-weight space the CoursePriorTerm
itself speaks.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Optional

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

    Attributes:
        x_init: starting parameters, shape (n_beamlets,).
        course_state: prior (mu, sigma_inv).
        delta_x: target delta = x_star - x_init, shape (n_beamlets,).
        trajectory: optional list of intermediate parameter vectors from
            the CG solve, length T+1 starting with x_init and ending with
            x_star. Populated by the real generator when record_trajectory
            is True; left None for the toy / dummy generators which have
            no notion of "intermediate". Used by curriculum / imitation
            training; see expand_trajectory_to_samples in data.py.
        meta: free-form per-sample metadata (plan name, phase id, seed,
            iteration count). Not used by training, but kept around for
            eval stratification and debugging.
    """

    x_init: np.ndarray
    course_state: CourseState
    delta_x: np.ndarray
    trajectory: Optional[List[np.ndarray]] = None
    meta: dict = field(default_factory=dict)
