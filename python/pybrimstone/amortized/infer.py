"""
Inference loop using the amortized course-prior network.

Phase 0: stub. The real inference loop (§7 of the prototype guide)
runs an outer course-state coordinate-ascent loop where each inner
phase update is `x_init + f_θ(x_init, course_state)` optionally
followed by a short CG polish for safety.

For Phase 0 we expose just the single-shot delta prediction so
test_smoke can verify the forward pass works on numpy inputs without
having to know about torch tensor handling.
"""

from __future__ import annotations

import numpy as np
import torch

from .network import AmortizedCoursePrior
from .types import CourseState


def predict_delta(
    model: AmortizedCoursePrior,
    x_init: np.ndarray,
    course_state: CourseState,
) -> np.ndarray:
    """Single forward pass: Δx = f_θ(x_init, course_state).

    Inputs / outputs are numpy. Caller is expected to apply this as
    `x_hat = x_init + delta` and then optionally feed `x_hat` into a
    short CG polish — but that lives in Phase 3, not here.
    """
    if x_init.shape != (model.phase_dim,):
        raise ValueError(
            f"x_init shape {x_init.shape} != (phase_dim={model.phase_dim},)"
        )

    course_flat = course_state.flatten()
    if course_flat.shape != (model.course_dim,):
        raise ValueError(
            f"course flat shape {course_flat.shape} != (course_dim={model.course_dim},)"
        )

    x_t = torch.from_numpy(np.asarray(x_init, dtype=np.float32)).unsqueeze(0)
    c_t = torch.from_numpy(np.asarray(course_flat, dtype=np.float32)).unsqueeze(0)

    model.eval()
    with torch.no_grad():
        delta_t = model(x_t, c_t).squeeze(0)
    return delta_t.detach().cpu().numpy().astype(np.float64)
