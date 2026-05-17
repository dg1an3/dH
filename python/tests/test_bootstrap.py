"""
Tests for BootstrapPhaseOptimizer + subsample_mask.

Headline integration test: drop BootstrapPhaseOptimizer into
HierarchicalBayes and confirm the pipeline still runs end-to-end. The
calibration verification (that bootstrap var_p actually IS shape-
stable across seeds) lives in the experiments script, since it's a
multi-seed sweep that takes longer than a unit test should.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.bootstrap import BootstrapPhaseOptimizer, subsample_mask
from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.dose_calc import gaussian_bump_dose_operator
from pybrimstone.hierarchical_bayes import HierarchicalBayes
from pybrimstone.kl_term import KLDivTerm
from pybrimstone.prescription import Prescription


# ---------------------------------------------------------------------------
# subsample_mask
# ---------------------------------------------------------------------------

class TestSubsampleMask:
    def test_fraction_one_returns_full(self):
        mask = np.array([1.0, 0.0, 1.0, 1.0])
        sub = subsample_mask(mask, fraction=1.0)
        assert np.allclose(sub, mask)

    def test_count_matches_fraction(self):
        mask = np.zeros(100)
        mask[:50] = 1.0  # 50 active voxels
        sub = subsample_mask(mask, fraction=0.4, rng=np.random.default_rng(0))
        # ceil(0.4 * 50) = 20
        assert int(sub.sum()) == 20

    def test_only_active_voxels_kept(self):
        mask = np.zeros(20)
        mask[5:15] = 1.0
        sub = subsample_mask(mask, fraction=0.5, rng=np.random.default_rng(0))
        # All chosen voxels must lie in the original active set.
        kept_idx = np.flatnonzero(sub > 0)
        active_idx = np.flatnonzero(mask > 0)
        assert set(kept_idx.tolist()).issubset(set(active_idx.tolist()))

    def test_reproducible_with_rng(self):
        mask = np.ones(20)
        s1 = subsample_mask(mask, fraction=0.5, rng=np.random.default_rng(42))
        s2 = subsample_mask(mask, fraction=0.5, rng=np.random.default_rng(42))
        assert np.allclose(s1, s2)

    def test_different_rngs_give_different_samples(self):
        mask = np.ones(50)
        s1 = subsample_mask(mask, fraction=0.5, rng=np.random.default_rng(1))
        s2 = subsample_mask(mask, fraction=0.5, rng=np.random.default_rng(2))
        assert not np.allclose(s1, s2)

    def test_rejects_invalid_fraction(self):
        mask = np.ones(5)
        with pytest.raises(ValueError, match="\\(0, 1\\]"):
            subsample_mask(mask, fraction=0.0)
        with pytest.raises(ValueError, match="\\(0, 1\\]"):
            subsample_mask(mask, fraction=1.5)

    def test_rejects_empty_mask(self):
        with pytest.raises(ValueError, match="no active"):
            subsample_mask(np.zeros(10), fraction=0.5)

    def test_at_least_one_voxel_kept(self):
        """Tiny structures still get at least one voxel."""
        mask = np.zeros(10)
        mask[0] = 1.0
        sub = subsample_mask(mask, fraction=0.01, rng=np.random.default_rng(0))
        assert sub.sum() >= 1


# ---------------------------------------------------------------------------
# BootstrapPhaseOptimizer
# ---------------------------------------------------------------------------

def _make_factory():
    """Test-fixture factory: 4-beamlet 5x5x5 PTV problem."""
    grid = (5, 5, 5)
    nx, ny, nz = grid
    n_voxels = nx * ny * nz
    n_beamlets = 4
    centers = np.array([[2.0, 2.0, 2.0 + i * 0.3] for i in range(n_beamlets)])
    D = gaussian_bump_dose_operator(grid, centers, sigma=1.0)
    full_mask = np.zeros(n_voxels)
    for i in range(1, 4):
        for j in range(1, 4):
            for k in range(1, 4):
                full_mask[i * ny * nz + j * nz + k] = 1.0

    def factory(rng):
        mask = full_mask if rng is None else subsample_mask(full_mask, 0.5, rng)
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.7, weight=1.0,
            bin_width=0.05, var_min=0.01, var_max=0.01,
        ))
        return p

    return factory, n_beamlets


class TestBootstrapPhaseOptimizer:
    def test_returns_mu_and_var(self):
        factory, n = _make_factory()
        opt = BootstrapPhaseOptimizer(
            factory, n_params=n, n_bootstrap=5,
            max_iter=20, tol=1e-3,
            adaptive_variance=(0.01, 0.1),
        )
        mu_p, var_p = opt(prior=None)
        assert mu_p.shape == (n,)
        assert var_p.shape == (n,)
        assert np.all(np.isfinite(mu_p))
        assert np.all(var_p > 0)

    def test_var_is_sample_variance_shape(self):
        """Sanity: with B=5 refits, var_p is the sample variance of the
        refit mu vectors, not the m_vAdaptVariance trace. So zero-noise
        refits should give near-zero var_p, and noisy refits larger."""
        # Easy way: zero subsample fraction is invalid, so just use
        # high tolerance + tight init to suppress noise.
        factory, n = _make_factory()
        opt = BootstrapPhaseOptimizer(
            factory, n_params=n, n_bootstrap=5,
            initial_params=np.zeros(n),
            max_iter=10, tol=1e-2,
            adaptive_variance=(0.01, 0.1),
        )
        _, var_p = opt(prior=None)
        # var_p comes from B independent CG runs on differently subsampled
        # masks; they should disagree to *some* extent, so var_p > floor.
        assert np.any(var_p > 1e-5)

    def test_rejects_low_n_bootstrap(self):
        factory, n = _make_factory()
        with pytest.raises(ValueError, match=">= 2"):
            BootstrapPhaseOptimizer(factory, n_params=n, n_bootstrap=1)

    def test_reproducible_with_same_seed(self):
        factory, n = _make_factory()
        kwargs = dict(
            n_params=n, n_bootstrap=4, max_iter=15, tol=1e-2,
            adaptive_variance=(0.01, 0.1), bootstrap_seed=42,
        )
        opt1 = BootstrapPhaseOptimizer(factory, **kwargs)
        opt2 = BootstrapPhaseOptimizer(factory, **kwargs)
        mu1, var1 = opt1(prior=None)
        mu2, var2 = opt2(prior=None)
        assert np.allclose(mu1, mu2)
        assert np.allclose(var1, var2)

    def test_different_seeds_change_var(self):
        factory, n = _make_factory()
        kwargs = dict(
            n_params=n, n_bootstrap=4, max_iter=15, tol=1e-2,
            adaptive_variance=(0.01, 0.1),
        )
        opt_a = BootstrapPhaseOptimizer(factory, bootstrap_seed=1, **kwargs)
        opt_b = BootstrapPhaseOptimizer(factory, bootstrap_seed=2, **kwargs)
        _, var_a = opt_a(prior=None)
        _, var_b = opt_b(prior=None)
        # Different seeds -> different subsample sets -> different var
        # (but mu_p uses the full data, so should match).
        assert not np.allclose(var_a, var_b)


# ---------------------------------------------------------------------------
# Integration with HierarchicalBayes
# ---------------------------------------------------------------------------

class TestBootstrapWithHierarchicalBayes:
    def test_drop_in_replacement(self):
        """HierarchicalBayes accepts BootstrapPhaseOptimizer in place of
        PhaseOptimizer with no other changes."""
        factory_a, n = _make_factory()
        factory_b, _ = _make_factory()
        phase_optimizers = [
            BootstrapPhaseOptimizer(
                factory_a, n_params=n, n_bootstrap=4,
                max_iter=15, tol=1e-2,
                adaptive_variance=(0.01, 0.1), bootstrap_seed=10,
            ),
            BootstrapPhaseOptimizer(
                factory_b, n_params=n, n_bootstrap=4,
                max_iter=15, tol=1e-2,
                adaptive_variance=(0.01, 0.1), bootstrap_seed=20,
            ),
        ]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(n), precision=0.1)
            for _ in phase_optimizers
        ]
        driver = HierarchicalBayes(phase_optimizers, priors)
        result = driver.run(max_outer_iters=3, tol=1e-2)

        assert result["mu_eta"].shape == (n,)
        assert result["var_eta"].shape == (n,)
        assert np.all(np.isfinite(result["mu_eta"]))
        assert np.all(result["var_eta"] > 0)
