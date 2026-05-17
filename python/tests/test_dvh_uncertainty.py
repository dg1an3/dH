"""
Tests for DVH uncertainty bands (HIERARCHICAL_BAYES_DESIGN.md Step 5).

Pipeline tested:
  sample_phase_posterior -> compute_dose -> compute_dvh -> compute_dvh_bands

Plus a full integration test that wires it onto a converged
HierarchicalBayes result, since this is the customer-facing deliverable.

Plotting tests use the Agg backend so they pass headlessly.
"""

import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")  # headless

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.dvh_uncertainty import (
    compute_dose,
    compute_dvh,
    compute_dvh_bands,
    dvh_uncertainty_bands,
    plot_dvh_bands,
    sample_phase_posterior,
)
from pybrimstone.hierarchical_bayes import HierarchicalBayes


# ---------------------------------------------------------------------------
# Sampling
# ---------------------------------------------------------------------------

class TestSamplePhasePosterior:
    def test_shape(self):
        s = sample_phase_posterior(
            mu_p=np.zeros(5), var_p=np.ones(5), n_samples=100,
        )
        assert s.shape == (100, 5)

    def test_mean_approaches_mu(self):
        rng = np.random.default_rng(0)
        mu = np.array([1.0, -2.0, 0.5])
        var = np.array([0.1, 0.1, 0.1])
        s = sample_phase_posterior(mu, var, n_samples=10_000, rng=rng)
        assert np.allclose(s.mean(axis=0), mu, atol=0.05)

    def test_std_approaches_sqrt_var(self):
        rng = np.random.default_rng(1)
        var = np.array([0.25, 1.0, 4.0])
        s = sample_phase_posterior(
            mu_p=np.zeros(3), var_p=var, n_samples=10_000, rng=rng,
        )
        assert np.allclose(s.std(axis=0), np.sqrt(var), rtol=0.05)

    def test_reproducible_with_rng(self):
        mu, var = np.zeros(3), np.ones(3)
        s1 = sample_phase_posterior(
            mu, var, n_samples=10, rng=np.random.default_rng(42),
        )
        s2 = sample_phase_posterior(
            mu, var, n_samples=10, rng=np.random.default_rng(42),
        )
        assert np.allclose(s1, s2)

    def test_rejects_mismatched_shapes(self):
        with pytest.raises(ValueError, match="!="):
            sample_phase_posterior(np.zeros(3), np.ones(4), n_samples=5)

    def test_rejects_nonpositive_variance(self):
        with pytest.raises(ValueError, match="positive"):
            sample_phase_posterior(np.zeros(3), np.array([1.0, 0.0, 1.0]), n_samples=5)


# ---------------------------------------------------------------------------
# Dose forward pass
# ---------------------------------------------------------------------------

class TestComputeDose:
    def test_matrix_shape_single(self):
        D = np.eye(10, 4)              # 10 voxels, 4 beamlets
        dose = compute_dose(D, np.ones(4))
        assert dose.shape == (10,)

    def test_matrix_shape_batched(self):
        D = np.eye(10, 4)
        samples = np.ones((50, 4))
        dose = compute_dose(D, samples)
        assert dose.shape == (50, 10)

    def test_matrix_linearity(self):
        rng = np.random.default_rng(0)
        D = rng.normal(size=(8, 4))
        w1, w2 = rng.normal(size=4), rng.normal(size=4)
        d1, d2 = compute_dose(D, w1), compute_dose(D, w2)
        combined = compute_dose(D, 2.0 * w1 + 3.0 * w2)
        assert np.allclose(combined, 2.0 * d1 + 3.0 * d2)

    def test_callable_single(self):
        def op(w):
            return w * 2.0
        dose = compute_dose(op, np.array([1.0, 2.0, 3.0]))
        assert np.allclose(dose, [2.0, 4.0, 6.0])

    def test_callable_batched(self):
        def op(w):
            return w * 2.0
        samples = np.array([[1.0, 2.0], [3.0, 4.0]])
        dose = compute_dose(op, samples)
        assert np.allclose(dose, [[2.0, 4.0], [6.0, 8.0]])


