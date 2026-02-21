"""
Reference implementation and tests for sigma estimation methods.

Mirrors SigmaEstimator.cpp from RtModel.
"""

import numpy as np
import pytest


# ---------------------------------------------------------------------------
# Constants from C++ code
# ---------------------------------------------------------------------------

DEFAULT_MIN_SIGMA = 0.1
DEFAULT_MAX_SIGMA = 16.0
PYRAMID_SCALE_FACTOR = 2.5

SLOW_CONVERGENCE_THRESHOLD = 0.001
FAST_CONVERGENCE_THRESHOLD = 0.1
SIGMA_INCREASE_RATE = 1.2
SIGMA_DECREASE_RATE = 0.85

WEIGHT_GEOMETRY = 1.0
WEIGHT_DOSE_GRADIENT = 2.0
WEIGHT_PRESCRIPTION = 1.5
WEIGHT_VOLUME = 0.8

DEFAULT_SIGMAS = [8.0, 3.2, 1.3, 0.5, 0.25]


# ---------------------------------------------------------------------------
# Reference implementations
# ---------------------------------------------------------------------------

def clamp_sigma(sigma, min_sigma=DEFAULT_MIN_SIGMA, max_sigma=DEFAULT_MAX_SIGMA):
    """Clamp sigma to valid range."""
    return max(min_sigma, min(max_sigma, sigma))


def sphericity(volume_cc, surface_area_cm2):
    """
    Calculate sphericity = (pi^(1/3) * (6V)^(2/3)) / A

    Matches SigmaEstimator::CalculateSphericity.
    Returns 1.0 for a perfect sphere, <1 for complex shapes.
    """
    if volume_cc <= 0 or surface_area_cm2 <= 0:
        return 0.5
    s = (np.pi ** (1.0 / 3.0)) * ((6.0 * volume_cc) ** (2.0 / 3.0)) / surface_area_cm2
    return max(0.0, min(1.0, s))


def estimate_from_geometry(volume_cc, surface_area_cm2, base_sigma=0.5):
    """
    Sigma estimate from geometric complexity.

    Matches SigmaEstimator::EstimateFromGeometry.
    Complex (low sphericity) → small sigma. Simple → large sigma.
    """
    sph = sphericity(volume_cc, surface_area_cm2)
    sv_ratio = surface_area_cm2 / volume_cc if volume_cc > 0 else 1.0

    complexity_shape = 1.0 - sph
    avg_sv = 1.0
    complexity_sv = sv_ratio / avg_sv

    complexity = 0.6 * complexity_shape + 0.4 * complexity_sv
    complexity = max(0.0, min(1.0, complexity))

    factor = 1.0 - 0.7 * complexity  # [0.3, 1.0]
    return clamp_sigma(base_sigma * factor)


def estimate_from_dose_gradient(avg_gradient, dose_std_dev, base_sigma=0.5):
    """
    Sigma estimate from dose gradient magnitude.

    Matches SigmaEstimator::EstimateFromDoseGradient.
    High gradient → small sigma. Low gradient → large sigma.
    """
    normalized_grad = min(avg_gradient / 5.0, 2.0)
    normalized_het = min(dose_std_dev / 5.0, 2.0)
    dose_complexity = 0.7 * normalized_grad + 0.3 * normalized_het
    return clamp_sigma(base_sigma / (1.0 + dose_complexity))


def estimate_from_prescription_range(min_dose, max_dose, base_sigma=0.5):
    """
    Sigma estimate from prescription dose range.

    Matches SigmaEstimator::EstimateFromPrescriptionRange.
    Wider range → finer sigma.
    """
    dose_range = max_dose - min_dose
    normalized_range = min(dose_range / 50.0, 2.0)
    return clamp_sigma(base_sigma / (0.5 + 0.5 * normalized_range))


def estimate_from_volume(volume_cc, base_sigma=0.5):
    """
    Sigma estimate from structure volume.

    Matches SigmaEstimator::EstimateFromVolume.
    Larger volumes → coarser sigma. Smaller → finer.
    """
    log_vol = np.log10(max(volume_cc, 1.0))
    factor = 0.5 + 0.25 * log_vol
    return clamp_sigma(base_sigma * factor)


