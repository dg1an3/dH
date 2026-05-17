# Hierarchical Bayes Design for Course-Level Treatment Planning

## Overview

This document specifies a hierarchical Bayes lift over the existing brimstone
single-phase optimizer. The deliverable is a Python-side driver that pools
information across phases of a treatment course via a Course-level latent,
producing DVH distributions (not just point estimates) and enabling adaptive
replanning workflows.

The design is intentionally minimal where it can be: the inner optimizer is
unchanged, the wrapper extension is one additional field in one callback, and
the new Python code is a single `ObjectiveTerm` subclass plus an outer-loop
driver. Most of the work is in validating that the existing adaptive-variance
state is calibrated enough to pool over — which is the open question this
design parks for an instrumentation pass rather than answering up front.

## Background: What the Existing System Provides

Three pieces of existing infrastructure determine the shape of this design.
References below cite source line numbers verified against the current branch.

**Single-phase variational Bayes (brimstone, in C++):**
- KL divergence term per structure (`KLDivTerm`)
- Adaptive variance `m_vAdaptVariance` updated as
  `1.0 / sum_of_orthogonalized_direction_contributions`
  (`RtModel/ConjGradOptimizer.cpp:348`). Shape resembles an inverse-curvature
  posterior precision; calibration TBD (see Open Question 1).
- Optional explicit free-energy computation toggled by
  `SetComputeFreeEnergy(true)`
  (`RtModel/include/ConjGradOptimizer.h:42`, `cpp:212`, with the
  `F = KL − Entropy` block at `cpp:352`).
- Four-level pyramid (`PlanPyramid::MAX_SCALES = 4`,
  `RtModel/include/PlanPyramid.h:22`) with active sigmas
  `{8.0, 3.2, 1.3, 0.5}` drawn from `DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25}`
  (`RtModel/PlanOptimizer.cpp:36`). The 5th array entry (0.25) is
  **dormant** — the loop at `PlanOptimizer.cpp:90, 284` only iterates
  `nLevel < MAX_SCALES`, so index 4 is never read. `CLAUDE.md`'s
  "4 levels: 8.0mm → 3.2mm → 1.3mm → 0.5mm" description is accurate
  to the active level count.

**Cython wrapper roadmap (`CYTHON_WRAPPER_DESIGN.md`):**
- `OptimizationCallback.on_iteration(iteration, level, cost, gradient_norm,
  beamlet_weights, dose) -> bool` (lines 115–128).
- `PlanOptimizer.add_objective(structure, ObjectiveTerm)` (line 77), where
  `ObjectiveTerm.evaluate(dose) -> (cost, gradient)` (line 100).
- Phase 5 already scopes "Custom objective functions from Python" (line 234).

**Sigma work in parallel (`docs/`):**
- `amortized_optimization_and_sigma_estimation.md` proposes
  `DynamicCovarianceOptimizer::SetPredictedAdaptiveVariance(...)`
  (lines 909–944) — σ flowing Python → C++.
- `Structure_Based_Sigma_Estimation_Methods.md` covers pyramid and histogram
  *kernel* sigmas — a different quantity from the adaptive variance σ this
  design pools over. The two efforts are orthogonal and can coexist.

## The Hierarchical Lift

**Generative model:**

```
η          ~ p(η)                       Course-level latent (per patient)
θ_p | η    ~ N(η, Σ_η)         p = 1..P phases
y_p | θ_p  ~ likelihood induced by KL_p(dose(θ_p) || target_p)
```

Here θ_p is a per-phase beamlet-weight vector (in the wrapper's flattened
representation), η is the Course-level "what the right intensity map looks
like for this patient, integrated across phases," and Σ_η is a diagonal
precision matrix learned alongside η.

**Inference via coordinate ascent at the Course level:**

```
repeat until η stabilizes:
    for each phase p in 1..P:
        run brimstone PlanOptimizer(plan_p) for K iterations
        harvest (μ_p, σ_p) from the final callback record
    pool: μ_η, σ_η = pool(μ_1..P, σ_1..P)          # see Pooling, below
    for each phase p:
        update phase_p.course_prior_term.target  = μ_η
        update phase_p.course_prior_term.weight  = 1 / σ_η²
```

