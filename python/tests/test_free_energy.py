"""
Tests for variational free-energy diagnostics (HIERARCHICAL_BAYES_DESIGN.md Step 4).

The headline validation: when phase optimizers report data-only variance
(no double-counting of the prior; see Step 3 caveat), F_total computed
from the HierarchicalBayes history is non-increasing across outer
iterations. This is what the design doc Step 4 asks us to verify before
trusting the pooling math.

A contrasting test confirms that the joint-variance (double-counting)
case does NOT produce monotone F_total -- documenting the failure mode
rather than just the success mode.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.free_energy import (
    free_energy_trajectory,
    gaussian_entropy_diag,
    phase_free_energy,
    total_free_energy,
)
from pybrimstone.hierarchical_bayes import HierarchicalBayes


# ---------------------------------------------------------------------------
# Phase optimizer mocks (shared with test_hierarchical_bayes structure)
# ---------------------------------------------------------------------------

def _analytical_phase_data_only(z_p: np.ndarray):
    """
    Phase optimizer for f(w) = 0.5 * ||w - z_p||^2 with data-only variance.

    Closed-form: w_p* = (z_p + precision*mu_eta) / (1 + precision).
    Reports variance from data Hessian only (= 1), not joint, so pooling
    doesn't re-count the prior. This is the regime where F_total should
    decrease monotonically.
    """
    def phase_opt(prior: CoursePriorTerm):
        prec = prior.precision
        mu_eta = prior.target_mu
        mu_p = (z_p + prec * mu_eta) / (1.0 + prec)
        var_p = np.ones_like(z_p)
        return mu_p, var_p
    return phase_opt


def _analytical_phase_joint_variance(z_p: np.ndarray):
    """
    Same closed-form mean, but reports JOINT variance var = 1/(1+precision).
    This is the double-counting failure mode from Step 3; F_total drifts.
    """
    def phase_opt(prior: CoursePriorTerm):
        prec = prior.precision
        mu_eta = prior.target_mu
        mu_p = (z_p + prec * mu_eta) / (1.0 + prec)
        var_p = 1.0 / (1.0 + prec)
        return mu_p, var_p
    return phase_opt


# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------

class TestGaussianEntropy:
    def test_unit_variance(self):
        """H(N(0,1)) per dim = 0.5*log(2*pi*e)."""
        h = gaussian_entropy_diag(np.array([1.0]))
        assert np.isclose(h, 0.5 * np.log(2.0 * np.pi * np.e))

    def test_scales_with_dim(self):
        """Entropy is additive across independent dims."""
        h1 = gaussian_entropy_diag(np.array([1.0]))
        h5 = gaussian_entropy_diag(np.ones(5))
        assert np.isclose(h5, 5.0 * h1)

    def test_increases_with_variance(self):
        """log is monotone, so larger var -> larger entropy."""
        h_small = gaussian_entropy_diag(np.array([0.1]))
        h_large = gaussian_entropy_diag(np.array([10.0]))
        assert h_large > h_small

    def test_rejects_nonpositive(self):
        with pytest.raises(ValueError, match="positive"):
            gaussian_entropy_diag(np.array([1.0, 0.0]))


class TestPhaseFreeEnergy:
    def test_no_prior_pull_when_mu_p_equals_mu_eta(self):
        n = 4
        mu = np.zeros(n)
        var = np.ones(n)
        f = phase_free_energy(
            data_cost=5.0,
            mu_p=mu, var_p=var,
            mu_eta=mu, precision_eta=np.full(n, 2.0),
        )
        # F = 5.0 + 0 - 0.5*4*log(2*pi*e)
        expected = 5.0 - 0.5 * 4 * np.log(2.0 * np.pi * np.e)
        assert np.isclose(f, expected)

    def test_prior_pull_is_quadratic(self):
        n = 3
        var = np.ones(n)
        prec = np.array([1.0, 2.0, 4.0])
        mu_p = np.array([1.0, 1.0, 1.0])
        mu_eta = np.zeros(n)
        f = phase_free_energy(
            data_cost=0.0,
            mu_p=mu_p, var_p=var,
            mu_eta=mu_eta, precision_eta=prec,
        )
        # prior_pull = 0.5*(1*1 + 2*1 + 4*1) = 3.5
        # entropy = 0.5*3*log(2*pi*e)
        expected = 3.5 - 0.5 * 3 * np.log(2.0 * np.pi * np.e)
        assert np.isclose(f, expected)

    def test_data_cost_adds_linearly(self):
        """F is linear in data_cost (the other terms are independent)."""
        n = 2
        common = dict(
            mu_p=np.array([0.5, -0.5]),
            var_p=np.ones(n),
            mu_eta=np.zeros(n),
            precision_eta=np.full(n, 1.0),
        )
        f_a = phase_free_energy(data_cost=10.0, **common)
        f_b = phase_free_energy(data_cost=20.0, **common)
        assert np.isclose(f_b - f_a, 10.0)


class TestTotalFreeEnergy:
    def test_matches_sum_of_phase_terms(self):
        n = 2
        history_entry = {
            "mu_eta": np.array([0.0, 0.0]),
            "var_eta": np.array([1.0, 1.0]),
            "phase_mus": [np.array([1.0, 0.0]), np.array([0.0, 1.0])],
            "phase_vars": [np.ones(n), np.ones(n)],
        }
        cost_fns = [
            lambda w: 0.5 * float(np.dot(w, w)),
            lambda w: 0.5 * float(np.dot(w, w)),
        ]
        total = total_free_energy(history_entry, cost_fns)

        prec_eta = 1.0 / history_entry["var_eta"]
        f1 = phase_free_energy(
            data_cost=0.5,
            mu_p=history_entry["phase_mus"][0],
            var_p=history_entry["phase_vars"][0],
            mu_eta=history_entry["mu_eta"],
            precision_eta=prec_eta,
        )
        f2 = phase_free_energy(
            data_cost=0.5,
            mu_p=history_entry["phase_mus"][1],
            var_p=history_entry["phase_vars"][1],
            mu_eta=history_entry["mu_eta"],
            precision_eta=prec_eta,
        )
        assert np.isclose(total, f1 + f2)

    def test_rejects_phase_count_mismatch(self):
        history_entry = {
            "mu_eta": np.zeros(2),
            "var_eta": np.ones(2),
            "phase_mus": [np.zeros(2), np.zeros(2)],
            "phase_vars": [np.ones(2), np.ones(2)],
        }
        with pytest.raises(ValueError, match="must match"):
            total_free_energy(history_entry, [lambda w: 0.0])


# ---------------------------------------------------------------------------
# Integration: monotone F_total under proper (data-only) variance
# ---------------------------------------------------------------------------

class TestMonotoneFreeEnergy:
    """
    Step-4 acceptance test: coordinate ascent + correctly-reported variance
    => F_total strictly non-increasing across outer iterations.
    """

    def _run(self, zs, init_mu, init_var, phase_factory):
        phase_opts = [phase_factory(z) for z in zs]
        priors = [
            CoursePriorTerm(target_mu=init_mu.copy(), precision=1.0 / init_var)
            for _ in zs
        ]
        driver = HierarchicalBayes(phase_opts, priors)
        result = driver.run(max_outer_iters=30, tol=1e-12)

        data_cost_fns = [
            (lambda z: lambda w: 0.5 * float(np.dot(w - z, w - z)))(z)
            for z in zs
        ]
        traj = free_energy_trajectory(result["history"], data_cost_fns)
        return result, traj

    def test_three_phase_quadratic_data_only_variance(self):
        zs = [
            np.array([0.0, 0.0]),
            np.array([2.0, 4.0]),
            np.array([4.0, 2.0]),
        ]
        _, traj = self._run(
            zs, np.zeros(2), np.ones(2),
            phase_factory=_analytical_phase_data_only,
        )
        assert len(traj) >= 2
        for prev, curr in zip(traj, traj[1:]):
            assert curr <= prev + 1e-10, \
                f"F_total increased: {prev:.6f} -> {curr:.6f}"

    def test_two_phase_data_only_variance(self):
        zs = [np.array([1.0, -1.0]), np.array([-2.0, 3.0])]
        _, traj = self._run(
            zs, np.zeros(2), np.ones(2),
            phase_factory=_analytical_phase_data_only,
        )
        for prev, curr in zip(traj, traj[1:]):
            assert curr <= prev + 1e-10, \
                f"F_total increased: {prev:.6f} -> {curr:.6f}"

    def test_strict_decrease_until_convergence(self):
        """F should strictly decrease while mu_eta is still moving."""
        zs = [np.array([0.0]), np.array([4.0]), np.array([8.0])]
        result, traj = self._run(
            zs, np.array([0.0]), np.array([1.0]),
            phase_factory=_analytical_phase_data_only,
        )

        # Find the first iteration where convergence-test gates further
        # change. All steps before that point should show STRICT decrease.
        n_iters = result["outer_iters"]
        for i in range(min(5, n_iters - 1)):
            assert traj[i + 1] < traj[i], \
                f"F not strictly decreasing at iter {i}: {traj[i]} -> {traj[i+1]}"

    def test_total_energy_is_finite(self):
        """Sanity: F_total is finite for normal inputs (no NaN/inf)."""
        zs = [np.array([0.0]), np.array([2.0])]
        _, traj = self._run(
            zs, np.array([0.0]), np.array([1.0]),
            phase_factory=_analytical_phase_data_only,
        )
        assert all(np.isfinite(traj))


# ---------------------------------------------------------------------------
# Failure-mode documentation: double-counting breaks monotonicity
# ---------------------------------------------------------------------------

class TestDoubleCountingPathology:
    """
    Documents the Step-3 caveat: reporting joint variance instead of
    data-only variance breaks F_total monotonicity. This isn't a bug in
    F_total or in the driver -- it's evidence that the phase wrapper is
    feeding the wrong variance, exactly as the design doc warns.
    """

    def test_joint_variance_breaks_monotonicity(self):
        zs = [np.array([0.0, 0.0]), np.array([2.0, 4.0]), np.array([4.0, 2.0])]
        init_mu = np.zeros(2)
        init_var = np.ones(2)

        phase_opts = [_analytical_phase_joint_variance(z) for z in zs]
        priors = [
            CoursePriorTerm(target_mu=init_mu.copy(), precision=1.0 / init_var)
            for _ in zs
        ]
        driver = HierarchicalBayes(phase_opts, priors)
        result = driver.run(max_outer_iters=20, tol=1e-12)

        data_cost_fns = [
            (lambda z: lambda w: 0.5 * float(np.dot(w - z, w - z)))(z)
            for z in zs
        ]
        traj = free_energy_trajectory(result["history"], data_cost_fns)

        # Find at least one iteration where F_total INCREASES, which is
        # the failure mode this test documents. If monotonicity ever held
        # under joint-variance reporting, the design doc's caveat would be
        # spurious and worth removing.
        any_increase = any(
            curr > prev + 1e-10 for prev, curr in zip(traj, traj[1:])
        )
        assert any_increase, (
            "Expected joint-variance reporting to break F_total monotonicity "
            "somewhere in the trajectory; if this assertion fails, the Step 3 "
            "caveat in HIERARCHICAL_BAYES_DESIGN.md may be wrong."
        )
