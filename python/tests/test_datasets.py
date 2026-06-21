"""
Tests for the dataset loaders (pybrimstone.datasets).

Covers the format-agnostic assembly core (influence_from_coo, masks,
planning_case_from_submatrices), the multi-phase course synthesizer that
bridges per-case benchmarks to the Course prior, the CORT .mat adapter
(round-tripped through a savemat fixture), and an end-to-end integration
through HierarchicalBayes + dvh_uncertainty_bands.

Run with --noconftest (the repo conftest hard-imports torch):
    pytest tests/test_datasets.py --noconftest
"""

import sys
from pathlib import Path

import numpy as np
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from pybrimstone.datasets import (
    PlanningCase,
    influence_from_coo,
    load_cort_case,
    make_phase_optimizers,
    mask_from_voxel_list,
    planning_case_from_submatrices,
    synthesize_course,
)
from pybrimstone.dvh_uncertainty import compute_dose, dvh_uncertainty_bands
from pybrimstone.hierarchical_bayes import HierarchicalBayes
from pybrimstone.prescription import Prescription


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

def _toy_case(n_voxels=40, n_beamlets=4, seed=0):
    rng = np.random.RandomState(seed)
    D = rng.rand(n_voxels, n_beamlets)
    half = n_voxels // 2
    ptv = np.zeros(n_voxels, dtype=bool)
    ptv[:half] = True
    oar = np.zeros(n_voxels, dtype=bool)
    oar[half:] = True
    return PlanningCase(
        name="toy",
        dose_matrix=D,
        structures={"PTV": ptv, "OAR": oar},
        prescription={"PTV": (0.4, 0.6)},
    )


# ---------------------------------------------------------------------------
# PlanningCase container
# ---------------------------------------------------------------------------

class TestPlanningCase:
    def test_shapes(self):
        case = _toy_case(40, 4)
        assert case.n_voxels == 40
        assert case.n_beamlets == 4
        assert not case.is_sparse

    def test_masks_coerced_to_bool_1d(self):
        case = _toy_case()
        for mask in case.structures.values():
            assert mask.dtype == bool
            assert mask.shape == (case.n_voxels,)

    def test_rejects_mask_shape_mismatch(self):
        with pytest.raises(ValueError, match="mask shape"):
            PlanningCase(
                name="bad",
                dose_matrix=np.ones((10, 3)),
                structures={"PTV": np.ones(7, dtype=bool)},
            )

    def test_rejects_empty_mask(self):
        with pytest.raises(ValueError, match="empty"):
            PlanningCase(
                name="bad",
                dose_matrix=np.ones((10, 3)),
                structures={"PTV": np.zeros(10, dtype=bool)},
            )

    def test_rejects_prescription_without_mask(self):
        with pytest.raises(ValueError, match="no mask"):
            PlanningCase(
                name="bad",
                dose_matrix=np.ones((10, 3)),
                structures={"PTV": np.ones(10, dtype=bool)},
                prescription={"GHOST": (0.1, 0.2)},
            )

    def test_grid_shape_consistency(self):
        D = np.ones((24, 2))
        case = PlanningCase(
            name="g",
            dose_matrix=D,
            structures={"PTV": np.ones(24, dtype=bool)},
            grid_shape=(2, 3, 4),
        )
        assert case.grid_shape == (2, 3, 4)
        with pytest.raises(ValueError, match="grid_shape"):
            PlanningCase(
                name="g",
                dose_matrix=D,
                structures={"PTV": np.ones(24, dtype=bool)},
                grid_shape=(2, 3, 5),
            )

    def test_dense_dvh_operator_is_matrix(self):
        case = _toy_case()
        op = case.as_dvh_operator()
        assert isinstance(op, np.ndarray)
        # dose = D @ w via compute_dose
        w = np.arange(case.n_beamlets, dtype=float)
        dose = compute_dose(op, w)
        assert np.allclose(dose, case.dose_matrix @ w)

    def test_dense_prescription_operator_is_matrix(self):
        case = _toy_case()
        op = case.as_prescription_operator()
        assert isinstance(op, np.ndarray)
        rx = Prescription(op)
        w = np.ones(case.n_beamlets)
        assert np.allclose(rx.compute_dose(w), case.dose_matrix @ w)
        gd = np.ones(case.n_voxels)
        assert np.allclose(rx.dose_adjoint(gd), case.dose_matrix.T @ gd)


