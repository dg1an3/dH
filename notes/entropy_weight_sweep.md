# Entropy regularizer weight (W) sweep

Date: 2026-07-20

Sweep of `BRIMSTONE_ENTROPY_WEIGHT` (referred to as *W*) over the separable
entropy regularizer, plus a beam-count comparison. Written up because the
previous determination of W left no record, and re-deriving it cost a session.

## Objective under test

`Prescription::operator()` (`RtModel/Prescription.cpp:388-467`) computes

```
F = KL - w * H
```

Two forms of `H`, selected by `BRIMSTONE_ENTROPY_SEPARABLE`:

- **softmax** (default, `w != 0`): `p = softmax(vInput)`, `H = -sum p_i log p_i`.
  Bounded by `log(nDim)`. Globally coupled -> dense Hessian; the code notes CG
  stalls at w=0.01.
- **separable** (`BRIMSTONE_ENTROPY_SEPARABLE=1`): `q_i = Sigmoid(x_i)`,
  `H = sum_i [-q_i ln q_i - (1-q_i) ln(1-q_i)]`. Bounded by `nDim * ln2`.
  Diagonal Hessian. Pushes each beamlet toward q=0.5.

**W does not transfer between the two forms.** Their `H` differ by ~10 orders of
magnitude on the same plan, so a W tuned for one is meaningless in the other.

## Configuration

| | |
|---|---|
| exe | `x64/Release/Brimstone.exe` (built 2026-07-17) |
| DICOM | `C:\Users\Derek\Downloads\hnum_dicom_micro\dicom_micro` |
| beams | 5 (and 7 where noted); 39 beamlets/beam |
| isocenter | (218.0, 218.0, -172.0) mm |
| driver | `python/automate_brimstone_ui.py` |
| env | `BRIMSTONE_ENTROPY_SEPARABLE=1`, `BRIMSTONE_DETERMINISTIC=1` |

Prescriptions:

| structure | type | interval | weight | precedent |
|---|---|---|---|---|
| PTV | Target | 60-70 Gy | 2.5 | 1 |
| Spinal Cord | OAR | 0-30 Gy | 2.5 | 1 |
| Parotid(L) | OAR | 0-30 Gy | 0.15 | 1 |
| Parotid(R) | OAR | 0-30 Gy | 0.15 | 1 |
| External | OAR | 10-50 Gy | 0.15 | 2 |

Beamlet count is hardcoded in two independent places, both `nBeamletCount = 19`
(`Brimstone/PlanSetupDlg.cpp:71`, `RtModel/PlanPyramid.cpp:113`), giving
`-19..+19` = 39 beamlets per beam. Changing it requires editing both and
rebuilding; the halving in PlanPyramid reaches 0 for small values, which is
untested.

`BRIMSTONE_DETERMINISTIC=1` removes the message-timing dependence of the
iteration count (`Brimstone/OptThread.cpp:101-118`), so runs are comparable
point to point.

## Prerequisite: dose-unit bug in the driver (fixed)

Before this sweep, `automate_brimstone_ui.py` wrote `int(dose_Gy * 100)` into the
dose edit boxes. The boxes hold **Gy as a plain integer** --
`OnBnClickedBtnSetinterval` (`Brimstone/PrescriptionToolbar.cpp:373`) divides by
100 to get the internal value (Gy/100). So every prescription was set 100x too
high: PTV 6000-7000 Gy, OARs 0-3000 Gy.

The `*100` at `PrescriptionToolbar.cpp:150` is the *display* half of that
round-trip (`GetMinDose()` is already Gy/100), not an extra scaling to
reproduce. Fixed in this branch.

All results below use corrected doses. Any earlier W determination predates the
fix and should be considered void.

## Results, 5 beams

Baseline is W=1e-12: small enough to be inert (`w*H ~ 4e-11`) but non-zero, so
the entropy block still runs and reports `H`. At exactly W=0 the block is
skipped (`Prescription.cpp:406`) and `H` reads 0.

`H_max = 195 * ln2 = 135.2`

