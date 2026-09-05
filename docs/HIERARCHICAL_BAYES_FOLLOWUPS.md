# Hierarchical Bayes — Follow-Up Items

Context: this document captures what's been settled across PRs #41
(`claude/hierarchical-bayes-design-RzNrS`) and #42
(`claude/sigma-calibration-experiment`), and what remains. The
canonical architecture is in `HIERARCHICAL_BAYES_DESIGN.md`; this is
the action list for picking up the work later without re-reading the
whole design doc.

## What's Settled

**Hierarchical-Bayes architecture (PR #41).** Five-step prototype path
all implemented in pure Python:

1. Adaptive variance surfaced through callback (`sigma_weights` kwarg).
2. `CoursePriorTerm` as a `BeamletObjectiveTerm` (Open Question 4
   resolved with the sibling base class option).
3. `HierarchicalBayes` coordinate-ascent driver + `pool_phases`
   precision-weighted Gaussian combination.
4. Variational free-energy monotonicity diagnostic (catches the
   double-counting failure mode).
5. DVH uncertainty-band sampling + `plot_dvh_bands` matplotlib helper.

**Pure-Python brimstone port (PR #41).** Lifted from the test-file
references into `pybrimstone.numerics` + `objective_terms.py` +
`kl_term.py` + `prescription.py` + `phase_optimizer.py`. KLDivTerm's
analytical gradient matches finite differences. Prescription routes
gradients through sigmoid transform + dose-calc adjoint.

**σ-calibration question (PR #42).**

- `m_vAdaptVariance` (the C++ optimizer's adaptive variance, surfaced
  as `sigma_weights`) is an **optimizer trace, not a calibrated
  posterior** — confirmed on three problems (5-beamlet brimstone,
  20-beamlet brimstone, 10-D ill-conditioned quadratic control).
  Don't pool it directly across phases.
- **Bootstrap-over-voxel-subsamples** *is* a real posterior estimator,
  but its quality depends on **beamlet identifiability**. On well-
  identified geometries (Gaussian-bump dose op, or TERMA with spread-
  out beamlet entry positions) it gives stable shape (corr 0.9+). On
  degenerate single-angle fans where beamlets share dose paths
  (pairwise cosine similarity > 0.5) it gives 0.28 shape — bootstrap
  honestly reports the lack of identifiability rather than producing
  a posterior that isn't there.
- Two complementary fixes for magnitude instability landed:
  **bounded line search** (`max_step_norm=20` in CG by default) and
  **normalize-before-pool** (`HierarchicalBayes(normalize_variance=True)`).
  Together they put bootstrap into ROW 2 on well-identified problems
  and unblock the hierarchical pool with a defensible variance source.

**TERMA + kernel dose calculator (PR #42).** Two-step physical model:
ray-traced TERMA along +z through a 3-D density volume, then
isotropic Gaussian kernel convolution. Linear matrix operator that
slots into `Prescription` unchanged. Tested against Beer-Lambert
(decay ratio matches `exp(-mu·rho·dz)` to 1e-6) and energy
conservation.

## Follow-Up Items

Items below are ordered by impact-per-effort, not strictly required
sequence. Cross-references to existing files are given so a reader
doesn't have to grep.

### 1. Multi-beam TERMA

**Motivation.** The TERMA calibration finding ("bootstrap shape
correlation drops to 0.28 on collinear beamlets, recovers to 0.72
when beamlets are spread laterally") predicts that **real clinical
plans with multi-angle beams** should be identifiable and bootstrap
should behave well. This is the most consequential prediction the
calibration work made; it should be tested.

**What to do.** Extend `TermaKernelDoseCalc` to accept a list of beam
directions (gantry angles). For each beam, ray-trace through the
rotated CT volume to compute that beam's TERMA. Sum across beams.
The matrix operator stays the same shape, just denser.

**Concrete plan.**
- New constructor argument `beam_directions`: list of (theta_gantry,
  theta_couch) tuples, or a list of unit vectors.
- Per-beam ray-tracing: rotate the density volume into beam frame
  via `scipy.ndimage.rotate` (or, more efficiently, rotate the ray
  paths through the unrotated volume — Siddon-style).
- Per-beam `beamlet_centers_2d` in that beam's entry plane.
- Sum the per-beam dose-operator columns into a single
  `(n_voxels, sum_of_beamlets)` matrix.

**Verification.** Re-run the σ-calibration sweep on a 5-beam multi-
angle plan. Expected: bootstrap shape correlation in ROW 1 (≥0.9).
If not, the identifiability story is more nuanced than this branch
of analysis assumed.

**File pointers.**
- `python/pybrimstone/terma_kernel_dose.py:31` — single-direction
  axis-aligned implementation.
- `python/experiments/sigma_calibration.py:182` —
  `_build_terma_calc` for the calibration variant; clone for
  multi-beam.

**Effort estimate.** ~2-3 hours. The rotation math is the only
non-trivial piece; Siddon ray-tracing is well-documented but
fiddly to get bin-count-correct.

### 2. Hutchinson Fisher diagonal variance estimator

**Motivation.** Option 3 in the calibration recommendation (after
sigma_weights and bootstrap). Doesn't depend on voxel-subsample
identifiability, so should produce stable shape even on the
degenerate collinear-beamlet TERMA case where bootstrap dropped to
0.28. Comparing it head-to-head with bootstrap closes the
calibration question definitively.

**What to do.** Estimate `diag(F)` where F is the Fisher information
matrix (or just the gradient covariance at the data, for a maximum-
likelihood estimator) via random Rademacher projections:

```
v_m ~ Rademacher (random ±1 vector, m = 1..M)
g_m = grad_L(mu_p + epsilon * v_m) - grad_L(mu_p - epsilon * v_m)
diag(F)_i ≈ (1 / (2*epsilon*M)) * sum_m (v_m)_i * (g_m)_i^2
var_p = 1 / diag(F)
```

M ~ 10-30 should suffice. Reuses the existing
`Prescription.evaluate()` gradient code — no new gradient
implementation needed.

**Concrete plan.**
- `python/pybrimstone/fisher.py`: `HutchinsonFisherPhaseOptimizer`
  drop-in replacement for `PhaseOptimizer`, same `(prior) ->
  (mu_p, var_p)` signature.
- Run the existing `--mode terma` calibration sweep on Fisher
  variance and compare against bootstrap on both the collinear
  and spread-beamlet TERMA setups.

**Verification.** Expected result: Fisher shape correlation ≥ 0.9
on both collinear and spread variants (because it doesn't rely on
identifiability through subsampling). If true, Fisher is the right
default variance source for hierarchical Bayes; bootstrap is an
alternative for cases where you specifically want to capture data-
dependent uncertainty.

**File pointers.**
- `python/pybrimstone/bootstrap.py` — template for the new
  class structure.
- `HIERARCHICAL_BAYES_DESIGN.md` — "The Calibration Open Question"
  → "Recommendation" lists Hutchinson Fisher as option 1.

**Effort estimate.** ~1 day. Bulk is the Rademacher-projection
implementation + tests; calibration re-run is mechanical.

### 3. Wire the C++ side: GetAdaptiveVariance + bounded line search

**Motivation.** PR #41 added the C++ accessor `GetAdaptiveVariance()`
in `ConjGradOptimizer.h` and the matching pybind11 binding, but
neither has been compile-verified — the Linux container can't build
the MFC-dependent C++ side. Same goes for porting the bounded line
search finding (Python-side, currently lives in
`pybrimstone.numerics.conjugate_gradient`) back into the C++
`ConjGradOptimizer::minimize` Brent step.

**What to do.**
1. On a Windows machine with the Brimstone Visual Studio toolchain,
   build the RtModel + pybind11 wrapper. Verify the accessor compiles
   and the binding round-trips a vector to numpy.
2. Add a `max_step_norm` member to `DynamicCovarianceOptimizer` (or
   the equivalent vnl-Brent wrapper) and clip the search lambda after
   the line minimization. Default to a sensible value (e.g., 20.0)
   that doesn't bind on normal optimization.

**Verification.** Re-run the σ-calibration experiment against the C++
optimizer (via the wrapper) and confirm:
- The pure-Python finding (m_vAdaptVariance is an optimizer trace)
  reproduces on the C++ side.
- The bounded line search resolves the sigmoid-saturation runaway
  in the C++ pipeline too.

**File pointers.**
- `RtModel/include/ConjGradOptimizer.h:42` —
  `GetAdaptiveVariance()` accessor added by PR #41.
- `python/rtmodel_bindings.cpp` — pybind11 binding (uncompiled).
- `python/pybrimstone/numerics/conjugate_gradient.py` —
  `max_step_norm` Python reference implementation.

**Effort estimate.** ~half-day on a working Windows build; longer if
the wrapper or RtModel itself has accumulated build-system rot.

### 4. Read the real MC-computed kernel files

**Motivation.** `TermaKernelDoseCalc` currently uses an isotropic
Gaussian as the energy deposition kernel. The repo ships real Monte-
Carlo-computed kernels in `Brimstone/6MV_kernel.dat`,
`Brimstone/15MV_kernel.dat`, `Brimstone/2MV_kernel.dat`. Using these
would make the dose model significantly more representative of real
megavoltage photon physics.

**What to do.**
1. Inspect the `.dat` files to determine the storage format
   (presumably tabulated as a function of polar angle and radial
   distance — the standard form for spherical convolution kernels).
2. Parse into a (n_radii, n_angles) array.
3. Construct the 3-D kernel by interpolating onto a Cartesian voxel
   grid centered on the kernel origin. The radial × polar
   tabulation gives an axially-symmetric kernel; rotate as needed
   per beam direction.
4. Convolve with TERMA instead of the Gaussian filter.

**Verification.** Compare dose profiles for a single uniform-density
slab against published depth-dose curves for the same beam energy.
Should match within a few percent at the depth of maximum.

**File pointers.**
- `Brimstone/6MV_kernel.dat`, `15MV_kernel.dat`, `2MV_kernel.dat` —
  in repo, format unknown to this work.
- `python/pybrimstone/terma_kernel_dose.py:154` —
  `dose_for_beamlet` where the Gaussian filter is applied.

**Effort estimate.** ~1-2 days, mostly figuring out the format and
getting the kernel rotation right.

### 5. Three-level hierarchy: Population → Patient → Phase

**Motivation.** Open Question 2 in `HIERARCHICAL_BAYES_DESIGN.md`.
For multi-fraction adaptive replanning across a patient cohort, the
natural model is a third level above Course: a Population prior
learned from many patients, individualizing into per-patient priors,
which then govern per-phase optimization.

**What to do.** Generalize `HierarchicalBayes` from two levels
(phase → eta) to N levels (phase → patient → ... → population) with
the same coordinate-ascent + pool_phases structure recursively
applied. Or stay with two levels but expose `eta` as a child of a
fixed Population prior that the user supplies.

**Verification.** Synthetic two-patient × 3-phase test where the
population prior is known; check both per-patient and per-phase
posteriors recover sensibly.

**File pointers.**
- `HIERARCHICAL_BAYES_DESIGN.md` — Open Question 2.
- `python/pybrimstone/hierarchical_bayes.py:78` —
  `HierarchicalBayes` class to extend.

**Effort estimate.** ~2 days if generalizing to N-level; ~half-day
if just adding a fixed Population prior at the top.

## Stretch / Research Items

- **Spatial-regularization prior** to break beamlet degeneracy on
  single-angle plans (alternative to the multi-beam fix in item 1).
  Could be a TV regularization or a smoothness prior on the beamlet
  intensity map. Would let SBRT-style single-angle plans use
  bootstrap meaningfully.
- **Per-pyramid-level Course prior**: design-doc Option B in the
  Pyramid Level Strategy section. Currently the prototype acts only
  at the finest pyramid level (Option A). Progressive application
  across levels could speed convergence but requires comparable σ
  semantics across levels, which the current optimizer doesn't
  guarantee.
- **Calibration on real DICOM plans** once the C++ wrapper builds
  (item 3 above). Replace the synthetic geometry with a real CT +
  RT-Struct + RT-Plan and re-run the calibration sweep. This is
  the gating step before any clinical/regulatory claim about DVH
  band coverage.

## Pointers to Existing Code

| File | Purpose |
|------|---------|
| `HIERARCHICAL_BAYES_DESIGN.md` | Canonical architecture + experiment record |
| `python/pybrimstone/objective_terms.py` | `BeamletObjectiveTerm` / `DoseObjectiveTerm` base |
| `python/pybrimstone/course_prior.py` | `CoursePriorTerm` |
| `python/pybrimstone/kl_term.py` | `KLDivTerm` (analytical gradient) |
| `python/pybrimstone/prescription.py` | `Prescription` composer + gradient routing |
| `python/pybrimstone/phase_optimizer.py` | `PhaseOptimizer` (CG + adaptive variance) |
| `python/pybrimstone/bootstrap.py` | `BootstrapPhaseOptimizer` + `subsample_mask` |
| `python/pybrimstone/hierarchical_bayes.py` | Driver + `pool_phases` (`normalize_variance` option) |
| `python/pybrimstone/free_energy.py` | F_total monotonicity diagnostic |
| `python/pybrimstone/dvh_uncertainty.py` | Step 5 deliverable: sampling + percentile bands |
| `python/pybrimstone/dose_calc.py` | `gaussian_bump_dose_operator` (prototype stand-in) |
| `python/pybrimstone/terma_kernel_dose.py` | `TermaKernelDoseCalc` (physical, single-direction) |
| `python/pybrimstone/numerics/` | Lifted histogram + KL + transform + CG primitives |
| `python/experiments/sigma_calibration.py` | Calibration sweep (4-mode CLI) |
| `python/tests/` | 260 tests, all passing under `--noconftest` |
