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

This is a subtle point worth flagging to Derek before training:

  - **The amortized net could predict in optimizer-param space**
    (matches the CG return), OR
  - **in beamlet-weight space** (matches the prior's language).

For the toy phase (5a) this is moot — the toy is a closed-form quadratic
without the sigmoid wrapper. For the real-data phase (5b) we have to pick.
Leaning toward optimizer-param space because that's what the downstream
CG-polish step expects, but documenting the choice explicitly.

## Files referenced

- `python/pybrimstone/course_prior.py` — the term itself, 97 lines, no surprises.
- `python/pybrimstone/hierarchical_bayes.py` — outer loop + `pool_phases`.
- `python/pybrimstone/phase_optimizer.py` — current inner callable.
- `python/pybrimstone/objective_terms.py` — `BeamletObjectiveTerm` base class.
- `python/pybrimstone/prescription.py` — composite objective, sigmoid bridge.
- `HIERARCHICAL_BAYES_DESIGN.md` — full design doc.
- `HIERARCHICAL_BAYES_FOLLOWUPS.md` — known issues / open questions.
