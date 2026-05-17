"""
Sigma-calibration instrumentation experiment.

Settles Open Question 1 from HIERARCHICAL_BAYES_DESIGN.md: is the
optimizer-internal adaptive variance (m_vAdaptVariance, surfaced via
the Step-1 sigma_weights callback) a *calibrated* posterior variance,
or just an optimizer trace that happens to live in the right shape?

Method: run the same fixed plan N times with different random initial
parameter vectors. Compare the converged sigma maps pairwise:

    shape stability     = mean off-diagonal of Pearson correlation matrix
                          of sigma vectors across seeds (per-beamlet)
    magnitude stability = coefficient of variation of per-seed mean(sigma)

Decision per the design doc:
    shape >= 0.9, magnitude CV <= 0.10  -> trust as posterior, pool directly
    shape >= 0.9, magnitude CV > 0.10   -> normalize per phase before pooling
    shape < 0.9                          -> not a posterior; use external
                                            variance (Hutchinson Fisher,
                                            bootstrap)

CAVEAT: This experiment runs against the pure-Python port of the
algorithm, not the C++ original. The Python port is a faithful
translation but subtle differences in line search, convergence test,
or numerical precision could shift the result. The conclusion below
is informative but should be confirmed against the C++ side when the
wrapper compile-verifies.
"""

from __future__ import annotations

import sys
from pathlib import Path
from typing import List, Tuple

import numpy as np

# Allow `python python/experiments/sigma_calibration.py` from repo root
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from pybrimstone import (
    KLDivTerm,
    PhaseOptimizer,
    Prescription,
    gaussian_bump_dose_operator,
)


# ---------------------------------------------------------------------------
# Fixed problem definition
# ---------------------------------------------------------------------------

GRID = (8, 8, 8)
N_BEAMLETS = 5


def _make_mask(predicate, grid=GRID):
    nx, ny, nz = grid
    n_voxels = nx * ny * nz
    mask = np.zeros(n_voxels)
    for i in range(nx):
        for j in range(ny):
            for k in range(nz):
                if predicate(i, j, k):
                    mask[i * ny * nz + j * nz + k] = 1.0
    return mask


def build_fixed_prescription() -> Tuple[Prescription, dict]:
    """The reference problem: PTV + 2 OARs, 5 beamlets, Gaussian-bump dose op."""
    centers = np.array([[3.0, 3.0, 3.5 + i * 0.5] for i in range(N_BEAMLETS)])
    D = gaussian_bump_dose_operator(GRID, centers, sigma=1.5)
    structures = {
        "PTV":           _make_mask(lambda i, j, k: 2 <= i <= 4 and 2 <= j <= 4 and 3 <= k <= 5),
        "OAR_anterior":  _make_mask(lambda i, j, k: i == 1 and 2 <= j <= 4 and 3 <= k <= 5),
        "OAR_posterior": _make_mask(lambda i, j, k: i == 5 and 2 <= j <= 4 and 3 <= k <= 5),
    }
    p = Prescription(D, use_transform=True)
    p.add_dose_term(KLDivTerm.from_interval(
        structures["PTV"], dose_min=0.55, dose_max=0.75, weight=2.0,
        bin_width=0.05, var_min=0.01, var_max=0.01,
    ))
    for oar in ("OAR_anterior", "OAR_posterior"):
        p.add_dose_term(KLDivTerm.from_interval(
            structures[oar], dose_min=0.0, dose_max=0.3, weight=1.0,
            bin_width=0.05, var_min=0.01, var_max=0.01,
        ))
    return p, structures


# ---------------------------------------------------------------------------
# Experiment driver
# ---------------------------------------------------------------------------

def run_one_seed(
    seed: int,
    init_scale: float = 0.5,
    max_iter: int = 30,
    tol: float = 1e-3,
    adaptive_variance: Tuple[float, float] = (0.01, 0.1),
) -> Tuple[np.ndarray, np.ndarray]:
    """Build a fresh Prescription + PhaseOptimizer and run with the given seed."""
    p, _ = build_fixed_prescription()
    rng = np.random.default_rng(seed)
    initial = rng.normal(size=N_BEAMLETS) * init_scale
    opt = PhaseOptimizer(
        p, n_params=N_BEAMLETS, initial_params=initial,
        max_iter=max_iter, tol=tol,
        adaptive_variance=adaptive_variance,
    )
    mu_p, var_p = opt(prior=None)
    return mu_p, var_p


