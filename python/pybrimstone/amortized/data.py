"""
Data generation for the amortized course-prior prototype.

Two generators:

  - generate_dummy_samples (Phase 0): deterministic-shape dummy tuples,
    zero targets. Used by test_smoke to verify plumbing.

  - generate_toy_samples (Phase 1a): closed-form Gaussian toy described
    in §5a of the prototype guide. Per-sample random PSD A and Gaussian
    b stand in for the data-fit objective; (mu, lam) parameterize a
    diagonal Gaussian course prior. The target delta_x is computed from
    the closed-form linear solve x_star = (A + diag(lam))^{-1}(b + lam*mu).

    The toy lives in an unbounded vector space. The post-sigmoid
    beamlet-weight prediction choice (per Phase 0 decision) applies to
    the Phase 1b real-data generator, not this toy: there is no sigmoid
    in the toy's physics. Phase 1a verifies the harness; Phase 1b
    addresses the boundedness question.

Phase 1b will add the real generator that calls
pybrimstone.PhaseOptimizer.__call__ and harvests (x_init, prior, x_star)
tuples from converged CG solves. That step requires Derek's sign-off
on the prediction-space mapping and likely a small extension to the
existing Prescription / PhaseOptimizer interfaces.
"""

from __future__ import annotations

from typing import List

import numpy as np

from .types import CourseState, Sample


# ---------------------------------------------------------------------------
# Phase 0: dummy
# ---------------------------------------------------------------------------


def generate_dummy_samples(
    n_samples: int,
    phase_dim: int,
    rng: np.random.Generator,
) -> List[Sample]:
    """Phase 0 stub: random inputs, zero targets.

    Smoke-test data. Targets are zeros so a freshly-zero-initialized
    network already has near-zero training loss — useful to confirm
    plumbing without any actual learning happening yet.

    Args:
        n_samples: number of tuples to generate.
        phase_dim: dimensionality of x_init and course_state.mu.
        rng: numpy Generator for reproducibility.

    Returns:
        list of Sample. delta_x is zero. mu and sigma_inv have plausible
        but random values.
    """
    out: List[Sample] = []
    for _ in range(int(n_samples)):
        x_init = rng.standard_normal(phase_dim)
        mu = rng.standard_normal(phase_dim) * 0.5
        sigma_inv = rng.uniform(0.1, 2.0, size=phase_dim)
        delta_x = np.zeros(phase_dim)
        out.append(
            Sample(
                x_init=x_init,
                course_state=CourseState(mu=mu, sigma_inv=sigma_inv),
                delta_x=delta_x,
            )
        )
    return out


# ---------------------------------------------------------------------------
# Phase 1a: closed-form Gaussian toy
# ---------------------------------------------------------------------------


def _random_psd_matrix(
    n: int,
    rng: np.random.Generator,
    *,
    cond_floor: float = 0.5,
) -> np.ndarray:
    """Random symmetric positive-definite matrix.

    Built as M @ M.T + cond_floor * I where M ~ N(0, 1/sqrt(n) * I_n).
    The 1/sqrt(n) scaling keeps eigenvalues O(1) regardless of n; the
    cond_floor lower-bounds the condition number so the closed-form
    solve doesn't blow up on the rare poorly-conditioned draw.
    """
    M = rng.standard_normal(size=(n, n)) / np.sqrt(n)
    A = M @ M.T + cond_floor * np.eye(n)
    return A


def generate_toy_samples(
    n_samples: int,
    phase_dim: int,
    rng: np.random.Generator,
    *,
    init_scale: float = 1.0,
    mu_scale: float = 0.5,
    lam_range: tuple = (0.1, 2.0),
) -> List[Sample]:
    """Closed-form Gaussian toy (§5a of the prototype guide).

    Per sample:
        A    ~ random PSD (n x n)
        b    ~ N(0, I_n)
        mu   ~ N(0, mu_scale^2 * I_n)
        lam  ~ Uniform(lam_range, n)
        x_init ~ N(0, init_scale^2 * I_n)
        x_star = solve(A + diag(lam), b + lam * mu)
        delta_x = x_star - x_init

    The (A, b) pair is latent — not part of the input. The network's
    best-possible predictor is therefore E[x_star | mu, lam] - x_init,
    which has a bilinear lam*mu structure that a linear baseline
    cannot capture but an MLP can. This is the regression signal we
    expect the network to recover.

    Args:
        n_samples: number of samples to generate.
        phase_dim: dimensionality n.
        rng: numpy Generator.
        init_scale: std of x_init.
        mu_scale: std of the prior mean.
        lam_range: (low, high) for uniform sampling of the precision lam.

    Returns:
        list of Sample.
    """
    out: List[Sample] = []
    lam_lo, lam_hi = float(lam_range[0]), float(lam_range[1])
    if not (0 < lam_lo < lam_hi):
        raise ValueError(f"lam_range must satisfy 0 < lo < hi; got {lam_range}")

    for _ in range(int(n_samples)):
        A = _random_psd_matrix(phase_dim, rng)
        b = rng.standard_normal(phase_dim)
        mu = rng.standard_normal(phase_dim) * mu_scale
        lam = rng.uniform(lam_lo, lam_hi, size=phase_dim)
        x_init = rng.standard_normal(phase_dim) * init_scale

        # x_star = (A + diag(lam))^{-1} (b + lam * mu)
        rhs = b + lam * mu
        x_star = np.linalg.solve(A + np.diag(lam), rhs)

        delta_x = x_star - x_init
        out.append(
            Sample(
                x_init=x_init.astype(np.float64),
                course_state=CourseState(
                    mu=mu.astype(np.float64),
                    sigma_inv=lam.astype(np.float64),
                ),
                delta_x=delta_x.astype(np.float64),
            )
        )

    return out


def samples_to_arrays(samples: List[Sample]) -> dict:
    """Stack a list of Sample into batched arrays for training.

    Returns a dict with keys:
        x_init:    (N, phase_dim)
        course:    (N, 2 * phase_dim)  -- concatenation of (mu, sigma_inv)
        delta_x:   (N, phase_dim)
    """
    if not samples:
        raise ValueError("samples_to_arrays needs at least one Sample")
    x = np.stack([s.x_init for s in samples], axis=0).astype(np.float64)
    c = np.stack([s.course_state.flatten() for s in samples], axis=0).astype(np.float64)
    d = np.stack([s.delta_x for s in samples], axis=0).astype(np.float64)
    return {"x_init": x, "course": c, "delta_x": d}
