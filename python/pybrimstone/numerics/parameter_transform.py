"""
Sigmoid optimizer-to-beamlet transform.

Port of the Transform/InvTransform/dTransform trio in
RtModel/Prescription.h/.cpp. Maps unbounded optimizer-space variables
to non-negative beamlet intensities via a scaled sigmoid:

    intensity = input_scale * sigmoid(SIGMOID_SCALE * param)

The transform is what enforces non-negative beamlet weights without
the optimizer needing inequality constraints. The diagonal Jacobian
d(intensity)/d(param) is used by Prescription to chain-rule gradients
from beamlet space back to optimizer space.
"""

from __future__ import annotations

import numpy as np


INPUT_SCALE = 0.5
SIGMOID_SCALE = 0.2


def sigmoid(x: np.ndarray | float, scale: float = SIGMOID_SCALE) -> np.ndarray | float:
    """Scaled sigmoid: 1 / (1 + exp(-scale * x))."""
    return 1.0 / (1.0 + np.exp(-x * scale))


def transform(v: np.ndarray | float, input_scale: float = INPUT_SCALE) -> np.ndarray | float:
    """Optimizer space -> beamlet space. Output is in (0, input_scale)."""
    return input_scale * sigmoid(v)


def inv_transform(v: np.ndarray | float, input_scale: float = INPUT_SCALE) -> np.ndarray | float:
    """Beamlet space -> optimizer space. Output is the logit, clamped to avoid infinities."""
    ratio = np.clip(np.asarray(v) / input_scale, 1e-10, 1.0 - 1e-10)
    return np.log(ratio / (1.0 - ratio)) / SIGMOID_SCALE


def d_transform(v: np.ndarray | float, input_scale: float = INPUT_SCALE) -> np.ndarray | float:
    """
    Diagonal Jacobian d(intensity)/d(param).

    Used by Prescription to chain-rule gradients from beamlet space back
    to optimizer space:
        grad_param = grad_beamlet * d_transform(param)
    """
    s = sigmoid(v)
    return input_scale * SIGMOID_SCALE * s * (1.0 - s)
