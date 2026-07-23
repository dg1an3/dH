# Datasets for Testing the Amortized Course Prior

This document catalogs the data needed to test the **amortized Course prior** —
the hierarchical-Bayes Course-level pooling (`HIERARCHICAL_BAYES_DESIGN.md`)
plus the amortized optimization work
(`docs/amortized_optimization_and_sigma_estimation.md`) — and where to get it.

The companion loader is `python/pybrimstone/datasets.py`. It normalizes every
source into a single `PlanningCase` (a beamlet→dose influence matrix + per-
structure masks) that plugs directly into the existing pipeline:
`dvh_uncertainty.compute_dose` / `dvh_uncertainty_bands`, `Prescription`,
`PhaseOptimizer`, and `HierarchicalBayes`.

## What the data must provide

Two capabilities are bundled in "amortized Course prior", and they impose
different requirements:

| Capability | Needs | Why |
|---|---|---|
| **Course prior** (`CoursePriorTerm` + `HierarchicalBayes`) | *Multiple phases per patient* | Pools per-phase beamlet-weight posteriors across one patient's course. |
| **Amortization** (init / σ networks) | *A cohort of patients* | Learns anatomy → warm-start / σ across many cases. Checklist opens with "Collect optimization trajectories from 100+ patients." |

Per-case ingredients consumed by the code:

- **CT density volume** + **structure masks** (PTV + OARs) — anatomy encoders;
  `KLDivTerm` / `Prescription` binning.
- **Beam geometry**, and ideally a **beamlet→dose influence matrix**.
  `compute_dose` accepts *either a matrix or a callable*, so a precomputed
  influence matrix exercises the whole pipeline **without** the (currently
  unbuilt) C++ TERMA + kernel dose engine.
- **Target DVHs / prescription** per structure.

One non-obvious constraint falls out of the σ-calibration finding
(`HIERARCHICAL_BAYES_DESIGN.md`, "Re-verifying on TERMA + Kernel Dose"):
bootstrap posterior quality collapses (shape corr 0.96 → 0.28) on **collinear
single-angle beamlets** and recovers on **multi-angle / spread** geometry.
Prefer datasets with **multi-angle beams**; SBRT single-arc cases are a known
stress case, not a good first test.

## Where to find them

### Tier 1 — synthetic, already in-repo (start here)

`python/experiments/sigma_calibration.py` plus the `dose_calc.py` /
`terma_kernel_dose.py` generators produce Gaussian-bump and TERMA phantom
geometries. This is the right substrate for unit/regression testing the prior
math, and the **only tier that runs today** (the C++/DICOM wrapper does not
build on Linux — see `HIERARCHICAL_BAYES_FOLLOWUPS.md` item 3).

For the multi-phase *course* dimension, synthesize phases from a single case
with `synthesize_course` (below).

### Tier 2 — public beamlet-matrix research sets (best fit, no C++ engine)

Because the pipeline accepts a dose-influence matrix directly, these slot in via
`PlanningCase`:

