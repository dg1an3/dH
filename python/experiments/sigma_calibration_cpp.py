"""Sigma-calibration experiment against the C++ optimizer.

HIERARCHICAL_BAYES_DESIGN.md records the verdict of sigma_calibration.py --
m_vAdaptVariance is an optimizer trace, not a calibrated posterior -- with the
caveat that it ran on the pure-Python port of DynamicCovarianceOptimizer, and
asks for a re-run on the C++ original once the bindings compile. This is that
re-run.

Same problems, same seeds, same statistics as sigma_calibration.py; only the
minimizer differs. The Python Prescription is handed to the C++
DynamicCovarianceOptimizer through rtmodel_core.PyCostFunction, so the Brent
line search (vnl_brent_minimizer), the Polak-Ribiere update and the
UpdateDynamicCovariance bookkeeping are all the C++ code paths that Brimstone
runs. Adaptive variance is read back with get_adaptive_variance() after
minimize(), which is the value the last iteration's UpdateDynamicCovariance
left -- what the Python callback harvests as sigma_weights.

Differences that cannot be matched and are reported rather than hidden:
  * ITER_MAX is a compile-time 500 in ConjGradOptimizer.cpp; the Python runs
    capped at 30-50. Iteration counts are printed per variant.
  * The Python PhaseOptimizer clips the line-search step (max_step_norm=20,
    the bounded-line-search fix). The C++ optimizer has no such clip, so this
    is the *unbounded* behavior the original ROW 3 verdict was about.
  * The C++ relative-change test uses vnl's x_tolerance; it is set to the
    same tol the Python runs used (1e-3, 1e-6 for the control).

Usage:
    python python/experiments/sigma_calibration_cpp.py [--n-seeds 20] [--no-python]
        [--module-dir <dir with rtmodel_core*.pyd>]
"""
from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(HERE))

from pybrimstone.native import import_rtmodel_core  # noqa: E402
import sigma_calibration as sc  # noqa: E402  (problem builders + stats)


# ---------------------------------------------------------------------------
# C++ optimizer driver
# ---------------------------------------------------------------------------

def cpp_minimize(rc, n, f, grad_f, x0, tol, line_tol=1e-4,
                 adaptive_variance=(0.01, 0.1)):
    """Run the C++ DynamicCovarianceOptimizer on (f, grad_f) from x0.

    Returns (x_final, f_final, n_iter, adaptive_variance_vector).
    """
    calls = {"f": 0, "g": 0}

    def fn(x, need_grad):
        if need_grad:
            calls["g"] += 1
            return float(f(x)), np.asarray(grad_f(x), dtype=np.float64)
        calls["f"] += 1
        return float(f(x)), None

    cost = rc.PyCostFunction(int(n), fn)
    opt = rc.ConjGradOptimizer(cost)
    opt.set_line_optimizer_tolerance(float(line_tol))
    opt.set_x_tolerance(float(tol))
    var_min, var_max = adaptive_variance
    opt.set_adaptive_variance(True, float(var_min), float(var_max))
    x_final = opt.minimize(np.asarray(x0, dtype=np.float64))
    av = np.asarray(opt.get_adaptive_variance(), dtype=np.float64)
    return x_final, float(opt.get_final_value()), int(opt.get_num_iterations()), av, calls


