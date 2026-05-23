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


# ---------------------------------------------------------------------------
# Phase 1b: real-data generator (synthetic plans, real CG)
# ---------------------------------------------------------------------------
#
# The real generator builds small but realistic Prescriptions
# (gaussian-bump dose operator + KLDivTerm + CoursePriorTerm), runs the
# actual polak_ribiere_cg used by PhaseOptimizer, and harvests the full
# parameter trajectory through optimization. Everything stored in
# Sample lives in beamlet-weight space (post-sigmoid `transform`),
# per Derek's Phase 0 decision.
#
# This generator does NOT touch the C++ side. The brief asks for a
# proposal-first markdown if any Cython hook were needed; here the
# existing polak_ribiere_cg callback already surfaces `x` per iteration,
# so no new hook is required. Compare conjugate_gradient.py:188.


from dataclasses import dataclass
from typing import Callable, Optional, Tuple

from .types import PhaseState  # noqa: F401 -- re-export-friendly


@dataclass
class PlanTemplate:
    """One synthetic phase-optimization problem.

    Holds enough state to spin up a fresh Prescription (with KLDivTerms
    bound to a fixed structure mask + dose interval) on each sample.
    The course-prior term and x_init vary per sample but the data-side
    objective stays fixed.

    Attributes:
        name: human-readable identifier — flows into Sample.meta.
        n_params: optimizer-parameter dimensionality. Equals the number
            of beamlets, so this is also the beamlet-weight-space dim.
        build_prescription: factory returning a fresh Prescription with
            its data-side terms already attached but with no beamlet-side
            CoursePriorTerm installed. The generator installs one per
            sample.
    """

    name: str
    n_params: int
    build_prescription: Callable[[], "object"]  # returns a Prescription


def build_random_plan_template(
    rng: np.random.Generator,
    *,
    name: Optional[str] = None,
    grid_shape: Tuple[int, int, int] = (6, 6, 6),
    n_beamlets: int = 12,
    sigma: float = 1.2,
    target_dose_range: Tuple[float, float] = (0.3, 0.7),
    n_target_voxels: int = 30,
) -> PlanTemplate:
    """Build a small, synthetic-but-realistic plan template.

    The dose operator is a Gaussian-bump matrix with random beamlet
    centers inside the grid. A single KLDivTerm encodes a "target
    structure should receive dose in [target_dose_range]" constraint
    over a randomly chosen voxel subset. Small enough that CG converges
    in tens of iterations on a laptop; large enough to be non-trivial.
    """
    # Generated up front and frozen so build_prescription returns the same
    # objective each time (only the course prior changes between samples).
    from ..prescription import Prescription
    from ..kl_term import KLDivTerm
    from ..dose_calc import gaussian_bump_dose_operator

    nx, ny, nz = grid_shape
    n_voxels = nx * ny * nz
    centers = rng.uniform(low=0.5, high=np.array([nx, ny, nz]) - 0.5,
                          size=(n_beamlets, 3))
    D = gaussian_bump_dose_operator(grid_shape, centers, sigma=sigma)

    mask = np.zeros(n_voxels, dtype=np.float64)
    target_idx = rng.choice(n_voxels, size=int(n_target_voxels), replace=False)
    mask[target_idx] = 1.0

    dose_lo, dose_hi = float(target_dose_range[0]), float(target_dose_range[1])

    def build_prescription():
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(mask, dose_lo, dose_hi))
        return p

    return PlanTemplate(
        name=name or f"plan_{rng.integers(0, 1_000_000):06d}",
        n_params=n_beamlets,
        build_prescription=build_prescription,
    )


def _record_trajectory_callback() -> Tuple[Callable, list, list]:
    """Build a callback that captures (iteration, x_params) per step.

    Returns:
        (callback, x_history, sigma_history) where x_history accumulates
        copies of the parameter vector at each iteration (post-step),
        and sigma_history accumulates the adaptive-variance vectors.
    """
    x_history: list = []
    sigma_history: list = []

    def cb(**kw):
        x = kw.get("x")
        if x is not None:
            x_history.append(np.asarray(x, dtype=np.float64).copy())
        sw = kw.get("sigma_weights")
        if sw is not None:
            sigma_history.append(np.asarray(sw, dtype=np.float64).copy())

    return cb, x_history, sigma_history


