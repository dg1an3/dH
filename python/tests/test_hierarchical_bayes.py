"""
Tests for the hierarchical-Bayes driver (HIERARCHICAL_BAYES_DESIGN.md Step 3).

Three test groups:
  - TestPoolPhases: closed-form sanity checks on the pooling math.
  - TestHierarchicalBayes: end-to-end coordinate ascent with mock phase
    optimizers (analytical solves; no CG noise).
  - TestHierarchicalBayesWithCG: integration with the polak_ribiere_cg
    reference from Step 1, including the Step-1 callback that surfaces
    sigma_weights (= m_vAdaptVariance).
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.hierarchical_bayes import HierarchicalBayes, pool_phases

# Import the Step-1 reference CG implementation for the integration test.
sys.path.insert(0, str(Path(__file__).parent))
from test_conjugate_gradient import polak_ribiere_cg


class TestPoolPhases:
    def test_single_phase_returns_input(self):
        mu = np.array([1.0, 2.0, 3.0])
        var = np.array([0.5, 0.5, 0.5])
        mu_eta, var_eta = pool_phases([mu], [var])
        assert np.allclose(mu_eta, mu)
        assert np.allclose(var_eta, var)

    def test_identical_phases_collapse_to_input(self):
        mu = np.array([1.0, -2.0, 0.5])
        var = np.array([0.3, 0.3, 0.3])
        mu_eta, var_eta = pool_phases([mu, mu, mu, mu], [var, var, var, var])
        assert np.allclose(mu_eta, mu)
        assert np.allclose(var_eta, var)

    def test_equal_variance_gives_simple_mean(self):
        """With identical variances, mu_eta should be the arithmetic mean."""
        mu_1 = np.array([0.0, 0.0])
        mu_2 = np.array([2.0, 4.0])
        var = np.array([1.0, 1.0])
        mu_eta, var_eta = pool_phases([mu_1, mu_2], [var, var])
        assert np.allclose(mu_eta, [1.0, 2.0])
        assert np.allclose(var_eta, var)

    def test_precision_weighted_mean(self):
        """Higher-precision phase pulls mu_eta closer."""
        mu_1 = np.array([0.0])
        mu_2 = np.array([10.0])
        var_1 = np.array([0.01])    # high precision (= 100)
        var_2 = np.array([100.0])    # low precision (= 0.01)
        mu_eta, _ = pool_phases([mu_1, mu_2], [var_1, var_2])
        # Precision-weighted: (100*0 + 0.01*10) / (100 + 0.01) ~= 0.001
        assert mu_eta[0] < 0.01
        assert mu_eta[0] > 0

    def test_pool_variance_is_per_beamlet_harmonic_mean(self):
        """var_eta[i] = 1 / mean(1/var_p[i])."""
        var_1 = np.array([1.0, 4.0])
        var_2 = np.array([1.0, 1.0])
        _, var_eta = pool_phases([np.zeros(2), np.zeros(2)], [var_1, var_2])
        # Dim 0: mean(1, 1) = 1, var_eta = 1
        # Dim 1: mean(0.25, 1) = 0.625, var_eta = 1.6
        assert np.isclose(var_eta[0], 1.0)
        assert np.isclose(var_eta[1], 1.0 / 0.625)

    def test_rejects_empty_input(self):
        with pytest.raises(ValueError, match="at least one phase"):
            pool_phases([], [])

    def test_rejects_mismatched_lengths(self):
        with pytest.raises(ValueError, match="same length"):
            pool_phases([np.zeros(3)], [np.zeros(3), np.zeros(3)])

    def test_rejects_nonpositive_variance(self):
        with pytest.raises(ValueError, match="positive"):
            pool_phases([np.zeros(3)], [np.array([1.0, 0.0, 1.0])])


class TestPoolPhasesNormalize:
    """
    The ROW 2 prescription from HIERARCHICAL_BAYES_DESIGN.md: normalize
    each phase's variance to mean=1 before pooling. Use when shape is
    stable across phases but absolute magnitude is not (e.g., bootstrap
    polluted by line-search runaway).
    """

    def test_normalize_off_matches_unnormalized(self):
        """normalize=False (default) should not change pool behavior."""
        mu_1 = np.array([0.0, 0.0])
        mu_2 = np.array([2.0, 4.0])
        var = np.array([1.0, 1.0])
        a = pool_phases([mu_1, mu_2], [var, var], normalize=False)
        b = pool_phases([mu_1, mu_2], [var, var])
        assert np.allclose(a[0], b[0])
        assert np.allclose(a[1], b[1])

    def test_normalize_strips_uniform_scale(self):
        """
        If two phases have variances that differ only by an overall scale,
        normalize=True should pool them as if they had the same magnitude.
        """
        mu_a = np.array([1.0, 2.0])
        mu_b = np.array([3.0, 4.0])
        var_small = np.array([0.1, 0.4])
        var_large = var_small * 1000.0  # same shape, 1000x scale

        # Without normalize: var_large gets ~zero weight (precision is tiny),
        # so the pool collapses to mu_a essentially.
        mu_no, _ = pool_phases([mu_a, mu_b], [var_small, var_large], normalize=False)

        # With normalize: both phases contribute equally because shape is
        # identical after rescale -> mu_eta is the simple mean.
        mu_yes, _ = pool_phases([mu_a, mu_b], [var_small, var_large], normalize=True)

        assert np.allclose(mu_yes, 0.5 * (mu_a + mu_b))
        # Without normalize, mu_no is much closer to mu_a than to mu_b.
        assert np.linalg.norm(mu_no - mu_a) < np.linalg.norm(mu_no - mu_b)

    def test_normalize_preserves_shape(self):
        """
        Per-phase normalization preserves relative per-beamlet shape:
        beamlets with bigger var stay bigger relative to the others
        within the same phase.
        """
        mu = np.array([0.0, 0.0, 0.0])
        # Phase a: shape [1, 2, 4]; phase b: shape [10, 20, 40] (same)
        var_a = np.array([1.0, 2.0, 4.0])
        var_b = var_a * 100
        _, var_eta = pool_phases([mu, mu], [var_a, var_b], normalize=True)
        # Pooled var_eta should have the same shape as the input shape.
        # After normalization both phases become [1/7*3, 2/7*3, 4/7*3]
        # = [3/7, 6/7, 12/7]; precision = 7/3, 7/6, 7/12; mean prec = same;
        # var_eta = inverse mean precision per beamlet -> shape preserved.
        ratio = var_eta / var_eta[0]
        assert np.allclose(ratio, var_a / var_a[0])

    def test_normalize_pool_variance_is_mean_normalized(self):
        """When inputs are already mean-1, var_eta has mean close to 1."""
        mu = np.zeros(4)
        var_a = np.array([0.5, 1.0, 1.5, 1.0])  # mean 1.0
        var_b = np.array([1.5, 0.5, 1.0, 1.0])  # mean 1.0
        _, var_eta = pool_phases([mu, mu], [var_a, var_b], normalize=True)
        # Per-phase normalize is a no-op here; sanity check.
        assert np.isclose(var_eta.mean(), 1.0, atol=0.1)


class TestHierarchicalBayes:
    """
    End-to-end coordinate ascent with analytical-solve mock phase optimizers.

    Phase p solves min_w 0.5*||w - z_p||^2 + prior_p(w). The closed-form
    minimizer is w_p* = (z_p + lambda*mu_eta) / (1 + lambda) when the prior
    has scalar precision lambda. Phase-level variance is taken from the DATA
    Hessian only (= 1 per beamlet here), not the joint Hessian, to avoid the
    double-counting pitfall flagged in HIERARCHICAL_BAYES_DESIGN.md Step 4:
    if the phase reports the joint-Hessian variance, pooling re-counts the
    prior precision and outer-loop convergence becomes O(1/n) instead of
    geometric.
    """

    @staticmethod
    def _analytical_phase(z_p: np.ndarray):
        """Return a phase_optimizer callable for f(w) = 0.5*||w - z_p||^2."""
        def phase_opt(prior: CoursePriorTerm):
            # Joint cost: 0.5*(w-z_p)^2 + 0.5*precision*(w-mu_eta)^2
            # d/dw = (w - z_p) + precision * (w - mu_eta) = 0
            # => w = (z_p + precision*mu_eta) / (1 + precision)
            prec = prior.precision
            mu_eta = prior.target_mu
            mu_p = (z_p + prec * mu_eta) / (1.0 + prec)
            # Data Hessian only: d^2/dw^2 of 0.5*||w - z_p||^2 is I.
            # Returning data variance avoids double-counting the prior; see
            # class docstring.
            var_p = np.ones_like(z_p)
            return mu_p, var_p
        return phase_opt

    def _build_driver(self, zs, init_mu, init_var):
        """Build a HierarchicalBayes with analytical phases at targets zs."""
        n = len(init_mu)
        phase_opts = [self._analytical_phase(z) for z in zs]
        priors = [
            CoursePriorTerm(target_mu=init_mu.copy(), precision=1.0 / init_var)
            for _ in zs
        ]
        return HierarchicalBayes(phase_opts, priors)

    def test_converges_to_mean_of_targets(self):
        """
        For symmetric quadratic phases, mu_eta should converge to the
        arithmetic mean of the per-phase targets, regardless of the prior
        strength (the symmetry of the setup makes the answer
        prior-independent).
        """
        zs = [
            np.array([0.0, 0.0]),
            np.array([2.0, 4.0]),
            np.array([4.0, 2.0]),
        ]
        init_mu = np.zeros(2)
        driver = self._build_driver(zs, init_mu, init_var=np.ones(2))

        result = driver.run(max_outer_iters=50, tol=1e-8)

        assert result["converged"]
        assert np.allclose(result["mu_eta"], [2.0, 2.0], atol=1e-6)

    def test_eta_stabilizes_within_few_outer_iterations(self):
        zs = [
            np.array([1.0, 1.0, 1.0]),
            np.array([2.0, 2.0, 2.0]),
        ]
        driver = self._build_driver(zs, np.zeros(3), np.ones(3))
        result = driver.run(max_outer_iters=50, tol=1e-8)

        assert result["converged"]
        # For two phases with these dynamics, convergence is fast.
        assert result["outer_iters"] < 30

    def test_history_records_per_phase_state(self):
        zs = [np.array([0.0]), np.array([4.0])]
        driver = self._build_driver(zs, np.array([2.0]), np.array([1.0]))
        result = driver.run(max_outer_iters=10)

        assert len(result["history"]) == result["outer_iters"]
        first = result["history"][0]
        assert "mu_eta" in first and "var_eta" in first
        assert len(first["phase_mus"]) == 2
        assert len(first["phase_vars"]) == 2

    def test_eta_monotonic_progress_toward_fixed_point(self):
        """
        mu_eta should approach the analytical fixed point monotonically.
        For the symmetric-quadratic case the fixed point is mean(zs).
        """
        zs = [np.array([0.0]), np.array([4.0])]
        target = np.array([2.0])
        driver = self._build_driver(zs, np.array([0.0]), np.array([1.0]))
        result = driver.run(max_outer_iters=30, tol=1e-12)

        distances = [np.linalg.norm(h["mu_eta"] - target) for h in result["history"]]
        # Non-increasing: every step should not move away from the fixed point.
        for prev, curr in zip(distances, distances[1:]):
            assert curr <= prev + 1e-12, \
                f"mu_eta moved away from fixed point: {prev} -> {curr}"

    def test_priors_are_mutated_in_place(self):
        """Driver updates CoursePriorTerm.target_mu and .precision in place."""
        zs = [np.array([0.0, 0.0]), np.array([2.0, 2.0])]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(2), precision=1.0),
            CoursePriorTerm(target_mu=np.zeros(2), precision=1.0),
        ]
        phase_opts = [self._analytical_phase(z) for z in zs]
        driver = HierarchicalBayes(phase_opts, priors)
        driver.run(max_outer_iters=5)

        # Both priors should now share the same updated target_mu (= mu_eta).
        assert np.allclose(priors[0].target_mu, priors[1].target_mu)
        assert np.allclose(priors[0].precision, priors[1].precision)

    def test_rejects_mismatched_phase_counts(self):
        priors = [CoursePriorTerm(np.zeros(2), 1.0)]
        opts = [
            self._analytical_phase(np.zeros(2)),
            self._analytical_phase(np.ones(2)),
        ]
        with pytest.raises(ValueError, match="equal length"):
            HierarchicalBayes(opts, priors)

    def test_rejects_zero_phases(self):
        with pytest.raises(ValueError, match="at least one"):
            HierarchicalBayes([], [])


class TestHierarchicalBayesWithCG:
    """
    Integration test: HierarchicalBayes wrapped around polak_ribiere_cg from
    Step 1. Exercises the full chain: inner CG with adaptive_variance enabled,
    sigma_weights flowing through the callback, CoursePriorTerm mutation
    between outer iterations.
    """

    @staticmethod
    def _make_cg_phase(z: np.ndarray, x0: np.ndarray):
        """
        Phase optimizer that uses polak_ribiere_cg on f(w) = 0.5*||w - z||^2
        plus the CoursePriorTerm pull, harvesting sigma_weights from the
        Step-1 callback.
        """
        def phase_opt(prior: CoursePriorTerm):
            def f(w):
                cost_data = 0.5 * np.dot(w - z, w - z)
                cost_prior, _ = prior.evaluate(w)
                return cost_data + cost_prior

            def grad_f(w):
                grad_data = w - z
                _, grad_prior = prior.evaluate(w)
                return grad_data + grad_prior

            last_sigma = [np.full_like(x0, 1.0)]
            def cb(**kw):
                if kw.get("sigma_weights") is not None:
                    last_sigma[0] = kw["sigma_weights"]

            mu_p, _, _, _ = polak_ribiere_cg(
                f, grad_f, x0,
                callback=cb,
                adaptive_variance=(0.01, 1.0),
                max_iter=200,
                tol=1e-6,
            )
            # Clamp to strictly positive (var bounds enforced by pool_phases).
            sigma_p = np.maximum(last_sigma[0], 1e-6)
            return mu_p, sigma_p

        return phase_opt

    def test_cg_phases_converge_to_target_mean(self):
        """
        Three quadratic phases at targets z1, z2, z3. Coordinate ascent
        should drive mu_eta toward mean(z) within tolerance.
        """
        zs = [
            np.array([0.0, 0.0, 0.0]),
            np.array([2.0, 4.0, -1.0]),
            np.array([4.0, -2.0, 1.0]),
        ]
        expected_mean = np.mean(zs, axis=0)

        x0 = np.zeros(3)
        phase_opts = [self._make_cg_phase(z, x0) for z in zs]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(3), precision=0.5)
            for _ in zs
        ]
        driver = HierarchicalBayes(phase_opts, priors)
        result = driver.run(max_outer_iters=15, tol=1e-4)

        # CG noise loosens tolerance vs the analytical test.
        assert np.allclose(result["mu_eta"], expected_mean, atol=1e-2), \
            f"mu_eta = {result['mu_eta']}, expected {expected_mean}"

    def test_sigma_weights_flow_through_to_pool(self):
        """
        Smoke test: confirm sigma_weights from the Step-1 callback is what
        the driver actually pools over. Tracks history for a 2-phase setup
        and asserts var_eta in history is finite and positive.
        """
        zs = [np.array([0.0, 0.0]), np.array([3.0, -2.0])]
        x0 = np.array([1.0, 1.0])
        phase_opts = [self._make_cg_phase(z, x0) for z in zs]
        priors = [
            CoursePriorTerm(target_mu=np.zeros(2), precision=0.1)
            for _ in zs
        ]
        driver = HierarchicalBayes(phase_opts, priors)
        result = driver.run(max_outer_iters=5)

        for h in result["history"]:
            assert np.all(np.isfinite(h["var_eta"]))
            assert np.all(h["var_eta"] > 0)