| Dataset | Contents | Format | Loader |
|---|---|---|---|
| **CORT** (Craft et al., *GigaScience* 2014, [doi:10.5524/100110](https://doi.org/10.5524/100110)) | Prostate, liver, H&N, TG-119 phantom; global dose-influence matrix + per-structure voxel lists | MATLAB `.mat` (sparse) | `load_cort_case` |
| **TROTS** (Breedveld & Heijmen, *Med. Phys.* 2017, [doi:10.1002/mp.12369](https://doi.org/10.1002/mp.12369); data: [zenodo 2708302](https://zenodo.org/record/2708302)) | ~120 cases (H&N, prostate, cervix…), multi-angle, per-structure sparse sub-matrices | MATLAB v7.3 (HDF5) | `load_trots_case` / `planning_case_from_submatrices` |
| **OpenKBP** (AAPM 2020 Grand Challenge, [GitHub](https://github.com/ababier/open-kbp)) | 340 H&N plans: structure masks + reference dose | CSV/NumPy | (use as the *amortized* cohort; per-voxel dose, not an influence matrix) |

CORT/TROTS provide the influence matrix and so are the natural fit for the
Course-prior + bootstrap math on real multi-angle physics. OpenKBP is the
natural training cohort for the amortized init/σ networks.

### Tier 3 — real DICOM clinical cohorts (the clinical-validation gate)

`HIERARCHICAL_BAYES_FOLLOWUPS.md` flags "Calibration on real DICOM plans" as the
gating step before any clinical claim, ingested via the existing
`SeriesDicomImporter`. Source from **TCIA** (The Cancer Imaging Archive):

- Single-course cohorts with CT + RTSTRUCT + RTPLAN/RTDOSE: **HNSCC**,
  **Head-Neck-PET-CT**.
- **Multi-phase / adaptive** collections (the Course prior's whole point):
  **4D-Lung** (respiratory phases), **Pancreatic-CT-CBCT-SEG** (repeat CBCT).
- **AAPM TG-119** standard cases — known-answer multi-structure phantom.

This tier requires the C++ wrapper to build.

## The hard constraint: multiple phases per patient

The Course prior needs **multiple phases per patient** — the one axis public
single-plan benchmarks (CORT/TROTS/OpenKBP) *don't* give you. Two ways to get
it:

1. **Synthesize** intra-course variability from a single case
   (`synthesize_course`) — runnable today, good for exercising the outer loop.
2. **Real intra-course variability** from TCIA repeat-imaging collections — the
   genuine article, needed for the clinical coverage claim (Tier 3).

## Using the loader

```python
import numpy as np
from pybrimstone.datasets import (
    load_cort_case, synthesize_course, make_phase_optimizers,
)
from pybrimstone import HierarchicalBayes, dvh_uncertainty_bands

# 1. Load a real per-case benchmark (Tier 2). Override keys if your
#    CORT export uses different MATLAB variable names.
case = load_cort_case(
    "CORT/Prostate",
    structure_names=["PTV", "Bladder", "Rectum"],
    prescription={"PTV": (70.0, 75.0)},
)

# 2. Derive a multi-phase course (the Course-prior bridge). dose_jitter
#    models per-fraction setup/density drift; 0.0 reproduces the base case.
phases = synthesize_course(case, n_phases=5, dose_jitter=0.05, seed=0)

# 3. Wire phase optimizers + Course priors for the hierarchical driver.
optimizers, priors = make_phase_optimizers(phases)
result = HierarchicalBayes(optimizers, priors, normalize_variance=True).run()

# 4. DVH uncertainty bands (Step 5 deliverable) from the pooled posterior.
bands = dvh_uncertainty_bands(
    result["mu_eta"], result["var_eta"],
    dose_operator=case.as_dvh_operator(),
    structures=case.structures,
)
```

### Bringing your own data

Any source reduces to a `PlanningCase`. If you have COO triplets for a global
influence matrix, use `influence_from_coo` + `mask_from_voxel_list`. If you have
one sub-matrix per structure (the TROTS layout), use
`planning_case_from_submatrices`, which stacks them into a single influence
matrix with contiguous per-structure masks.

## Loader status / caveats

- **`PlanningCase`, `influence_from_coo`, `mask_from_voxel_list`,
  `planning_case_from_submatrices`, `synthesize_course`, `make_phase_optimizers`**
  — fully unit-tested (`python/tests/test_datasets.py`), including an end-to-end
  `HierarchicalBayes` + `dvh_uncertainty_bands` integration run.
- **`load_cort_case`** — tested via a `scipy.io.savemat` round-trip fixture. The
  exact MATLAB variable names vary across CORT exports; inspect with
  `scipy.io.whosmat(path)` and override `dose_key` / `voilist_key` if needed.
- **`load_trots_case`** — TROTS v7.3 files nest MATLAB structs behind HDF5
  object references; the traversal is **best-effort and unverified against a
  shipped sample** (none in-repo, no network in CI). The conversion core
  (`planning_case_from_submatrices`) *is* tested. If the traversal raises on
  your file, extract the per-structure `A` blocks with `h5py` yourself and call
  `planning_case_from_submatrices` directly.
- Optional deps: `scipy` (CORT, sparse matrices), `h5py` (TROTS). The module
  imports without them; the relevant loaders raise a clear `ImportError`.

## References

- `HIERARCHICAL_BAYES_DESIGN.md` — Course prior architecture + σ-calibration record.
- `HIERARCHICAL_BAYES_FOLLOWUPS.md` — "Calibration on real DICOM plans" gate (item 3, stretch items).
- `docs/amortized_optimization_and_sigma_estimation.md` — amortized init/σ networks; "100+ patients" data-collection step.
- `python/pybrimstone/datasets.py` — the loader.
- `python/tests/test_datasets.py` — loader tests.
