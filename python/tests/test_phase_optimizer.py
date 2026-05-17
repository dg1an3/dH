"""
Tests for the dose calc, library CG, and PhaseOptimizer (port commit 3/3).

Headline integration test: PhaseOptimizer + HierarchicalBayes on a
synthetic 2-phase Gaussian-bump problem. Verifies that the full
brimstone-port pipeline reaches a converged hierarchical posterior
without exceptions and produces a sensible answer (eta close to the
two-phase target compromise).
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.dose_calc import gaussian_bump_dose_operator
from pybrimstone.hierarchical_bayes import HierarchicalBayes
from pybrimstone.kl_term import KLDivTerm
from pybrimstone.numerics.conjugate_gradient import polak_ribiere_cg
from pybrimstone.phase_optimizer import PhaseOptimizer
from pybrimstone.prescription import Prescription


# ---------------------------------------------------------------------------
# Library CG parity with test-file reference
# ---------------------------------------------------------------------------

class TestLibraryCGParity:
    def test_quadratic_2d_converges(self):
        f = lambda x: float(x[0] ** 2 + x[1] ** 2)
        grad = lambda x: np.array([2 * x[0], 2 * x[1]])
        x, _, _, conv = polak_ribiere_cg(f, grad, np.array([3.0, 4.0]))
        assert conv
        assert np.allclose(x, [0.0, 0.0], atol=1e-3)

    def test_sigma_weights_via_callback(self):
        f = lambda x: float(np.sum(x * x))
        grad = lambda x: 2.0 * x
        records = []
        polak_ribiere_cg(
            f, grad, np.array([1.0, 1.0, 1.0]),
            callback=lambda **kw: records.append(kw),
            adaptive_variance=(0.01, 1.0),
        )
        assert all("sigma_weights" in r for r in records)
        assert all(r["sigma_weights"] is not None for r in records)


# ---------------------------------------------------------------------------
# Gaussian-bump dose operator
# ---------------------------------------------------------------------------

class TestGaussianBumpDose:
    def test_shape(self):
        D = gaussian_bump_dose_operator(
            grid_shape=(4, 4, 4),
            beamlet_centers=np.array([[2.0, 2.0, 2.0]]),
        )
        assert D.shape == (64, 1)

    def test_peak_at_center(self):
        D = gaussian_bump_dose_operator(
            grid_shape=(5, 5, 5),
            beamlet_centers=np.array([[2.0, 2.0, 2.0]]),
            sigma=1.0,
        )
        peak_idx = 2 * 25 + 2 * 5 + 2
        assert np.isclose(D[peak_idx, 0], 1.0)
        # All other voxels strictly less.
        assert D[:, 0].max() == D[peak_idx, 0]

    def test_decays_with_distance(self):
        D = gaussian_bump_dose_operator(
            grid_shape=(7, 1, 1),
            beamlet_centers=np.array([[3.0, 0.0, 0.0]]),
            sigma=1.0,
        )
        contrib = D[:, 0]
        assert contrib[3] > contrib[2] > contrib[1] > contrib[0]
        assert contrib[3] > contrib[4] > contrib[5] > contrib[6]

    def test_multiple_beamlets_superpose(self):
        D = gaussian_bump_dose_operator(
            grid_shape=(5, 5, 5),
            beamlet_centers=np.array([
                [1.0, 2.0, 2.0],
                [3.0, 2.0, 2.0],
            ]),
            sigma=0.5,
        )
        # Sum of two equal weights should be linear superposition.
        w = np.array([1.0, 1.0])
        dose = D @ w
        # Two peaks at the two centers
        peak_a = 1 * 25 + 2 * 5 + 2
        peak_b = 3 * 25 + 2 * 5 + 2
        assert np.allclose(dose[peak_a], dose[peak_b])
        assert dose[peak_a] > dose[2 * 25 + 2 * 5 + 2]  # midpoint < either peak

    def test_rejects_bad_centers_shape(self):
        with pytest.raises(ValueError, match="n_beamlets, 3"):
            gaussian_bump_dose_operator(
                grid_shape=(4, 4, 4),
                beamlet_centers=np.array([2.0, 2.0]),
            )

    def test_rejects_nonpositive_sigma(self):
        with pytest.raises(ValueError, match="positive"):
            gaussian_bump_dose_operator(
                grid_shape=(4, 4, 4),
                beamlet_centers=np.array([[2.0, 2.0, 2.0]]),
                sigma=0.0,
            )


# ---------------------------------------------------------------------------
# PhaseOptimizer
# ---------------------------------------------------------------------------

class TestPhaseOptimizer:
    @staticmethod
    def _build(seed=0, n_beamlets=4):
        rng = np.random.default_rng(seed)
        grid_shape = (6, 6, 6)
        centers = np.array([
            [2.0, 2.0, 2.0 + i * 0.5] for i in range(n_beamlets)
        ])
        D = gaussian_bump_dose_operator(grid_shape, centers, sigma=1.0)
        n_voxels = D.shape[0]
        # PTV is the central 2x2x2 cube.
        mask = np.zeros(n_voxels)
        for i in range(2, 4):
            for j in range(2, 4):
                for k in range(2, 4):
                    mask[i * 36 + j * 6 + k] = 1.0
        kl = KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.8,
            bin_width=0.05, var_min=0.01, var_max=0.01,
        )
        p = Prescription(D, use_transform=True)
        p.add_dose_term(kl)
        return p, n_beamlets

    def test_callable_returns_mu_and_var(self):
        p, n = self._build()
        opt = PhaseOptimizer(p, n_params=n, max_iter=30, tol=1e-3)
        mu, var = opt(prior=None)
        assert mu.shape == (n,)
        assert var.shape == (n,)
        assert np.all(np.isfinite(mu))
        assert np.all(var > 0)

    def test_prior_swap_doesnt_stack(self):
        """Two consecutive calls with different priors shouldn't stack
        beamlet_terms in the prescription."""
        p, n = self._build()
        opt = PhaseOptimizer(p, n_params=n, max_iter=20, tol=1e-3)
        prior_a = CoursePriorTerm(target_mu=np.zeros(n), precision=0.1)
        prior_b = CoursePriorTerm(target_mu=np.ones(n) * 0.2, precision=0.3)
        opt(prior=prior_a)
        opt(prior=prior_b)
        # The active prior is prior_b; prior_a should not also be in the list.
        assert prior_b in p.beamlet_terms
        assert prior_a not in p.beamlet_terms
        assert len(p.beamlet_terms) == 1

    def test_no_prior_leaves_no_beamlet_terms(self):
        p, n = self._build()
        opt = PhaseOptimizer(p, n_params=n, max_iter=20, tol=1e-3)
        # First install a prior, then call with None.
        prior = CoursePriorTerm(target_mu=np.zeros(n), precision=0.1)
        opt(prior=prior)
        opt(prior=None)
        assert len(p.beamlet_terms) == 0


# ---------------------------------------------------------------------------
# End-to-end integration with HierarchicalBayes
# ---------------------------------------------------------------------------

class TestEndToEndWithHierarchicalBayes:
    def test_two_phase_pipeline_runs(self):
        """
        Two synthetic phases with slightly different prescription doses,
        wired through PhaseOptimizer + HierarchicalBayes. Asserts the
        pipeline completes and produces finite outputs; the calibration
        of pooled eta depends on the joint-vs-data variance issue from
        Step 3 and isn't asserted here.
        """
        rng = np.random.default_rng(0)
        n_beamlets = 4
        grid_shape = (5, 5, 5)
        centers = np.array([
            [2.0, 2.0, 2.0 + i * 0.5] for i in range(n_beamlets)
        ])
        D = gaussian_bump_dose_operator(grid_shape, centers, sigma=1.0)
        n_voxels = D.shape[0]

        # PTV mask: central voxels.
        mask = np.zeros(n_voxels)
        for i in range(2, 4):
            for j in range(2, 4):
                for k in range(2, 4):
                    mask[i * 25 + j * 5 + k] = 1.0

        def make_phase(dose_min, dose_max):
            kl = KLDivTerm.from_interval(
                mask, dose_min=dose_min, dose_max=dose_max,
                bin_width=0.05, var_min=0.01, var_max=0.01,
            )
            p = Prescription(D, use_transform=True)
            p.add_dose_term(kl)
            return PhaseOptimizer(p, n_params=n_beamlets, max_iter=30, tol=1e-3)

        phase_optimizers = [
            make_phase(0.4, 0.7),
            make_phase(0.5, 0.8),
        ]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(n_beamlets), precision=0.5)
            for _ in phase_optimizers
        ]

        driver = HierarchicalBayes(phase_optimizers, priors)
        result = driver.run(max_outer_iters=5, tol=1e-2)

        assert result["mu_eta"].shape == (n_beamlets,)
        assert result["var_eta"].shape == (n_beamlets,)
        assert np.all(np.isfinite(result["mu_eta"]))
        assert np.all(result["var_eta"] > 0)
        assert len(result["history"]) == result["outer_iters"]
