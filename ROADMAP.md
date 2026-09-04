# Roadmap: 3D planning, a Morphome plan library, and active inference

Date: 2026-09-04

Three goals, in dependency order. Each section states what the code does
today, what the goal requires, and what is unresolved. Companion documents:
`HIERARCHICAL_BAYES_DESIGN.md` (course prior), `DATASETS.md` (multi-phase
data requirements), `docs/amortized_optimization_and_sigma_estimation.md`
(learned initialization and sigma prediction), and
`notes/entropy_weight_sweep.md` (the current free-energy objective).

```
1. 2D fluence maps (true 3D planning)
      |
2. Morphome thorax plan library
      |
3. Free energy -> active inference (adaptive replanning)
```

Step 2 needs step 1 because plans optimized under the current coplanar beam
model are not reusable as priors or initializations for the 3D model. Step 3
needs step 2 because active inference is only meaningful with an observation
loop, and multi-phase per-patient data is what supplies it.

## 1. Extend planning to 3D

### What is already 3D

- Dose matrices (`Plan::m_pDose`, `VolumeReal` = `itk::Image<float,3>`),
  the spherical energy kernel convolution (`SphereConvolve`), the CT density
  volume, and structure regions (multi-slice contours resampled to volumes).
- Histograms bin over 3D regions, so the KL objective is already volumetric.

### What is 2D: the beam model

- `CBeam::IntensityMap` is `itk::Image<VOXEL_REAL, 1>`. Beamlets are the
  pencil beam shifted along one in-plane axis: 19 shifts per side at 4 mm
  spacing at the finest level, halved per pyramid level
  (`RtModel/PlanPyramid.cpp:113`).
- Beams carry a gantry angle only (`Beam.h`): no couch angle, no cross-plane
  offset. Every plan is coplanar with a fan of beamlets in the axial plane.

### Required changes

- Intensity map becomes a 2D image (leaf direction x cross-plane direction).
  `OnIntensityMapChanged`, `GetBeamlet(shift)`, and the beamlet accumulation
  in `Plan::GetDoseMatrix` need a 2D index.
- `BeamDoseCalc` ray-trace offsets in both axes; beamlet TERMA generation per
  grid cell.
- `PlanPyramid` beamlet generation and `InvFiltIntensityMap` become 2D
  (separable filtering is enough: apply the existing 1D inverse filter along
  each axis).
- `PlanOptimizer::StateVectorToIntensityMap` and the sigmoid parameterization
  flatten the 2D map; nothing conceptual changes there.
- Optional: couch angle for non-coplanar beams. Not required for step 2.

### Scale consequences

- State vector grows from about 7 x 39 to about 7 x 39 x 39 (~10k
  variables). Conjugate gradient copes. The optional explicit free-energy
  path (`DynamicCovarianceOptimizer::SetComputeFreeEnergy`) builds a
  covariance from orthogonalized CG directions and will not scale as-is;
  treat it as diagnostic-only until step 3 replaces it.
- Current test data is a 5-slice micro series. A full thorax series at the
  0.5 mm finest pyramid level is expensive in beamlet storage and
  convolution. The 4 mm default dose resolution should become the working
  setting, and the four-level pyramid schedule (`DEFAULT_LEVELSIGMA`,
  `CG_TOLERANCE`) needs re-tuning on a full series.
- Beamlet storage: 7 x 1521 volumes at dose resolution. Either store
  beamlets sparsely (they are compact) or compute dose on the fly from the
  fluence map. Decide before the first full-series run.

### Verification

- The pywinauto driver (`python/automate_brimstone_ui.py`,
  `run_brimstone_knee_7beam.bat`) and the objective-file output
  (`BRIMSTONE_OBJECTIVE_FILE`) give a regression baseline. Confirm that a 2D
  map collapsed to a single row reproduces the current 1D objective before
  optimizing full maps.