| W | KL | dKL | H | H/H_max | dH | w*H / KL | dH%/dKL% |
|---|---|---|---|---|---|---|---|
| 1e-12 | 0.005598 | -- | 43.33 | 32.1% | -- | ~0 | -- |
| 1e-6 | 0.005867 | +4.8% | 43.47 | 32.2% | +0.3% | 0.7% | 0.1 |
| 3e-6 | 0.005746 | +2.6% | 45.81 | 33.9% | +5.7% | 2.4% | 2.2 |
| 1e-5 | 0.005769 | +3.0% | 42.64 | 31.5% | -1.6% | 7.4% | -- |
| 3e-5 | 0.005881 | +5.1% | 43.62 | 32.3% | +0.7% | 22% | 0.1 |
| 1e-4 | 0.006087 | +8.7% | 43.74 | 32.4% | +0.9% | 72% | 0.1 |
| **3e-4** | **0.006815** | **+21.7%** | **67.12** | **49.7%** | **+54.9%** | -- | **2.5** |
| 1e-3 | 0.015058 | +169% | 92.70 | 68.6% | +114% | -- | 0.7 |
| 3e-3 | 0.039080 | +598% | 109.41 | 80.9% | +152% | -- | 0.3 |

**The knee is at W = 3e-4.** Below it the regularizer is inert in the sense that
matters: `H` stays near ~43 across a 100x range of W, non-monotone, dipping
*below* baseline at 1e-5, while KL slowly degrades. Above it the exchange rate
collapses toward the q=0.5 collapse.

That sub-3e-4 band was originally described here as "noise". That is not
supported: the only same-binary repeat measured (7 beams, W=3e-4) reproduced to
0.014% on KL and 0.025% on H, whereas the band spans ~7% in H -- two to three
orders of magnitude larger. So small W does perturb the solution; it just does
not perturb it in the direction of raising entropy. Caveat on the caveat: that
repeat is at W=3e-4, and scatter could well be larger in the dead zone where the
entropy gradient is weak and CG has less to anchor it. A repeat at W=1e-5 would
settle it. Either way the knee conclusion is untouched -- +21.7% KL and +54.9% H
at 3e-4 dwarf both readings.

The knee is not pinned precisely; 1e-4 to 3e-4 is unexplored and the true
optimum may be nearer 2e-4.

### Filling the 1e-4 -> 3e-4 gap (2026-07-23): knee is at ~2e-4

Fresh `x64/Release/Brimstone.exe` (rebuilt 2026-07-23 17:05 from this branch),
5 beams, `BRIMSTONE_ENTROPY_SEPARABLE=1`, `BRIMSTONE_DETERMINISTIC=1`. The 1e-4
and 3e-4 anchors were re-run on this binary and reproduce the table above to
<0.2% (1e-4: H 43.80 vs 43.74; 3e-4: H 67.12 vs 67.12), so the new interior
points are directly comparable. Interior points are n=2; `H` mean shown.

| W | KL (mean) | H (mean) | H/H_max | n |
|---|---|---|---|---|
| 1e-12 | 0.005728 | 43.54 | 32.2% | 2 |
| 1e-4 | 0.006098 | 43.80 | 32.4% | 1 |
| 1.5e-4 | 0.006293 | 45.84 | 33.9% | 2 |
| **2e-4** | **0.006064** | **64.00** | **47.3%** | **2** |
| 2.5e-4 | 0.006571 | 59.73 | 44.2% | 2 |
| 3e-4 | 0.006813 | 67.12 | 49.6% | 1 |

**The entropy term does nothing until ~2e-4, then engages abruptly.** `H` sits
at the inert baseline (~43-46) through 1.5e-4, then jumps to ~64 at 2e-4 -- a
step, not a ramp. The 2e-4 jump reproduces to 0.02% on H across the two passes,
so it is a real feature of the objective, not CG scatter. The note's earlier
guess ("nearer 2e-4") is confirmed and sharpened: the onset sits between 1.5e-4
and 2e-4. So the operative "knee" -- where the regularizer starts to bite -- is
~2e-4, below the 3e-4 identified from the coarse grid (3e-4 is simply the first
coarse point past the onset).