Each inner `PlanOptimizer(plan_p)` call is unmodified brimstone — the prior
pull from η enters as one additional `ObjectiveTerm` (see below). The outer
loop is pure Python.

**Pooling (mean-field, diagonal):**

```
σ_η⁻² = (1/P) · Σ_p σ_p⁻²              # precision-weighted average
μ_η   = σ_η² · (1/P) · Σ_p σ_p⁻² μ_p
```

This is the closed-form Gaussian pooling for fixed Σ_η; if we want to estimate
Σ_η as a hyperparameter, the same loop carries an inverse-gamma update on the
diagonal precisions. Start with the closed form; only add the hyperparameter
update if η fails to stabilize.

**Why this is worth building:**
- DVH uncertainty bands as a first-class output (the deliverable that sells
  the MDDT story).
- Adaptive replanning gets principled: each new fraction's data is one more
  phase contributing to η.
- Semantic fault injection (eventual SIFPA work) wants to perturb posterior
  σ, not just μ — this design forces σ to be a first-class quantity at the
  Python boundary, which is what SIFPA observability requires too.

## Architectural Insight: Course Prior as Just Another `ObjectiveTerm`

The wrapper's `PlanOptimizer.add_objective(structure, ObjectiveTerm)`
interface (`CYTHON_WRAPPER_DESIGN.md:77`) already supports everything the
inner side of the hierarchical loop needs. A `CoursePriorTerm` is just an
`ObjectiveTerm` whose `evaluate(dose)` returns a quadratic penalty pulling
beamlet weights toward μ_η:

```python
class CoursePriorTerm(ObjectiveTerm):
    """Pulls per-phase beamlet weights toward a Course-level mean."""

    def __init__(self, target_mu: np.ndarray, precision: np.ndarray):
        self.target_mu = target_mu     # shape (n_beamlets,)
        self.precision = precision     # shape (n_beamlets,), = 1 / sigma_eta**2

    def evaluate(self, beamlet_weights: np.ndarray):
        # NB: prior is over beamlet weights, not dose — implementation may
        # bypass the dose-space evaluate() path and hook in at the
        # weight-space gradient stage. See "Wiring Note" below.
        diff = beamlet_weights - self.target_mu
        cost = 0.5 * np.sum(self.precision * diff * diff)
        grad = self.precision * diff
        return cost, grad
```

**Wiring note:** The Phase-5 `ObjectiveTerm` interface as currently specified
takes `dose`, not `beamlet_weights`. A prior over beamlet weights either (a)
needs a sibling `BeamletObjectiveTerm` base class added to the wrapper, or
(b) needs the prior expressed in dose space (a Gaussian pull on dose with
covariance computed from the dose-calc Jacobian — heavier but possibly
preferable on physical-interpretability grounds). Resolve as part of the
implementation; default recommendation is option (a) — smaller change,
preserves linear structure.

Either way, the optimizer doesn't need to know hierarchical Bayes exists; it
sees one more term added to its prescription.

## Required Wrapper Extension: Expose σ in the Callback

The current `on_iteration` callback signature (`CYTHON_WRAPPER_DESIGN.md:115–128`)
exposes `beamlet_weights` and `dose` but not the adaptive-variance state. The
outer loop needs `σ_p` per phase to pool, so the callback gains one field:

```python
class OptimizationCallback:
    def on_iteration(
        self,
        iteration: int,
        level: int,
        cost: float,
        gradient_norm: float,
        beamlet_weights: np.ndarray,
        sigma_weights: np.ndarray,    # NEW — mirrors m_vAdaptVariance
        dose: np.ndarray,
    ) -> bool:
        ...
```

Implementation cost: one additional Cython memoryview marshaling
`m_vAdaptVariance` (a `CVectorN<>` at `RtModel/include/ConjGradOptimizer.h:93`)
into a NumPy array. The state already exists in C++; we just don't surface it.

**Reconciliation with the amortized doc:** That doc proposes σ flowing
Python → C++ (`SetPredictedAdaptiveVariance`,
`amortized_optimization_and_sigma_estimation.md:909–944`). Hierarchical Bayes
needs σ flowing C++ → Python. Both directions are wanted; the marshaling
code is shared and should be implemented as one PR. Anyone touching the
wrapper for either reason should land both halves.