def combine_estimates(estimates, weights):
    """
    Weighted average combination of sigma estimates.

    Matches SigmaEstimator::CombineEstimates.
    """
    if not estimates:
        return 0.5
    weight_sum = sum(weights[:len(estimates)])
    if weight_sum <= 0:
        return estimates[0]
    weighted_sum = sum(e * w for e, w in zip(estimates, weights))
    return weighted_sum / weight_sum


def generate_pyramid_sequence(base_sigma, n_levels, scale_factor=PYRAMID_SCALE_FACTOR):
    """
    Generate pyramid sigma sequence from finest to coarsest.

    Matches SigmaEstimator::GeneratePyramidSequence.
    sigmas[n_levels-1] = base_sigma (finest)
    sigmas[i] = sigmas[i+1] * scale_factor (going coarser)
    """
    sigmas = [0.0] * n_levels
    sigmas[n_levels - 1] = clamp_sigma(base_sigma)
    for i in range(n_levels - 2, -1, -1):
        sigmas[i] = clamp_sigma(sigmas[i + 1] * scale_factor)
    return sigmas


def adapt_sigma_from_convergence(current_sigma, cost_improvement, gradient_norm,
                                 slow_thresh=SLOW_CONVERGENCE_THRESHOLD,
                                 fast_thresh=FAST_CONVERGENCE_THRESHOLD,
                                 increase_rate=SIGMA_INCREASE_RATE,
                                 decrease_rate=SIGMA_DECREASE_RATE):
    """
    Adapt sigma based on convergence behavior.

    Matches SigmaEstimator::AdaptSigmaFromConvergence.
    """
    if cost_improvement < slow_thresh and gradient_norm < 0.01:
        return clamp_sigma(current_sigma * increase_rate)
    elif cost_improvement > fast_thresh and gradient_norm > 0.1:
        return clamp_sigma(current_sigma * decrease_rate)
    return current_sigma


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestClampSigma:
    def test_within_range(self):
        assert clamp_sigma(1.0) == 1.0

    def test_below_min(self):
        assert clamp_sigma(0.01) == DEFAULT_MIN_SIGMA

    def test_above_max(self):
        assert clamp_sigma(100.0) == DEFAULT_MAX_SIGMA


class TestSphericity:
    def test_perfect_sphere(self):
        """Sphere of radius 1cm: V=4/3*pi, A=4*pi."""
        r = 1.0
        V = (4.0 / 3.0) * np.pi * r ** 3
        A = 4.0 * np.pi * r ** 2
        s = sphericity(V, A)
        assert abs(s - 1.0) < 1e-6

    def test_elongated_shape(self):
        """Elongated shapes have sphericity < 1."""
        # Very rough: a thin cylinder
        V = 1.0  # cc
        A = 10.0  # cm^2 (much higher surface area than a sphere of same volume)
        s = sphericity(V, A)
        assert s < 0.8

    def test_invalid_inputs(self):
        assert sphericity(0, 10) == 0.5
        assert sphericity(10, 0) == 0.5


class TestEstimateFromGeometry:
    def test_sphere_gets_larger_sigma(self):
        """Simple (spherical) structures should get larger sigma."""
        r = 2.0
        V = (4.0 / 3.0) * np.pi * r ** 3
        A = 4.0 * np.pi * r ** 2
        sigma_sphere = estimate_from_geometry(V, A)

        # Elongated structure
        sigma_complex = estimate_from_geometry(1.0, 15.0)

        assert sigma_sphere > sigma_complex

    def test_returns_within_range(self):
        sigma = estimate_from_geometry(50.0, 100.0)
        assert DEFAULT_MIN_SIGMA <= sigma <= DEFAULT_MAX_SIGMA


class TestEstimateFromDoseGradient:
    def test_high_gradient_smaller_sigma(self):
        sigma_high = estimate_from_dose_gradient(10.0, 5.0)
        sigma_low = estimate_from_dose_gradient(0.5, 0.5)
        assert sigma_high < sigma_low

    def test_zero_gradient(self):
        sigma = estimate_from_dose_gradient(0.0, 0.0)
        assert abs(sigma - 0.5) < 1e-6


