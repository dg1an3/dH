"""
Smoke tests for the pybrimstone.numerics library lift.

The reference implementations of histogram/transform/kl_divergence live
in test_histogram.py, test_parameter_transform.py, test_kl_divergence.py
and stay there as inline behavioral checks. This file verifies that the
lifted library modules at pybrimstone.numerics.* produce numerically
identical results on a few representative inputs -- if the lift drifted
from the reference, these tests catch it.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

# Library
from pybrimstone.numerics import (
    INPUT_SCALE,
    SIGMOID_SCALE,
    compute_gbins,
    conv_gauss,
    convolve_target_gbins,
    cumulative_bins,
    d_transform,
    dvp_to_target_bins,
    gauss,
    histogram_bin_dose,
    inv_transform,
    kl_divergence_calc_over_target,
    kl_divergence_target_over_calc,
    make_gaussian_kernel,
    set_interval,
    sigmoid,
    transform,
)

# Reference (inline impls in the existing test files)
sys.path.insert(0, str(Path(__file__).parent))
from test_histogram import (
    compute_gbins as ref_compute_gbins,
    cumulative_bins as ref_cumulative_bins,
    histogram_bin_dose as ref_histogram_bin_dose,
    make_gaussian_kernel as ref_make_gaussian_kernel,
)
from test_kl_divergence import (
    convolve_target_gbins as ref_convolve_target_gbins,
    dvp_to_target_bins as ref_dvp_to_target_bins,
    kl_divergence_calc_over_target as ref_kl_ct,
    set_interval as ref_set_interval,
)
from test_parameter_transform import (
    d_transform as ref_d_transform,
    inv_transform as ref_inv_transform,
    sigmoid as ref_sigmoid,
    transform as ref_transform,
)


# ---------------------------------------------------------------------------
# Histogram parity
# ---------------------------------------------------------------------------

class TestHistogramLiftParity:
    def test_gaussian_kernel_matches_reference(self):
        for sigma in [0.05, 0.1, 0.3, 1.0]:
            lib = make_gaussian_kernel(sigma=sigma, bin_width=0.1)
            ref = ref_make_gaussian_kernel(sigma=sigma, bin_width=0.1)
            assert np.allclose(lib, ref)

    def test_bin_dose_matches_reference(self):
        rng = np.random.default_rng(0)
        dose = rng.uniform(0, 5, size=50)
        region = (rng.uniform(size=50) > 0.3).astype(float)
        lib = histogram_bin_dose(dose, region, 0.0, 0.1)
        ref = ref_histogram_bin_dose(dose, region, 0.0, 0.1)
        assert np.allclose(lib, ref)

    def test_compute_gbins_matches_reference(self):
        rng = np.random.default_rng(1)
        dose = rng.uniform(0, 2, size=100)
        region = np.ones(100)
        lib_gbins, lib_bins = compute_gbins(dose, region, 0.0, 0.1, 0.01, 0.01)
        ref_gbins, ref_bins = ref_compute_gbins(dose, region, 0.0, 0.1, 0.01, 0.01)
        assert np.allclose(lib_gbins, ref_gbins)
        assert np.allclose(lib_bins, ref_bins)

    def test_cumulative_bins_matches_reference(self):
        bins = np.array([0.1, 0.2, 0.3, 0.2, 0.1, 0.05, 0.05])
        assert np.allclose(cumulative_bins(bins), ref_cumulative_bins(bins))


# ---------------------------------------------------------------------------
# Parameter transform parity
# ---------------------------------------------------------------------------

class TestParameterTransformParity:
    def test_sigmoid_matches_reference(self):
        x = np.linspace(-5, 5, 30)
        assert np.allclose(sigmoid(x), ref_sigmoid(x))

    def test_transform_round_trip(self):
        params = np.linspace(-3, 3, 20)
        out = transform(params)
        recovered = inv_transform(out)
        assert np.allclose(params, recovered, atol=1e-8)

    def test_d_transform_matches_reference(self):
        x = np.linspace(-4, 4, 25)
        assert np.allclose(d_transform(x), ref_d_transform(x))

    def test_constants_match(self):
        assert INPUT_SCALE == 0.5
        assert SIGMOID_SCALE == 0.2


# ---------------------------------------------------------------------------
# KL divergence parity
# ---------------------------------------------------------------------------

class TestKlDivergenceParity:
    def test_set_interval_matches_reference(self):
        lib = set_interval(0.5, 1.0)
        ref = ref_set_interval(0.5, 1.0)
        assert np.allclose(lib, ref)

    def test_dvp_to_target_bins_matches_reference(self):
        bin_width = 0.1
        get_bin = lambda v: int(np.floor(v / bin_width))
        dvps = set_interval(0.0, 1.0)
        lib = dvp_to_target_bins(dvps, bin_width, get_bin)
        ref = ref_dvp_to_target_bins(dvps, bin_width, get_bin)
        assert np.allclose(lib, ref)

    def test_kl_calc_over_target_matches_reference(self):
        rng = np.random.default_rng(0)
        p = rng.uniform(0.01, 1.0, size=20)
        p = p / p.sum()
        q = rng.uniform(0.01, 1.0, size=20)
        q = q / q.sum()
        assert np.isclose(
            kl_divergence_calc_over_target(p, q),
            ref_kl_ct(p, q),
        )

    def test_kl_target_over_calc_finite(self):
        """Sanity that the second-mode formula doesn't NaN."""
        calc = np.array([0.0, 0.0, 0.5, 0.5])
        target = np.array([0.3, 0.4, 0.2, 0.1])
        assert np.isfinite(kl_divergence_target_over_calc(calc, target))

    def test_convolve_target_gbins_matches_reference(self):
        target = np.array([0.0, 0.0, 5.0, 5.0, 0.0, 0.0])
        k = make_gaussian_kernel(sigma=0.1, bin_width=0.1)
        lib = convolve_target_gbins(target, k, k)
        ref = ref_convolve_target_gbins(target, k, k)
        assert np.allclose(lib, ref)


# ---------------------------------------------------------------------------
# End-to-end: full bin -> convolve -> KL pipeline using the library
# ---------------------------------------------------------------------------

class TestNumericsPipeline:
    def test_full_pipeline_runs(self):
        """Sanity: build a synthetic dose, build a target DVH, compute KL."""
        rng = np.random.default_rng(0)
        n_voxels = 500
        dose = rng.uniform(0, 2, size=n_voxels)
        region = np.ones(n_voxels)
        bin_width = 0.1

        # Calc DVH
        calc_gbins, _ = compute_gbins(
            dose, region, min_value=0.0, bin_width=bin_width,
            var_min=0.01, var_max=0.01,
        )

        # Target DVH from a prescription interval
        dvps = set_interval(1.0, 1.5)
        get_bin = lambda v: int(np.floor(v / bin_width))
        target_bins = dvp_to_target_bins(dvps, bin_width, get_bin)
        k = make_gaussian_kernel(sigma=0.1, bin_width=bin_width)
        target_gbins = convolve_target_gbins(target_bins, k, k)

        # KL
        kl = kl_divergence_calc_over_target(calc_gbins, target_gbins)
        assert np.isfinite(kl)
        assert kl > 0  # different distributions => positive KL