- The planar view now draws contours and isodose lines matched to the GDI
  view (PR #53); `BRIMSTONE_LEGACY_PLANAR=1` keeps the GDI view available
  for side-by-side checks. The view needs a sagittal/coronal option once
  fluence is 2D.

## 2. Morphome thorax plan library

### Purpose

A library of converged plans across patients is the training set that two
existing designs are waiting for:

- The amortized optimization work (learned initialization, learned sigma
  schedule) needs collected runs.
- The course prior in `HIERARCHICAL_BAYES_DESIGN.md` pools phases through a
  latent; a population of plans is what a Population -> Patient -> Phase
  hierarchy (open question 2 in that document) would learn from.

Thorax first because respiratory motion supplies real intra-course
variability, which `DATASETS.md` identifies as the hard constraint for the
course prior, and which step 3 needs as its latent dynamics.

### Data requirements

- The importer (`SeriesDicomImporter`) needs CT slices plus an RTSTRUCT in
  one folder. Open question: whether Morphome ships structure sets for the
  thorax cases or contours are needed. The DVH page's structure editor and
  the prescription toolbar assume named ROIs; TG-263 naming
  (`python/TG263_README.md`) should be applied at import.
- Multiple phases per patient (4D phases or repeat imaging) are needed for
  steps 2b and 3. Single-phase cases still serve the amortization library.

### Deliverables

1. Batch driver: for each case, import, set up beams (fixed template per
   site to start), apply a site prescription, optimize, and write the
   converged objective, final beamlet weights, DVHs, and dose. Reuse the
   existing objective-file mechanism; add a weights/DVH dump next to it.
   Headless operation (no GUI) is preferable; the Cython wrapper in
   `CYTHON_WRAPPER_DESIGN.md` / `python/pybrimstone` is the route.
2. Library format: one directory per case with plan XML (`PlanXmlFile`),
   weights, DVH curves, and the run's environment settings, so runs are
   reproducible. Note the observed run-to-run drift in the converged
   objective at the fifth significant figure even with
   `BRIMSTONE_DETERMINISTIC=1`; pin it down before treating library values
   as ground truth.
3. Summary statistics across the library: per-structure DVH bands, weight
   distributions per beam angle, and convergence traces per pyramid level.
   These feed the sigma predictor and the learned initializer.

## 3. Extending free energy toward active inference

### What exists

- The objective is `F = KL - w * H` (`Prescription::operator()`), with a
  softmax or separable entropy `H` over beamlet weights. This is variational
  free energy minimization with the prescription DVH as the preferred
  outcome; the sigmoid-parameterized weights are the variational parameters
  and the adaptive variance approximates a posterior.
- The sigma calibration experiment (`python/experiments/sigma_calibration.py`)
  showed `m_vAdaptVariance` is an optimizer trace, not a calibrated
  posterior. `HIERARCHICAL_BAYES_DESIGN.md` recommends an external estimator
  (Hutchinson Fisher diagonal) as the first cut.

### What active inference adds

Three components the model does not have, each with a natural counterpart in
adaptive replanning:

| Active inference component | Radiotherapy counterpart |
|---|---|
| Latent states evolving between observations | Patient anatomy and setup for the next fraction |
| Generative model: action -> expected observation | Plan update -> expected delivered dose / next-fraction image |
| Expected free energy for action selection | Pragmatic term: expected KL to prescription (already exists). Epistemic term: information gain about patient-specific variance |

Without the observation loop, active inference collapses back into what the
optimizer already does. So the extension is only meaningful once
multi-phase data per patient exists, which is the same requirement the course
prior has.

### Feasibility assessment

Feasible in a bounded form:

1. Replace the adaptive-variance posterior with a calibrated one (Fisher
   diagonal first; Laplace or low-rank later). Prerequisite for everything
   below.
2. Treat the course prior as the belief state over patient-specific
   parameters. This is the hierarchical driver already prototyped in
   `python/pybrimstone/course_prior.py`.
3. Define the action space as the plan update between fractions and the
   observation as the next phase's anatomy (or delivered dose
   reconstruction).
4. Expected free energy per candidate action = expected prescription KL
   under the predictive distribution of the next phase, minus expected
   information gain about the patient latent. The pragmatic term reuses the
   existing objective evaluated over sampled phases; the epistemic term is
   the expected reduction in posterior entropy of the course latent.
5. Policy selection can start as a small discrete set (keep plan, replan on
   current phase, replan on pooled posterior mean) before any continuous
   policy search.

Not feasible in the current form: a single-phase, single-observation plan
has no dynamics to infer, and the covariance built from CG directions is not
a posterior. Both are addressed by steps 1 and 2 above.

### Open questions

- Which observation model for the thorax: 4D phases as the "next
  observation," or repeat imaging across fractions? The former is
  available in TCIA 4D-Lung; the latter is what the clinic actually sees.
- Whether the epistemic term ever changes the chosen action in practice.
  If it does not, active inference reduces to the course prior plus
  robust replanning, and the simpler framing should be kept.
- Entropy form. The separable entropy (`BRIMSTONE_ENTROPY_SEPARABLE=1`,
  W = 2e-4 from the sweep) is the working regularizer; the softmax form's
  W does not transfer. Any change to the variational family needs a new
  sweep.

## Immediate next steps

1. Decide the 2D fluence representation and beamlet storage strategy
   (section 1, scale consequences) before generating any library plans.
2. Confirm Morphome thorax data contents: structure sets, phases per
   patient, slice spacing.
3. Run one full-series thorax case through the current coplanar model as a
   baseline for timing and memory, using the existing automation.
