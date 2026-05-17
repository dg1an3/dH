"""
Polak-Ribiere conjugate gradient with dynamic-covariance adaptive variance.

Port of RtModel/ConjGradOptimizer.cpp. The reference test impl in
python/tests/test_conjugate_gradient.py was the seed; this is the
library version that downstream phase_optimizer code consumes.

Two intertwined algorithms:
  - polak_ribiere_cg: the outer CG loop with Brent line minimization
  - update_dynamic_covariance: per-iteration adaptive-variance update
    that mirrors DynamicCovarianceOptimizer::UpdateDynamicCovariance
    (asymmetric backward/forward GSO loops included).

The callback signature includes sigma_weights = the current adaptive
variance vector, which is what the hierarchical-Bayes outer loop
harvests for pooling.
"""

from __future__ import annotations

from typing import Callable, Optional, Tuple

import numpy as np
from scipy.optimize import minimize_scalar


ITER_MAX = 500
ZEPS = 1e-10


def brent_line_minimize(
    f: Callable[[np.ndarray], float],
    point: np.ndarray,
    direction: np.ndarray,
    tol: float = 1e-4,
) -> Tuple[float, float]:
    """Line minimization along a direction. Matches vnl_brent_minimizer usage."""
    def line_func(lam):
        return f(point + lam * direction)
    result = minimize_scalar(line_func, method="brent", options={"xtol": tol})
    return result.x, result.fun


def update_dynamic_covariance(
    ortho_basis: np.ndarray,
    searched_dir: np.ndarray,
    direction: np.ndarray,
    iteration: int,
    var_min: float,
    var_max: float,
) -> np.ndarray:
    """
    Per-iteration adaptive-variance update. Mutates ortho_basis and
    searched_dir in place; returns the per-parameter variance vector.
    Mirrors ConjGradOptimizer.cpp:281-349 exactly, including the
    asymmetric backward/forward GSO loops.
    """
    n_dim = ortho_basis.shape[0]

    dir_norm = direction / np.linalg.norm(direction)
    searched_dir[:, iteration] = dir_norm
    ortho_basis[:, iteration] = dir_norm

    # Backward GSO: re-orthogonalize prior searched directions against
    # later ortho columns (excluding the just-added one at `iteration`).
    for j in range(iteration - 1, -1, -1):
        v = searched_dir[:, j].copy()
        for jo in range(j + 1, iteration):
            v -= np.dot(v, ortho_basis[:, jo]) * ortho_basis[:, jo]
        norm = np.linalg.norm(v)
        if norm > 1e-12:
            ortho_basis[:, j] = v / norm

    # Forward GSO: orthogonalize future basis columns against everything
    # seen so far, including the new direction.
    for j in range(iteration + 1, n_dim):
        v = searched_dir[:, j].copy()
        for jo in range(j - 1, -1, -1):
            v -= np.dot(v, ortho_basis[:, jo]) * ortho_basis[:, jo]
        norm = np.linalg.norm(v)
        if norm > 1e-12:
            ortho_basis[:, j] = v / norm

    # Diagonal scaling: scale_n = 4^n / 4^iter for n < iter, else 1.
    # Diagonal entry = 1 / (scale*(var_max-var_min) + var_min).
    scaling = np.zeros((n_dim, n_dim))
    for n in range(n_dim):
        scale = (4.0 ** n / 4.0 ** iteration) if n < iteration else 1.0
        scaling[n, n] = 1.0 / (scale * (var_max - var_min) + var_min)

    covar = ortho_basis.T @ scaling @ ortho_basis
    sigma_weights = 1.0 / covar.sum(axis=0)
    return sigma_weights


def polak_ribiere_cg(
    f: Callable[[np.ndarray], float],
    grad_f: Callable[[np.ndarray], np.ndarray],
    x0: np.ndarray,
    tol: float = 1e-3,
    max_iter: int = ITER_MAX,
    line_tol: float = 1e-4,
    callback: Optional[Callable] = None,
    adaptive_variance: Optional[Tuple[float, float]] = None,
) -> Tuple[np.ndarray, float, int, bool]:
    """
    Polak-Ribiere conjugate gradient minimization.

    Mirrors DynamicCovarianceOptimizer::minimize. Convergence test
    matches the C++ formula: 2*|f_old - f_new| <= tol * (|f_old| + |f_new| + ZEPS).

    Args:
        adaptive_variance: If a (var_min, var_max) tuple, the dynamic-
            covariance update runs each iteration and the resulting
            per-parameter variance is passed to the callback as
            sigma_weights. This is what the hierarchical-Bayes outer
            loop harvests for pooling.
        callback: optional callable invoked at each iteration with
            kwargs (iteration, x, f_val, sigma_weights). Return value
            is currently ignored (TODO: support early-stop return).

    Returns:
        (x_final, f_final, n_iter, converged)
    """
    x = x0.copy()
    g = -grad_f(x)
    if np.linalg.norm(g) < 1e-8:
        g = np.random.randn(len(x)) * tol

    d = g.copy()
    f_val = f(x)
    converged = False
    n_dim = len(x)

    if adaptive_variance is not None:
        var_min, var_max = adaptive_variance
        ortho_basis = np.eye(n_dim)
        searched_dir = np.eye(n_dim)
        sigma_weights = np.full(n_dim, var_max)
    else:
        ortho_basis = None
        searched_dir = None
        sigma_weights = None

    iteration = 0
    for iteration in range(max_iter):
        lam, f_new = brent_line_minimize(f, x, d, tol=line_tol)
        x = x + lam * d

        if 2.0 * abs(f_val - f_new) <= tol * (abs(f_val) + abs(f_new) + ZEPS):
            converged = True
            f_val = f_new
            break
        f_val = f_new

        if adaptive_variance is not None and iteration < n_dim:
            sigma_weights = update_dynamic_covariance(
                ortho_basis, searched_dir, d, iteration, var_min, var_max,
            )

        if callback:
            callback(iteration=iteration, x=x, f_val=f_val, sigma_weights=sigma_weights)

        gg = np.dot(g, g)
        if gg == 0.0:
            converged = True
            break

        g_prev = g.copy()
        g = -grad_f(x)
        dgg = np.dot(g, g) - np.dot(g_prev, g)
        d = g + (dgg / gg) * d

    return x, f_val, iteration + 1, converged
