"""
Tests for Prescription (port commit 2/3).

The Prescription is the composition / gradient-routing layer. The
gating test is again a finite-difference check on the full chain:
  optimizer params -> transform -> beamlets -> dose -> KL cost
and the symmetric gradient back through all three Jacobians.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.course_prior import CoursePriorTerm
from pybrimstone.kl_term import KLDivTerm
from pybrimstone.numerics import inv_transform, transform
from pybrimstone.objective_terms import BeamletObjectiveTerm, DoseObjectiveTerm
from pybrimstone.prescription import Prescription


# ---------------------------------------------------------------------------
# Construction & basic API
# ---------------------------------------------------------------------------

class TestPrescriptionConstruction:
    def test_accepts_matrix_dose_operator(self):
        D = np.eye(10, 4)
        p = Prescription(D)
        assert p._dose_matrix is not None

    def test_accepts_callable_pair(self):
        def fwd(w):
            return w * 2
        def adj(g):
            return g * 2
        p = Prescription((fwd, adj))
        assert p._dose_forward is not None
        assert p._dose_adjoint is not None

    def test_rejects_callable_without_adjoint(self):
        with pytest.raises(TypeError):
            Prescription(lambda w: w)

    def test_rejects_1d_matrix(self):
        with pytest.raises(ValueError, match="2-D"):
            Prescription(np.zeros(5))

    def test_add_dose_term_type_checked(self):
        p = Prescription(np.eye(5, 3))
        with pytest.raises(TypeError, match="DoseObjectiveTerm"):
            p.add_dose_term(CoursePriorTerm(np.zeros(3), precision=1.0))

    def test_add_beamlet_term_type_checked(self):
        p = Prescription(np.eye(5, 3))
        bogus = type("Bogus", (), {})()
        with pytest.raises(TypeError, match="BeamletObjectiveTerm"):
            p.add_beamlet_term(bogus)


# ---------------------------------------------------------------------------
# Dose forward / adjoint
# ---------------------------------------------------------------------------

class TestPrescriptionDoseRouting:
    def test_matrix_forward(self):
        D = np.array([[1.0, 0.0], [0.5, 0.5], [0.0, 1.0]])
        p = Prescription(D)
        w = np.array([2.0, 4.0])
        dose = p.compute_dose(w)
        assert np.allclose(dose, [2.0, 3.0, 4.0])

    def test_matrix_adjoint_matches_transpose(self):
        rng = np.random.default_rng(0)
        D = rng.normal(size=(8, 4))
        p = Prescription(D)
        g = rng.normal(size=8)
        adj = p.dose_adjoint(g)
        assert np.allclose(adj, D.T @ g)

    def test_callable_pair_used(self):
        def fwd(w):
            return w * 3.0
        def adj(g):
            return g * 3.0
        p = Prescription((fwd, adj))
        assert np.allclose(p.compute_dose(np.array([1.0, 2.0])), [3.0, 6.0])
        assert np.allclose(p.dose_adjoint(np.array([1.0, 2.0])), [3.0, 6.0])


# ---------------------------------------------------------------------------
# End-to-end gradient validation
# ---------------------------------------------------------------------------

class TestPrescriptionGradient:
    """Finite-difference check on grad_params from Prescription.evaluate."""

    @staticmethod
    def _fd_grad(p, params, eps=1e-5):
        grad = np.zeros_like(params)
        for i in range(len(params)):
            up = params.copy()
            up[i] += eps
            down = params.copy()
            down[i] -= eps
            grad[i] = (p.evaluate_cost_only(up) - p.evaluate_cost_only(down)) / (2 * eps)
        return grad

    def _build(self, use_transform=True, with_course_prior=False, seed=0):
        rng = np.random.default_rng(seed)
        n_voxels, n_beamlets = 20, 5
        D = np.abs(rng.normal(size=(n_voxels, n_beamlets))) + 0.1
        mask = np.zeros(n_voxels)
        mask[:10] = 1.0
        kl = KLDivTerm.from_interval(
            structure_mask=mask, dose_min=0.4, dose_max=0.8,
            bin_width=0.1, var_min=0.04, var_max=0.04, weight=1.0,
        )
        p = Prescription(D, use_transform=use_transform)
        p.add_dose_term(kl)
        if with_course_prior:
            mu = np.zeros(n_beamlets)
            prior = CoursePriorTerm(target_mu=mu, precision=0.5)
            p.add_beamlet_term(prior)
        return p, n_beamlets

    def test_grad_matches_fd_dose_term_only(self):
        p, n = self._build(use_transform=True, with_course_prior=False, seed=1)
        rng = np.random.default_rng(10)
        params = rng.normal(size=n) * 0.5
        _, ana = p.evaluate(params)
        fd = self._fd_grad(p, params, eps=1e-4)
        assert np.allclose(ana, fd, atol=1e-3, rtol=1e-2), \
            f"max abs diff: {np.abs(ana-fd).max():.6f}\nana: {ana}\nfd:  {fd}"

    def test_grad_matches_fd_with_course_prior(self):
        """Verifies the beamlet-term gradient is added correctly."""
        p, n = self._build(use_transform=True, with_course_prior=True, seed=2)
        rng = np.random.default_rng(20)
        params = rng.normal(size=n) * 0.5
        _, ana = p.evaluate(params)
        fd = self._fd_grad(p, params, eps=1e-4)
        assert np.allclose(ana, fd, atol=1e-3, rtol=1e-2), \
            f"max abs diff: {np.abs(ana-fd).max():.6f}\nana: {ana}\nfd:  {fd}"

    def test_grad_matches_fd_no_transform(self):
        """Without the sigmoid transform, grad_params == grad_beamlet."""
        p, n = self._build(use_transform=False, with_course_prior=True, seed=3)
        rng = np.random.default_rng(30)
        # Positive beamlet weights since no transform clips to (0, INPUT_SCALE).
        params = rng.uniform(0.1, 0.4, size=n)
        _, ana = p.evaluate(params)
        fd = self._fd_grad(p, params, eps=1e-4)
        assert np.allclose(ana, fd, atol=1e-3, rtol=1e-2)


# ---------------------------------------------------------------------------
# Composition behavior
# ---------------------------------------------------------------------------

class TestPrescriptionComposition:
    def test_term_weights_scale_cost(self):
        D = np.eye(10, 4)
        mask = np.ones(10)
        kl_a = KLDivTerm.from_interval(mask, 0.4, 0.8, weight=1.0)
        kl_b = KLDivTerm.from_interval(mask, 0.4, 0.8, weight=2.0)

        # Same prescription twice: cost with weight=2 should be 2x weight=1.
        p_a = Prescription(D, use_transform=False)
        p_a.add_dose_term(kl_a)
        p_b = Prescription(D, use_transform=False)
        p_b.add_dose_term(kl_b)

        w = np.full(4, 0.3)
        assert np.isclose(p_b.evaluate_cost_only(w), 2.0 * p_a.evaluate_cost_only(w))

    def test_empty_prescription_zero_cost(self):
        p = Prescription(np.eye(5, 3))
        cost, grad = p.evaluate(np.zeros(3))
        assert cost == 0.0
        assert np.allclose(grad, 0.0)
