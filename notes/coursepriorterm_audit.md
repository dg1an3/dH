# CoursePriorTerm Audit

> Quick orientation note for the amortized-course-prior prototype. Not a design
> doc — just "where the thing actually lives and what shape it has".

## tl;dr

`CoursePriorTerm` exists as a real class in
[`python/pybrimstone/course_prior.py`](../python/pybrimstone/course_prior.py).
It is exactly the quadratic pull described in the prototype brief:

```
cost(w)  = 0.5 * Σ_i precision[i] * (w[i] - target_mu[i])**2
grad(w)  = precision * (w - target_mu)
```

This is the term the amortized network `f_θ` is approximating. The
prototype's `CourseState = (mu, sigma_inv)` maps 1:1 onto
`CoursePriorTerm(target_mu, precision)`.

## Where it sits in the stack

```
HierarchicalBayes.run()                # python/pybrimstone/hierarchical_bayes.py
  └─ for outer_iter:
        for (phase_optimizer, prior) in zip(...):
            mu_p, var_p = phase_optimizer(prior)   # ← inner CG, SLOW
        mu_eta, var_eta = pool_phases(mus, vars)
        for prior in priors:
            prior.target_mu = mu_eta
            prior.precision = 1.0 / var_eta
```

The inner `phase_optimizer(prior)` is a callable. In the current code it is
`PhaseOptimizer.__call__` (`python/pybrimstone/phase_optimizer.py`), which
installs the prior into a `Prescription`, runs `polak_ribiere_cg`, and harvests
the final beamlet weights plus the adaptive-variance vector.

The amortized prototype slots in **here**: replace (or hot-start) the inner
callable with a learned `f_θ(x_phase_init, course_state)` that predicts
`Δx_phase ≈ x_phase_star - x_phase_init`.

## Concrete shapes (for `types.py`)

From reading `CoursePriorTerm.__init__`:

| field          | shape           | meaning                                                     |
| -------------- | --------------- | ----------------------------------------------------------- |
| `target_mu`    | `(n_beamlets,)` | course-level posterior mean, in **beamlet-weight** space    |
| `precision`    | `(n_beamlets,)` | `1 / sigma_eta²`; diagonal — no off-diagonal terms exist    |

There is no batch axis. There is no per-voxel quantity. There is no segment
or fluence-map structure visible at this layer — the term operates on a flat
beamlet-weight vector. Whatever shape the underlying dose operator has is
flattened by the time it reaches `CoursePriorTerm.evaluate`.

## What `phase_optimizer` returns (and therefore what the amortized net targets)

From `PhaseOptimizer.__call__`:

```python
x_final, _, _, _ = polak_ribiere_cg(...)         # unbounded optimizer params
var_p = np.maximum(last_sigma[0], 1e-6)           # adaptive variance
return x_final, var_p
```

Note: `x_final` is in **optimizer-parameter space** (pre-sigmoid), not
beamlet-weight space. `CoursePriorTerm`, however, lives in beamlet-weight
space. The `Prescription` bridges the two via `numerics.transform`
(sigmoid).

**Resolved: predict in beamlet-weight space (post-sigmoid).** Derek's
Phase 0 decision. The Phase 1b real-data generator applies `transform`
to the CG-trajectory optimizer params before storage; `Sample.x_init`,
`Sample.delta_x`, and `Sample.trajectory[t]` are all in
beamlet-weight space, matching the natural language of `CoursePriorTerm`
itself. Beamlet-weight space is bounded `(0, INPUT_SCALE=0.5)` so the
network output is also implicitly bounded; we do not currently constrain
this via the network head (a softplus/sigmoid head is a later concern
if predictions drift outside the bounds).

## Trajectory recording

**Resolved: record the full CG trajectory.** Derek's Phase 1b decision.
Each `Sample` from `generate_real_samples` carries
`trajectory: List[np.ndarray]` of length `n_iter + 1`, starting with
`x_init` and ending with `x_star`, each entry in beamlet-weight space.
`expand_trajectory_to_samples` is the canonical way to turn this into
per-step training tuples for curriculum / imitation training.

No Cython hooks were needed for this — `polak_ribiere_cg` already
surfaces `x` per iteration through its existing `callback` argument
(see `python/pybrimstone/numerics/conjugate_gradient.py:188`).

## Files referenced

- `python/pybrimstone/course_prior.py` — the term itself, 97 lines, no surprises.
- `python/pybrimstone/hierarchical_bayes.py` — outer loop + `pool_phases`.
- `python/pybrimstone/phase_optimizer.py` — current inner callable.
- `python/pybrimstone/objective_terms.py` — `BeamletObjectiveTerm` base class.
- `python/pybrimstone/prescription.py` — composite objective, sigmoid bridge.
- `HIERARCHICAL_BAYES_DESIGN.md` — full design doc.
- `HIERARCHICAL_BAYES_FOLLOWUPS.md` — known issues / open questions.
