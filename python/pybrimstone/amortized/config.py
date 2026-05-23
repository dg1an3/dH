"""
Prototype configuration.

One dataclass, no YAML, no Hydra. Phase 4 can add a loader if needed.
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass
class PrototypeConfig:
    """Knobs for the amortized course-prior prototype.

    The split is intentional: data-shape knobs (phase_dim, course_dim)
    and training knobs (hidden, lr, epochs, batch_size) live together
    because the prototype is tiny and they all change together when
    moving from toy data to real data.

    course_dim defaults to 2 * phase_dim because CourseState.flatten()
    concatenates (mu, sigma_inv) — keep this in sync with types.py.
    """

    phase_dim: int = 64
    course_dim: int = 128       # = 2 * phase_dim by default
    hidden: int = 256

    # Training
    lr: float = 1e-3
    epochs: int = 20
    batch_size: int = 64
    val_fraction: float = 0.1
    weight_decay: float = 1e-4
    # Early-stopping: keep the snapshot with lowest val MSE. Each sample's
    # latent (A, b) in the toy is not part of the input, so the MLP can
    # memorize per-sample mappings if left to overfit. Always-on keep-best
    # is the cheapest defense.
    keep_best: bool = True

    # Inference
    n_outer: int = 5            # outer course-state updates
    polish_iters: int = 10      # CG safety polish per phase per outer iter
    seed: int = 0
