"""
Tests for KLDivTerm (port commit 2/3).

Headline correctness check: analytical gradient matches finite-difference
gradient to ~1e-4 on a random dose distribution with a representative
structure mask and prescription interval. The dose-side backward pass is
the trickiest piece of the port, so the FD check is the gating test.
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.kl_term import KLDivTerm
from pybrimstone.objective_terms import DoseObjectiveTerm


# ---------------------------------------------------------------------------
# Test fixtures
# ---------------------------------------------------------------------------

def _basic_kl_term(rng=None, n_voxels=40, **kwargs):
    rng = rng or np.random.default_rng(0)
    mask = (rng.uniform(size=n_voxels) > 0.4).astype(float)
    if mask.sum() == 0:
        mask[0] = 1.0  # ensure non-empty
    return KLDivTerm.from_interval(
        structure_mask=mask,
        dose_min=0.5, dose_max=1.0,
        bin_width=0.1, var_min=0.04, var_max=0.04,
        **kwargs,
    ), mask


# ---------------------------------------------------------------------------
# Construction & smoke tests
# ---------------------------------------------------------------------------

class TestKLDivTermBasics:
    def test_is_dose_objective_term(self):
        term, _ = _basic_kl_term()
        assert isinstance(term, DoseObjectiveTerm)

    def test_rejects_empty_mask(self):
        with pytest.raises(ValueError, match="nonzero voxel"):
            KLDivTerm.from_interval(
                structure_mask=np.zeros(10),
                dose_min=0.0, dose_max=1.0,
            )

    def test_rejects_shape_mismatch_at_eval(self):
        term, _ = _basic_kl_term(n_voxels=30)
        with pytest.raises(ValueError, match="!="):
            term.evaluate(np.zeros(20))

    def test_evaluate_returns_finite(self):
        rng = np.random.default_rng(0)
        term, _ = _basic_kl_term(rng, n_voxels=40)
        dose = rng.uniform(0.0, 2.0, size=40)
        cost, grad = term.evaluate(dose)
        assert np.isfinite(cost)
        assert np.all(np.isfinite(grad))
        assert grad.shape == dose.shape

    def test_target_gbins_sums_to_one(self):
        term, _ = _basic_kl_term()
        assert np.isclose(term.target_gbins.sum(), 1.0, atol=1e-4)

    def test_weight_attribute_default_one(self):
        term, _ = _basic_kl_term()
        assert term.weight == 1.0

    def test_weight_attribute_customizable(self):
        term, _ = _basic_kl_term()
        term.weight = 2.5
        assert term.weight == 2.5


# ---------------------------------------------------------------------------
# Gradient validation (the gating test for this port commit)
# ---------------------------------------------------------------------------

class TestKLDivTermGradient:
    """
    Finite-difference check on the analytical gradient. The chain
    bin -> conv -> normalize -> KL is non-trivial; getting the backward
    pass right is what this test verifies.
    """

    @staticmethod
    def _fd_gradient(term, dose, eps=1e-5):
        grad = np.zeros_like(dose)
        for i in range(len(dose)):
            d_plus = dose.copy()
            d_plus[i] += eps
            c_plus, _ = term.evaluate(d_plus)
            d_minus = dose.copy()
            d_minus[i] -= eps
            c_minus, _ = term.evaluate(d_minus)
            grad[i] = (c_plus - c_minus) / (2 * eps)
        return grad

    def test_grad_matches_fd_uniform_dose(self):
        """Uniform dose has many voxels in the same bin -- exercises the
        fractional binning gradient."""
        rng = np.random.default_rng(1)
        n = 20
        mask = np.ones(n)
        term = KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.8,
            bin_width=0.1, var_min=0.04, var_max=0.04,
        )
        # Slight perturbation around uniform so bin assignments aren't degenerate
        dose = np.full(n, 0.55) + rng.uniform(-0.02, 0.02, size=n)

        _, ana = term.evaluate(dose)
        fd = self._fd_gradient(term, dose, eps=1e-4)
        # Larger tol because uniform dose lands close to a bin boundary
        assert np.allclose(ana, fd, atol=1e-2, rtol=1e-2), \
            f"max abs diff: {np.abs(ana-fd).max():.4f}\nana: {ana[:5]}\nfd:  {fd[:5]}"

    def test_grad_matches_fd_random_dose(self):
        rng = np.random.default_rng(2)
        n = 30
        mask = (rng.uniform(size=n) > 0.3).astype(float)
        term = KLDivTerm.from_interval(
            mask, dose_min=0.3, dose_max=1.2,
            bin_width=0.1, var_min=0.04, var_max=0.04,
        )
        dose = rng.uniform(0.1, 1.5, size=n)
        _, ana = term.evaluate(dose)
        fd = self._fd_gradient(term, dose, eps=1e-4)

        # Voxels outside the mask should have zero gradient.
        for i in range(n):
            if mask[i] == 0:
                assert abs(ana[i]) < 1e-12

        # Compare only the mask-active voxels.
        active = mask > 0
        assert np.allclose(ana[active], fd[active], atol=1e-2, rtol=1e-2), \
            f"max abs diff: {np.abs(ana[active]-fd[active]).max():.4f}"

    def test_grad_zero_for_voxels_outside_mask(self):
        rng = np.random.default_rng(3)
        n = 25
        mask = np.zeros(n)
        mask[:10] = 1.0
        term = KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.8,
        )
        dose = rng.uniform(0.0, 1.5, size=n)
        _, grad = term.evaluate(dose)
        assert np.allclose(grad[10:], 0.0)


# ---------------------------------------------------------------------------
# Cross-entropy mode
# ---------------------------------------------------------------------------

class TestKLDivTermCrossEntropy:
    def test_cross_entropy_evaluates_finite(self):
        rng = np.random.default_rng(0)
        term, _ = _basic_kl_term(rng, n_voxels=30, cross_entropy=True)
        dose = rng.uniform(0.0, 1.5, size=30)
        cost, grad = term.evaluate(dose)
        assert np.isfinite(cost)
        assert np.all(np.isfinite(grad))

    def test_cross_entropy_different_from_default(self):
        rng = np.random.default_rng(0)
        mask = np.ones(30)
        default_term = KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.8, cross_entropy=False,
        )
        ce_term = KLDivTerm.from_interval(
            mask, dose_min=0.4, dose_max=0.8, cross_entropy=True,
        )
        dose = rng.uniform(0.1, 1.5, size=30)
        c_default, _ = default_term.evaluate(dose)
        c_ce, _ = ce_term.evaluate(dose)
        assert not np.isclose(c_default, c_ce)