## The Calibration Open Question

`m_vAdaptVariance[nDim] = 1.0 / sum` at `RtModel/ConjGradOptimizer.cpp:348`
has the shape of an inverse-curvature posterior precision, where `sum`
accumulates contributions from orthogonalized conjugate-gradient search
directions. Whether the resulting σ is *calibrated* to actual posterior
variance — or is merely an optimizer-internal regularizer that happens to
live in the right shape — is unresolved. The hierarchical pooling formulas
above assume the former.

**Instrumentation experiment to settle this (one afternoon's work after the
callback extension lands):**

```
For one fixed plan, run brimstone twice with different random initial
beamlet weights. Compare the converged sigma maps σ_a and σ_b:

  shape stability     = corr(σ_a, σ_b) across beamlets
  magnitude stability = mean(σ_a) / mean(σ_b)

Decision table:
  shape ≳ 0.9, magnitude in [0.9, 1.1] → trust as posterior, pool directly
  shape ≳ 0.9, magnitude unstable      → normalize per phase before pooling
  shape < 0.9                          → optimizer trace, not posterior;
                                          use external variance (Hutchinson
                                          Fisher diag estimate, or bootstrap
                                          over voxel subsamples)
```

The cost of guessing wrong is concrete: in the third case, pooling produces a
spurious η whose σ_η doesn't track real posterior uncertainty, and the DVH
uncertainty bands at the end of the pipeline will be miscalibrated (likely
too tight). This makes the experiment a hard prerequisite for the DVH-band
deliverable, even though it's not blocking for the coordinate-ascent loop
itself.

### Experiment Result

Ran via `python/experiments/sigma_calibration.py` against the pure-Python
port of the algorithm. Three variants over 20 random initial parameter
seeds each:

| Variant                                  | Shape corr | Magnitude CV | Decision row |
|------------------------------------------|------------|--------------|--------------|
| 5-beamlet brimstone (PTV + 2 OAR)        | 0.18       | 2.74         | **ROW 3**    |
| 20-beamlet brimstone (more CG headroom)  | 0.02       | 0.23         | **ROW 3**    |
| 10-D ill-conditioned quadratic (control) | 0.84       | 0.065        | borderline   |

**Verdict: ROW 3 on realistic brimstone problems. m_vAdaptVariance is an
optimizer trace, not a calibrated posterior, and should NOT be pooled
directly across phases.**

What the control variant tells us: on a clean isotropic-ish quadratic
the algorithm *almost* produces a posterior (shape corr 0.84, magnitude
stable). So the σ formula is doing something curvature-related when
the optimization landscape is well-behaved. The realistic-problem
failure comes from (a) the sigmoid parameterization, which makes the
landscape flat in saturation regions and causes the Brent line search
to find degenerate stepsizes, and (b) the KL term's nonlinearity through
binning + convolution. Both effects produce orthogonal-basis columns
that don't reflect genuine Hessian information.

**Caveat:** The experiment ran on the pure-Python port
(`pybrimstone.phase_optimizer.PhaseOptimizer`), not on the C++ original.
The port is a faithful translation of `UpdateDynamicCovariance`
(`ConjGradOptimizer.cpp:281-349`), so behavior should transfer, but
the C++ side may diverge subtly (different VNL line search internals,
different convergence test gating). Re-run on the C++ optimizer once
the wrapper compile-verifies, before treating this as the final word.

### Recommendation

Use an external posterior-variance estimator for the hierarchical
pool, not the optimizer-internal `sigma_weights`. Three options in
increasing implementation cost:

1. **Hutchinson Fisher diagonal** — at convergence, estimate
   `diag(F)` where F = E[∇L ∇L^T] (per-voxel-perturbation gradient
   covariance) via M random Rademacher vectors:
   `diag(F)_i ≈ (1/M) Σ_m (v_m^T ∇L)^2 · δ_im` for each parameter i.
   Per-phase variance = `1 / diag(F)`. Same gradient code, no extra
   solves. Likely M ~ 10-30 to get stable estimates. Recommended
   first cut.
