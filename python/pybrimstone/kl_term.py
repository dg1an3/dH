"""
KL-divergence dose-fit objective term.

Port of RtModel/KLDivTerm.cpp wrapped as a DoseObjectiveTerm: takes a
per-voxel dose, returns (KL cost, gradient w.r.t. dose).

Forward chain:
    dose[v]  --bin (fractional)-->  bins[j]
    bins[j]  --conv(kernel)-->     gbins[i]
    gbins[i] --normalize(/sum)-->  pdf[i]
    pdf, target -->  KL(pdf || target_pdf)

Backward chain (analytical):
    d KL / d pdf[i]   = log(pdf[i] / (target[i] + EPS) + EPS) + 1
                       (matches d/dp[p log(p/q + e)] with the EPS
                        regularization the C++ uses)
    d KL / d gbins[i] = (d KL / d pdf[i]) / region_sum
    d KL / d bins[j]  = sum_i kernel[i-j] * (d KL / d gbins[i])
                       (adjoint of np.convolve(bins, kernel))
    d KL / d dose[v]  = (d KL / d bins[bin_idx_v + 1]
                         - d KL / d bins[bin_idx_v]) * region[v] / bin_width
                       (linear interpolation backward)

The +EPS regularization in the C++ Eval keeps the term finite when the
target pdf has zero bins (common above the prescription dose). We mirror
it exactly so the gradient is consistent with the cost in those regions.
"""

from __future__ import annotations

from typing import Optional, Tuple

import numpy as np

from .numerics import (
    EPS,
    conv_gauss,
    dvp_to_target_bins,
    convolve_target_gbins,
    make_gaussian_kernel,
    set_interval,
)
from .objective_terms import DoseObjectiveTerm


