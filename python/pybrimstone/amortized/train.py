"""
Training loop for the amortized course-prior network.

Phase 0: stub. Real implementation lands in Phase 2 once Phase 1a
(toy data) is producing learnable tuples.
"""

from __future__ import annotations

from typing import List

import numpy as np

from .config import PrototypeConfig
from .network import AmortizedCoursePrior
from .types import Sample


def train(samples: List[Sample], config: PrototypeConfig) -> AmortizedCoursePrior:
    """Phase 0 stub — returns an untrained (zero-init) network.

    Validates the sample shapes match the config so misconfigurations
    fail loudly at the start of a future training run rather than
    silently producing garbage targets.

    Raises:
        ValueError: if any sample dim mismatches the config.
    """
    if not samples:
        raise ValueError("train() needs at least one Sample")

    for i, s in enumerate(samples):
        if s.x_init.shape != (config.phase_dim,):
            raise ValueError(
                f"sample[{i}].x_init shape {s.x_init.shape} != "
                f"(phase_dim={config.phase_dim},)"
            )
        if s.delta_x.shape != (config.phase_dim,):
            raise ValueError(
                f"sample[{i}].delta_x shape {s.delta_x.shape} != "
                f"(phase_dim={config.phase_dim},)"
            )
        course_flat_dim = int(np.asarray(s.course_state.flatten()).shape[0])
        if course_flat_dim != config.course_dim:
            raise ValueError(
                f"sample[{i}].course_state.flatten() length {course_flat_dim} != "
                f"course_dim={config.course_dim}"
            )

    return AmortizedCoursePrior(
        phase_dim=config.phase_dim,
        course_dim=config.course_dim,
        hidden=config.hidden,
    )