# ---------------------------------------------------------------------------
# DVH binning
# ---------------------------------------------------------------------------

class TestComputeDvh:
    def test_dvh_at_zero_is_one(self):
        dose = np.array([0.0, 1.0, 2.0, 3.0])
        mask = np.ones(4, dtype=bool)
        dvh = compute_dvh(dose, mask, np.array([0.0]))
        assert np.isclose(dvh[0], 1.0)

    def test_uniform_dose_step(self):
        """All voxels at dose=5.0; DVH(d) = 1 for d <= 5, 0 above."""
        dose = np.full(10, 5.0)
        mask = np.ones(10, dtype=bool)
        bins = np.array([0.0, 2.5, 5.0, 5.1, 10.0])
        dvh = compute_dvh(dose, mask, bins)
        assert np.allclose(dvh, [1.0, 1.0, 1.0, 0.0, 0.0])

    def test_monotone_non_increasing(self):
        rng = np.random.default_rng(0)
        dose = rng.uniform(0, 10, size=100)
        mask = np.ones(100, dtype=bool)
        bins = np.linspace(0, 10, 50)
        dvh = compute_dvh(dose, mask, bins)
        diffs = np.diff(dvh)
        assert np.all(diffs <= 1e-12), \
            f"DVH not non-increasing: max positive diff = {diffs.max()}"

    def test_respects_structure_mask(self):
        """DVH only counts voxels in the structure."""
        dose = np.array([1.0, 10.0, 1.0, 10.0])
        mask = np.array([True, False, True, False])  # only low-dose voxels
        dvh = compute_dvh(dose, mask, np.array([0.5, 5.0]))
        # All structure voxels have dose=1, so DVH(0.5)=1, DVH(5)=0.
        assert np.allclose(dvh, [1.0, 0.0])

    def test_single_voxel_structure(self):
        dose = np.array([3.0, 7.0])
        mask = np.array([False, True])
        bins = np.array([0.0, 5.0, 8.0])
        dvh = compute_dvh(dose, mask, bins)
        assert np.allclose(dvh, [1.0, 1.0, 0.0])

    def test_empty_structure_raises(self):
        with pytest.raises(ValueError, match="empty"):
            compute_dvh(np.zeros(5), np.zeros(5, dtype=bool), np.array([0.0]))


# ---------------------------------------------------------------------------
# Percentile bands
# ---------------------------------------------------------------------------

class TestComputeDvhBands:
    def test_percentile_ordering(self):
        """For p1 <= p2, band[p1] <= band[p2] everywhere."""
        rng = np.random.default_rng(0)
        n_samples, n_voxels = 50, 30
        dose_samples = rng.uniform(0, 10, size=(n_samples, n_voxels))
        mask = np.ones(n_voxels, dtype=bool)
        bins = np.linspace(0, 10, 20)
        bands = compute_dvh_bands(dose_samples, mask, bins, percentiles=(5, 50, 95))
        assert np.all(bands[0] <= bands[1] + 1e-12)
        assert np.all(bands[1] <= bands[2] + 1e-12)

    def test_zero_variance_collapses_band(self):
        """Identical samples => 5/50/95 all equal."""
        dose = np.array([1.0, 2.0, 3.0, 4.0])
        n_samples = 10
        dose_samples = np.tile(dose, (n_samples, 1))
        mask = np.ones(4, dtype=bool)
        bins = np.array([0.5, 2.5, 4.5])
        bands = compute_dvh_bands(dose_samples, mask, bins, percentiles=(5, 50, 95))
        assert np.allclose(bands[0], bands[1])
        assert np.allclose(bands[1], bands[2])

    def test_median_matches_single_sample_dvh(self):
        """With one sample, every percentile equals that sample's DVH."""
        dose = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
        mask = np.ones(5, dtype=bool)
        bins = np.array([0.0, 2.5, 4.5])
        single_dvh = compute_dvh(dose, mask, bins)
        bands = compute_dvh_bands(dose[None, :], mask, bins, percentiles=(50,))
        assert np.allclose(bands[0], single_dvh)

    def test_rejects_percentiles_out_of_range(self):
        with pytest.raises(ValueError, match="\\[0, 100\\]"):
            compute_dvh_bands(
                np.zeros((3, 5)), np.ones(5, dtype=bool),
                np.array([0.0]), percentiles=(-5,),
            )

    def test_band_width_scales_with_variance(self):
        rng = np.random.default_rng(0)
        n_voxels = 50
        mask = np.ones(n_voxels, dtype=bool)
        bins = np.linspace(0, 10, 20)

        narrow = rng.normal(loc=5.0, scale=0.1, size=(100, n_voxels))
        wide = rng.normal(loc=5.0, scale=2.0, size=(100, n_voxels))

        b_narrow = compute_dvh_bands(narrow, mask, bins, percentiles=(5, 95))
        b_wide = compute_dvh_bands(wide, mask, bins, percentiles=(5, 95))

        width_narrow = (b_narrow[1] - b_narrow[0]).mean()
        width_wide = (b_wide[1] - b_wide[0]).mean()
        assert width_wide > width_narrow