2. **Bootstrap over voxel subsamples** — re-run inner CG B times with
   random voxel subsamples; per-parameter variance = sample variance
   of the resulting μ. Higher cost (B full CG runs) but doesn't rely
   on the linearization implicit in Fisher.
3. **Full Laplace** — materialize the Hessian `∇²L(μ*)` and invert.
   Tractable for small (~50 beamlet) problems; impractical at clinical
   beamlet counts.

The `PhaseOptimizer.__call__` interface stays the same (returns
`(mu_p, var_p)`); only the source of `var_p` changes.

### Bootstrap Implementation + Re-run

Implemented `BootstrapPhaseOptimizer` in `python/pybrimstone/bootstrap.py`
as a drop-in replacement for `PhaseOptimizer`. Method: B inner CG
refits on random voxel subsamples (default 50% per structure), then
per-parameter sample variance across the converged μ vectors.

Re-running the calibration sweep with bootstrap variance (20 outer
seeds, B=15 refits each, 50% voxel subsample per structure):

| Variance source                            | Shape corr | Magnitude CV | Magnitude mean | Decision |
|--------------------------------------------|-----------:|-------------:|---------------:|----------|
| `m_vAdaptVariance` (sigma_weights)         |      0.18  |        2.74  |          0.13  | ROW 3    |
| Bootstrap (voxel subsample, B=15)          |  **0.94**  |        4.18  |       2642.71  | ROW 2    |

**Bootstrap recovers shape stability** (0.94 ≥ 0.9 threshold) — it
genuinely captures which beamlets are uncertain. That confirms the
diagnosis that `m_vAdaptVariance` is an optimizer trace; bootstrap
on the same problem produces real posterior structure.

**But magnitude is still unstable** (CV 4.18, mean ~10⁴ × the
sigma_weights mean). Diagnosis: this is the **line-search runaway**
fragility flagged in the end-to-end demo. The Brent line search
inside CG can find degenerate stepsizes that push optimizer-space
params to ~10⁷, where the sigmoid saturates. When this happens on a
single bootstrap refit out of B=15, the cross-refit variance for that
beamlet is dominated by the outlier and blows up. Bootstrap is
faithfully reporting the optimizer's runaway as posterior
uncertainty — which it technically is, but not the kind we wanted
to measure.

**Two paths forward:**

1. **Normalize per phase before pooling** (the ROW 2 prescription).
   Divide each phase's `var_p` by `mean(var_p)` before passing to
   `pool_phases`. The pool then weights phases by *relative* per-
   beamlet uncertainty, treating magnitude as an arbitrary scale.
   Cheap; one line in `HierarchicalBayes.run()`. Does not address
   the underlying CG fragility.
2. **Fix the line-search runaway**. Replace Brent line minimization
   with a bounded variant (clip lambda to a sensible range, or use a
   trust-region step instead of an unbounded line search). Then
   bootstrap magnitudes should stabilize too, moving the verdict to
   ROW 1 (calibrated posterior, pool directly). Larger change; touches
   the inner optimizer.

Option 1 unblocks hierarchical Bayes today with a defensible (if
slightly weaker) variance estimate. Option 2 is the principled fix
and benefits the rest of the pipeline (the demo plot's saturated
peak goes away too). They're complementary.

### Implemented Both Fixes

**Bounded line search** (option 2) — `polak_ribiere_cg` and
`brent_line_minimize` gained a `max_step_norm` kwarg. When set, the
step `lambda * direction` is clipped to L2 norm `max_step_norm` in
parameter space, preventing Brent from wandering into sigmoid-
saturation regions where the cost is flat and `lambda` can balloon.
`PhaseOptimizer` and `BootstrapPhaseOptimizer` default to
`max_step_norm=20.0` (large enough not to bind on legitimate steps;
small enough to catch sigmoid saturation, which starts at
`|x|·SIGMOID_SCALE > ~5`). Backward-compatible: `polak_ribiere_cg`
defaults to `max_step_norm=None` so existing tests keep passing.