**KL is not a usable discriminant in this band; only H is.** This settles the
dead-zone-scatter caveat left open above. Measured *same-binary* run-to-run
scatter at W=1e-12 (deepest dead zone, entropy gradient ~0):

```
1e-12  kl=0.00592661  H=43.58
1e-12  kl=0.00552984  H=43.49
```

`H` reproduces to 0.2%, but **KL spans 7.2%** -- two deterministic runs of the
same binary, same inputs, landing in CG basins that differ by 7% in KL. (Runs
are not bit-identical even under `BRIMSTONE_DETERMINISTIC`; residual thread-order
FP reduction in the beamlet calc remains.) That is the same ~7% the note called
out at 3e-4, now pinned to the *dead zone* specifically -- exactly where the note
predicted scatter would be largest. Consequence: the few-percent KL "degradation"
across the sub-2e-4 points is within this basin noise and must not be read as
regularizer effect. In particular the tempting reading that 2e-4 is a low-KL
sweet spot (its KL, 0.00606, sits below its neighbors) is **not supported** --
that KL gap is smaller than the 7% baseline scatter. What survives is the `H`
signal, whose scatter is <1.3% throughout.

**Best W, stated carefully.** If the separable term is used at all, ~2e-4 is the
operating point: it is the smallest W that fully engages the regularizer
(`H` 43->64), and everything above it (2.5e-4, 3e-4, and the decade beyond)
only adds KL cost -- clearly above the noise floor by 2.5e-4 -- while `H` climbs
toward the q=0.5 collapse. But this does not revise the closing verdict below:
even at its best operating point the term buys entropy the parameterization does
not want. Use the parameterization, not the prior.

### Scaling W by value ratio is wrong

The first bracket for this sweep was chosen so that `w*H / KL` landed in the
1-25% range. That heuristic failed: at W=1e-4, `w*H` is 72% of the objective and
`H` still moves only 0.9%.

What steers CG is the gradient, not the value. `H` sums ~195 per-beamlet terms of
O(0.7), so its value is large (~43) while its per-element gradient
`log(qc/q) * dSigmoid(x)` is O(0.1) -- `dSigmoid` caps at `s/4 = 0.125` for
`s = 0.5`. KL is the reverse: small in value, concentrated in gradient. An
objective that looks entropy-dominated by value can still be entirely KL-driven.

**Scale W by gradient magnitude, not by `w*H / KL`.**

## Results, 7 beams

KL is not comparable across beam counts, so this is a self-contained pair.

`H_max = 273 * ln2 = 189.2`

| W | KL | H | H/H_max | dKL | dH | dH%/dKL% |
|---|---|---|---|---|---|---|
| 1e-12 | 0.004960 | 52.50 | 27.7% | -- | -- | -- |
| 3e-4 | 0.005558 | 80.66 | 42.6% | +12.1% | +53.6% | **4.4** |

**W=3e-4 is about twice as efficient at 7 beams** -- nearly the same entropy gain
for roughly half the KL cost (exchange 4.4 vs 2.5). More beams means more degrees
of freedom to absorb the entropy pressure.

Two side observations:

- The 7-beam regularized plan (KL 0.005558) ties the 5-beam *unregularized* one
  (0.005598). Going 5->7 beams pays for the regularizer outright.
- Baseline entropy fraction is *lower* at 7 beams (27.7%) than 5 (32.1%) -- more
  beamlets sharing the same dose sit nearer their rails, not further.

## Open questions

1. **Is raising H desirable at all?** Not established, and every W that does
   anything makes KL worse. Whether the entropy bought is worth it is a DVH
   question, not an objective-value one -- compare the 7-beam 3e-4 plan against
   its baseline in the viewer before adopting 3e-4. (Superseded in practice by
   the height sweep below, which shows H maximized at the KL optimum.)

   NOTE: an earlier version of this section read "~28-32% of max entropy, i.e.
   not saturated". That inference was wrong. Binary entropy is flat near q=0.5
   and steep near the rails, so 28% of max corresponds to a typical `q ~ 0.048`
   -- strongly bimodal, sitting near a rail. The plans *are* saturated in that
   sense. Sparse beamlet solutions are expected and fine in IMRT; the concern is
   gradient suppression, and at q~0.048 with s=0.5, `dSigmoid = s*q*(1-q) ~
   0.023` against a peak of `s/4 = 0.125` -- about 18% of peak. Suppressed, not
   frozen.
