"""
Reference implementation and tests for the sigmoid parameter transform.

Mirrors the Transform/InvTransform/dTransform logic in Prescription.h/.cpp.
The transform maps linear optimizer space to intensity (beamlet weight) space.
"""

import numpy as np
import pytest


# ---------------------------------------------------------------------------
# Reference implementations
# ---------------------------------------------------------------------------

# Default constants from C++ code
INPUT_SCALE = 0.5
SIGMOID_SCALE = 0.2


def sigmoid(x, scale=SIGMOID_SCALE):
    """Basic sigmoid function."""
    return 1.0 / (1.0 + np.exp(-x * scale))


def transform(v, input_scale=INPUT_SCALE):
    """
    Transform from optimizer parameter space to intensity space.

    Matches Prescription::Transform:
      intensity = input_scale * sigmoid(param)

    This ensures beamlet weights are always positive.
    """
    return input_scale * sigmoid(v)


def inv_transform(v, input_scale=INPUT_SCALE):
    """
    Inverse transform from intensity space back to optimizer parameters.

    Matches Prescription::InvTransform:
      param = sigmoid_inv(intensity / input_scale)
    """
    # Clamp to avoid log(0) or log(inf)
    ratio = np.clip(v / input_scale, 1e-10, 1.0 - 1e-10)
    return np.log(ratio / (1.0 - ratio)) / SIGMOID_SCALE


def d_transform(v, input_scale=INPUT_SCALE):
    """
    Derivative of the transform (Jacobian diagonal).

    Matches Prescription::dTransform:
      d(intensity)/d(param) = input_scale * sigmoid'(param)

    sigmoid'(x) = scale * sigmoid(x) * (1 - sigmoid(x))
    """
    s = sigmoid(v)
    return input_scale * SIGMOID_SCALE * s * (1.0 - s)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestSigmoid:
    def test_at_zero(self):
        assert abs(sigmoid(0.0) - 0.5) < 1e-10

    def test_range(self):
        """Sigmoid output is always in (0, 1)."""
        x = np.linspace(-100, 100, 1000)
        s = sigmoid(x)
        assert np.all(s > 0)
        assert np.all(s < 1)

    def test_monotonic(self):
        x = np.linspace(-10, 10, 100)
        s = sigmoid(x)
        assert np.all(np.diff(s) > 0)

    def test_symmetry(self):
        """sigmoid(-x) = 1 - sigmoid(x)."""
        x = np.array([0.5, 1.0, 2.0, 5.0])
        assert np.allclose(sigmoid(-x), 1.0 - sigmoid(x))


class TestTransform:
    def test_positive_output(self):
        """Transformed values are always positive (beamlet weights > 0)."""
        params = np.linspace(-10, 10, 100)
        intensities = transform(params)
        assert np.all(intensities > 0)

    def test_bounded(self):
        """Output bounded by [0, input_scale]."""
        params = np.linspace(-100, 100, 1000)
        intensities = transform(params)
        assert np.all(intensities < INPUT_SCALE)
        assert np.all(intensities > 0)

    def test_at_zero(self):
        """transform(0) = input_scale * 0.5."""
        assert abs(transform(0.0) - INPUT_SCALE * 0.5) < 1e-10

    def test_array_input(self):
        params = np.array([-1.0, 0.0, 1.0])
        result = transform(params)
        assert result.shape == (3,)


class TestInvTransform:
    def test_round_trip(self):
        """inv_transform(transform(x)) ≈ x."""
        params = np.array([-2.0, -1.0, 0.0, 1.0, 2.0])
        recovered = inv_transform(transform(params))
        assert np.allclose(params, recovered, atol=1e-8)

    def test_forward_round_trip(self):
        """transform(inv_transform(y)) ≈ y for valid intensities."""
        intensities = np.array([0.05, 0.1, 0.2, 0.3, 0.45])
        recovered = transform(inv_transform(intensities))
        assert np.allclose(intensities, recovered, atol=1e-8)

    def test_at_midpoint(self):
        """inv_transform(input_scale * 0.5) = 0."""
        assert abs(inv_transform(INPUT_SCALE * 0.5)) < 1e-8


class TestDTransform:
    def test_positive(self):
        """Derivative is always positive (transform is monotonically increasing)."""
        params = np.linspace(-10, 10, 100)
        deriv = d_transform(params)
        assert np.all(deriv > 0)

    def test_maximum_at_zero(self):
        """Derivative is maximal at param=0 (steepest part of sigmoid)."""
        params = np.linspace(-5, 5, 101)
        deriv = d_transform(params)
        max_idx = np.argmax(deriv)
        assert abs(params[max_idx]) < 0.1

    def test_numerical_gradient(self):
        """dTransform should match numerical differentiation of transform."""
        params = np.array([-2.0, -1.0, 0.0, 1.0, 2.0])
        h = 1e-7
        numerical = (transform(params + h) - transform(params - h)) / (2 * h)
        analytical = d_transform(params)
        assert np.allclose(numerical, analytical, atol=1e-5)

    def test_symmetry(self):
        """dTransform(-x) = dTransform(x) (derivative of sigmoid is symmetric)."""
        x = np.array([0.5, 1.0, 2.0, 5.0])
        assert np.allclose(d_transform(-x), d_transform(x))
