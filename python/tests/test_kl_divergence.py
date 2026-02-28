"""
Reference implementation and tests for KL-divergence evaluation.

Mirrors KLDivTerm.cpp from RtModel.
"""

import numpy as np
import pytest

from tests.test_histogram import (
    make_gaussian_kernel,
    conv_gauss,
    gauss,
)


EPS = 1e-5


# ---------------------------------------------------------------------------
# Reference implementations
# ---------------------------------------------------------------------------

def dvp_to_target_bins(dvps, bin_width, get_bin_for_value):
    """
    Convert dose-volume points (DVPs) to target histogram bins.

    Matches KLDivTerm::GetTargetBins.

    Args:
        dvps: Nx2 array where dvps[i] = [dose_value, cumulative_fraction]
              dvps[0][1] = 1.0, dvps[-1][1] = 0.0
        bin_width: histogram bin width
        get_bin_for_value: function mapping dose value to bin index

    Returns:
        target_bins: array of per-bin density values
    """
    assert abs(dvps[0][1] - 1.0) < 1e-6, "DVPs must start at 100%"
    assert abs(dvps[-1][1] - 0.0) < 1e-6, "DVPs must end at 0%"

    high = dvps[-1][0]
    n_bins = get_bin_for_value(high) + 1
    target_bins = np.zeros(n_bins)

    for i in range(len(dvps) - 1):
        inter_min = dvps[i][0]
        inter_max = dvps[i + 1][0]

        # Slope = change in cumulative fraction per unit dose, then per bin
        inter_slope = (dvps[i][1] - dvps[i + 1][1]) / (inter_max - inter_min)
        inter_slope /= bin_width

        bin_lo = get_bin_for_value(inter_min)
        bin_hi = get_bin_for_value(inter_max)

        for b in range(bin_lo, min(bin_hi + 1, n_bins)):
            target_bins[b] = inter_slope

    return target_bins


def convolve_target_gbins(target_bins, kernel_var_min, kernel_var_max):
    """
    Convolve target bins with Gaussian kernels and normalize.

    Matches KLDivTerm::GetTargetGBins:
      target_gbins = 0.5 * conv(target, kernel_max) + 0.5 * conv(target, kernel_min)
      then normalize to sum=1.
    """
    convolved_max = conv_gauss(target_bins, kernel_var_max)
    convolved_min = conv_gauss(target_bins, kernel_var_min)

    target_gbins = 0.5 * convolved_max + 0.5 * convolved_min

    total = np.sum(target_gbins)
    if total > 0:
        target_gbins /= total

    return target_gbins


def set_interval(low, high, fraction=1.0, use_midpoint=False):
    """
    Create DVP matrix from an interval specification.

    Matches KLDivTerm::SetInterval.
    """
    if use_midpoint:
        dvps = np.array([
            [low, 1.0],
            [low - (low - high) / 3.0, 2.0 / 3.0],
            [low - 2.0 * (low - high) / 3.0, 1.0 / 3.0],
            [high, 0.0],
        ])
    else:
        dvps = np.array([
            [low, fraction],
            [high, 0.0],
        ])
    return dvps


def kl_divergence_calc_over_target(calc_gpdf, target_gpdf):
    """
    KL(calc || target) — the default mode (!m_bTargetCrossEntropy).

    Matches the C++ Eval when m_bTargetCrossEntropy = false:
      sum += calc[i] * log(calc[i] / (target[i] + EPS) + EPS)
    """
    kl = 0.0
    for i in range(len(calc_gpdf)):
        c = calc_gpdf[i]
        t = target_gpdf[i] if i < len(target_gpdf) else 0.0
        kl += c * np.log(c / (t + EPS) + EPS)
    return kl


def kl_divergence_target_over_calc(calc_gpdf, target_gpdf):
    """
    KL(target || calc) — the m_bTargetCrossEntropy mode.

    Matches the C++ Eval when m_bTargetCrossEntropy = true:
      sum += target[i] * log(target[i] / (calc[i] + EPS) + EPS)
    """
    kl = 0.0
    for i in range(len(target_gpdf)):
        t = target_gpdf[i]
        c = calc_gpdf[i] if i < len(calc_gpdf) else 0.0
        kl += t * np.log(t / (c + EPS) + EPS)
    return kl


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestSetInterval:
    def test_simple_interval(self):
        dvps = set_interval(0.0, 1.0)
        assert len(dvps) == 2
        assert dvps[0][1] == 1.0
        assert dvps[1][1] == 0.0

    def test_midpoint_interval(self):
        dvps = set_interval(0.0, 0.9, use_midpoint=True)
        assert len(dvps) == 4
        assert dvps[0][1] == 1.0
        assert abs(dvps[1][1] - 2.0 / 3.0) < 1e-10
        assert abs(dvps[2][1] - 1.0 / 3.0) < 1e-10
        assert dvps[3][1] == 0.0

    def test_midpoint_doses_monotonic(self):
        dvps = set_interval(0.0, 0.9, use_midpoint=True)
        for i in range(len(dvps) - 1):
            assert dvps[i][0] <= dvps[i + 1][0]