class TestEstimateFromPrescriptionRange:
    def test_wide_range_smaller_sigma(self):
        sigma_wide = estimate_from_prescription_range(0.0, 70.0)
        sigma_narrow = estimate_from_prescription_range(0.0, 5.0)
        assert sigma_wide < sigma_narrow

    def test_zero_range(self):
        sigma = estimate_from_prescription_range(50.0, 50.0)
        assert abs(sigma - 0.5 / 0.5) < 0.1  # ~1.0


class TestEstimateFromVolume:
    def test_large_volume_larger_sigma(self):
        sigma_large = estimate_from_volume(500.0)
        sigma_small = estimate_from_volume(5.0)
        assert sigma_large > sigma_small

    def test_one_cc(self):
        # log10(1) = 0, factor = 0.5, sigma = 0.25
        sigma = estimate_from_volume(1.0)
        assert abs(sigma - 0.25) < 1e-6


class TestCombineEstimates:
    def test_single_estimate(self):
        assert combine_estimates([0.5], [1.0]) == 0.5

    def test_equal_weights(self):
        result = combine_estimates([0.3, 0.7], [1.0, 1.0])
        assert abs(result - 0.5) < 1e-10

    def test_weighted(self):
        result = combine_estimates([0.3, 0.7], [3.0, 1.0])
        expected = (0.3 * 3.0 + 0.7 * 1.0) / 4.0
        assert abs(result - expected) < 1e-10

    def test_empty(self):
        assert combine_estimates([], []) == 0.5


class TestGeneratePyramidSequence:
    def test_length(self):
        sigmas = generate_pyramid_sequence(0.5, n_levels=5)
        assert len(sigmas) == 5

    def test_finest_is_base(self):
        sigmas = generate_pyramid_sequence(0.5, n_levels=4)
        assert abs(sigmas[-1] - 0.5) < 1e-10

    def test_coarsest_larger(self):
        sigmas = generate_pyramid_sequence(0.5, n_levels=4)
        assert sigmas[0] > sigmas[-1]

    def test_monotonically_decreasing(self):
        sigmas = generate_pyramid_sequence(0.5, n_levels=5)
        for i in range(len(sigmas) - 1):
            assert sigmas[i] >= sigmas[i + 1]

    def test_default_sigmas_structure(self):
        """Generated sequence should resemble the default [8.0, 3.2, 1.3, 0.5, 0.25]."""
        sigmas = generate_pyramid_sequence(0.25, n_levels=5)
        # Coarsest should be larger
        assert sigmas[0] > 5.0
        # Finest should be 0.25
        assert abs(sigmas[4] - 0.25) < 1e-10

    def test_clamped(self):
        sigmas = generate_pyramid_sequence(0.01, n_levels=10)
        for s in sigmas:
            assert s >= DEFAULT_MIN_SIGMA
            assert s <= DEFAULT_MAX_SIGMA


class TestAdaptSigmaFromConvergence:
    def test_slow_convergence_increases(self):
        sigma = adapt_sigma_from_convergence(1.0, 0.0005, 0.005)
        assert sigma > 1.0

    def test_fast_convergence_decreases(self):
        sigma = adapt_sigma_from_convergence(1.0, 0.2, 0.5)
        assert sigma < 1.0

    def test_normal_convergence_unchanged(self):
        sigma = adapt_sigma_from_convergence(1.0, 0.05, 0.05)
        assert sigma == 1.0

    def test_clamped_on_increase(self):
        sigma = adapt_sigma_from_convergence(15.0, 0.0001, 0.001)
        assert sigma <= DEFAULT_MAX_SIGMA

    def test_increase_rate(self):
        sigma = adapt_sigma_from_convergence(1.0, 0.0005, 0.005)
        assert abs(sigma - SIGMA_INCREASE_RATE) < 1e-6

    def test_decrease_rate(self):
        sigma = adapt_sigma_from_convergence(1.0, 0.2, 0.5)
        assert abs(sigma - SIGMA_DECREASE_RATE) < 1e-6
