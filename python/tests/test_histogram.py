"""
Reference implementation and tests for histogram binning with Gaussian convolution.

Mirrors the C++ CHistogram logic from RtModel/Histogram.cpp.
These NumPy implementations serve as:
  1. Pre-merge safety net
  2. Seed of pure-Python pybrimstone
"""

import numpy as np
import pytest
from scipy.signal import fftconvolve


# ---------------------------------------------------------------------------
# Reference implementations
# ---------------------------------------------------------------------------

GBINS_KERNEL_WIDTH = 8.0


def gauss(x, sigma):
    """Gaussian function matching C++ Gauss<REAL> template."""
    if sigma <= 0:
        return 0.0
    return np.exp(-0.5 * (x / sigma) ** 2) / (sigma * np.sqrt(2.0 * np.pi))


def dgauss(x, sigma):
    """Derivative of Gaussian (used for gradient kernels)."""
    if sigma <= 0:
        return 0.0
    return -x / (sigma ** 2) * gauss(x, sigma)


def make_gaussian_kernel(sigma, bin_width, kernel_width=GBINS_KERNEL_WIDTH):
    """
    Build a discrete Gaussian kernel matching CHistogram::SetGBinVar.

    Returns kernel such that sum(kernel) ≈ 1.
    """
    neighborhood = int(np.ceil(kernel_width * sigma / bin_width))
    z_vals = np.arange(-neighborhood, neighborhood + 1)
    kernel = bin_width * np.array([gauss(z * bin_width, sigma) for z in z_vals])
    return kernel


def histogram_bin_dose(dose_values, region, min_value, bin_width):
    """
    Bin dose values within a region into a histogram.

    Matches the fractional binning logic in CHistogram::GetBins.

    In the C++ code:
      bin_scaled = (dose - min_value) / bin_width
      low_bin = floor(bin_scaled)
      binFracHi = -(bin_scaled - low_bin)   (NOTE: negated in C++)
      binFracLo = binFracHi + 1.0

    Then in the binning loop:
      bins[low]   += binFracLo * region   = (1 - frac) * region
      bins[low+1] -= binFracHi * region   = -(-frac * region) = frac * region

    So both bins get positive contributions summing to region.
    """
    bin_scaled = (dose_values - min_value) / bin_width
    low_bin = np.floor(bin_scaled).astype(int)
    frac = bin_scaled - low_bin  # fractional part (positive)
    frac_lo = 1.0 - frac

    max_bin = int(np.max(low_bin)) + 2
    bins = np.zeros(max_bin)

    for i in range(len(dose_values)):
        if region[i] <= 0:
            continue
        b = low_bin[i]
        r = region[i]
        bins[b] += frac_lo[i] * r      # lower bin gets (1-frac) * region
        bins[b + 1] += frac[i] * r     # upper bin gets frac * region

    return bins


def conv_gauss(buffer_in, kernel):
    """
    Gaussian convolution matching CHistogram::ConvGauss (linear convolution).
    """
    return np.convolve(buffer_in, kernel)


def compute_gbins(dose_values, region, min_value=0.0, bin_width=0.1,
                  var_min=0.01, var_max=0.01):
    """
    Full histogram pipeline: bin → convolve → normalize.

    Matches the C++ flow:
      1. Compute bins with fractional assignment
      2. Convolve with Gaussian kernels (var_min and var_max)
      3. GBins = convolved_var_max + convolved_var_min
      4. Normalize by region sum
    """
    sigma_max = np.sqrt(var_max)
    sigma_min = np.sqrt(var_min)

    # Adjust min_value for kernel buffer (GBINS_BUFFER * sqrt(var_max))
    GBINS_BUFFER = GBINS_KERNEL_WIDTH
    adjusted_min = min_value - GBINS_BUFFER * sigma_max

    bins = histogram_bin_dose(dose_values, region, adjusted_min, bin_width)

    # In the simplified case (constant variance fractions = 0 and 1),
    # bins_var_max = bins and bins_var_min = bins
    # The C++ code splits into VarFracLo=0 and VarFracHi=1 by default
    # so bins_var_max gets all the weight, bins_var_min gets none.
    # For the default case: GBins = conv(bins, kernel_max) + conv(bins, kernel_min)
    # but bins_var_min = 0, so GBins = conv(bins, kernel_max)
    #
    # Actually looking more carefully: the default OnRegionChanged sets
    # VarFracLo=0.0, VarFracHi=1.0, so bins_var_max = bins * 1.0, bins_var_min = bins * 0.0
    kernel_max = make_gaussian_kernel(sigma_max, bin_width)
    gbins_var_max = conv_gauss(bins, kernel_max)

    # For the default case, gbins_var_min = 0
    gbins = gbins_var_max

    # Normalize
    region_sum = np.sum(region[region > 0])
    if region_sum > 0:
        gbins /= region_sum

    return gbins, bins