**Normalize-before-pool** (option 1) — `pool_phases` and
`HierarchicalBayes` gained a `normalize` / `normalize_variance` flag.
When True, each phase's variance vector is rescaled to mean 1 before
pooling, so the precision-weighted combination uses only the
*relative* per-beamlet uncertainty shape and ignores absolute
magnitude. Default off (preserves prior behavior).

Re-running the calibration sweep with both fixes in place:

| Variance source                                       | Shape corr | Magnitude CV | Magnitude mean |
|-------------------------------------------------------|-----------:|-------------:|---------------:|
| `m_vAdaptVariance` (sigma_weights)                    |      0.18  |        2.74  |          0.13  |
| Bootstrap (before fixes)                              |      0.94  |        4.18  |       2642.71  |
| **Bootstrap with bounded line search**                |  **0.96**  |    **0.41**  |     **80.26**  |
| Bootstrap with bounded line search + normalize-pool   |  effectively ROW 1 once pooled  |        |

The bounded line search alone gave a **10× improvement in magnitude
stability** (CV 4.18 → 0.41) and a **33× reduction in magnitude
scale** (mean 2643 → 80). The end-to-end demo plot's runaway behavior
disappeared as well — optimizer-space params now stay in the [-30,
30] range, beamlet-space weights distribute across non-saturated
values, and `var_eta` actually varies per-beamlet (instead of being
stuck at the default fallback).

The remaining 0.41 magnitude CV is below 0.10 only with
normalize-before-pool turned on. The hierarchical pool now operates
on shape-stable, magnitude-normalized variances — the ROW 2
prescription, implemented cleanly.

## Pyramid Level Strategy

The 4-level pyramid (8.0 → 0.5 mm active voxels) raises a where-to-apply
question for the Course prior:

**Option A — finest-only (recommended for prototype):**
The hierarchical outer loop wraps level-0 optimization only. Coarser levels
(3 through 1) run independently per phase, then the prior pull is enabled
for level 0. Clean, easy to reason about, and σ at level 0 means roughly
what we want it to mean.

**Option B — progressive across levels:**
Prior at all levels with weight scaled by level (e.g., weight ∝ voxel
spacing⁻²). More aggressive shrinkage but σ at coarse levels covers far more
parameters per dimension and may not be commensurable with σ at fine levels
without renormalization.

Recommendation: Option A for the first prototype. Revisit after the
calibration experiment — if σ is well-calibrated at all levels, Option B is
strictly more powerful.

*Aside on the dormant 5th sigma:* if `MAX_SCALES` is ever bumped to 5
to activate the 0.25 mm level, the σ-calibration question gets sharper
at the new bottom-of-pyramid (less per-voxel data per beamlet), and
Option B becomes more attractive. Worth keeping in mind but out of
scope here.

## Prototype Path

Execute in this order; each step is independently testable.

**Step 1 — Extend the callback (wrapper change).**
Add `sigma_weights` to `OptimizationCallback.on_iteration` and marshal
`m_vAdaptVariance` into it. Coordinate with the amortized doc's
Python→C++ direction so both flow through one PR. Test: a no-op callback
that records `sigma_weights` per iteration should show the array changing
shape with pyramid level and converging in magnitude across iterations.

**Step 2 — Implement `CoursePriorTerm`.**
Pure Python, subclass of `ObjectiveTerm` (or `BeamletObjectiveTerm` per the
Wiring Note above). Test as a standalone regularizer on a single plan:
behavior should match ridge regression on beamlet weight space.

**Step 3 — `HierarchicalBayes` driver.**
Instantiates P `PlanOptimizer` objects (one per phase), runs each for K
iterations via `optimize(callback=...)`, harvests `(μ_p, σ_p)` from the
final callback record, pools to `(μ_η, σ_η)`, updates each
`CoursePriorTerm`, repeats. Coordinate ascent at the Course level.
Convergence diagnostic: η should stabilize across outer iterations.