def run_calibration_sweep(n_seeds: int = 20) -> Tuple[np.ndarray, np.ndarray]:
    """Returns (mus, vars), each shape (n_seeds, N_BEAMLETS)."""
    mus = []
    vars_ = []
    for s in range(n_seeds):
        mu_p, var_p = run_one_seed(seed=s)
        mus.append(mu_p)
        vars_.append(var_p)
    return np.stack(mus), np.stack(vars_)


# ---------------------------------------------------------------------------
# Stats
# ---------------------------------------------------------------------------

def shape_stability(vars_: np.ndarray) -> float:
    """
    Mean off-diagonal of the Pearson correlation matrix of sigma vectors
    across seeds. 1.0 = identical shape; ~0 = no relationship.
    """
    n_seeds = vars_.shape[0]
    # Correlate sigma vectors across seeds. If sigma is essentially
    # constant for some seeds, np.corrcoef will warn / return NaN; mask
    # those out of the mean.
    corr = np.corrcoef(vars_)
    if n_seeds < 2:
        return float("nan")
    iu = np.triu_indices(n_seeds, k=1)
    off = corr[iu]
    off = off[np.isfinite(off)]
    if off.size == 0:
        return float("nan")
    return float(off.mean())


def magnitude_stability(vars_: np.ndarray) -> Tuple[float, float, float]:
    """
    Coefficient of variation of per-seed mean sigma, plus the raw mean
    sigma range. Returns (mean_per_seed_mean, std_per_seed_mean, cv).
    """
    per_seed = vars_.mean(axis=1)
    mean_ = float(per_seed.mean())
    std_ = float(per_seed.std())
    cv = std_ / mean_ if mean_ > 0 else float("nan")
    return mean_, std_, cv


def verdict(shape_corr: float, cv: float) -> str:
    if not np.isfinite(shape_corr):
        return "INDETERMINATE (NaN correlation; sigma may be degenerate)"
    if shape_corr >= 0.9 and cv <= 0.10:
        return "ROW 1: trust as posterior, pool directly"
    if shape_corr >= 0.9:
        return "ROW 2: shape stable but magnitude variable -> normalize per phase before pooling"
    return "ROW 3: not a posterior -> use external variance (Hutchinson Fisher / bootstrap)"


# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------

def make_plot(mus: np.ndarray, vars_: np.ndarray, out_path: str) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    n_seeds, n_beamlets = vars_.shape
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))

    # Sigma per beamlet, scattered across seeds
    ax = axes[0]
    for b in range(n_beamlets):
        ax.scatter([b] * n_seeds, vars_[:, b], alpha=0.5, s=30)
    # Median per beamlet
    medians = np.median(vars_, axis=0)
    ax.plot(range(n_beamlets), medians, "k-", label="median across seeds")
    ax.set_xlabel("Beamlet index")
    ax.set_ylabel("Adaptive variance (m_vAdaptVariance)")
    ax.set_title(f"Sigma values across {n_seeds} random inits")
    ax.set_xticks(range(n_beamlets))
    ax.grid(True, alpha=0.3)
    ax.legend()

    # Per-seed mean sigma (magnitude check)
    ax = axes[1]
    per_seed_mean = vars_.mean(axis=1)
    ax.bar(range(n_seeds), per_seed_mean)
    ax.axhline(per_seed_mean.mean(), color="r", linestyle="--",
               label=f"overall mean = {per_seed_mean.mean():.4f}")
    ax.set_xlabel("Seed index")
    ax.set_ylabel("Mean sigma over beamlets")
    ax.set_title("Per-seed mean sigma (magnitude stability)")
    ax.grid(True, alpha=0.3)
    ax.legend()

    fig.tight_layout()
    fig.savefig(out_path, dpi=110)
    plt.close(fig)


