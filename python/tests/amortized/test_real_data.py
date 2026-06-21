"""
Phase 1b tests for the real-data generator (synthetic plans + real CG).

These tests exercise the full pipeline that Phase 2 will train on:
build_random_plan_template -> generate_real_samples -> Sample with
trajectory in beamlet-weight space. They deliberately use tiny grids
and small max_iter so each test completes in well under a second.

What they verify:
  - Output Sample shapes and beamlet-weight bounds (post-sigmoid).
  - Trajectory consistency: trajectory[0] == x_init, trajectory[-1] is
    close to x_init + delta_x.
  - Reproducibility: same RNG state -> same samples.
  - expand_trajectory_to_samples produces the expected sample count and
    keeps the right metadata.
  - The endpoint reduces the objective relative to the starting point
    (CG is a descent method; this is a meaningful sanity gate).

Disclaimer: still on synthetic data. These checks confirm the
plumbing is correct, not that the algorithm has been validated on a
real clinical workload.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from pybrimstone.amortized.data import (
    PlanTemplate,
    build_random_plan_template,
    expand_trajectory_to_samples,
    generate_real_samples,
)
from pybrimstone.amortized.types import Sample


# Small enough to make the suite zippy. Real corpus generation in the
# notebook uses bigger grids and more samples.
_GRID = (4, 4, 4)
_N_BEAMLETS = 6
_N_TARGET = 8


@pytest.fixture
def small_template():
    rng = np.random.default_rng(0)
    return build_random_plan_template(
        rng,
        grid_shape=_GRID,
        n_beamlets=_N_BEAMLETS,
        n_target_voxels=_N_TARGET,
    )


def test_template_has_expected_dims(small_template):
    assert small_template.n_params == _N_BEAMLETS
    p = small_template.build_prescription()
    # Two factory calls produce independent Prescriptions sharing the
    # same dose matrix.
    p2 = small_template.build_prescription()
    assert p is not p2
    assert p._dose_matrix.shape == (np.prod(_GRID), _N_BEAMLETS)


def test_generate_real_samples_basic_shapes(small_template):
    rng = np.random.default_rng(1)
    samples = generate_real_samples(
        [small_template],
        n_samples_per_template=3,
        rng=rng,
        max_iter=10,
        tol=1e-2,
    )
    assert len(samples) == 3
    for s in samples:
        assert isinstance(s, Sample)
        assert s.x_init.shape == (_N_BEAMLETS,)
        assert s.delta_x.shape == (_N_BEAMLETS,)
        assert s.course_state.mu.shape == (_N_BEAMLETS,)
        assert s.course_state.sigma_inv.shape == (_N_BEAMLETS,)
        # Beamlet weights live in (0, INPUT_SCALE=0.5).
        assert np.all(s.x_init > 0.0) and np.all(s.x_init < 0.5)
        assert np.all(s.course_state.mu > 0.0) and np.all(s.course_state.mu < 0.5)
        # x_init + delta_x is x_star, also in bounded beamlet space.
        x_star = s.x_init + s.delta_x
        assert np.all(x_star > -1e-9) and np.all(x_star < 0.5 + 1e-9)


def test_trajectory_endpoints_match_x_init_and_x_star(small_template):
    rng = np.random.default_rng(2)
    samples = generate_real_samples(
        [small_template],
        n_samples_per_template=2,
        rng=rng,
        max_iter=10,
        tol=1e-2,
        record_trajectory=True,
    )
    for s in samples:
        assert s.trajectory is not None
        assert len(s.trajectory) >= 2
        # First entry is the starting point.
        assert np.allclose(s.trajectory[0], s.x_init)
        # Last entry is x_star = x_init + delta_x.
        assert np.allclose(s.trajectory[-1], s.x_init + s.delta_x)


def test_trajectory_disabled_yields_none(small_template):
    rng = np.random.default_rng(3)
    samples = generate_real_samples(
        [small_template],
        n_samples_per_template=2,
        rng=rng,
        max_iter=8,
        tol=1e-2,
        record_trajectory=False,
    )
    for s in samples:
        assert s.trajectory is None
        # delta_x and shapes still populated.
        assert s.delta_x.shape == (_N_BEAMLETS,)


def test_generator_is_reproducible(small_template):
    samples_a = generate_real_samples(
        [small_template], n_samples_per_template=2,
        rng=np.random.default_rng(42), max_iter=8, tol=1e-2,
    )
    samples_b = generate_real_samples(
        [small_template], n_samples_per_template=2,
        rng=np.random.default_rng(42), max_iter=8, tol=1e-2,
    )
    for a, b in zip(samples_a, samples_b):
        assert np.allclose(a.x_init, b.x_init)
        assert np.allclose(a.delta_x, b.delta_x)
        assert np.allclose(a.course_state.mu, b.course_state.mu)
        assert np.allclose(a.course_state.sigma_inv, b.course_state.sigma_inv)


def test_cg_descends_objective_endpoint(small_template):
    """The CG endpoint achieves no-worse objective than the starting point.

    Uses the meta['f_final'] field and a re-evaluation of the prescription
    at x_init for the comparison. CG is a descent method, so this is a
    correctness check on the plumbing — if it fails, either the gradient
    is wrong or we're recording x_init from the wrong place.
    """
    rng = np.random.default_rng(7)
    samples = generate_real_samples(
        [small_template], n_samples_per_template=3,
        rng=rng, max_iter=20, tol=1e-3,
    )
    # Re-evaluate the prescription at x_init's optimizer-param preimage to
    # get the starting objective. Easiest is to invert transform.
    from pybrimstone.numerics.parameter_transform import inv_transform
    from pybrimstone.course_prior import CoursePriorTerm

    for s in samples:
        p = small_template.build_prescription()
        p.add_beamlet_term(
            CoursePriorTerm(s.course_state.mu.copy(), s.course_state.sigma_inv.copy())
        )
        params_init = inv_transform(s.x_init)
        f_init = p.evaluate_cost_only(params_init)
        assert s.meta["f_final"] <= f_init + 1e-6, (
            f"CG endpoint f={s.meta['f_final']} > start f={f_init}"
        )


def test_expand_trajectory_count_matches_skip_and_endpoint():
    """expand_trajectory_to_samples math:
    expanded per source sample = (traj_len - skip_first) when include_endpoint=True
                                = (traj_len - skip_first - 1) when include_endpoint=False
    """
    rng = np.random.default_rng(11)
    template = build_random_plan_template(
        rng, grid_shape=_GRID, n_beamlets=_N_BEAMLETS, n_target_voxels=_N_TARGET,
    )
    samples = generate_real_samples(
        [template], n_samples_per_template=2,
        rng=rng, max_iter=10, tol=1e-2,
    )
    traj_lens = [len(s.trajectory) for s in samples]

    expanded = expand_trajectory_to_samples(samples, skip_first=1, include_endpoint=True)
    expected = sum(tl - 1 for tl in traj_lens)
    assert len(expanded) == expected

    expanded_no_end = expand_trajectory_to_samples(
        samples, skip_first=1, include_endpoint=False
    )
    expected_no_end = sum(max(0, tl - 2) for tl in traj_lens)
    assert len(expanded_no_end) == expected_no_end


def test_expand_trajectory_pass_through_for_non_trajectory_samples():
    rng = np.random.default_rng(13)
    template = build_random_plan_template(
        rng, grid_shape=_GRID, n_beamlets=_N_BEAMLETS, n_target_voxels=_N_TARGET,
    )
    samples = generate_real_samples(
        [template], n_samples_per_template=2,
        rng=rng, max_iter=8, tol=1e-2, record_trajectory=False,
    )
    expanded = expand_trajectory_to_samples(samples)
    assert len(expanded) == len(samples)
    for orig, out in zip(samples, expanded):
        assert out is orig  # pass-through is by reference


def test_expanded_samples_carry_step_metadata():
    rng = np.random.default_rng(17)
    template = build_random_plan_template(
        rng, grid_shape=_GRID, n_beamlets=_N_BEAMLETS, n_target_voxels=_N_TARGET,
    )
    samples = generate_real_samples(
        [template], n_samples_per_template=1, rng=rng, max_iter=10, tol=1e-2,
    )
    expanded = expand_trajectory_to_samples(samples, skip_first=1, include_endpoint=True)
    for s in expanded:
        assert "traj_step" in s.meta
        assert "traj_len" in s.meta
        assert s.meta["traj_step"] >= 1
        assert s.meta["traj_step"] < s.meta["traj_len"]


def test_mixed_templates_rejected_with_different_n_params():
    rng = np.random.default_rng(19)
    a = build_random_plan_template(rng, grid_shape=_GRID, n_beamlets=4, n_target_voxels=4)
    b = build_random_plan_template(rng, grid_shape=_GRID, n_beamlets=8, n_target_voxels=4)
    with pytest.raises(ValueError, match="n_params"):
        generate_real_samples([a, b], n_samples_per_template=1, rng=rng, max_iter=2)