class TestDVPToTargetBins:
    def test_uniform_interval(self):
        """Uniform interval should produce constant bins."""
        bin_width = 0.1
        get_bin = lambda v: int(np.floor(v / bin_width))

        dvps = set_interval(0.0, 1.0)
        target = dvp_to_target_bins(dvps, bin_width, get_bin)

        # All bins in [0, 1.0] should have the same slope
        # slope = (1.0 - 0.0) / (1.0 - 0.0) / bin_width = 10.0
        expected_slope = 1.0 / 1.0 / bin_width
        for i in range(len(target)):
            assert abs(target[i] - expected_slope) < 1e-10

    def test_narrow_interval(self):
        bin_width = 0.1
        get_bin = lambda v: int(np.floor(v / bin_width))

        dvps = set_interval(0.5, 0.6)
        target = dvp_to_target_bins(dvps, bin_width, get_bin)

        # Only bin 5 should be nonzero
        assert target[5] > 0
        # Bins outside the interval should be zero
        for i in range(5):
            assert target[i] == 0.0

    def test_uniform_slope(self):
        """Uniform interval [0, 1] should produce constant-slope bins.

        The target bins are unnormalized at this stage — they represent
        the derivative of the cumulative DVH. Normalization happens later
        in GetTargetGBins after Gaussian convolution.
        """
        bin_width = 0.1
        get_bin = lambda v: int(np.floor(v / bin_width))

        dvps = set_interval(0.0, 1.0)
        target = dvp_to_target_bins(dvps, bin_width, get_bin)

        # slope = cumulative_change / dose_range / bin_width = 1.0/1.0/0.1 = 10.0
        expected_slope = 1.0 / 1.0 / bin_width
        for val in target:
            assert abs(val - expected_slope) < 1e-10

        # After convolution and normalization, this will become a proper pdf.
        # Verify that by running through the full pipeline:
        k = make_gaussian_kernel(sigma=0.1, bin_width=bin_width)
        gbins = convolve_target_gbins(target, k, k)
        assert abs(np.sum(gbins) - 1.0) < 1e-6


class TestKLDivergence:
    def test_identical_distributions(self):
        """KL divergence of a distribution with itself should be ~0."""
        p = np.array([0.1, 0.2, 0.4, 0.2, 0.1])
        kl = kl_divergence_calc_over_target(p, p)
        # Not exactly 0 due to EPS terms, but very close
        assert kl < 0.01

    def test_nonnegative(self):
        """KL divergence should be non-negative (Gibbs' inequality)."""
        p = np.array([0.3, 0.4, 0.2, 0.1])
        q = np.array([0.25, 0.25, 0.25, 0.25])
        kl = kl_divergence_calc_over_target(p, q)
        assert kl >= -1e-10  # allow tiny floating point error

    def test_asymmetric(self):
        """KL(P||Q) != KL(Q||P) in general."""
        p = np.array([0.5, 0.3, 0.2])
        q = np.array([0.1, 0.1, 0.8])
        kl_pq = kl_divergence_calc_over_target(p, q)
        kl_qp = kl_divergence_calc_over_target(q, p)
        assert abs(kl_pq - kl_qp) > 0.01

    def test_target_cross_entropy_mode(self):
        """KL(target||calc) should also be non-negative."""
        calc = np.array([0.3, 0.4, 0.2, 0.1])
        target = np.array([0.25, 0.25, 0.25, 0.25])
        kl = kl_divergence_target_over_calc(calc, target)
        assert kl >= -1e-10

    def test_zero_target_handled(self):
        """Zero target bins should not cause NaN/inf."""
        calc = np.array([0.3, 0.4, 0.2, 0.1])
        target = np.array([0.0, 0.0, 0.5, 0.5])
        kl = kl_divergence_calc_over_target(calc, target)
        assert np.isfinite(kl)

    def test_zero_calc_handled(self):
        """Zero calc bins should not cause NaN/inf."""
        calc = np.array([0.0, 0.0, 0.5, 0.5])
        target = np.array([0.3, 0.4, 0.2, 0.1])
        kl = kl_divergence_target_over_calc(calc, target)
        assert np.isfinite(kl)

    def test_different_lengths(self):
        """Mismatched lengths should be handled (extra bins treated as 0)."""
        calc = np.array([0.2, 0.3, 0.3, 0.2])
        target = np.array([0.5, 0.5])
        kl = kl_divergence_calc_over_target(calc, target)
        assert np.isfinite(kl)


class TestConvolveTargetGBins:
    def test_normalized(self):
        target = np.array([0.0, 0.0, 10.0, 10.0, 0.0, 0.0])
        k_min = make_gaussian_kernel(sigma=0.1, bin_width=0.1)
        k_max = make_gaussian_kernel(sigma=0.1, bin_width=0.1)
        gbins = convolve_target_gbins(target, k_min, k_max)
        assert abs(np.sum(gbins) - 1.0) < 1e-6

    def test_smoothed(self):
        """Convolution should spread the distribution."""
        target = np.zeros(20)
        target[10] = 1.0  # delta
        k_min = make_gaussian_kernel(sigma=0.2, bin_width=0.1)
        k_max = make_gaussian_kernel(sigma=0.2, bin_width=0.1)
        gbins = convolve_target_gbins(target, k_min, k_max)
        # Should be wider than original
        nonzero = np.where(gbins > 1e-6)[0]
        assert len(nonzero) > 1