def main(n_seeds: int = 20, out_path: str = "/tmp/sigma_calibration.png") -> dict:
    print(f"Running sigma-calibration sweep, n_seeds={n_seeds}...")
    mus, vars_ = run_calibration_sweep(n_seeds=n_seeds)

    shape_corr = shape_stability(vars_)
    mean_, std_, cv = magnitude_stability(vars_)
    decision = verdict(shape_corr, cv)

    print()
    print("=" * 64)
    print("SIGMA CALIBRATION RESULTS")
    print("=" * 64)
    print(f"  n_seeds:           {n_seeds}")
    print(f"  beamlets:          {N_BEAMLETS}")
    print(f"  sigma min/max:     {vars_.min():.6f} / {vars_.max():.6f}")
    print()
    print(f"  shape_stability    {shape_corr:.4f}    (mean off-diag corr)")
    print(f"    threshold:        >= 0.9 for 'posterior-like' shape")
    print()
    print(f"  per-seed mean sigma:")
    print(f"    mean:             {mean_:.6f}")
    print(f"    std:              {std_:.6f}")
    print(f"    CV:               {cv:.4f}")
    print(f"    threshold:        <= 0.10 for calibrated magnitude")
    print()
    print(f"  VERDICT: {decision}")
    print("=" * 64)
    print()

    # Per-beamlet detail
    print("Per-beamlet sigma stats:")
    print(f"  {'beamlet':<10}{'min':>12}{'median':>12}{'max':>12}{'CV':>10}")
    for b in range(N_BEAMLETS):
        col = vars_[:, b]
        cv_b = col.std() / col.mean() if col.mean() > 0 else float("nan")
        print(f"  {b:<10}{col.min():>12.6f}{np.median(col):>12.6f}{col.max():>12.6f}{cv_b:>10.4f}")

    make_plot(mus, vars_, out_path)
    print(f"\nPlot saved to {out_path}")

    return {
        "shape_corr": shape_corr,
        "magnitude_cv": cv,
        "magnitude_mean": mean_,
        "magnitude_std": std_,
        "vars": vars_,
        "mus": mus,
        "verdict": decision,
    }


# ---------------------------------------------------------------------------
# Control variants
# ---------------------------------------------------------------------------

def run_higher_dim_variant(n_beamlets: int = 20, n_seeds: int = 20) -> dict:
    """
    Same brimstone-shape problem but with more beamlets, so CG has more
    iterations to run before n_dim caps the AV update. If shape stability
    is genuinely problem-driven (and not just an artifact of CG converging
    before AV converges), more beamlets should not change the verdict.
    """
    from pybrimstone import (
        KLDivTerm, PhaseOptimizer, Prescription, gaussian_bump_dose_operator,
    )
    centers = np.array([
        [3.0 + 0.05 * b, 3.0 + 0.05 * b, 3.0 + (b / n_beamlets) * 2.0]
        for b in range(n_beamlets)
    ])
    D = gaussian_bump_dose_operator(GRID, centers, sigma=1.5)
    structures = {
        "PTV": _make_mask(lambda i, j, k: 2 <= i <= 4 and 2 <= j <= 4 and 3 <= k <= 5),
    }

    vars_ = []
    for s in range(n_seeds):
        rng = np.random.default_rng(s)
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            structures["PTV"], dose_min=0.55, dose_max=0.75, weight=2.0,
            bin_width=0.05, var_min=0.01, var_max=0.01,
        ))
        opt = PhaseOptimizer(
            p, n_params=n_beamlets,
            initial_params=rng.normal(size=n_beamlets) * 0.5,
            max_iter=50, tol=1e-3,
            adaptive_variance=(0.01, 0.1),
        )
        _, var_p = opt(prior=None)
        vars_.append(var_p)
    vars_ = np.stack(vars_)

    shape_corr = shape_stability(vars_)
    mean_, std_, cv = magnitude_stability(vars_)
    return {
        "label": f"Brimstone {n_beamlets}-beamlet",
        "n_beamlets": n_beamlets,
        "shape_corr": shape_corr,
        "magnitude_cv": cv,
        "magnitude_mean": mean_,
        "magnitude_std": std_,
        "vars": vars_,
        "verdict": verdict(shape_corr, cv),
    }


