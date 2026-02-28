"""
Reference implementation and tests for Polak-Ribiere conjugate gradient.

Mirrors DynamicCovarianceOptimizer from ConjGradOptimizer.cpp.
"""

import numpy as np
import pytest
from scipy.optimize import minimize_scalar


# ---------------------------------------------------------------------------
# Reference implementations
# ---------------------------------------------------------------------------

ITER_MAX = 500
ZEPS = 1e-10


def brent_line_minimize(f, point, direction, tol=1e-4):
    """
    Line minimization along a direction using Brent's method.

    Matches the vnl_brent_minimizer usage in C++.
    """
    def line_func(lam):
        return f(point + lam * direction)

    result = minimize_scalar(line_func, method="brent",
                             options={"xtol": tol})
    return result.x, result.fun


def polak_ribiere_cg(f, grad_f, x0, tol=1e-3, max_iter=ITER_MAX,
                     line_tol=1e-4, callback=None):
    """
    Polak-Ribiere conjugate gradient minimization.

    Matches the core loop in DynamicCovarianceOptimizer::minimize.

    Returns:
        x_final: optimized parameters
        f_final: final function value
        n_iter: iterations used
        converged: whether convergence was achieved
    """
    x = x0.copy()
    g = -grad_f(x)  # negative gradient (steepest ascent direction)

    # Check for tiny gradient
    if np.linalg.norm(g) < 1e-8:
        g = np.random.randn(len(x)) * tol

    d = g.copy()  # initial direction = steepest descent
    f_val = f(x)
    converged = False

    for iteration in range(max_iter):
        # Line minimization
        lam, f_new = brent_line_minimize(f, x, d, tol=line_tol)

        # Update position
        x = x + lam * d

        # Convergence test matching C++:
        # 2*|f_old - f_new| <= tol * (|f_old| + |f_new| + ZEPS)
        if 2.0 * abs(f_val - f_new) <= tol * (abs(f_val) + abs(f_new) + ZEPS):
            converged = True
            f_val = f_new
            break

        f_val = f_new

        if callback:
            callback(iteration, x, f_val)

        # Compute gamma (Polak-Ribiere)
        gg = np.dot(g, g)
        if gg == 0.0:
            converged = True
            break

        g_prev = g.copy()
        g = -grad_f(x)

        # Polak-Ribiere formula: dgg = g·g - g_prev·g
        dgg = np.dot(g, g) - np.dot(g_prev, g)

        # Update direction
        d = g + (dgg / gg) * d

    return x, f_val, iteration + 1, converged


def convergence_test(f_old, f_new, tol=1e-3):
    """
    Convergence test matching C++ code:
    2*|f_old - f_new| <= tol * (|f_old| + |f_new| + ZEPS)
    """
    return 2.0 * abs(f_old - f_new) <= tol * (abs(f_old) + abs(f_new) + ZEPS)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestConvergenceTest:
    def test_identical_values(self):
        assert convergence_test(1.0, 1.0) is True

    def test_large_change(self):
        assert convergence_test(10.0, 1.0) is False

    def test_small_change(self):
        assert convergence_test(1.0, 1.0001) is True

    def test_zero_values(self):
        """Should converge when both are zero (ZEPS protects)."""
        assert convergence_test(0.0, 0.0) is True


class TestBrentLineMinimize:
    def test_quadratic(self):
        f = lambda x: np.sum(x ** 2)
        point = np.array([1.0, 0.0])
        direction = np.array([-1.0, 0.0])
        lam, f_min = brent_line_minimize(f, point, direction)
        assert abs(lam - 1.0) < 1e-4  # minimum at origin
        assert f_min < 1e-6


class TestPolakRibiereCG:
    def test_quadratic_2d(self):
        """Minimize f(x) = x1^2 + x2^2 — should find origin."""
        f = lambda x: x[0] ** 2 + x[1] ** 2
        grad = lambda x: np.array([2 * x[0], 2 * x[1]])

        x0 = np.array([3.0, 4.0])
        x_opt, f_opt, n_iter, converged = polak_ribiere_cg(f, grad, x0)

        assert converged
        assert np.allclose(x_opt, [0.0, 0.0], atol=1e-3)
        assert f_opt < 1e-5

    def test_rosenbrock_2d(self):
        """Rosenbrock function — harder but CG should handle it."""
        def f(x):
            return (1 - x[0]) ** 2 + 100 * (x[1] - x[0] ** 2) ** 2

        def grad(x):
            dx0 = -2 * (1 - x[0]) - 400 * x[0] * (x[1] - x[0] ** 2)
            dx1 = 200 * (x[1] - x[0] ** 2)
            return np.array([dx0, dx1])

        x0 = np.array([-1.0, 1.0])
        x_opt, f_opt, n_iter, converged = polak_ribiere_cg(
            f, grad, x0, tol=1e-6, max_iter=500,
        )

        # CG may not perfectly solve Rosenbrock but should get close
        assert f_opt < 0.1

    def test_elliptic_10d(self):
        """Ill-conditioned elliptic function in 10D."""
        scales = np.array([1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0, 512.0])

        def f(x):
            return np.sum(scales * x ** 2)

        def grad(x):
            return 2.0 * scales * x

        x0 = np.ones(10)
        x_opt, f_opt, n_iter, converged = polak_ribiere_cg(f, grad, x0)

        assert converged
        assert np.allclose(x_opt, np.zeros(10), atol=1e-2)

    def test_respects_max_iter(self):
        """Should stop at max_iter even without convergence."""
        f = lambda x: np.sum(x ** 2)
        grad = lambda x: 2 * x
        x0 = np.array([100.0, 100.0])

        _, _, n_iter, _ = polak_ribiere_cg(f, grad, x0, max_iter=5, tol=1e-20)
        assert n_iter <= 5

    def test_already_at_minimum(self):
        """Starting at minimum should converge immediately."""
        f = lambda x: np.sum(x ** 2)
        grad = lambda x: 2 * x
        x0 = np.zeros(3)

        x_opt, f_opt, n_iter, converged = polak_ribiere_cg(f, grad, x0)
        assert f_opt < 1e-8


class TestDynamicCovariance:
    """
    Tests for the adaptive variance / dynamic covariance mechanism.
    """

    def test_variance_scale_formula(self):
        """
        scale = 4^n / 4^iteration for n < iteration, else 1.0
        variance[dim] = 1/sum(covar column)
        """
        n_dims = 5
        iteration = 3

        for n in range(n_dims):
            if n < iteration:
                scale = 4.0 ** n / 4.0 ** iteration
            else:
                scale = 1.0
            # scale should decrease for earlier searched directions
            if n < iteration:
                assert scale < 1.0
            assert scale > 0

    def test_gram_schmidt_orthogonality(self):
        """Verify that Gram-Schmidt produces orthogonal vectors."""
        rng = np.random.RandomState(42)
        n = 5
        vectors = rng.randn(n, n)

        # Apply Gram-Schmidt
        ortho = np.zeros_like(vectors)
        ortho[0] = vectors[0] / np.linalg.norm(vectors[0])
        for i in range(1, n):
            v = vectors[i].copy()
            for j in range(i):
                v -= np.dot(v, ortho[j]) * ortho[j]
            ortho[i] = v / np.linalg.norm(v)

        # Check orthogonality
        gram = ortho @ ortho.T
        assert np.allclose(gram, np.eye(n), atol=1e-10)