2. **Saturation diagnosis is unresolved.** Under the 100x doses the *softmax* H
   read 1.35e-09 (near-total collapse). Post-fix the *separable* H reads healthy.
   Those are different functions of the parameters, so the improvement cannot be
   cleanly attributed to the dose fix rather than the change of measure. A
   softmax-form run under corrected doses would settle it.
3. **Sigmoid parameterization** (`SIGMOID_SCALE` height = 0.2 at
   `Prescription.cpp:14`; `InputScale` steepness = 0.5, registry-backed) is
   untouched here. Height is compile-time but cancels exactly out of the
   adaptive-variance path (`Prescription.cpp:543-554`), so sweeping it moves only
   the beamlet ceiling -- a clean sweep. Steepness does *not* cancel
   (`varSlope = dSigmoid(x,s)/dSigmoid(0,s)`), so sweeping it confounds the
   parameterization with the variance normalization.

## Sigmoid height sweep (supersedes the W thread)

7 beams, W=1e-12 (regularizer off), separable, **new binary** (see caveat below).
Height is `BRIMSTONE_SIGMOID_SCALE`, the transform amplitude
`weight = height * Sigmoid(x, s)` -- i.e. the per-beamlet weight ceiling.

Typical `q` is recovered by inverting the mean per-beamlet binary entropy
`H/nDim`. **Binary entropy is symmetric under `q <-> 1-q`, so it cannot
distinguish the top rail from the bottom rail.** Both branches are listed; the
choice between them below is physical reasoning, not measurement.

| height | KL | x vs 0.2 | H | H/nDim | typical q |
|---|---|---|---|---|---|
| 0.05 | 0.048597 | 9.8x | 3.57 | 0.013 | ~0.998 or 0.002 |
| 0.1 | 0.017802 | 3.6x | 14.04 | 0.051 | ~0.991 or 0.009 |
| **0.2** | **0.004967** | **1.00** | **52.87** | **0.194** | ~0.049 or 0.951 |
| 0.4 | 0.023231 | 4.7x | 16.29 | 0.060 | ~0.012 or 0.988 |

**0.2 is a sharp optimum; the existing value is validated.** A factor of two in
either direction costs 3.6-4.7x in KL. The two ends fail for opposite reasons:

- **ceiling too low** (0.05, 0.1): beamlets cannot deliver 60-70 Gy at any
  weight, so they max out. KL blows up from unreachability, not conditioning.
- **ceiling too high** (0.4): beamlets need less weight, `q` falls to ~0.012,
  deep into the sigmoid's flat region. `dSigmoid = s*q*(1-q) ~ 0.0057`, **4.5%
  of peak**, versus 18% at height 0.2. Gradient suppression.

So 0.2 sits between an unreachability wall and a vanishing-gradient wall.

### This closes out the entropy-regularizer thread

`H` is *maximized* at exactly the height that minimizes KL. At the
parameterization's natural optimum the plan is already as de-saturated as this
axis permits, so the separable regularizer has nothing to add -- and what it
does add costs KL while pushing toward a uniform half-max fluence field.

That is worth stating plainly: the separable term's fixed point is `q = 0.5` for
*every* beamlet (`dH/dx_i = ln((1-q)/q) * dSigmoid`), i.e. a flat fluence across
all beamlets. For IMRT, where most beamlets should be off, that is close to the
opposite of what is wanted. The monotone KL degradation once the term engages is
the term making progress toward a target we do not want, not overshooting a good
one. Use the parameterization, not the prior.

### Binary caveat

Everything above the height sweep was measured on the Jul-17 build; the height
sweep and after are on the build that added `SigmoidParams.h`. Cross-binary
regression at 7 beams / W=3e-4: KL +0.096%, H +0.068%. The likely cause is loss
of constant folding -- `SIGMOID_SCALE` was a compile-time literal and is now
runtime-initialized, changing FP association at every site, which CG amplifies.
Plausible, not verified: the same-binary repeat sample is n=1, which is no basis
for a noise estimate. **Do not compare numbers across the two builds.**