def generate_real_samples(
    templates: List[PlanTemplate],
    n_samples_per_template: int,
    rng: np.random.Generator,
    *,
    max_iter: int = 100,
    tol: float = 1e-3,
    adaptive_variance: Tuple[float, float] = (0.01, 1.0),
    max_step_norm: Optional[float] = 20.0,
    record_trajectory: bool = True,
    mu_scale: float = 0.25,
    prec_range: Tuple[float, float] = (0.05, 2.0),
) -> List[Sample]:
    """Generate Sample tuples by running real CG on synthetic plans.

    Per sample:
      1. Draw a random initial point in optimizer-param space (~N(0, 1)).
      2. Draw a random course prior:
            mu       ~ N(0, mu_scale^2 * I), clipped to (0, INPUT_SCALE)
            sigma_inv ~ Uniform(prec_range, n_beamlets)
      3. Install the CoursePriorTerm into a fresh Prescription.
      4. Run polak_ribiere_cg with the trajectory-recording callback.
      5. Apply `transform` to convert all recorded params to beamlet-
         weight space; build Sample with that trajectory.

    The output Sample's x_init, delta_x, and trajectory are all in
    beamlet-weight space (post-sigmoid). The CoursePriorTerm in step 3
    is built directly in beamlet space — which is the space it lives in
    natively.

    Args:
        templates: list of PlanTemplate. All templates must share the
            same n_params (we mix them into one corpus with a shared
            phase_dim).
        n_samples_per_template: how many (x_init, course_state) draws
            per template.
        rng: numpy Generator.
        max_iter / tol / adaptive_variance / max_step_norm: forwarded
            to polak_ribiere_cg.
        record_trajectory: if False, only the endpoint is stored
            (trajectory=None). Default True.
        mu_scale: std of the prior mean draw, pre-clip to (0, INPUT_SCALE).
        prec_range: (low, high) for uniform sigma_inv draws.

    Returns:
        list of Sample with .trajectory populated when record_trajectory.
        Length = len(templates) * n_samples_per_template.
    """
    if not templates:
        raise ValueError("generate_real_samples needs at least one template")

    n_params_set = {t.n_params for t in templates}
    if len(n_params_set) > 1:
        raise ValueError(
            f"all templates must share n_params; got {sorted(n_params_set)}"
        )
    n_params = templates[0].n_params

    from ..course_prior import CoursePriorTerm
    from ..numerics.conjugate_gradient import polak_ribiere_cg
    from ..numerics.parameter_transform import transform, INPUT_SCALE

    out: List[Sample] = []
    eps = 1e-3 * INPUT_SCALE
    bound_lo, bound_hi = eps, INPUT_SCALE - eps

    for template in templates:
        for k in range(int(n_samples_per_template)):
            sample_seed = int(rng.integers(0, 2**31 - 1))

            x0_params = rng.standard_normal(n_params)
            # Draw mu in beamlet space, clipped to (0, INPUT_SCALE).
            mu_raw = rng.normal(loc=INPUT_SCALE / 2, scale=mu_scale, size=n_params)
            mu_w = np.clip(mu_raw, bound_lo, bound_hi)
            sigma_inv = rng.uniform(prec_range[0], prec_range[1], size=n_params)

            prior = CoursePriorTerm(target_mu=mu_w, precision=sigma_inv)
            prescription = template.build_prescription()
            prescription.add_beamlet_term(prior)

            def f(p):
                return prescription.evaluate_cost_only(p)

            def grad_f(p):
                _, g = prescription.evaluate(p)
                return g

            cb, x_history, _sigma_history = _record_trajectory_callback()

            x_final, f_final, n_iter, converged = polak_ribiere_cg(
                f, grad_f, x0_params.copy(),
                callback=cb if record_trajectory else None,
                adaptive_variance=adaptive_variance,
                max_iter=max_iter,
                tol=tol,
                max_step_norm=max_step_norm,
            )

            w_init = transform(x0_params)
            w_star = transform(x_final)
            delta_w = w_star - w_init

            traj_w: Optional[List[np.ndarray]] = None
            if record_trajectory:
                # x_history holds x_1..x_T; prepend x_0 then apply transform.
                params_traj = [x0_params.copy()] + x_history + [x_final.copy()]
                # The last x_history entry equals x_final on tol-convergence,
                # but the loop break can also fire on the zero-gradient path
                # before the callback runs — explicitly appending x_final
                # ensures the trajectory always ends at the returned solution.
                # Deduplicate any exact end-of-history duplicate.
                if (
                    len(params_traj) >= 2
                    and np.array_equal(params_traj[-1], params_traj[-2])
                ):
                    params_traj = params_traj[:-1]
                traj_w = [
                    np.asarray(transform(p), dtype=np.float64) for p in params_traj
                ]

            out.append(
                Sample(
                    x_init=w_init.astype(np.float64),
                    course_state=CourseState(
                        mu=mu_w.astype(np.float64),
                        sigma_inv=sigma_inv.astype(np.float64),
                    ),
                    delta_x=delta_w.astype(np.float64),
                    trajectory=traj_w,
                    meta={
                        "plan": template.name,
                        "sample_seed": sample_seed,
                        "n_iter": int(n_iter),
                        "converged": bool(converged),
                        "f_final": float(f_final),
                    },
                )
            )

    return out


def expand_trajectory_to_samples(
    samples: List[Sample],
    *,
    include_endpoint: bool = True,
    skip_first: int = 1,
) -> List[Sample]:
    """Expand each trajectory-bearing Sample into one Sample per step.

    For curriculum / imitation training on the full CG trajectory
    (§9 item 5 of the prototype brief, greenlit by Derek): every
    intermediate (x_init, course_state, x_t) becomes a training tuple
    with delta_x = x_t - x_init. The augmented corpus is typically
    10-50x larger than the endpoint-only corpus.

    Samples lacking a trajectory are passed through unchanged.

    Args:
        samples: list of Sample. Those with .trajectory=None pass through.
        include_endpoint: keep the converged endpoint (trajectory[-1]).
        skip_first: drop the first `skip_first` trajectory entries.
            Default 1 skips trajectory[0] = x_init (delta_x = 0 is a
            trivial target that the constant-zero predictor solves
            perfectly).

    Returns:
        new list of Sample. Originals are not mutated; trajectories
        on the returned samples are stripped (trajectory=None) to keep
        the augmented corpus small.
    """
    out: List[Sample] = []
    for s in samples:
        if s.trajectory is None:
            out.append(s)
            continue

        traj = s.trajectory
        end = len(traj) if include_endpoint else len(traj) - 1
        for t in range(max(0, int(skip_first)), end):
            x_t = traj[t]
            out.append(
                Sample(
                    x_init=s.x_init.copy(),
                    course_state=CourseState(
                        mu=s.course_state.mu.copy(),
                        sigma_inv=s.course_state.sigma_inv.copy(),
                    ),
                    delta_x=(x_t - s.x_init).astype(np.float64),
                    trajectory=None,
                    meta={**s.meta, "traj_step": t, "traj_len": len(traj)},
                )
            )
    return out