# ---------------------------------------------------------------------------
# End-to-end pipeline
# ---------------------------------------------------------------------------

class TestDvhUncertaintyBandsPipeline:
    def _setup(self, n_voxels=30, n_beamlets=8, seed=0):
        rng = np.random.default_rng(seed)
        D = np.abs(rng.normal(size=(n_voxels, n_beamlets)))
        mu_p = np.abs(rng.normal(loc=1.0, scale=0.3, size=n_beamlets))
        var_p = np.full(n_beamlets, 0.05)
        structures = {
            "PTV": np.zeros(n_voxels, dtype=bool),
            "OAR": np.zeros(n_voxels, dtype=bool),
        }
        structures["PTV"][: n_voxels // 2] = True
        structures["OAR"][n_voxels // 2 :] = True
        return D, mu_p, var_p, structures

    def test_pipeline_returns_per_structure_bands(self):
        D, mu_p, var_p, structures = self._setup()
        result = dvh_uncertainty_bands(
            mu_p, var_p, D, structures,
            n_samples=200, rng=np.random.default_rng(0),
        )
        assert set(result.keys()) == set(structures.keys())
        for name, (bins, bands) in result.items():
            assert bands.shape == (3, bins.size)

    def test_pipeline_bands_are_valid_dvh(self):
        """Bands should be in [0,1] and non-increasing in dose."""
        D, mu_p, var_p, structures = self._setup()
        result = dvh_uncertainty_bands(
            mu_p, var_p, D, structures,
            n_samples=100, rng=np.random.default_rng(0),
        )
        for name, (bins, bands) in result.items():
            assert np.all(bands >= 0.0) and np.all(bands <= 1.0)
            for row in bands:
                diffs = np.diff(row)
                assert np.all(diffs <= 1e-12), \
                    f"{name}: DVH not non-increasing"

    def test_pipeline_auto_dose_bins(self):
        """If dose_bins=None, bins are chosen from sample range."""
        D, mu_p, var_p, structures = self._setup()
        result = dvh_uncertainty_bands(
            mu_p, var_p, D, structures, n_samples=50, dose_bins=None,
            rng=np.random.default_rng(0),
        )
        for _, (bins, _) in result.items():
            assert bins[0] == 0.0
            assert bins[-1] > 0.0
            assert bins.size == 100


# ---------------------------------------------------------------------------
# Integration with HierarchicalBayes (Step 3) output
# ---------------------------------------------------------------------------

class TestEndToEndWithDriver:
    """
    The headline Step-5 demo: take a converged HierarchicalBayes result,
    pipe (mu_eta, var_eta) into dvh_uncertainty_bands, get per-structure
    percentile bands. This is what the customer sees.
    """

    @staticmethod
    def _analytical_phase(z_p):
        def phase_opt(prior):
            prec = prior.precision
            mu_eta = prior.target_mu
            mu_p = (z_p + prec * mu_eta) / (1.0 + prec)
            var_p = np.ones_like(z_p) * 0.1
            return mu_p, var_p
        return phase_opt

    def test_full_pipeline_on_converged_driver(self):
        rng = np.random.default_rng(0)
        n_beamlets = 6
        zs = [
            rng.normal(loc=1.0, scale=0.5, size=n_beamlets),
            rng.normal(loc=1.0, scale=0.5, size=n_beamlets),
        ]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(n_beamlets), precision=1.0)
            for _ in zs
        ]
        phase_opts = [self._analytical_phase(z) for z in zs]
        driver = HierarchicalBayes(phase_opts, priors)
        # tol relaxed: Step 5's deliverable doesn't need tight eta convergence,
        # just a reasonable (mu_eta, var_eta) to sample from for DVH bands.
        result = driver.run(max_outer_iters=50, tol=1e-3)
        assert result["converged"]

        n_voxels = 40
        D = np.abs(rng.normal(size=(n_voxels, n_beamlets)))
        structures = {
            "PTV": np.zeros(n_voxels, dtype=bool),
        }
        structures["PTV"][:20] = True

        bands = dvh_uncertainty_bands(
            result["mu_eta"], result["var_eta"],
            D, structures, n_samples=150,
            rng=np.random.default_rng(1),
        )

        bins, ptv_bands = bands["PTV"]
        # 5th <= 50th <= 95th everywhere
        assert np.all(ptv_bands[0] <= ptv_bands[1] + 1e-12)
        assert np.all(ptv_bands[1] <= ptv_bands[2] + 1e-12)
        # Each band is a valid DVH
        for row in ptv_bands:
            assert np.all(row >= 0.0) and np.all(row <= 1.0)