class KLDivTerm(DoseObjectiveTerm):
    """
    KL-divergence objective term for a single structure.

    Args:
        structure_mask: shape (n_voxels,), boolean or {0,1}. The
            structure-of-interest mask. Voxels outside the mask are
            ignored when binning.
        dvps: dose-volume points, shape (N, 2), starting at fraction
            1.0 and ending at 0.0. Use the SetInterval convenience to
            build common prescriptions.
        weight: relative weight in the composite Prescription cost.
        bin_width: histogram bin width in dose units (Gy).
        min_dose: minimum dose value for the bin axis. Typically 0.
        var_min, var_max: adaptive-variance bounds; the kernel sigma
            is sqrt(var_max) in the default (VarFracLo=0, VarFracHi=1)
            regime.
        cross_entropy: if True, use KL(target || calc) instead of
            KL(calc || target). Matches m_bTargetCrossEntropy in C++.
    """

    def __init__(
        self,
        structure_mask: np.ndarray,
        dvps: np.ndarray,
        weight: float = 1.0,
        bin_width: float = 0.1,
        min_dose: float = 0.0,
        var_min: float = 0.01,
        var_max: float = 0.01,
        cross_entropy: bool = False,
    ):
        self.structure_mask = np.asarray(structure_mask).astype(float)
        self.region_sum = float(self.structure_mask.sum())
        if self.region_sum <= 0:
            raise ValueError("structure_mask must have at least one nonzero voxel")

        self.weight = float(weight)
        self.bin_width = float(bin_width)
        self.min_dose = float(min_dose)
        self.var_min = float(var_min)
        self.var_max = float(var_max)
        self.cross_entropy = bool(cross_entropy)

        self.sigma_max = float(np.sqrt(self.var_max))
        self.sigma_min = float(np.sqrt(self.var_min))
        from .numerics import GBINS_KERNEL_WIDTH
        self.kernel_buffer = GBINS_KERNEL_WIDTH
        self.adjusted_min = self.min_dose - self.kernel_buffer * self.sigma_max

        self.kernel_max = make_gaussian_kernel(self.sigma_max, self.bin_width)
        self.kernel_min = make_gaussian_kernel(self.sigma_min, self.bin_width)

        self.dvps = np.asarray(dvps, dtype=np.float64)
        self.target_gbins = self._build_target_gbins()

    # ------------------------------------------------------------------
    # Convenience constructor
    # ------------------------------------------------------------------

    @classmethod
    def from_interval(
        cls,
        structure_mask: np.ndarray,
        dose_min: float,
        dose_max: float,
        fraction: float = 1.0,
        use_midpoint: bool = False,
        **kwargs,
    ) -> "KLDivTerm":
        dvps = set_interval(dose_min, dose_max, fraction=fraction, use_midpoint=use_midpoint)
        return cls(structure_mask=structure_mask, dvps=dvps, **kwargs)

    # ------------------------------------------------------------------
    # Target histogram setup
    # ------------------------------------------------------------------

    def _get_bin(self, value: float) -> int:
        return int(np.floor((value - self.adjusted_min) / self.bin_width))

    def _build_target_gbins(self) -> np.ndarray:
        target_bins = dvp_to_target_bins(self.dvps, self.bin_width, self._get_bin)
        return convolve_target_gbins(target_bins, self.kernel_min, self.kernel_max)

    # ------------------------------------------------------------------
    # Forward + backward
    # ------------------------------------------------------------------

    def _bin_dose(self, dose: np.ndarray) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """Fractional bin; returns (bins, low_bin_per_voxel, in_range_mask)."""
        bin_scaled = (dose - self.adjusted_min) / self.bin_width
        low_bin = np.floor(bin_scaled).astype(int)
        frac = bin_scaled - low_bin
        frac_lo = 1.0 - frac

        max_bin = int(np.max(low_bin)) + 2
        bins = np.zeros(max_bin)
        # Vectorized fractional accumulation; loop is per-voxel for clarity.
        # For the prototype this is fast enough; a real port would use
        # np.add.at or scipy.sparse for large voxel counts.
        for v in range(len(dose)):
            if self.structure_mask[v] <= 0:
                continue
            b = low_bin[v]
            r = self.structure_mask[v]
            bins[b] += frac_lo[v] * r
            bins[b + 1] += frac[v] * r
        return bins, low_bin, np.ones_like(dose, dtype=bool)

    def _conv_adjoint(self, dgbins: np.ndarray, n_bins: int) -> np.ndarray:
        """
        Adjoint of np.convolve(bins, self.kernel_max). For Toeplitz M with
        M[i,j] = kernel[i-j], (M.T @ y)[j] = sum_k kernel[k] * y[j+k].
        Implemented as np.convolve(y, kernel[::-1]) sliced to length n_bins.
        """
        K = self.kernel_max
        full = np.convolve(dgbins, K[::-1])
        # The block of length n_bins starting at index len(K)-1 contains
        # the dot products (M.T y)[j] for j = 0..n_bins-1.
        start = len(K) - 1
        return full[start : start + n_bins]

    def evaluate(self, dose: np.ndarray) -> Tuple[float, np.ndarray]:
        dose = np.asarray(dose, dtype=np.float64)
        if dose.shape != self.structure_mask.shape:
            raise ValueError(
                f"dose shape {dose.shape} != structure_mask shape {self.structure_mask.shape}"
            )

        bins, low_bin, _ = self._bin_dose(dose)
        gbins = conv_gauss(bins, self.kernel_max)
        pdf = gbins / self.region_sum

        # Pad/truncate target to align with pdf length for elementwise ops.
        n = pdf.size
        target = np.zeros(n)
        m = min(self.target_gbins.size, n)
        target[:m] = self.target_gbins[:m]

        if self.cross_entropy:
            # KL(target || pdf): sum_i target[i] * log(target[i] / (pdf[i]+EPS) + EPS)
            cost = float(np.sum(target * np.log(target / (pdf + EPS) + EPS)))
            # d/dpdf[i]: -target[i] / (pdf[i] + EPS) / (target[i]/(pdf[i]+EPS) + EPS)
            # = -target[i] / ((pdf[i] + EPS) * (target[i]/(pdf[i]+EPS) + EPS))
            # = -target[i] / (target[i] + EPS * (pdf[i]+EPS))
            denom = target + EPS * (pdf + EPS)
            dpdf = -target / np.where(denom > 0, denom, EPS)
        else:
            # KL(pdf || target): sum_i pdf[i] * log(pdf[i] / (target[i]+EPS) + EPS)
            arg = pdf / (target + EPS) + EPS
            cost = float(np.sum(pdf * np.log(arg)))
            # d/dpdf[i] [pdf log(pdf/(target+EPS) + EPS)]
            # = log(arg) + pdf * (1/arg) * (1/(target+EPS))
            # = log(arg) + (pdf/(target+EPS)) / arg
            # The second term simplifies to (1 - EPS/arg) (since pdf/(t+EPS)=arg-EPS).
            # Compute directly for numerical stability:
            dpdf = np.log(arg) + (pdf / (target + EPS)) / arg

        dgbins = dpdf / self.region_sum
        dbins = self._conv_adjoint(dgbins, bins.size)

        # Backward through the fractional binning.
        grad = np.zeros_like(dose)
        for v in range(len(dose)):
            if self.structure_mask[v] <= 0:
                continue
            j = low_bin[v]
            grad[v] = (dbins[j + 1] - dbins[j]) * self.structure_mask[v] / self.bin_width

        return cost, grad
