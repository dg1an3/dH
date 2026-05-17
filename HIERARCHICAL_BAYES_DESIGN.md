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
- Five-level pyramid with `DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25}`
  (`RtModel/PlanOptimizer.cpp:36`).
  *Note:* `CLAUDE.md` and `CYTHON_WRAPPER_DESIGN.md` both describe four
  levels — those references are stale; source of truth is the array above.

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

## Pyramid Level Strategy

The 5-level pyramid (8.0 → 0.25 mm voxels) raises a where-to-apply question
for the Course prior:

**Option A — finest-only (recommended for prototype):**
The hierarchical outer loop wraps level-0 optimization only. Coarser levels
(4 through 1) run independently per phase, then the prior pull is enabled
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

## Documentation Cleanup Identified

This design depends on facts that are stated incorrectly in other docs:

- `DEFAULT_LEVELSIGMA[]` has **5** entries (`{8.0, 3.2, 1.3, 0.5, 0.25}`,
  `PlanOptimizer.cpp:36`), not 4. `CLAUDE.md` and `CYTHON_WRAPPER_DESIGN.md`
  both say "4 levels" and should be updated. (Not part of this design; flagged
  for a follow-up doc PR.)

## Open Questions

1. **σ calibration** — settled by the instrumentation experiment above.
   Blocking only for the band-coverage claim in validation diagnostic 4.
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