def run_quadratic_control(n_dim: int = 10, n_seeds: int = 20) -> dict:
    """
    Pure isotropic quadratic in n_dim. No sigmoid, no KL, no dose calc --
    just the CG + adaptive-variance algorithm running on the cleanest
    possible problem. If sigma is genuinely problem-curvature-driven,
    this should produce maximally stable sigma (since the curvature is
    the identity for every seed).

    If even this control shows instability, the conclusion is that the
    adaptive-variance algorithm produces an optimizer trace, not a
    posterior, regardless of problem structure.
    """
    from pybrimstone.numerics.conjugate_gradient import polak_ribiere_cg

    scales = np.arange(1.0, n_dim + 1)  # ill-conditioned: scales[i] = i+1
    f = lambda x: float(np.sum(scales * x * x))
    grad = lambda x: 2.0 * scales * x

    vars_ = []
    for s in range(n_seeds):
        rng = np.random.default_rng(s)
        x0 = rng.normal(size=n_dim) * 2.0
        last_sigma = [np.full(n_dim, 0.1)]

        def cb(**kw):
            sw = kw.get("sigma_weights")
            if sw is not None:
                last_sigma[0] = sw

        polak_ribiere_cg(
            f, grad, x0,
            callback=cb,
            adaptive_variance=(0.01, 0.1),
            max_iter=50, tol=1e-6,
        )
        vars_.append(last_sigma[0])
    vars_ = np.stack(vars_)

    shape_corr = shape_stability(vars_)
    mean_, std_, cv = magnitude_stability(vars_)
    return {
        "label": f"Pure quadratic {n_dim}-D",
        "n_beamlets": n_dim,
        "shape_corr": shape_corr,
        "magnitude_cv": cv,
        "magnitude_mean": mean_,
        "magnitude_std": std_,
        "vars": vars_,
        "verdict": verdict(shape_corr, cv),
    }


def make_summary_plot(results: List[dict], out_path: str) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    n = len(results)
    fig, axes = plt.subplots(1, n, figsize=(5.5 * n, 4.5), squeeze=False)
    for ax, res in zip(axes[0], results):
        vars_ = res["vars"]
        n_seeds, n_b = vars_.shape
        for b in range(n_b):
            ax.scatter([b] * n_seeds, vars_[:, b], alpha=0.5, s=20)
        medians = np.median(vars_, axis=0)
        ax.plot(range(n_b), medians, "k-", lw=2, label="median")
        ax.set_xlabel("Parameter index")
        ax.set_ylabel("Adaptive variance")
        ax.set_title(
            f"{res['label']}\n"
            f"shape corr = {res['shape_corr']:.3f}, "
            f"magnitude CV = {res['magnitude_cv']:.3f}"
        )
        ax.grid(True, alpha=0.3)
        ax.set_yscale("log")
        ax.legend(loc="upper right", fontsize=8)
    fig.tight_layout()
    fig.savefig(out_path, dpi=110)
    plt.close(fig)