# ---------------------------------------------------------------------------
# Assembly helpers
# ---------------------------------------------------------------------------

class TestInfluenceFromCoo:
    def test_dense_matches_manual(self):
        rows = [0, 1, 2, 2]
        cols = [0, 1, 0, 1]
        vals = [1.0, 2.0, 3.0, 4.0]
        D = influence_from_coo(rows, cols, vals, shape=(3, 2), sparse=False)
        expected = np.array([[1.0, 0.0], [0.0, 2.0], [3.0, 4.0]])
        assert isinstance(D, np.ndarray)
        assert np.allclose(D, expected)

    def test_duplicate_triplets_sum(self):
        D = influence_from_coo([0, 0], [0, 0], [1.0, 2.5], shape=(1, 1), sparse=False)
        assert D[0, 0] == 3.5

    def test_sparse_and_dense_agree(self):
        sp = pytest.importorskip("scipy.sparse")
        rows = [0, 3, 5, 5]
        cols = [0, 1, 2, 0]
        vals = [1.0, 2.0, 3.0, 4.0]
        Dd = influence_from_coo(rows, cols, vals, shape=(6, 3), sparse=False)
        Ds = influence_from_coo(rows, cols, vals, shape=(6, 3), sparse=True)
        assert sp.issparse(Ds)
        assert np.allclose(Ds.toarray(), Dd)

    def test_rejects_out_of_range(self):
        with pytest.raises(ValueError, match="out of range"):
            influence_from_coo([5], [0], [1.0], shape=(3, 2), sparse=False)

    def test_rejects_ragged(self):
        with pytest.raises(ValueError, match="same length"):
            influence_from_coo([0, 1], [0], [1.0], shape=(2, 2), sparse=False)


class TestMaskFromVoxelList:
    def test_basic(self):
        mask = mask_from_voxel_list([0, 2, 4], n_voxels=5)
        assert mask.tolist() == [True, False, True, False, True]

    def test_out_of_range(self):
        with pytest.raises(ValueError, match="out of range"):
            mask_from_voxel_list([7], n_voxels=5)


class TestSubmatrixStacking:
    def test_stacks_and_masks(self):
        ptv_block = np.ones((3, 2))
        oar_block = 2 * np.ones((4, 2))
        case = planning_case_from_submatrices(
            "stacked", {"PTV": ptv_block, "OAR": oar_block}
        )
        assert case.n_voxels == 7
        assert case.n_beamlets == 2
        # PTV occupies first 3 rows, OAR the next 4 -- contiguous & disjoint.
        assert case.structures["PTV"].sum() == 3
        assert case.structures["OAR"].sum() == 4
        assert not np.any(case.structures["PTV"] & case.structures["OAR"])
        # The stacked influence matrix preserves each block's values.
        assert np.allclose(case.dose_matrix[case.structures["PTV"]], 1.0)
        assert np.allclose(case.dose_matrix[case.structures["OAR"]], 2.0)

    def test_rejects_beamlet_mismatch(self):
        with pytest.raises(ValueError, match="beamlets"):
            planning_case_from_submatrices(
                "bad", {"A": np.ones((2, 3)), "B": np.ones((2, 4))}
            )

    def test_sparse_blocks(self):
        sp = pytest.importorskip("scipy.sparse")
        blocks = {"A": sp.csr_matrix(np.ones((2, 2))), "B": sp.csr_matrix(2 * np.ones((3, 2)))}
        case = planning_case_from_submatrices("s", blocks)
        assert case.is_sparse
        assert case.n_voxels == 5
        # sparse dvh operator is a callable that reproduces D @ w
        op = case.as_dvh_operator()
        assert callable(op)
        w = np.array([1.0, 1.0])
        dose = op(w)
        assert dose.shape == (5,)
        assert np.allclose(dose[:2], 2.0)   # block A rows = 1+1
        assert np.allclose(dose[2:], 4.0)   # block B rows = 2+2