# ---------------------------------------------------------------------------
# Plotting (headless)
# ---------------------------------------------------------------------------

class TestPlotDvhBands:
    def test_returns_axes_with_data(self):
        import matplotlib.pyplot as plt
        bins = np.linspace(0, 10, 20)
        bands = np.stack([
            np.linspace(0.8, 0.0, 20),  # 5th
            np.linspace(0.9, 0.1, 20),  # 50th
            np.linspace(1.0, 0.2, 20),  # 95th
        ])
        fig, ax = plt.subplots()
        try:
            ax = plot_dvh_bands(bins, bands, structure_name="PTV", ax=ax)
            # One line for the median.
            assert len(ax.get_lines()) == 1
            # One PolyCollection for the band fill.
            from matplotlib.collections import PolyCollection
            assert any(isinstance(c, PolyCollection) for c in ax.collections)
        finally:
            plt.close(fig)

    def test_single_percentile_no_band_fill(self):
        """With only the median percentile, no shaded region."""
        import matplotlib.pyplot as plt
        bins = np.linspace(0, 10, 5)
        bands = np.array([np.linspace(0.9, 0.1, 5)])
        fig, ax = plt.subplots()
        try:
            ax = plot_dvh_bands(
                bins, bands, structure_name="Solo", ax=ax,
                percentiles=(50.0,),
            )
            from matplotlib.collections import PolyCollection
            assert not any(isinstance(c, PolyCollection) for c in ax.collections)
        finally:
            plt.close(fig)

    def test_rejects_mismatched_band_shape(self):
        import matplotlib.pyplot as plt
        bins = np.linspace(0, 10, 5)
        bands = np.zeros((2, 5))
        fig, ax = plt.subplots()
        try:
            with pytest.raises(ValueError, match="must match"):
                plot_dvh_bands(
                    bins, bands, ax=ax, percentiles=(5, 50, 95),
                )
        finally:
            plt.close(fig)