def run_bootstrap_variant(
    n_seeds: int = 20,
    n_bootstrap: int = 15,
    subsample_fraction: float = 0.5,
) -> dict:
    """
    Calibration sweep using BootstrapPhaseOptimizer instead of the
    optimizer-internal sigma_weights. If the bootstrap variance is a
    genuine posterior estimate, multiple random initializations of the
    *outer* CG should produce nearly identical bootstrap variance
    (since it's a property of the data, not the optimizer init).

    Returns the same dict shape as the other run_* functions so it
    plugs into make_summary_plot.
    """
    from pybrimstone import KLDivTerm, Prescription, gaussian_bump_dose_operator
    from pybrimstone.bootstrap import BootstrapPhaseOptimizer, subsample_mask

    grid = GRID
    nx, ny, nz = grid
    n_voxels = nx * ny * nz
    n_beamlets = N_BEAMLETS
    centers = np.array([[3.0, 3.0, 3.5 + i * 0.5] for i in range(n_beamlets)])
    D = gaussian_bump_dose_operator(grid, centers, sigma=1.5)
    structures = {
        "PTV":           _make_mask(lambda i, j, k: 2 <= i <= 4 and 2 <= j <= 4 and 3 <= k <= 5),
        "OAR_anterior":  _make_mask(lambda i, j, k: i == 1 and 2 <= j <= 4 and 3 <= k <= 5),
        "OAR_posterior": _make_mask(lambda i, j, k: i == 5 and 2 <= j <= 4 and 3 <= k <= 5),
    }

    def factory(rng):
        masks = {
            name: m if rng is None else subsample_mask(m, subsample_fraction, rng)
            for name, m in structures.items()
        }
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            masks["PTV"], dose_min=0.55, dose_max=0.75, weight=2.0,
            bin_width=0.05, var_min=0.01, var_max=0.01,
        ))
        for oar in ("OAR_anterior", "OAR_posterior"):
            p.add_dose_term(KLDivTerm.from_interval(
                masks[oar], dose_min=0.0, dose_max=0.3, weight=1.0,
                bin_width=0.05, var_min=0.01, var_max=0.01,
            ))
        return p

    vars_ = []
    for s in range(n_seeds):
        rng = np.random.default_rng(s)
        opt = BootstrapPhaseOptimizer(
            factory, n_params=n_beamlets,
            n_bootstrap=n_bootstrap,
            initial_params=rng.normal(size=n_beamlets) * 0.5,
            max_iter=30, tol=1e-3,
            adaptive_variance=(0.01, 0.1),
            bootstrap_seed=10_000 + s,
        )
        _, var_p = opt(prior=None)
        vars_.append(var_p)
    vars_ = np.stack(vars_)

    shape_corr = shape_stability(vars_)
    mean_, std_, cv = magnitude_stability(vars_)
    return {
        "label": f"Bootstrap variance (B={n_bootstrap}, frac={subsample_fraction})",
        "n_beamlets": n_beamlets,
        "shape_corr": shape_corr,
        "magnitude_cv": cv,
        "magnitude_mean": mean_,
        "magnitude_std": std_,
        "vars": vars_,
        "verdict": verdict(shape_corr, cv),
    }