def _sweep(rc, label, n, build_problem, x0_of_seed, n_seeds, tol, max_iter_py,
           adaptive_variance=(0.01, 0.1)):
    """Run n_seeds C++ minimizations; build_problem(seed) -> (f, grad_f)."""
    if TOL_OVERRIDE is not None:
        tol = TOL_OVERRIDE
    vars_, iters, finals = [], [], []
    t0 = time.time()
    for s in range(n_seeds):
        f, grad_f = build_problem(s)
        _, f_final, n_iter, av, _ = cpp_minimize(rc, n, f, grad_f, x0_of_seed(s), tol,
                                                adaptive_variance=adaptive_variance)
        vars_.append(np.maximum(av, 1e-6))   # same floor as PhaseOptimizer
        iters.append(n_iter)
        finals.append(f_final)
    vars_ = np.stack(vars_)
    # UpdateDynamicCovariance divides by a column sum with no guard, so an
    # unsearched/degenerate direction yields inf. Count them, then exclude
    # them from the statistics (inf would poison the correlation and CV).
    n_inf = int(np.sum(~np.isfinite(vars_)))
    finite = np.where(np.isfinite(vars_), vars_, np.nan)
    stats_vars = np.where(np.isnan(finite), np.nanmax(finite), finite) if n_inf else vars_
    shape_corr = sc.shape_stability(stats_vars)
    mean_, std_, cv = sc.magnitude_stability(stats_vars)
    return {
        "label": label, "backend": "C++", "n_beamlets": n,
        "shape_corr": shape_corr, "magnitude_cv": cv,
        "magnitude_mean": mean_, "magnitude_std": std_,
        "iters_mean": float(np.mean(iters)), "iters_max": int(np.max(iters)),
        "f_final_mean": float(np.mean(finals)), "f_final_std": float(np.std(finals)),
        "n_inf": n_inf, "tol": tol,
        "python_max_iter": max_iter_py, "seconds": time.time() - t0,
        "vars": vars_, "verdict": sc.verdict(shape_corr, cv),
    }


TOL_OVERRIDE = None   # set by --tol to run every C++ variant at one tolerance


def _presc_problem(p):
    return (lambda x: p.evaluate_cost_only(x)), (lambda x: p.evaluate(x)[1])


# ---- variants mirroring sigma_calibration.py ------------------------------

def variant_bump5(rc, n_seeds):
    def build(seed):
        p, _ = sc.build_fixed_prescription()
        return _presc_problem(p)
    x0 = lambda s: np.random.default_rng(s).normal(size=sc.N_BEAMLETS) * 0.5
    return _sweep(rc, "Gaussian-bump 5-beamlet, sigma_weights", sc.N_BEAMLETS,
                  build, x0, n_seeds, tol=1e-3, max_iter_py=30)


def variant_bump20(rc, n_seeds, n_beamlets=20):
    from pybrimstone import KLDivTerm, Prescription, gaussian_bump_dose_operator
    centers = np.array([[3.0 + 0.05 * b, 3.0 + 0.05 * b, 3.0 + (b / n_beamlets) * 2.0]
                        for b in range(n_beamlets)])
    D = gaussian_bump_dose_operator(sc.GRID, centers, sigma=1.5)
    ptv = sc._make_mask(lambda i, j, k: 2 <= i <= 4 and 2 <= j <= 4 and 3 <= k <= 5)

    def build(seed):
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            ptv, dose_min=0.55, dose_max=0.75, weight=2.0,
            bin_width=0.05, var_min=0.01, var_max=0.01))
        return _presc_problem(p)
    x0 = lambda s: np.random.default_rng(s).normal(size=n_beamlets) * 0.5
    return _sweep(rc, f"Gaussian-bump {n_beamlets}-beamlet, sigma_weights", n_beamlets,
                  build, x0, n_seeds, tol=1e-3, max_iter_py=50)


def variant_quadratic(rc, n_seeds, n_dim=10):
    scales = np.arange(1.0, n_dim + 1)
    f = lambda x: float(np.sum(scales * x * x))
    grad = lambda x: 2.0 * scales * x
    x0 = lambda s: np.random.default_rng(s).normal(size=n_dim) * 2.0
    return _sweep(rc, f"Pure quadratic {n_dim}-D (control)", n_dim,
                  lambda seed: (f, grad), x0, n_seeds, tol=1e-6, max_iter_py=50)