*Caveat surfaced during implementation:* the σ_p harvested from each
phase must reflect the **data-only** posterior variance, not the
joint (data + prior) posterior. If the phase reports the joint
variance — which is what `m_vAdaptVariance` will tend to do if the
prior is enabled during the inner solve — pooling re-counts the prior
precision and the outer-loop dynamics degrade from geometric to
O(1/n) convergence. Either (a) compute σ_p with the prior disabled
in the inner solve, (b) subtract the prior precision before pooling
(`σ_p,data⁻² = σ_p,joint⁻² − λ_prior`), or (c) accept slow convergence
as evidence the inner solve is operating in a regime where σ is
dominated by the prior. The Step-3 reference implementation in
`python/pybrimstone/hierarchical_bayes.py` is correct either way;
the responsibility falls on the phase-optimizer wrapper.

**Step 4 — Validate against explicit free energy.**
Turn on `SetComputeFreeEnergy(true)` per phase, sum across phases plus the
Course prior energy:

```
F_total = Σ_p F_p + KL(q(η) || p(η))
```

Coordinate ascent on the hierarchical model should monotonically decrease
`F_total`. If it doesn't, the pooling math is wrong (most likely culprit:
double-counting the prior precision, since it appears both in the inner
optimization as the `CoursePriorTerm` weight and in the pooling formula).

*Implementation status:* Python reference in
`python/pybrimstone/free_energy.py`; F_total decomposes as
`Σ_p [data_cost_p(μ_p) + 0.5 Σ precision_eta · (μ_p − μ_η)² − H(q_p)]`
with H the diagonal Gaussian entropy. Acceptance test under analytical
phase mocks with data-only variance reporting confirms strict monotone
decrease across outer iterations. A contrasting test under
joint-variance reporting confirms the failure mode predicted in Step 3 —
F_total drifts upward — so the diagnostic actually catches the
double-counting bug rather than just rubber-stamping correctness. The
C++ counterpart wires `DynamicCovarianceOptimizer::GetFreeEnergy()`
(`ConjGradOptimizer.h:54`) per phase plus a Course-level cross term;
that's a follow-up once the wrapper exposes both halves.

**Step 5 — DVH uncertainty bands (the deliverable).**
Sample beamlet weights from the converged per-phase `q(θ_p) = N(μ_p, σ_p²)`,
recompute dose for each sample (parallel `optimizer.optimize()`-free dose
calc — pure forward pass), bin into structure DVHs, plot 5th/50th/95th
percentile bands per structure. This is what gets demoed.

*Implementation status:* Python reference in
`python/pybrimstone/dvh_uncertainty.py` with the full pipeline
`sample_phase_posterior → compute_dose → compute_dvh → compute_dvh_bands`
plus a top-level `dvh_uncertainty_bands(...)` and a `plot_dvh_bands(...)`
matplotlib helper (imported lazily). `compute_dose` accepts either a
beamlet-to-dose matrix or a callable, so the real C++ TERMA + kernel
convolution can slot in once exposed through the wrapper without
changing the rest of the pipeline. The demo image generated from a
synthetic 2-phase plan shows the customer-facing artifact.

## Phasing Within the Wrapper Roadmap

Slots into `CYTHON_WRAPPER_DESIGN.md` as "Phase 4.5":
- Depends on Phase 3 (callbacks) **plus** the σ extension above.
- Depends on Phase 5's "custom objective functions from Python" capability for
  `CoursePriorTerm`. Strictly, this makes it "Phase 5.5" — but the σ
  extension and the `CoursePriorTerm` design can be specified now and
  implemented as soon as those phases land.

Nothing here blocks Phases 1–4. The visualization work in Phase 4
(pymedphys integration) plus this design's Step 5 together produce the
DVH-with-uncertainty-bands artifact essentially for free.

## Validation

Four diagnostics, in order of strictness:

1. **Stabilization:** η_t converges across outer iterations
   (`||η_{t+1} − η_t|| / ||η_t||` decays).
2. **Energy:** total `F = Σ_p F_p + KL(q(η)||p(η))` decreases monotonically
   under the coordinate-ascent updates.
3. **Posterior calibration:** bootstrap re-runs of a single phase give σ maps
   stable in shape and magnitude (the instrumentation experiment above).
4. **Clinical:** DVH uncertainty bands cover the empirical phase-to-phase
   variability on a held-out plan within their nominal coverage probability.

(1) and (2) are necessary for correctness. (3) is necessary for the bands
in (4) to mean anything. (4) is the customer-facing claim.