def run_full_sweep(out_path: str = "/tmp/sigma_calibration_full.png") -> List[dict]:
    """Run all three variants and produce a side-by-side comparison plot."""
    print("\n" + "=" * 64)
    print("Variant 1: brimstone 5-beamlet (the main result)")
    print("=" * 64)
    main_result = main(n_seeds=20, out_path="/tmp/sigma_calibration_main.png")
    main_result["label"] = "Brimstone 5-beamlet"
    main_result["n_beamlets"] = N_BEAMLETS

    print("\n" + "=" * 64)
    print("Variant 2: brimstone 20-beamlet (CG room to converge AV through more dims)")
    print("=" * 64)
    hi_result = run_higher_dim_variant(n_beamlets=20, n_seeds=20)
    print(f"  shape_stability    {hi_result['shape_corr']:.4f}")
    print(f"  magnitude CV       {hi_result['magnitude_cv']:.4f}")
    print(f"  VERDICT: {hi_result['verdict']}")

    print("\n" + "=" * 64)
    print("Variant 3: pure 10D ill-conditioned quadratic (algorithm in isolation)")
    print("=" * 64)
    quad_result = run_quadratic_control(n_dim=10, n_seeds=20)
    print(f"  shape_stability    {quad_result['shape_corr']:.4f}")
    print(f"  magnitude CV       {quad_result['magnitude_cv']:.4f}")
    print(f"  VERDICT: {quad_result['verdict']}")

    make_summary_plot([main_result, hi_result, quad_result], out_path)
    print(f"\nSummary plot saved to {out_path}")

    print("\n" + "=" * 64)
    print("OVERALL VERDICT")
    print("=" * 64)
    verdicts = [r["verdict"].split(":")[0] for r in [main_result, hi_result, quad_result]]
    if all(v == "ROW 3" for v in verdicts):
        print("  All three variants reach ROW 3.")
        print("  m_vAdaptVariance is an optimizer trace, NOT a calibrated posterior.")
        print("  Recommendation: replace with external variance for hierarchical pooling.")
        print("    Options: Fisher information diagonal (Hutchinson trace estimator),")
        print("             bootstrap over voxel subsamples, or Laplace approximation.")
    elif main_result["verdict"].startswith("ROW 1") and quad_result["verdict"].startswith("ROW 1"):
        print("  All variants show calibrated posterior behavior.")
        print("  Recommendation: pool sigma directly per HIERARCHICAL_BAYES_DESIGN.md formulas.")
    else:
        print(f"  Verdicts vary across variants: {verdicts}")
        print(f"  Suggests sigma calibration is problem-dependent. Use per-problem instrumentation.")
    print("=" * 64)

    return [main_result, hi_result, quad_result]


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--n-seeds", type=int, default=20)
    parser.add_argument("--mode", type=str, default="full",
                       choices=["main", "full", "bootstrap", "compare"],
                       help="'main' = just the 5-beamlet sigma_weights experiment; "
                            "'full' = three sigma_weights variants; "
                            "'bootstrap' = bootstrap-variance variant only; "
                            "'compare' = sigma_weights vs bootstrap side-by-side")
    parser.add_argument("--out", type=str,
                       default="/tmp/sigma_calibration.png")
    args = parser.parse_args()

    if args.mode == "main":
        main(n_seeds=args.n_seeds, out_path=args.out)
    elif args.mode == "full":
        run_full_sweep(out_path=args.out)
    elif args.mode == "bootstrap":
        print("Bootstrap-variance calibration sweep")
        print("=" * 64)
        res = run_bootstrap_variant(n_seeds=args.n_seeds)
        print(f"  shape_stability    {res['shape_corr']:.4f}")
        print(f"  magnitude CV       {res['magnitude_cv']:.4f}")
        print(f"  magnitude mean     {res['magnitude_mean']:.6f}")
        print(f"  VERDICT: {res['verdict']}")
        make_summary_plot([res], args.out)
        print(f"Plot saved to {args.out}")
    elif args.mode == "compare":
        print("Variant A: sigma_weights (5-beamlet brimstone, n_seeds=20)")
        print("-" * 64)
        sigma_main = main(n_seeds=20, out_path="/tmp/sigma_calibration_sigma.png")
        sigma_main["label"] = "sigma_weights (m_vAdaptVariance)"
        sigma_main["n_beamlets"] = N_BEAMLETS

        print("\nVariant B: bootstrap variance (5-beamlet brimstone, B=15)")
        print("-" * 64)
        boot = run_bootstrap_variant(n_seeds=args.n_seeds, n_bootstrap=15)
        print(f"  shape_stability    {boot['shape_corr']:.4f}")
        print(f"  magnitude CV       {boot['magnitude_cv']:.4f}")
        print(f"  magnitude mean     {boot['magnitude_mean']:.6f}")
        print(f"  VERDICT: {boot['verdict']}")

        make_summary_plot([sigma_main, boot], args.out)
        print(f"\nComparison plot saved to {args.out}")

        print("\n" + "=" * 64)
        print("COMPARISON SUMMARY")
        print("=" * 64)
        print(f"  {'variant':<40}{'shape':>8}{'CV':>8}  verdict")
        print(f"  {'sigma_weights (m_vAdaptVariance)':<40}"
              f"{sigma_main['shape_corr']:>8.3f}{sigma_main['magnitude_cv']:>8.3f}"
              f"  {sigma_main['verdict'].split(':')[0]}")
        print(f"  {'bootstrap (voxel subsample, B=15)':<40}"
              f"{boot['shape_corr']:>8.3f}{boot['magnitude_cv']:>8.3f}"
              f"  {boot['verdict'].split(':')[0]}")
        print("=" * 64)
