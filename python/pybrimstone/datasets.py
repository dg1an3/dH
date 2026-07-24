"""
Dataset loaders for testing the amortized Course prior.

The hierarchical-Bayes Course prior (see HIERARCHICAL_BAYES_DESIGN.md) and the
amortized optimization work (see docs/amortized_optimization_and_sigma_estimation.md)
both need treatment-planning cases expressed in a form the pure-Python pipeline
already consumes:

  - a beamlet -> dose *influence matrix* D of shape (n_voxels, n_beamlets),
    so that ``dose = D @ beamlet_weights``. This is exactly what
    ``dvh_uncertainty.compute_dose`` and ``Prescription`` accept, which means a
    precomputed influence matrix lets us exercise the whole pipeline WITHOUT the
    (currently unbuilt) C++ TERMA + kernel dose engine.
  - per-structure voxel *masks* (PTV, OARs), shape (n_voxels,).

This module provides one normalized container (:class:`PlanningCase`) plus
adapters that populate it from:

  * **CORT** -- Common Optimization for Radiation Therapy (Craft et al.,
    GigaScience 2014). MATLAB ``.mat`` files holding a sparse dose-influence
    matrix and per-structure voxel-index lists. ``load_cort_case``.
  * **TROTS** -- The Radiotherapy Optimization Test Set (Breedveld & Heijmen
    2017). MATLAB v7.3 (HDF5) files with per-structure sparse sub-matrices.
    ``load_trots_case`` (+ the format-agnostic ``planning_case_from_submatrices``).
  * **synthetic multi-phase courses** -- ``synthesize_course`` turns a single
    case into P per-phase cases with controlled intra-course variability. This
    is the bridge to the *Course* prior specifically: public benchmarks are
    per-case (one plan), but the Course prior pools across phases of one
    patient's course. Until real repeat-imaging DICOM is wired through the C++
    importer, synthesized phases are how the outer loop gets exercised.

The CORT/TROTS file schemas are documented per their respective READMEs but are
not pinned by a sample shipped in this repo (no network in CI), so the I/O shims
are thin and delegate to tested, format-agnostic assembly helpers
(``influence_from_coo``, ``planning_case_from_submatrices``). Verify the keys
against your download and override them if they differ.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import (
    Callable,
    Dict,
    List,
    Mapping,
    Optional,
    Sequence,
    Tuple,
    Union,
)

import numpy as np

DoseMatrix = object  # np.ndarray or scipy.sparse matrix (kept duck-typed)


# ---------------------------------------------------------------------------
# Sparse-without-hard-dependency helpers
# ---------------------------------------------------------------------------

def _is_sparse(x: object) -> bool:
    """True for a scipy.sparse matrix without importing scipy at module load."""
    return (
        not isinstance(x, np.ndarray)
        and hasattr(x, "tocsr")
        and hasattr(x, "shape")
        and hasattr(x, "dot")
    )


def _require(module: str):
    try:
        return __import__(module)
    except ImportError as exc:  # pragma: no cover - exercised only when missing
        raise ImportError(
            f"{module!r} is required for this dataset loader. Install it with "
            f"`pip install {module}`."
        ) from exc


# ---------------------------------------------------------------------------
# Normalized case container
# ---------------------------------------------------------------------------

@dataclass
class PlanningCase:
    """
    A single treatment-planning instance in pipeline-ready form.

    Attributes:
        name: human-readable identifier.
        dose_matrix: beamlet -> dose influence operator of shape
            (n_voxels, n_beamlets). Either a dense ``np.ndarray`` or a
            scipy.sparse matrix. ``dose = dose_matrix @ beamlet_weights``.
        structures: maps structure name -> boolean voxel mask, shape
            (n_voxels,). PTV(s) and OARs.
        prescription: optional maps structure name -> (dose_min, dose_max)
            interval in Gy, used to build target DVHs (KLDivTerm) when wiring
            phase optimizers. Structures absent from this map get no data term.
        grid_shape: optional (nx, ny, nz) so masks can be reshaped back to 3-D
            (e.g. for spatial mask perturbation in ``synthesize_course``).
        meta: free-form provenance dict (source, beam angles, units, ...).
    """

    name: str
    dose_matrix: DoseMatrix
    structures: Dict[str, np.ndarray]
    prescription: Dict[str, Tuple[float, float]] = field(default_factory=dict)
    grid_shape: Optional[Tuple[int, int, int]] = None
    meta: Dict[str, object] = field(default_factory=dict)

    def __post_init__(self) -> None:
        self.structures = {
            name: np.asarray(mask).astype(bool).reshape(-1)
            for name, mask in self.structures.items()
        }
        self.validate()

    # -- shape accessors ---------------------------------------------------

    @property
    def n_voxels(self) -> int:
        return int(self.dose_matrix.shape[0])

    @property
    def n_beamlets(self) -> int:
        return int(self.dose_matrix.shape[1])

    @property
    def is_sparse(self) -> bool:
        return _is_sparse(self.dose_matrix)

    def validate(self) -> "PlanningCase":
        dm = self.dose_matrix
        if not (isinstance(dm, np.ndarray) or _is_sparse(dm)):
            raise TypeError(
                "dose_matrix must be a 2-D np.ndarray or a scipy.sparse matrix, "
                f"got {type(dm).__name__}"
            )
        if len(dm.shape) != 2:
            raise ValueError(f"dose_matrix must be 2-D, got shape {dm.shape}")
        if isinstance(dm, np.ndarray) and dm.ndim != 2:
            raise ValueError(f"dose_matrix must be 2-D, got shape {dm.shape}")
        if not self.structures:
            raise ValueError("at least one structure mask is required")
        for name, mask in self.structures.items():
            if mask.shape != (self.n_voxels,):
                raise ValueError(
                    f"structure {name!r} mask shape {mask.shape} != "
                    f"(n_voxels={self.n_voxels},)"
                )
            if not mask.any():
                raise ValueError(f"structure {name!r} mask is empty")
        if self.grid_shape is not None:
            nx, ny, nz = self.grid_shape
            if nx * ny * nz != self.n_voxels:
                raise ValueError(
                    f"grid_shape {self.grid_shape} product != n_voxels {self.n_voxels}"
                )
        for name in self.prescription:
            if name not in self.structures:
                raise ValueError(
                    f"prescription names structure {name!r} with no mask"
                )
        return self

    # -- dose-operator adapters -------------------------------------------

    def as_dvh_operator(self) -> Union[np.ndarray, Callable[[np.ndarray], np.ndarray]]:
        """
        Operator for ``dvh_uncertainty.compute_dose`` / ``dvh_uncertainty_bands``.

        Returns the dense matrix directly (fast vectorized path) or, for a
        sparse influence matrix, a callable ``w -> dose`` that compute_dose's
        callable branch accepts.
        """
        if self.is_sparse:
            mat = self.dose_matrix

            def forward(w: np.ndarray) -> np.ndarray:
                return np.asarray(mat @ np.asarray(w, dtype=np.float64)).reshape(-1)

            return forward
        return self.dose_matrix

    def as_prescription_operator(
        self,
    ) -> Union[np.ndarray, Tuple[Callable, Callable]]:
        """
        Operator for ``Prescription``. Dense -> the matrix; sparse -> a
        (forward, adjoint) callable pair, which Prescription accepts directly.
        """
        if self.is_sparse:
            mat = self.dose_matrix

            def forward(w: np.ndarray) -> np.ndarray:
                return np.asarray(mat @ np.asarray(w, dtype=np.float64)).reshape(-1)

            def adjoint(grad_dose: np.ndarray) -> np.ndarray:
                return np.asarray(
                    mat.T @ np.asarray(grad_dose, dtype=np.float64)
                ).reshape(-1)

            return (forward, adjoint)
        return self.dose_matrix


# ---------------------------------------------------------------------------
# Format-agnostic assembly helpers (the tested core both adapters funnel into)
# ---------------------------------------------------------------------------

def influence_from_coo(
    rows: Sequence[int],
    cols: Sequence[int],
    vals: Sequence[float],
    shape: Tuple[int, int],
    sparse: bool = True,
) -> DoseMatrix:
    """
    Assemble an (n_voxels, n_beamlets) influence matrix from COO triplets.

    Both CORT and TROTS store dose-influence data sparsely; this is the shared
    sink for "I have (voxel, beamlet, value) triplets, give me a matrix the
    pipeline can use".

    Args:
        rows: voxel indices (0-based).
        cols: beamlet indices (0-based).
        vals: influence values.
        shape: (n_voxels, n_beamlets).
        sparse: if True and scipy is available, return a CSR matrix; otherwise
            a dense ndarray.

    Returns:
        CSR sparse matrix (scipy) or dense ndarray of the given shape.
    """
    rows = np.asarray(rows, dtype=np.int64)
    cols = np.asarray(cols, dtype=np.int64)
    vals = np.asarray(vals, dtype=np.float64)
    if not (rows.shape == cols.shape == vals.shape):
        raise ValueError(
            f"rows/cols/vals must be same length; got {rows.shape}, "
            f"{cols.shape}, {vals.shape}"
        )
    n_voxels, n_beamlets = shape
    if rows.size and (rows.max() >= n_voxels or rows.min() < 0):
        raise ValueError("row index out of range for shape")
    if cols.size and (cols.max() >= n_beamlets or cols.min() < 0):
        raise ValueError("col index out of range for shape")

    if sparse:
        try:
            from scipy import sparse as sp  # type: ignore

            return sp.coo_matrix((vals, (rows, cols)), shape=shape).tocsr()
        except ImportError:
            pass  # fall through to dense
    dense = np.zeros(shape, dtype=np.float64)
    np.add.at(dense, (rows, cols), vals)
    return dense


def mask_from_voxel_list(voxel_indices: Sequence[int], n_voxels: int) -> np.ndarray:
    """Boolean (n_voxels,) mask from a 0-based list of in-structure voxels."""
    idx = np.asarray(voxel_indices, dtype=np.int64).reshape(-1)
    mask = np.zeros(n_voxels, dtype=bool)
    if idx.size:
        if idx.max() >= n_voxels or idx.min() < 0:
            raise ValueError("voxel index out of range for n_voxels")
        mask[idx] = True
    return mask


def planning_case_from_submatrices(
    name: str,
    submatrices: Mapping[str, DoseMatrix],
    n_beamlets: Optional[int] = None,
    prescription: Optional[Mapping[str, Tuple[float, float]]] = None,
    meta: Optional[Mapping[str, object]] = None,
) -> PlanningCase:
    """
    Build a PlanningCase from *per-structure* dose-influence sub-matrices.

    TROTS (and some CORT exports) store one (n_struct_voxels, n_beamlets) block
    per structure rather than one global matrix over a shared voxel grid. This
    stacks them into a single (sum_of_struct_voxels, n_beamlets) influence
    matrix and builds the corresponding contiguous masks.

    The stacked voxel ordering is: structures in iteration order, each block's
    rows contiguous. Masks select each structure's own block. This is a faithful
    optimization-space representation (every structure voxel that has dose
    influence is present) even though it is not a dense anatomical grid -- which
    is exactly what the DVH/Course-prior math needs.

    Args:
        name: case name.
        submatrices: structure name -> (n_struct_voxels, n_beamlets) matrix
            (dense ndarray or scipy.sparse). All must share n_beamlets.
        n_beamlets: optional explicit beamlet count (checked against blocks).
        prescription: optional structure -> (dose_min, dose_max) intervals.
        meta: optional provenance.

    Returns:
        PlanningCase with a vertically-stacked influence matrix.
    """
    if not submatrices:
        raise ValueError("submatrices is empty")

    blocks: List[DoseMatrix] = []
    structures: Dict[str, np.ndarray] = {}
    offset = 0
    ncols: Optional[int] = n_beamlets
    any_sparse = any(_is_sparse(m) for m in submatrices.values())

    # First pass: total rows + column-count agreement.
    total_rows = 0
    for sname, mat in submatrices.items():
        if len(mat.shape) != 2:
            raise ValueError(f"submatrix {sname!r} must be 2-D, got {mat.shape}")
        r, c = mat.shape
        if ncols is None:
            ncols = c
        elif c != ncols:
            raise ValueError(
                f"submatrix {sname!r} has {c} beamlets, expected {ncols}"
            )
        total_rows += r

    # Second pass: stack + masks.
    for sname, mat in submatrices.items():
        r = mat.shape[0]
        mask = np.zeros(total_rows, dtype=bool)
        mask[offset : offset + r] = True
        structures[sname] = mask
        blocks.append(mat)
        offset += r

    if any_sparse:
        sp = _require("scipy").sparse  # type: ignore[attr-defined]
        dose_matrix: DoseMatrix = sp.vstack(
            [b if _is_sparse(b) else sp.csr_matrix(b) for b in blocks]
        ).tocsr()
    else:
        dose_matrix = np.vstack([np.asarray(b, dtype=np.float64) for b in blocks])

    return PlanningCase(
        name=name,
        dose_matrix=dose_matrix,
        structures=structures,
        prescription=dict(prescription or {}),
        meta=dict(meta or {}),
    )


# ---------------------------------------------------------------------------
# CORT adapter (scipy .mat)
# ---------------------------------------------------------------------------

def load_cort_case(
    case_dir: Union[str, Path],
    name: Optional[str] = None,
    dose_file: Optional[str] = None,
    voilist_file: Optional[str] = None,
    dose_key: str = "D",
    voilist_key: str = "v",
    structure_names: Optional[Sequence[str]] = None,
    prescription: Optional[Mapping[str, Tuple[float, float]]] = None,
    sparse: bool = True,
) -> PlanningCase:
    """
    Load a CORT case directory into a PlanningCase.

    CORT (Craft et al., GigaScience 2014, https://doi.org/10.5524/100110) ships
    each problem as a folder containing, among others:

      * ``<NAME>_D.mat``       -- the global dose-influence matrix ``D``,
                                  sparse, shape (n_voxels, n_beamlets).
      * ``<NAME>_VOILIST.mat`` -- per-structure voxel-index lists (a MATLAB
                                  cell array, 1-based indices into D's rows).

    The exact variable names vary across CORT exports; override ``dose_key`` /
    ``voilist_key`` if your download differs (inspect with
    ``scipy.io.whosmat(path)``).

    Args:
        case_dir: directory holding the case's ``.mat`` files.
        name: case name (defaults to the directory name).
        dose_file / voilist_file: filenames (default ``<NAME>_D.mat`` /
            ``<NAME>_VOILIST.mat``).
        dose_key / voilist_key: MATLAB variable names within those files.
        structure_names: names for each VOI entry, in order. Defaults to
            ``voi0, voi1, ...``.
        prescription: optional structure -> (dose_min, dose_max) intervals.
        sparse: keep the influence matrix sparse (recommended at clinical size).

    Returns:
        PlanningCase.
    """
    sio = _require("scipy").io  # type: ignore[attr-defined]
    case_dir = Path(case_dir)
    if not case_dir.is_dir():
        raise NotADirectoryError(f"CORT case dir not found: {case_dir}")
    case_name = name or case_dir.name

    dose_path = case_dir / (dose_file or f"{case_name}_D.mat")
    voi_path = case_dir / (voilist_file or f"{case_name}_VOILIST.mat")
    if not dose_path.exists():
        raise FileNotFoundError(f"dose-influence file not found: {dose_path}")
    if not voi_path.exists():
        raise FileNotFoundError(f"VOI list file not found: {voi_path}")

    dose_mat = sio.loadmat(str(dose_path))
    if dose_key not in dose_mat:
        raise KeyError(
            f"key {dose_key!r} not in {dose_path.name}; "
            f"available: {[k for k in dose_mat if not k.startswith('__')]}"
        )
    D = dose_mat[dose_key]
    if _is_sparse(D):
        D = D.tocsr() if sparse else np.asarray(D.todense(), dtype=np.float64)
    else:
        D = np.asarray(D, dtype=np.float64)
        if sparse:
            try:
                from scipy import sparse as sp  # type: ignore

                D = sp.csr_matrix(D)
            except ImportError:
                pass
    n_voxels = int(D.shape[0])

    voi_mat = sio.loadmat(str(voi_path))
    if voilist_key not in voi_mat:
        raise KeyError(
            f"key {voilist_key!r} not in {voi_path.name}; "
            f"available: {[k for k in voi_mat if not k.startswith('__')]}"
        )
    voi_cell = voi_mat[voilist_key]
    voxel_lists = _unpack_matlab_cell(voi_cell)

    if structure_names is not None and len(structure_names) != len(voxel_lists):
        raise ValueError(
            f"structure_names has {len(structure_names)} entries but found "
            f"{len(voxel_lists)} VOI lists"
        )
    names = (
        list(structure_names)
        if structure_names is not None
        else [f"voi{i}" for i in range(len(voxel_lists))]
    )

    structures: Dict[str, np.ndarray] = {}
    for sname, vlist in zip(names, voxel_lists):
        # CORT voxel indices are 1-based MATLAB indices into D's rows.
        idx0 = np.asarray(vlist, dtype=np.int64).reshape(-1) - 1
        structures[sname] = mask_from_voxel_list(idx0, n_voxels)

    return PlanningCase(
        name=case_name,
        dose_matrix=D,
        structures=structures,
        prescription=dict(prescription or {}),
        meta={"source": "CORT", "case_dir": str(case_dir)},
    )


def _unpack_matlab_cell(cell: object) -> List[np.ndarray]:
    """
    Normalize a loadmat cell-array / struct-of-arrays into a list of 1-D arrays.

    scipy.io.loadmat represents a MATLAB cell array as an object ndarray; the
    nesting depth depends on the source shape. This flattens the common
    (1, N) / (N, 1) / (N,) object-array cases into N entries.
    """
    arr = np.asarray(cell, dtype=object).reshape(-1)
    out: List[np.ndarray] = []
    for entry in arr:
        out.append(np.asarray(entry).reshape(-1))
    return out


# ---------------------------------------------------------------------------
# TROTS adapter (h5py v7.3)
# ---------------------------------------------------------------------------

def load_trots_case(
    mat_path: Union[str, Path],
    name: Optional[str] = None,
    prescription: Optional[Mapping[str, Tuple[float, float]]] = None,
) -> PlanningCase:
    """
    Load a TROTS MATLAB v7.3 (HDF5) case into a PlanningCase.

    TROTS (Breedveld & Heijmen, Med. Phys. 2017,
    https://doi.org/10.1002/mp.12369; data at https://zenodo.org/record/2708302)
    stores, per case, a ``data`` struct whose ``matrix`` cell holds one
    dose-influence sub-matrix ``A`` per scored structure/objective, plus a
    ``patient`` struct with ``StructureNames``.

    Because TROTS v7.3 files nest MATLAB structs behind HDF5 object references,
    the traversal here is best-effort and unverified against a shipped sample
    (none in-repo; no network in CI). The *conversion* logic, once sub-matrices
    are extracted, is the tested ``planning_case_from_submatrices`` path. If the
    traversal raises on your file, extract the per-structure ``A`` blocks
    yourself (e.g. with h5py) and call ``planning_case_from_submatrices``
    directly.

    Args:
        mat_path: path to the ``*.mat`` v7.3 file.
        name: case name (defaults to file stem).
        prescription: optional structure -> (dose_min, dose_max) intervals.

    Returns:
        PlanningCase assembled from the per-structure sub-matrices.
    """
    h5py = _require("h5py")
    mat_path = Path(mat_path)
    if not mat_path.exists():
        raise FileNotFoundError(f"TROTS file not found: {mat_path}")
    case_name = name or mat_path.stem

    submatrices: Dict[str, DoseMatrix] = {}
    with h5py.File(str(mat_path), "r") as f:  # type: ignore[attr-defined]
        if "data" not in f or "patient" not in f:
            raise KeyError(
                "expected top-level 'data' and 'patient' groups; this does not "
                "look like a TROTS v7.3 file. Extract sub-matrices manually and "
                "use planning_case_from_submatrices()."
            )
        names = _trots_read_string_array(f, f["patient"]["StructureNames"])
        matrix_refs = np.asarray(f["data"]["matrix"]).reshape(-1)
        for i, ref in enumerate(matrix_refs):
            block = _trots_read_sparse(f, f[ref])
            sname = names[i] if i < len(names) else f"struct{i}"
            submatrices[sname] = block

    return planning_case_from_submatrices(
        name=case_name,
        submatrices=submatrices,
        prescription=prescription,
        meta={"source": "TROTS", "path": str(mat_path)},
    )


def _trots_read_string_array(f, dataset) -> List[str]:  # pragma: no cover - needs real file
    """Decode a MATLAB v7.3 cell-of-char into a list of Python strings."""
    out: List[str] = []
    for ref in np.asarray(dataset).reshape(-1):
        chars = np.asarray(f[ref]).reshape(-1)
        out.append("".join(chr(int(c)) for c in chars))
    return out


def _trots_read_sparse(f, group):  # pragma: no cover - needs real file
    """
    Read one TROTS dose sub-matrix. MATLAB sparse arrays serialize to v7.3 as a
    group with 'data'/'ir'/'jc' (CSC). Fall back to a dense dataset.
    """
    sp = _require("scipy").sparse  # type: ignore[attr-defined]
    if hasattr(group, "keys") and {"data", "ir", "jc"}.issubset(set(group.keys())):
        data = np.asarray(group["data"]).reshape(-1)
        ir = np.asarray(group["ir"]).reshape(-1)
        jc = np.asarray(group["jc"]).reshape(-1)
        n_cols = jc.size - 1
        n_rows = int(ir.max()) + 1 if ir.size else 0
        return sp.csc_matrix((data, ir, jc), shape=(n_rows, n_cols)).tocsr()
    return np.asarray(group, dtype=np.float64)


# ---------------------------------------------------------------------------
# Synthetic multi-phase course (the bridge to the Course prior)
# ---------------------------------------------------------------------------

def synthesize_course(
    base_case: PlanningCase,
    n_phases: int,
    dose_jitter: float = 0.05,
    seed: Optional[int] = None,
) -> List[PlanningCase]:
    """
    Derive P per-phase cases from one base case to exercise the Course prior.

    Public benchmarks (CORT/TROTS/OpenKBP) are per-case: one plan per patient.
    The Course prior pools beamlet-weight posteriors across *phases* of one
    course (HIERARCHICAL_BAYES_DESIGN.md). Until real repeat-imaging DICOM is
    importable, this models intra-course variability by perturbing the
    dose-influence matrix per phase -- a stand-in for setup/density/calibration
    drift between fractions. Structures (anatomy) are shared across phases.

    Each phase's influence matrix is the base scaled by per-beamlet lognormal
    noise: ``D_p[:, b] = D[:, b] * exp(eps),  eps ~ N(0, dose_jitter^2)``. With
    ``dose_jitter == 0`` every phase reproduces the base exactly (useful as a
    test fixture / sanity control).

    Args:
        base_case: the single-plan PlanningCase to derive phases from.
        n_phases: number of phases P (>= 1).
        dose_jitter: std of the per-beamlet log-scale perturbation.
        seed: optional RNG seed for reproducibility.

    Returns:
        list of P PlanningCase objects sharing structures/prescription, named
        ``<base>_phase{p}``.
    """
    if n_phases < 1:
        raise ValueError(f"n_phases must be >= 1, got {n_phases}")
    if dose_jitter < 0:
        raise ValueError(f"dose_jitter must be >= 0, got {dose_jitter}")
    rng = np.random.default_rng(seed)

    phases: List[PlanningCase] = []
    for p in range(n_phases):
        if dose_jitter == 0:
            Dp = base_case.dose_matrix
        else:
            scale = np.exp(rng.normal(0.0, dose_jitter, size=base_case.n_beamlets))
            if base_case.is_sparse:
                sp = _require("scipy").sparse  # type: ignore[attr-defined]
                Dp = base_case.dose_matrix @ sp.diags(scale)
                Dp = Dp.tocsr()
            else:
                Dp = base_case.dose_matrix * scale[None, :]
        phases.append(
            PlanningCase(
                name=f"{base_case.name}_phase{p}",
                dose_matrix=Dp,
                structures={k: v.copy() for k, v in base_case.structures.items()},
                prescription=dict(base_case.prescription),
                grid_shape=base_case.grid_shape,
                meta={**base_case.meta, "phase": p, "of_course": base_case.name},
            )
        )
    return phases


# ---------------------------------------------------------------------------
# Wiring helpers: case(s) -> HierarchicalBayes inputs
# ---------------------------------------------------------------------------

def make_phase_optimizers(
    phases: Sequence[PlanningCase],
    prescription: Optional[Mapping[str, Tuple[float, float]]] = None,
    course_prior_precision: float = 1e-3,
    kl_kwargs: Optional[Mapping[str, object]] = None,
    optimizer_kwargs: Optional[Mapping[str, object]] = None,
) -> Tuple[list, list]:
    """
    Build ``(phase_optimizers, course_prior_terms)`` ready for HierarchicalBayes.

    For each phase this assembles a :class:`Prescription` with one ``KLDivTerm``
    per prescribed structure (target DVH from its (dose_min, dose_max) interval),
    wraps it in a :class:`PhaseOptimizer`, and pairs it with a fresh
    :class:`CoursePriorTerm` (the driver overwrites ``target_mu``/``precision``
    after the first pool, so the initial values are just a weak placeholder).

    Args:
        phases: per-phase PlanningCases (e.g. from ``synthesize_course``). All
            must share n_beamlets and structure names.
        prescription: structure -> (dose_min, dose_max) intervals. Falls back to
            each phase's ``.prescription``; at least one source must be present.
        course_prior_precision: initial scalar precision for the placeholder
            CoursePriorTerm.
        kl_kwargs: extra kwargs forwarded to ``KLDivTerm.from_interval``.
        optimizer_kwargs: extra kwargs forwarded to ``PhaseOptimizer``.

    Returns:
        (phase_optimizers, course_prior_terms) -- both length P, the exact
        inputs ``HierarchicalBayes(phase_optimizers, course_prior_terms)`` wants.
    """
    from .course_prior import CoursePriorTerm
    from .kl_term import KLDivTerm
    from .phase_optimizer import PhaseOptimizer
    from .prescription import Prescription

    if not phases:
        raise ValueError("need at least one phase")
    kl_kwargs = dict(kl_kwargs or {})
    optimizer_kwargs = dict(optimizer_kwargs or {})

    n_beamlets = phases[0].n_beamlets
    for ph in phases:
        if ph.n_beamlets != n_beamlets:
            raise ValueError("all phases must share n_beamlets")

    phase_optimizers: list = []
    course_prior_terms: list = []
    for ph in phases:
        rx = dict(ph.prescription)
        if prescription:
            rx.update(prescription)
        if not rx:
            raise ValueError(
                f"phase {ph.name!r} has no prescription intervals; pass "
                "`prescription=` or set PlanningCase.prescription"
            )

        rx_term = Prescription(ph.as_prescription_operator())
        for sname, (dose_min, dose_max) in rx.items():
            mask = ph.structures[sname]
            rx_term.add_dose_term(
                KLDivTerm.from_interval(
                    structure_mask=mask.astype(float),
                    dose_min=dose_min,
                    dose_max=dose_max,
                    **kl_kwargs,
                )
            )

        phase_optimizers.append(
            PhaseOptimizer(rx_term, n_params=n_beamlets, **optimizer_kwargs)
        )
        course_prior_terms.append(
            CoursePriorTerm(
                target_mu=np.zeros(n_beamlets),
                precision=course_prior_precision,
            )
        )

    return phase_optimizers, course_prior_terms
