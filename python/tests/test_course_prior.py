"""
Tests for CoursePriorTerm (HIERARCHICAL_BAYES_DESIGN.md Step 2).

The standalone behavioral test from the design doc: "Test as a standalone
regularizer on a single plan: behavior should match ridge regression on
beamlet weight space." Verified by composing CoursePriorTerm with a
synthetic linear upstream cost and checking the joint minimizer matches the
closed-form ridge solution.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

# Make pybrimstone importable when running with --noconftest
sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import BeamletObjectiveTerm, CoursePriorTerm


class TestCoursePriorBasics:
    def test_cost_is_zero_at_mean(self):
        mu = np.array([1.0, 2.0, 3.0])
        term = CoursePriorTerm(target_mu=mu, precision=1.0)
        cost, grad = term.evaluate(mu)
        assert cost == 0.0
        assert np.allclose(grad, 0.0)

    def test_cost_is_quadratic(self):
        mu = np.zeros(4)
        term = CoursePriorTerm(target_mu=mu, precision=1.0)
        w = np.array([1.0, 1.0, 1.0, 1.0])
        cost_1, _ = term.evaluate(w)
        cost_2, _ = term.evaluate(2.0 * w)
        # Doubling displacement should quadruple cost.
        assert np.isclose(cost_2, 4.0 * cost_1)

    def test_gradient_matches_analytical(self):
        rng = np.random.RandomState(0)
        mu = rng.randn(5)
        precision = np.abs(rng.randn(5)) + 0.1
        term = CoursePriorTerm(target_mu=mu, precision=precision)

        w = rng.randn(5)
        _, grad = term.evaluate(w)

        expected = precision * (w - mu)
        assert np.allclose(grad, expected)

    def test_cost_matches_analytical(self):
        rng = np.random.RandomState(1)
        mu = rng.randn(6)
        precision = np.abs(rng.randn(6)) + 0.1
        term = CoursePriorTerm(target_mu=mu, precision=precision)

        w = rng.randn(6)
        cost, _ = term.evaluate(w)

        diff = w - mu
        expected = 0.5 * np.sum(precision * diff * diff)
        assert np.isclose(cost, expected)

    def test_scalar_precision_broadcasts(self):
        mu = np.zeros(3)
        scalar = CoursePriorTerm(target_mu=mu, precision=2.5)
        array_ = CoursePriorTerm(target_mu=mu, precision=np.full(3, 2.5))

        w = np.array([1.0, -1.0, 0.5])
        c_s, g_s = scalar.evaluate(w)
        c_a, g_a = array_.evaluate(w)

        assert np.isclose(c_s, c_a)
        assert np.allclose(g_s, g_a)

    def test_heterogeneous_precision_per_beamlet(self):
        """Per-beamlet precision should weight each dim's contribution."""
        mu = np.zeros(3)
        precision = np.array([10.0, 1.0, 0.1])
        term = CoursePriorTerm(target_mu=mu, precision=precision)

        # Unit displacement in each dim
        for i in range(3):
            w = np.zeros(3)
            w[i] = 1.0
            cost, _ = term.evaluate(w)
            assert np.isclose(cost, 0.5 * precision[i])


class TestCoursePriorValidation:
    def test_rejects_2d_target_mu(self):
        with pytest.raises(ValueError, match="must be 1-D"):
            CoursePriorTerm(target_mu=np.zeros((3, 3)), precision=1.0)

    def test_rejects_mismatched_precision_shape(self):
        with pytest.raises(ValueError, match="must match"):
            CoursePriorTerm(target_mu=np.zeros(4), precision=np.zeros(3))

    def test_rejects_negative_precision(self):
        with pytest.raises(ValueError, match="non-negative"):
            CoursePriorTerm(target_mu=np.zeros(3), precision=-1.0)

    def test_evaluate_rejects_mismatched_weights(self):
        term = CoursePriorTerm(target_mu=np.zeros(4), precision=1.0)
        with pytest.raises(ValueError, match="must match"):
            term.evaluate(np.zeros(5))


