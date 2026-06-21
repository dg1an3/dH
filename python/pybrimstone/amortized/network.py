"""
Amortized course-prior network.

Predicts Δx_phase given (x_phase_current, course_state). A 3-layer MLP
is the right starting point — see §11 of the prototype guide: don't
reach for transformers, GNNs, or equivariant architectures until the
MLP demonstrably plateaus and you can articulate why.

Initialization note. The final Linear layer's weights and bias are
zeroed so the freshly-constructed network is the identity prior
(predicts Δx = 0 regardless of input). This makes a no-op
amortized loop equivalent to running CG from x_init with the same
n_outer / polish_iters budget — a clean baseline for the eval A/B.
"""

from __future__ import annotations

import torch
import torch.nn as nn


class AmortizedCoursePrior(nn.Module):
    """f_θ: (x_phase, course_state) → Δx_phase.

    Args:
        phase_dim: length of x_phase (also the output dim).
        course_dim: length of the flattened course state.
        hidden: width of the two hidden layers.
    """

    def __init__(self, phase_dim: int, course_dim: int, hidden: int = 256):
        super().__init__()
        self.phase_dim = int(phase_dim)
        self.course_dim = int(course_dim)
        self.hidden = int(hidden)

        self.net = nn.Sequential(
            nn.Linear(self.phase_dim + self.course_dim, self.hidden),
            nn.SiLU(),
            nn.Linear(self.hidden, self.hidden),
            nn.SiLU(),
            nn.Linear(self.hidden, self.phase_dim),
        )

        # Zero-init the final layer → identity prior at construction time.
        final = self.net[-1]
        nn.init.zeros_(final.weight)
        nn.init.zeros_(final.bias)

    def forward(self, x_phase: torch.Tensor, course_state: torch.Tensor) -> torch.Tensor:
        if x_phase.shape[-1] != self.phase_dim:
            raise ValueError(
                f"x_phase last dim {x_phase.shape[-1]} != phase_dim {self.phase_dim}"
            )
        if course_state.shape[-1] != self.course_dim:
            raise ValueError(
                f"course_state last dim {course_state.shape[-1]} != course_dim {self.course_dim}"
            )
        z = torch.cat([x_phase, course_state], dim=-1)
        return self.net(z)