New-binary 7-beam W=1e-12 baseline: `kl=0.004966579  entropy=52.86549`
(Jul-17 equivalent: `0.004960316 / 52.49837`).

## Raw data

`free_energy`/`kl`/`entropy` as written by the `BRIMSTONE_OBJECTIVE_FILE` hook
(`Brimstone/BrimstoneView.cpp:644-670`).

```
# 5 beams
1e-12  free_energy=0.005598310686  kl=0.005598310729  entropy=43.33448849
1e-6   free_energy=0.005823161038  kl=0.005866630588  entropy=43.46954959
3e-6   free_energy=0.005608300952  kl=0.00574573777   entropy=45.81227269
1e-5   free_energy=0.005342213916  kl=0.005768593829  entropy=42.63799134
3e-5   free_energy=0.004573008584  kl=0.005881489181  entropy=43.61601991
1e-4   free_energy=0.001712933584  kl=0.00608699239   entropy=43.74058806
3e-4   free_energy=-0.01332093534  kl=0.006815459049  entropy=67.12131463
1e-3   free_energy=-0.07764590368  kl=0.01505793695   entropy=92.70384063
3e-3   free_energy=-0.2891472055   kl=0.03907985142   entropy=109.409019

# 7 beams
1e-12  free_energy=0.004960315875  kl=0.004960315927  entropy=52.49837375
3e-4   free_energy=-0.01863942873  kl=0.005558337481  entropy=80.6592207
```

Superseded (100x dose bug, retained only to show W does not transfer between
entropy forms):

```
softmax    W=0.005  kl=0.004715949201  entropy=1.354871251e-09   (w*H ~ 1e-9 of KL: inert)
separable  W=0.005  kl=0.05348999864   entropy=120.0335158       (w*H ~ 11x KL: dominant)
```

```
# sigmoid height sweep, 7 beams, W=1e-12, separable, new binary
0.05  free_energy=0.04859692301  kl=0.04859692302  entropy=3.574994619
0.1   free_energy=0.0178015622   kl=0.01780156222  entropy=14.04449446
0.2   free_energy=0.004966578774 kl=0.004966578827 entropy=52.86548505
0.4   free_energy=0.02323116425  kl=0.02323116426  entropy=16.28754371

# cross-binary regression, 7 beams W=3e-4
Jul-17 build   kl=0.005558337481  entropy=80.6592207
Jul-17 repeat  kl=0.00555910284   entropy=80.63928308
new build      kl=0.005563666058  entropy=80.71388731
```

```
# 1e-4 -> 3e-4 gap fill, 5 beams, separable, deterministic
# fresh Release|x64 built 2026-07-23 17:05. Interior points n=2.
1e-4    free_energy=0.001717390482  kl=0.00609759155   entropy=43.80201068
1.5e-4  free_energy=-0.0006254924138 kl=0.006297439718 entropy=46.15288088
1.5e-4  free_energy=-0.000542399992  kl=0.006287694143 entropy=45.5339609
2e-4    free_energy=-0.006741011888  kl=0.006058116058 entropy=63.99563973
2e-4    free_energy=-0.006732931791  kl=0.006069261618 entropy=64.01096705
2.5e-4  free_energy=-0.008325029464  kl=0.00655886576  entropy=59.53558089
2.5e-4  free_energy=-0.008394737065  kl=0.006583913855 entropy=59.91460368
3e-4    free_energy=-0.01332425812   kl=0.00681311422  entropy=67.12457446

# dead-zone same-binary KL scatter control, 5 beams, W=1e-12
1e-12   free_energy=0.005926613247  kl=0.00592661329  entropy=43.58245032
1e-12   free_energy=0.005529839676  kl=0.00552983972  entropy=43.48875725
```

Sweep driven by an orchestrator (per-W detached launch + `BRIMSTONE_ATTACH_PID`
attach + wait on `BRIMSTONE_OBJECTIVE_FILE`) wrapping `automate_brimstone_ui.py`;
the loop itself was run out of tree and is not committed.