def variant_terma(rc, n_seeds):
    from pybrimstone import KLDivTerm, Prescription
    calc, _ = sc._build_terma_calc()
    D = calc.build_dose_operator()
    structures = sc._build_terma_structures(sc.TERMA_GRID)

    def build(seed):
        p = Prescription(D, use_transform=True)
        p.add_dose_term(KLDivTerm.from_interval(
            structures["PTV"], dose_min=0.005, dose_max=0.010, weight=2.0,
            bin_width=0.001, var_min=1e-6, var_max=1e-6))
        for oar in ("OAR_anterior", "OAR_posterior"):
            p.add_dose_term(KLDivTerm.from_interval(
                structures[oar], dose_min=0.0, dose_max=0.003, weight=1.0,
                bin_width=0.001, var_min=1e-6, var_max=1e-6))
        return _presc_problem(p)
    n = sc.TERMA_N_BEAMLETS
    x0 = lambda s: np.random.default_rng(s).normal(size=n) * 0.5
    return _sweep(rc, "TERMA + kernel 5-beamlet, sigma_weights", n,
                  build, x0, n_seeds, tol=1e-3, max_iter_py=40)


# ---- Python port, same session, for a like-for-like comparison ------------

def python_results(n_seeds):
    out = []
    r = sc.main(n_seeds=n_seeds, out_path=str(HERE / "_sigma_calibration_py.png")) \
        if hasattr(sc, "main") else None
    if r is not None:
        r = dict(r); r["label"] = "Gaussian-bump 5-beamlet, sigma_weights"; out.append(r)
    out.append(sc.run_higher_dim_variant(20, n_seeds))
    out.append(sc.run_quadratic_control(10, n_seeds))
    out.append(sc.run_terma_sigma_variant(n_seeds))
    for r in out:
        r["backend"] = "Python"
    return out


# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n-seeds", type=int, default=20)
    ap.add_argument("--no-python", action="store_true",
                    help="skip re-running the Python port for comparison")
    ap.add_argument("--module-dir", default=None,
                    help="directory holding rtmodel_core*.pyd (default: newest build/*/bin/*)")
    ap.add_argument("--tol", type=float, default=None,
                    help="override the relative-change tolerance for every C++ variant "
                         "(the per-variant defaults match the Python runs: 1e-3, 1e-6 control)")
    args = ap.parse_args()
    global TOL_OVERRIDE
    TOL_OVERRIDE = args.tol

    rc = import_rtmodel_core(args.module_dir)
    print("rtmodel_core:", rc.__file__)
    if TOL_OVERRIDE is not None:
        print("C++ tolerance override:", TOL_OVERRIDE)

    cpp = [variant_bump5(rc, args.n_seeds),
           variant_bump20(rc, args.n_seeds),
           variant_quadratic(rc, args.n_seeds),
           variant_terma(rc, args.n_seeds)]
    py = [] if args.no_python else python_results(args.n_seeds)

    print("\n%-44s %-7s %10s %12s %14s %9s %5s %s" % (
        "variant", "backend", "shape corr", "magnitude CV", "magnitude mean", "iters", "inf", "verdict"))
    for r in cpp + py:
        iters = ("%.1f/%d" % (r["iters_mean"], r["iters_max"])) if "iters_mean" in r else "-"
        ninf = str(r.get("n_inf", "-"))
        print("%-44s %-7s %10.3f %12.3f %14.4g %9s %5s %s" % (
            r["label"][:44], r["backend"], r["shape_corr"], r["magnitude_cv"],
            r["magnitude_mean"], iters, ninf, r["verdict"].split(":")[0]))

    # side-by-side: variants are built in the same order in both lists
    if py:
        print("\nC++ vs Python (same seeds, same problems):")
        for r, q in zip(cpp, py):
            print("  %-44s shape %.3f vs %.3f   CV %.3f vs %.3f   verdict %s vs %s" % (
                r["label"][:44], r["shape_corr"], q["shape_corr"],
                r["magnitude_cv"], q["magnitude_cv"], r["verdict"], q["verdict"]))
    return cpp, py


if __name__ == "__main__":
    main()