## Connection to Existing Work

- **`CYTHON_WRAPPER_DESIGN.md`** — primary integration point. Extends the
  callback signature; uses `add_objective` for `CoursePriorTerm`.
- **`docs/amortized_optimization_and_sigma_estimation.md`** — shares the σ
  marshaling code (opposite direction). The `SetPredictedAdaptiveVariance`
  proposal there and the `CoursePriorTerm` here both need σ to be a
  first-class quantity at the Python boundary; both should land together.
- **`docs/Structure_Based_Sigma_Estimation_Methods.md`** — orthogonal. Those
  methods estimate pyramid and histogram *kernel* sigmas. The adaptive
  variance σ pooled here is a different quantity that lives at a different
  layer. Both can coexist; neither blocks the other.

## Documentation Cleanup

Initial drafts of this doc incorrectly described the pyramid as having
five active levels, conflating the 5-entry `DEFAULT_LEVELSIGMA[]` array
with the level count. The pyramid actually has **4 active levels**
(`PlanPyramid::MAX_SCALES = 4`, `RtModel/include/PlanPyramid.h:22`);
the 5th array entry is unreachable from the optimizer loop. `CLAUDE.md`
is accurate on this point. The Step-3 and Step-5 implementations don't
depend on the level count at all (they operate on the converged
posterior, not the pyramid stages), so the correction is purely
documentary.

One minor inaccuracy worth flagging in `CLAUDE.md`: the "Modifying
optimization parameters" section says "Pyramid level sigmas:
`DEFAULT_LEVELSIGMA[]` in PlanPyramid" — the array is actually defined
in `RtModel/PlanOptimizer.cpp:36`, not in PlanPyramid. Trivial fix.

## Open Questions

1. **σ calibration** — *Resolved.* The instrumentation experiment above
   (`python/experiments/sigma_calibration.py`) shows that
   `m_vAdaptVariance` is an optimizer trace, not a calibrated posterior,
   on realistic brimstone problems. Use an external variance estimator
   for the hierarchical pool (Hutchinson Fisher diagonal recommended as
   the first cut). The pool formulas themselves are unchanged; only the
   source of `var_p` changes. Re-verify against the C++ optimizer once
   the wrapper compile-verifies before treating as final.
2. **Pooling level** — for multi-fraction adaptive replanning we may want a
   three-level hierarchy: Population → Patient → Phase, with the Population
   prior learned across patients. Out of scope for the prototype; flagged
   so the `HierarchicalBayes` driver's interface doesn't paint us into a
   two-level corner.
3. **Prior weight as hyperparameter vs. fixed by Bayes** — the
   `CoursePriorTerm.weight` is `1/σ_η²` in the strict-Bayes reading, but
   the existing brimstone prescription combines terms with hand-tuned weights.
   First prototype: use the Bayes-fixed weight; treat global rebalancing as
   a single tunable scalar applied to the whole `F_total`.
4. **`evaluate(dose)` vs. `evaluate(beamlet_weights)`** — *Resolved during
   Step 2 implementation.* Adopted option (a): added a `BeamletObjectiveTerm`
   base class as a sibling of the dose-space `ObjectiveTerm`. `CoursePriorTerm`
   subclasses it. See `python/pybrimstone/course_prior.py`. The dose-space
   `ObjectiveTerm` interface in `CYTHON_WRAPPER_DESIGN.md` is unchanged.

## References

- `CYTHON_WRAPPER_DESIGN.md`
- `docs/amortized_optimization_and_sigma_estimation.md`
- `docs/Structure_Based_Sigma_Estimation_Methods.md`
- `RtModel/include/ConjGradOptimizer.h` — `SetComputeFreeEnergy` (line 42),
  `m_bComputeFreeEnergy` (line 82), `m_vAdaptVariance` (line 93)
- `RtModel/ConjGradOptimizer.cpp` — `SetComputeFreeEnergy` body (line 212),
  adaptive-variance update (line 348), free-energy block (line 352)
- `RtModel/PlanOptimizer.cpp` — `DEFAULT_LEVELSIGMA` (line 36), per-level
  sigma lookup (line 294)
- `CLAUDE.md` — architectural overview (level count stale)