class TestRidgeRegressionEquivalence:
    """
    The acceptance test called out in HIERARCHICAL_BAYES_DESIGN.md Step 2:
    composing CoursePriorTerm with a quadratic upstream cost should yield
    the closed-form ridge solution.

    Upstream cost:  0.5 * ||Aw - b||^2
    Prior:          0.5 * lambda * ||w - mu||^2 = CoursePriorTerm
    Total gradient: A^T(Aw - b) + lambda*(w - mu) = 0
    Solution:       w* = (A^T A + lambda I)^{-1} (A^T b + lambda mu)
    """

    def _setup(self, m=20, n=8, seed=0):
        rng = np.random.RandomState(seed)
        A = rng.randn(m, n)
        b = rng.randn(m)
        mu = rng.randn(n)
        return A, b, mu

    def test_zero_displacement_matches_least_squares(self):
        """With precision=0 the prior should vanish; total minimizer = OLS."""
        A, b, mu = self._setup()
        term = CoursePriorTerm(target_mu=mu, precision=0.0)

        # OLS: w* = (A^T A)^{-1} A^T b
        ols = np.linalg.solve(A.T @ A, A.T @ b)

        # Joint: upstream gradient at OLS is zero; prior gradient is zero
        # because precision=0. So OLS satisfies the joint stationarity.
        cost_prior, grad_prior = term.evaluate(ols)
        assert cost_prior == 0.0
        assert np.allclose(grad_prior, 0.0)

    def test_stationarity_at_ridge_solution(self):
        """Closed-form ridge solution should make the joint gradient vanish."""
        A, b, mu = self._setup()
        lam = 0.7
        term = CoursePriorTerm(target_mu=mu, precision=lam)

        n = A.shape[1]
        ridge = np.linalg.solve(A.T @ A + lam * np.eye(n), A.T @ b + lam * mu)

        # Upstream gradient at ridge: A^T (A w - b)
        upstream_grad = A.T @ (A @ ridge - b)
        # Prior gradient at ridge:
        _, prior_grad = term.evaluate(ridge)

        total = upstream_grad + prior_grad
        assert np.allclose(total, 0.0, atol=1e-10), \
            f"joint gradient not zero at closed-form ridge solution: {total}"

    def test_stationarity_with_heterogeneous_precision(self):
        """Per-beamlet precision generalizes the scalar ridge case."""
        A, b, mu = self._setup(seed=2)
        n = A.shape[1]
        rng = np.random.RandomState(3)
        lam_diag = np.abs(rng.randn(n)) + 0.5
        term = CoursePriorTerm(target_mu=mu, precision=lam_diag)

        # Generalized ridge: w* = (A^T A + diag(lam))^{-1} (A^T b + lam*mu)
        ridge = np.linalg.solve(
            A.T @ A + np.diag(lam_diag),
            A.T @ b + lam_diag * mu,
        )

        upstream_grad = A.T @ (A @ ridge - b)
        _, prior_grad = term.evaluate(ridge)
        total = upstream_grad + prior_grad
        assert np.allclose(total, 0.0, atol=1e-10)

    def test_stronger_prior_pulls_solution_toward_mu(self):
        """Sanity: as precision -> infinity, solution -> mu."""
        A, b, mu = self._setup(seed=4)
        n = A.shape[1]

        def solve_for_lambda(lam):
            term = CoursePriorTerm(target_mu=mu, precision=lam)
            return np.linalg.solve(
                A.T @ A + lam * np.eye(n),
                A.T @ b + lam * mu,
            ), term

        w_weak, _ = solve_for_lambda(0.01)
        w_strong, _ = solve_for_lambda(1e6)

        dist_weak = np.linalg.norm(w_weak - mu)
        dist_strong = np.linalg.norm(w_strong - mu)

        assert dist_strong < dist_weak
        assert np.allclose(w_strong, mu, atol=1e-3)


class TestBaseClassContract:
    def test_base_class_raises_not_implemented(self):
        term = BeamletObjectiveTerm()
        with pytest.raises(NotImplementedError):
            term.evaluate(np.zeros(3))

    def test_course_prior_is_subclass(self):
        term = CoursePriorTerm(target_mu=np.zeros(2), precision=1.0)
        assert isinstance(term, BeamletObjectiveTerm)