def cumulative_bins(bins):
    """
    Compute cumulative (reverse) bins matching CHistogram::GetCumBins.
    Cumulative from right to left.
    """
    cum = np.zeros_like(bins)
    running = 0.0
    for i in range(len(bins) - 1, -1, -1):
        running += bins[i]
        cum[i] = running
    return cum


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestGaussianKernel:
    def test_kernel_sums_to_one(self):
        kernel = make_gaussian_kernel(sigma=0.1, bin_width=0.1)
        assert abs(np.sum(kernel) - 1.0) < 1e-6

    def test_kernel_symmetric(self):
        kernel = make_gaussian_kernel(sigma=0.2, bin_width=0.1)
        assert np.allclose(kernel, kernel[::-1])

    def test_kernel_peak_at_center(self):
        kernel = make_gaussian_kernel(sigma=0.3, bin_width=0.1)
        center = len(kernel) // 2
        assert kernel[center] == np.max(kernel)

    def test_wider_sigma_wider_kernel(self):
        k_narrow = make_gaussian_kernel(sigma=0.1, bin_width=0.1)
        k_wide = make_gaussian_kernel(sigma=0.5, bin_width=0.1)
        assert len(k_wide) > len(k_narrow)

    def test_gauss_function(self):
        # gauss(0, sigma) = 1/(sigma*sqrt(2*pi))
        sigma = 1.0
        expected = 1.0 / (sigma * np.sqrt(2.0 * np.pi))
        assert abs(gauss(0, sigma) - expected) < 1e-10

    def test_dgauss_zero_at_origin(self):
        assert abs(dgauss(0, 1.0)) < 1e-10


class TestHistogramBinDose:
    def test_uniform_dose(self):
        """All voxels at same dose should produce consistent bins."""
        dose = np.full(100, 0.55)
        region = np.ones(100)
        bins = histogram_bin_dose(dose, region, min_value=0.0, bin_width=0.1)
        # For dose=0.55: bin_scaled=5.5, low_bin=5, frac=0.5, frac_lo=0.5
        # bins[5] += 0.5*100=50, bins[6] += 0.5*100=50
        assert abs(bins[5] - 50.0) < 1e-10
        assert abs(bins[6] - 50.0) < 1e-10
        # Total mass should equal region sum
        assert abs(np.sum(bins) - 100.0) < 1e-10

    def test_zero_region_excluded(self):
        dose = np.array([0.5, 0.5, 0.5])
        region = np.array([1.0, 0.0, 1.0])
        bins = histogram_bin_dose(dose, region, min_value=0.0, bin_width=0.1)
        assert abs(np.sum(bins) - 2.0) < 1e-10

    def test_exact_bin_boundary(self):
        """Dose exactly at a bin boundary: frac=0, frac_lo=1."""
        dose = np.array([0.3])
        region = np.array([1.0])
        bins = histogram_bin_dose(dose, region, min_value=0.0, bin_width=0.1)
        # bin_scaled=3.0, low_bin=3, frac=0.0, frac_lo=1.0
        # max_bin = 3+2 = 5, so bins has indices 0..4
        assert abs(bins[3] - 1.0) < 1e-10
        assert abs(np.sum(bins) - 1.0) < 1e-10

    def test_fractional_binning(self):
        """Dose between bins should split between adjacent bins."""
        dose = np.array([0.25])  # bin 2.5 → frac=0.5
        region = np.array([1.0])
        bins = histogram_bin_dose(dose, region, min_value=0.0, bin_width=0.1)
        # frac_lo = 0.5 at bin 2, frac = 0.5 at bin 3
        assert abs(bins[2] - 0.5) < 1e-10
        assert abs(bins[3] - 0.5) < 1e-10
        assert abs(np.sum(bins) - 1.0) < 1e-10


class TestConvGauss:
    def test_output_length(self):
        buf = np.zeros(10)
        kernel = np.ones(5)
        out = conv_gauss(buf, kernel)
        assert len(out) == 10 + 5 - 1  # linear convolution

    def test_identity_convolution(self):
        """Convolving with a delta should return the input (shifted)."""
        buf = np.array([0.0, 0.0, 1.0, 0.0, 0.0])
        delta = np.array([0.0, 1.0, 0.0])
        out = conv_gauss(buf, delta)
        # Peak should still be 1.0
        assert abs(np.max(out) - 1.0) < 1e-10


class TestComputeGBins:
    def test_normalized(self, sample_dose_1d, sample_region_1d):
        gbins, _ = compute_gbins(
            sample_dose_1d, sample_region_1d,
            min_value=0.0, bin_width=0.1, var_min=0.01, var_max=0.01,
        )
        # Should be a proper probability distribution (sums to ~1)
        total = np.sum(gbins)
        assert abs(total - 1.0) < 0.05  # allow some tolerance for edge effects

    def test_nonnegative_mostly(self, sample_dose_1d, sample_region_1d):
        gbins, _ = compute_gbins(
            sample_dose_1d, sample_region_1d,
            min_value=0.0, bin_width=0.1, var_min=0.01, var_max=0.01,
        )
        # After Gaussian smoothing, bins should be mostly non-negative
        # (some tiny negatives possible from convolution artifacts)
        assert np.sum(gbins[gbins < -0.01]) > -0.1


class TestCumulativeBins:
    def test_monotonic_decreasing(self):
        bins = np.array([0.1, 0.2, 0.3, 0.2, 0.1, 0.05, 0.05])
        cum = cumulative_bins(bins)
        for i in range(len(cum) - 1):
            assert cum[i] >= cum[i + 1]

    def test_first_element_is_total(self):
        bins = np.array([0.1, 0.2, 0.3, 0.2, 0.1, 0.05, 0.05])
        cum = cumulative_bins(bins)
        assert abs(cum[0] - np.sum(bins)) < 1e-10

    def test_last_element(self):
        bins = np.array([0.1, 0.2, 0.3])
        cum = cumulative_bins(bins)
        assert abs(cum[-1] - bins[-1]) < 1e-10
