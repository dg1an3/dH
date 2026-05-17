"""
KL-divergence computation between calculated and target dose-volume
histograms.

Port of RtModel/KLDivTerm.cpp. The KL term is the core data-fit
objective: it pulls a structure's computed DVH toward a prescribed
DVH (specified as dose-volume points).

The EPS regularization in the C++ formula
    sum += calc * log(calc / (target + EPS) + EPS)
keeps the term finite when target bins are zero (common in practice
above the prescription dose). We mirror that exactly to preserve
gradient compatibility once gradients are wired through Prescription.
"""

from __future__ import annotations

from typing import Callable

import numpy as np

from .histogram import conv_gauss


EPS = 1e-5


def dvp_to_target_bins(
    dvps: np.ndarray,
    bin_width: float,
    get_bin_for_value: Callable[[float], int],
) -> np.ndarray:
    """
    Convert dose-volume points (DVPs) to a piecewise-linear target
    histogram. Matches KLDivTerm::GetTargetBins.

    Args:
        dvps: shape (N, 2). dvps[i] = [dose, cumulative_fraction].
            Must start at fraction 1.0 and end at fraction 0.0.
        bin_width: histogram bin width
        get_bin_for_value: callable mapping a dose value to bin index

    Returns:
        target_bins: shape (n_bins,), the per-bin density (slope of the
            cumulative DVH at each bin). Unnormalized; convolve and
            normalize via convolve_target_gbins to get a proper pdf.
    """
    if abs(dvps[0][1] - 1.0) > 1e-6:
        raise ValueError("DVPs must start at cumulative fraction 1.0")
    if abs(dvps[-1][1] - 0.0) > 1e-6:
        raise ValueError("DVPs must end at cumulative fraction 0.0")

    high = dvps[-1][0]
    n_bins = get_bin_for_value(high) + 1
    target_bins = np.zeros(n_bins)

    for i in range(len(dvps) - 1):
        inter_min = dvps[i][0]
        inter_max = dvps[i + 1][0]
        inter_slope = (dvps[i][1] - dvps[i + 1][1]) / (inter_max - inter_min) / bin_width

        bin_lo = get_bin_for_value(inter_min)
        bin_hi = get_bin_for_value(inter_max)
        for b in range(bin_lo, min(bin_hi + 1, n_bins)):
            target_bins[b] = inter_slope

    return target_bins


def convolve_target_gbins(
    target_bins: np.ndarray,
    kernel_var_min: np.ndarray,
    kernel_var_max: np.ndarray,
) -> np.ndarray:
    """
    Convolve target bins with the var_min and var_max kernels and
    normalize. Matches KLDivTerm::GetTargetGBins:

        target_gbins = 0.5 * conv(target, k_max) + 0.5 * conv(target, k_min)
        target_gbins /= sum(target_gbins)
    """
    convolved_max = conv_gauss(target_bins, kernel_var_max)
    convolved_min = conv_gauss(target_bins, kernel_var_min)
    target_gbins = 0.5 * convolved_max + 0.5 * convolved_min
    total = float(np.sum(target_gbins))
    if total > 0:
        target_gbins = target_gbins / total
    return target_gbins


def set_interval(
    low: float,
    high: float,
    fraction: float = 1.0,
    use_midpoint: bool = False,
) -> np.ndarray:
    """
    Build DVPs from a dose-interval prescription. Matches KLDivTerm::SetInterval.

    Two modes:
        - Step DVP (default): the prescription is "fraction of volume should
          receive dose between low and high"; two DVPs, [low, fraction] and
          [high, 0.0].
        - Midpoint mode: an inverted ramp with four DVPs at low,
          low + (high-low)/3, low + 2(high-low)/3, high.
    """
    if use_midpoint:
        return np.array([
            [low, 1.0],
            [low - (low - high) / 3.0, 2.0 / 3.0],
            [low - 2.0 * (low - high) / 3.0, 1.0 / 3.0],
            [high, 0.0],
        ])
    return np.array([[low, fraction], [high, 0.0]])


def kl_divergence_calc_over_target(
    calc_gpdf: np.ndarray,
    target_gpdf: np.ndarray,
) -> float:
    """
    KL(calc || target) -- the default mode (m_bTargetCrossEntropy = false).

    Matches C++ Eval:
        sum += calc[i] * log(calc[i] / (target[i] + EPS) + EPS)
    """
    n = len(calc_gpdf)
    total = 0.0
    for i in range(n):
        c = calc_gpdf[i]
        t = target_gpdf[i] if i < len(target_gpdf) else 0.0
        total += c * np.log(c / (t + EPS) + EPS)
    return float(total)


def kl_divergence_target_over_calc(
    calc_gpdf: np.ndarray,
    target_gpdf: np.ndarray,
) -> float:
    """
    KL(target || calc) -- the m_bTargetCrossEntropy = true mode.
    Matches the C++ Eval with the calc/target arguments swapped.
    """
    n = len(target_gpdf)
    total = 0.0
    for i in range(n):
        t = target_gpdf[i]
        c = calc_gpdf[i] if i < len(calc_gpdf) else 0.0
        total += t * np.log(t / (c + EPS) + EPS)
    return float(total)
