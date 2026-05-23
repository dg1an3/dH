"""
Phase 0 smoke test for the amortized course-prior scaffold.

What this checks (and doesn't):
  - All amortized modules import.
  - Shapes flow through: PhaseState / CourseState / Sample build, the
    network's forward pass returns the right shape, infer.predict_delta
    round-trips numpy → torch → numpy.
  - train()'s shape-validation rejects mismatched samples loudly.

It deliberately does NOT check that anything is *learned*. The network
is zero-initialized in its last layer (identity prior at construction),
so it returns exactly zeros — that's the Phase 0 contract. Once
Phase 1a (toy data + real training) lands, a separate test will
exercise actual learning.
"""

import sys
from pathlib import Path

import numpy as np
import pytest
import torch

sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from pybrimstone.amortized import (
    AmortizedCoursePrior,
    CourseState,
    PhaseState,
    PrototypeConfig,
    Sample,
)
from pybrimstone.amortized.data import generate_dummy_samples
from pybrimstone.amortized.infer import predict_delta
from pybrimstone.amortized.train import train


PHASE_DIM = 64
COURSE_DIM = 2 * PHASE_DIM


def test_types_construct():
    """Dataclasses build with the shapes documented in types.py."""
    x = np.zeros(PHASE_DIM)
    mu = np.zeros(PHASE_DIM)
    sigma_inv = np.ones(PHASE_DIM)

    ps = PhaseState(x=x, phase_id=3)
    cs = CourseState(mu=mu, sigma_inv=sigma_inv)
    sample = Sample(x_init=x, course_state=cs, delta_x=np.zeros(PHASE_DIM))

    assert ps.phase_id == 3
    assert cs.dim == PHASE_DIM
    assert cs.flatten().shape == (COURSE_DIM,)
    assert sample.x_init.shape == (PHASE_DIM,)


def test_network_forward_pass_shapes():
    net = AmortizedCoursePrior(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    x = torch.randn(8, PHASE_DIM)
    c = torch.randn(8, COURSE_DIM)
    delta = net(x, c)
    assert delta.shape == (8, PHASE_DIM)


def test_network_is_identity_prior_at_init():
    """Final layer is zero-init → output is zeros for any input."""
    net = AmortizedCoursePrior(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    x = torch.randn(4, PHASE_DIM)
    c = torch.randn(4, COURSE_DIM)
    with torch.no_grad():
        delta = net(x, c)
    assert torch.allclose(delta, torch.zeros_like(delta))


def test_network_rejects_wrong_input_dims():
    net = AmortizedCoursePrior(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    bad_x = torch.randn(2, PHASE_DIM + 1)
    good_c = torch.randn(2, COURSE_DIM)
    with pytest.raises(ValueError, match="phase_dim"):
        net(bad_x, good_c)

    good_x = torch.randn(2, PHASE_DIM)
    bad_c = torch.randn(2, COURSE_DIM - 7)
    with pytest.raises(ValueError, match="course_dim"):
        net(good_x, bad_c)


def test_dummy_data_generation():
    rng = np.random.default_rng(42)
    samples = generate_dummy_samples(n_samples=5, phase_dim=PHASE_DIM, rng=rng)
    assert len(samples) == 5
    for s in samples:
        assert s.x_init.shape == (PHASE_DIM,)
        assert s.course_state.mu.shape == (PHASE_DIM,)
        assert s.course_state.sigma_inv.shape == (PHASE_DIM,)
        assert s.delta_x.shape == (PHASE_DIM,)
        # Phase 0 contract: dummy targets are zero.
        assert np.all(s.delta_x == 0.0)


def test_train_stub_returns_zero_init_network():
    cfg = PrototypeConfig(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    rng = np.random.default_rng(0)
    samples = generate_dummy_samples(n_samples=4, phase_dim=PHASE_DIM, rng=rng)
    net = train(samples, cfg)
    assert isinstance(net, AmortizedCoursePrior)
    # Phase 0 contract: train() does no actual training yet.
    delta = predict_delta(net, samples[0].x_init, samples[0].course_state)
    assert delta.shape == (PHASE_DIM,)
    assert np.allclose(delta, 0.0)


def test_train_rejects_shape_mismatch():
    cfg = PrototypeConfig(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    bad_sample = Sample(
        x_init=np.zeros(PHASE_DIM + 1),                     # wrong size
        course_state=CourseState(
            mu=np.zeros(PHASE_DIM), sigma_inv=np.ones(PHASE_DIM)
        ),
        delta_x=np.zeros(PHASE_DIM),
    )
    with pytest.raises(ValueError, match="x_init shape"):
        train([bad_sample], cfg)


def test_infer_predict_delta_numpy_roundtrip():
    net = AmortizedCoursePrior(phase_dim=PHASE_DIM, course_dim=COURSE_DIM)
    rng = np.random.default_rng(7)
    x_init = rng.standard_normal(PHASE_DIM)
    cs = CourseState(
        mu=rng.standard_normal(PHASE_DIM),
        sigma_inv=rng.uniform(0.1, 2.0, size=PHASE_DIM),
    )
    delta = predict_delta(net, x_init, cs)
    assert isinstance(delta, np.ndarray)
    assert delta.shape == (PHASE_DIM,)
    assert delta.dtype == np.float64
