"""
Gaussian-convolved dose-volume histogram primitives.

Port of RtModel/Histogram.cpp::CHistogram. The reference and behavioral
tests live in python/tests/test_histogram.py.
"""

from __future__ import annotations

import numpy as np


GBINS_KERNEL_WIDTH = 8.0


def gauss(x: float, sigma: float) -> float:
    """Gaussian function. Matches C++ Gauss<REAL>."""
    if sigma <= 0:
        return 0.0
    return float(np.exp(-0.5 * (x / sigma) ** 2) / (sigma * np.sqrt(2.0 * np.pi)))


def dgauss(x: float, sigma: float) -> float:
    """Derivative of Gaussian. Used for HistogramGradient kernels."""
    if sigma <= 0:
        return 0.0
    return -x / (sigma ** 2) * gauss(x, sigma)


def make_gaussian_kernel(
    sigma: float,
    bin_width: float,
    kernel_width: float = GBINS_KERNEL_WIDTH,
) -> np.ndarray:
    """
    Discrete Gaussian kernel matching CHistogram::SetGBinVar.

    The kernel is bin_width * gauss(z * bin_width, sigma) at z in
    [-N, N] where N = ceil(kernel_width * sigma / bin_width). The
    bin_width factor makes the kernel sum approximately 1 (it's a
    Riemann sum of the continuous Gaussian).
    """
    neighborhood = int(np.ceil(kernel_width * sigma / bin_width))
    z_vals = np.arange(-neighborhood, neighborhood + 1)
    return bin_width * np.array([gauss(z * bin_width, sigma) for z in z_vals])


def histogram_bin_dose(
    dose_values: np.ndarray,
    region: np.ndarray,
    min_value: float,
    bin_width: float,
) -> np.ndarray:
    """
    Fractional dose binning matching CHistogram::GetBins.

    Each voxel's region weight is split between the two adjacent bins
    proportional to the fractional position. Voxels with region <= 0
    are excluded.
    """
    bin_scaled = (dose_values - min_value) / bin_width
    low_bin = np.floor(bin_scaled).astype(int)
    frac = bin_scaled - low_bin
    frac_lo = 1.0 - frac

    max_bin = int(np.max(low_bin)) + 2
    bins = np.zeros(max_bin)

    for i in range(len(dose_values)):
        if region[i] <= 0:
            continue
        b = low_bin[i]
        r = region[i]
        bins[b] += frac_lo[i] * r
        bins[b + 1] += frac[i] * r

    return bins


def conv_gauss(buffer_in: np.ndarray, kernel: np.ndarray) -> np.ndarray:
    """Linear convolution matching CHistogram::ConvGauss."""
    return np.convolve(buffer_in, kernel)


def compute_gbins(
    dose_values: np.ndarray,
    region: np.ndarray,
    min_value: float = 0.0,
    bin_width: float = 0.1,
    var_min: float = 0.01,
    var_max: float = 0.01,
) -> tuple[np.ndarray, np.ndarray]:
    """
    Full bin -> convolve -> normalize histogram pipeline.

    Returns (gbins, bins): the Gaussian-convolved normalized histogram
    and the raw fractional bins. Matches the C++ flow with the default
    (VarFracLo=0, VarFracHi=1) regime where the var_max kernel carries
    all the weight.
    """
    sigma_max = np.sqrt(var_max)
    GBINS_BUFFER = GBINS_KERNEL_WIDTH
    adjusted_min = min_value - GBINS_BUFFER * sigma_max

    bins = histogram_bin_dose(dose_values, region, adjusted_min, bin_width)
    kernel_max = make_gaussian_kernel(sigma_max, bin_width)
    gbins = conv_gauss(bins, kernel_max)

    region_sum = float(np.sum(region[region > 0]))
    if region_sum > 0:
        gbins = gbins / region_sum
    return gbins, bins


def cumulative_bins(bins: np.ndarray) -> np.ndarray:
    """Right-to-left cumulative sum (the DVH form). Matches GetCumBins."""
    return np.cumsum(bins[::-1])[::-1].copy()
