"""
Data generation for the amortized course-prior prototype.

Phase 0: stub. Returns deterministic dummy tuples so the smoke test
can verify shapes flow through end-to-end.

Phase 1a will add the toy Gaussian closed-form generator described in
§5a of the prototype guide. Phase 1b will add the real generator that
calls pybrimstone.PhaseOptimizer.__call__ and harvests (x_init, prior,
x_star) tuples from converged CG solves.
"""

from __future__ import annotations

from typing import List

import numpy as np

from .types import CourseState, Sample


def generate_dummy_samples(
    n_samples: int,
    phase_dim: int,
    rng: np.random.Generator,
) -> List[Sample]:
    """Phase 0 stub: random inputs, zero targets.

    Smoke-test data. Targets are zeros so a freshly-zero-initialized
    network already has near-zero training loss — useful to confirm
    plumbing without any actual learning happening yet.

    Args:
        n_samples: number of tuples to generate.
        phase_dim: dimensionality of x_init and course_state.mu.
        rng: numpy Generator for reproducibility.

    Returns:
        list of Sample. delta_x is zero. mu and sigma_inv have plausible
        but random values.
    """
    out: List[Sample] = []
    for _ in range(int(n_samples)):
        x_init = rng.standard_normal(phase_dim)
        mu = rng.standard_normal(phase_dim) * 0.5
        sigma_inv = rng.uniform(0.1, 2.0, size=phase_dim)
        delta_x = np.zeros(phase_dim)
        out.append(
            Sample(
                x_init=x_init,
                course_state=CourseState(mu=mu, sigma_inv=sigma_inv),
                delta_x=delta_x,
            )
        )
    return out