# ---------------------------------------------------------------------------
# Multi-phase course synthesis (the Course-prior bridge)
# ---------------------------------------------------------------------------

class TestSynthesizeCourse:
    def test_phase_count_and_naming(self):
        base = _toy_case()
        phases = synthesize_course(base, n_phases=3, seed=1)
        assert len(phases) == 3
        assert [p.name for p in phases] == ["toy_phase0", "toy_phase1", "toy_phase2"]
        for p in phases:
            assert p.n_voxels == base.n_voxels
            assert p.n_beamlets == base.n_beamlets
            assert set(p.structures) == set(base.structures)

    def test_zero_jitter_reproduces_base(self):
        base = _toy_case()
        phases = synthesize_course(base, n_phases=2, dose_jitter=0.0, seed=1)
        for p in phases:
            assert np.allclose(p.dose_matrix, base.dose_matrix)

    def test_jitter_perturbs_distinctly(self):
        base = _toy_case()
        phases = synthesize_course(base, n_phases=3, dose_jitter=0.1, seed=2)
        # Phases differ from base and from each other.
        assert not np.allclose(phases[0].dose_matrix, base.dose_matrix)
        assert not np.allclose(phases[0].dose_matrix, phases[1].dose_matrix)
        # Perturbation is per-beamlet (column-wise scaling): ratio is constant
        # within a column.
        ratio = phases[0].dose_matrix / base.dose_matrix
        assert np.allclose(ratio, ratio[0][None, :])

    def test_shares_prescription(self):
        base = _toy_case()
        phases = synthesize_course(base, n_phases=2, seed=0)
        for p in phases:
            assert p.prescription == base.prescription

    def test_reproducible_with_seed(self):
        base = _toy_case()
        a = synthesize_course(base, n_phases=2, dose_jitter=0.1, seed=7)
        b = synthesize_course(base, n_phases=2, dose_jitter=0.1, seed=7)
        for pa, pb in zip(a, b):
            assert np.allclose(pa.dose_matrix, pb.dose_matrix)

    def test_rejects_bad_args(self):
        base = _toy_case()
        with pytest.raises(ValueError, match="n_phases"):
            synthesize_course(base, n_phases=0)
        with pytest.raises(ValueError, match="dose_jitter"):
            synthesize_course(base, n_phases=2, dose_jitter=-1.0)


# ---------------------------------------------------------------------------
# CORT adapter -- round-trip through a savemat fixture
# ---------------------------------------------------------------------------

