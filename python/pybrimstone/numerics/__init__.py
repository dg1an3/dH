"""
Numerical primitives for the pure-Python brimstone port.

These modules lift the reference implementations from python/tests/
into library code that the Prescription, KLDivTerm, and PhaseOptimizer
layers consume. The reference test files (test_histogram.py,
test_kl_divergence.py, test_parameter_transform.py) stay in place as
behavioral validation -- they exercise the same math, just inline.

Modules:
    histogram          Gaussian-convolved DVH (CHistogram port)
    parameter_transform Sigmoid optimizer-to-beamlet transform
                       (Prescription.cpp::Transform port)
    kl_divergence      DVP-to-target-bins + KL formulas (KLDivTerm port)
"""

from .histogram import (
    GBINS_KERNEL_WIDTH,
    compute_gbins,
    conv_gauss,
    cumulative_bins,
    dgauss,
    gauss,
    histogram_bin_dose,
    make_gaussian_kernel,
)
from .parameter_transform import (
    INPUT_SCALE,
    SIGMOID_SCALE,
    d_transform,
    inv_transform,
    sigmoid,
    transform,
)
from .kl_divergence import (
    EPS,
    convolve_target_gbins,
    dvp_to_target_bins,
    kl_divergence_calc_over_target,
    kl_divergence_target_over_calc,
    set_interval,
)

__all__ = [
    # histogram
    "GBINS_KERNEL_WIDTH",
    "compute_gbins",
    "conv_gauss",
    "cumulative_bins",
    "dgauss",
    "gauss",
    "histogram_bin_dose",
    "make_gaussian_kernel",
    # parameter_transform
    "INPUT_SCALE",
    "SIGMOID_SCALE",
    "d_transform",
    "inv_transform",
    "sigmoid",
    "transform",
    # kl_divergence
    "EPS",
    "convolve_target_gbins",
    "dvp_to_target_bins",
    "kl_divergence_calc_over_target",
    "kl_divergence_target_over_calc",
    "set_interval",
]