class TestLoadCortCase:
    def _write_fixture(self, tmp_path, name="Demo"):
        sio = pytest.importorskip("scipy.io")
        sp = pytest.importorskip("scipy.sparse")
        case_dir = tmp_path / name
        case_dir.mkdir()
        # 6-voxel, 3-beamlet influence matrix.
        D = np.array(
            [
                [1.0, 0.0, 0.0],
                [0.5, 0.5, 0.0],
                [0.0, 1.0, 0.0],
                [0.0, 0.0, 1.0],
                [0.2, 0.2, 0.6],
                [0.0, 0.0, 0.4],
            ]
        )
        sio.savemat(str(case_dir / f"{name}_D.mat"), {"D": sp.csr_matrix(D)})
        # MATLAB 1-based voxel lists per structure (cell array).
        voi = np.empty((2,), dtype=object)
        voi[0] = np.array([[1], [2], [3]])  # PTV -> voxels 0,1,2
        voi[1] = np.array([[4], [5], [6]])  # OAR -> voxels 3,4,5
        sio.savemat(str(case_dir / f"{name}_VOILIST.mat"), {"v": voi})
        return case_dir, D

    def test_round_trip(self, tmp_path):
        case_dir, D = self._write_fixture(tmp_path)
        case = load_cort_case(
            case_dir,
            structure_names=["PTV", "OAR"],
            prescription={"PTV": (0.5, 0.7)},
        )
        assert case.name == "Demo"
        assert case.n_voxels == 6
        assert case.n_beamlets == 3
        assert set(case.structures) == {"PTV", "OAR"}
        assert case.structures["PTV"].nonzero()[0].tolist() == [0, 1, 2]
        assert case.structures["OAR"].nonzero()[0].tolist() == [3, 4, 5]
        assert case.meta["source"] == "CORT"
        # Influence matrix recovered (sparse).
        recovered = case.dose_matrix.toarray() if case.is_sparse else case.dose_matrix
        assert np.allclose(recovered, D)

    def test_default_structure_names(self, tmp_path):
        case_dir, _ = self._write_fixture(tmp_path)
        case = load_cort_case(case_dir)
        assert set(case.structures) == {"voi0", "voi1"}

    def test_missing_files_raise(self, tmp_path):
        empty = tmp_path / "Empty"
        empty.mkdir()
        with pytest.raises(FileNotFoundError):
            load_cort_case(empty)

    def test_bad_key_raises(self, tmp_path):
        case_dir, _ = self._write_fixture(tmp_path)
        with pytest.raises(KeyError, match="not in"):
            load_cort_case(case_dir, dose_key="NOPE")


# ---------------------------------------------------------------------------
# End-to-end integration: case -> phases -> HierarchicalBayes / DVH bands
# ---------------------------------------------------------------------------

class TestIntegration:
    def test_hierarchical_bayes_runs_on_synthetic_course(self):
        base = _toy_case(n_voxels=30, n_beamlets=4, seed=3)
        phases = synthesize_course(base, n_phases=3, dose_jitter=0.05, seed=4)
        optimizers, priors = make_phase_optimizers(
            phases,
            optimizer_kwargs={"max_iter": 15},
        )
        assert len(optimizers) == len(priors) == 3

        hb = HierarchicalBayes(optimizers, priors, normalize_variance=True)
        result = hb.run(max_outer_iters=3, tol=1e-6)

        mu_eta = result["mu_eta"]
        var_eta = result["var_eta"]
        assert mu_eta.shape == (base.n_beamlets,)
        assert var_eta.shape == (base.n_beamlets,)
        assert np.all(np.isfinite(mu_eta))
        assert np.all(var_eta > 0)
        # Driver mutated the shared prior toward the pooled mean.
        for prior in priors:
            assert np.allclose(prior.target_mu, mu_eta)

    def test_dvh_bands_on_case(self):
        case = _toy_case(n_voxels=30, n_beamlets=4, seed=5)
        mu = np.full(case.n_beamlets, 0.5)
        var = np.full(case.n_beamlets, 0.01)
        bands = dvh_uncertainty_bands(
            mu, var,
            dose_operator=case.as_dvh_operator(),
            structures=case.structures,
            n_samples=16,
            rng=np.random.default_rng(0),
        )
        assert set(bands) == set(case.structures)
        for _name, (dose_bins, band) in bands.items():
            # bands shape (n_percentiles, n_bins); DVH is non-increasing & in [0,1].
            assert band.shape[0] == 3
            assert band.min() >= 0.0 and band.max() <= 1.0

    def test_make_phase_optimizers_requires_prescription(self):
        base = _toy_case()
        # Strip the prescription so neither source provides intervals.
        base.prescription = {}
        phases = synthesize_course(base, n_phases=2, seed=0)
        with pytest.raises(ValueError, match="prescription"):
            make_phase_optimizers(phases)
