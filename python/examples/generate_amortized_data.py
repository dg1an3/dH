#!/usr/bin/env python3
"""
Generate a corpus of (x_init, course_state, trajectory) tuples from the
real pybrimstone CG solver, for training the amortized course-prior
network.

Phase 1b deliverable per the prototype brief §5b acceptance criterion:
produce >=1000 tuples and write them to disk. Spot-checks the first
sample by re-running the prescription's objective at x_init versus the
recorded f_final to confirm CG actually reduced the objective.

Output: a pickle file at data/amortized_samples/corpus.pkl by default,
holding a list of Sample (with .trajectory populated).

Run:
    python python/examples/generate_amortized_data.py
    python python/examples/generate_amortized_data.py --n-plans 100 --per-plan 20

The script uses synthetic gaussian-bump dose operators and a single
KLDivTerm per plan, varied per draw. It does NOT consume DICOM input
yet -- that lift is deferred per the brief §3 ("Don't bother with real
DICOM corpus until the amortization itself is validated").

Disclaimer: training data emitted here is for the amortized-prior
prototype only. Do not use any model trained on it for actual planning.
"""

from __future__ import annotations

import argparse
import pickle
import sys
import time
from pathlib import Path

import numpy as np


def _setup_path() -> None:
    here = Path(__file__).resolve().parent
    repo_python = here.parent
    if str(repo_python) not in sys.path:
        sys.path.insert(0, str(repo_python))


_setup_path()

from pybrimstone.amortized.data import (
    build_random_plan_template,
    generate_real_samples,
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    p.add_argument("--n-plans", type=int, default=50,
                   help="Number of distinct plan templates.")
    p.add_argument("--per-plan", type=int, default=20,
                   help="Samples per plan template.")
    p.add_argument("--grid", type=int, nargs=3, default=(8, 8, 8),
                   metavar=("NX", "NY", "NZ"),
                   help="Dose-grid shape.")
    p.add_argument("--n-beamlets", type=int, default=16)
    p.add_argument("--n-target-voxels", type=int, default=40)
    p.add_argument("--max-iter", type=int, default=80,
                   help="CG max iterations per sample.")
    p.add_argument("--tol", type=float, default=1e-3)
    p.add_argument("--seed", type=int, default=0)
    p.add_argument("--out", type=Path,
                   default=Path("data/amortized_samples/corpus.pkl"),
                   help="Pickle output path (created if missing).")
    p.add_argument("--no-trajectory", action="store_true",
                   help="Skip trajectory recording (smaller pickle).")
    return p.parse_args()


def spot_check(samples: list, templates_by_name: dict) -> None:
    """For the first few samples, re-evaluate the objective at x_init and
    confirm the recorded f_final is lower.

    This is the brief's §5b acceptance check: 'does the recovered x_star
    actually reduce the objective relative to x_init when fed back to
    the solver?'. The PlanTemplate is needed to rebuild a Prescription
    identical to the one CG saw -- that's why we still have the
    templates_by_name dict in scope here.
    """
    from pybrimstone.course_prior import CoursePriorTerm
    from pybrimstone.numerics.parameter_transform import inv_transform

    print("\n--- spot check (first 3 samples) ---")
    for s in samples[:3]:
        template = templates_by_name[s.meta["plan"]]
        p = template.build_prescription()
        p.add_beamlet_term(
            CoursePriorTerm(s.course_state.mu.copy(), s.course_state.sigma_inv.copy())
        )
        params_init = inv_transform(s.x_init)
        f_init = p.evaluate_cost_only(params_init)
        f_final = s.meta["f_final"]
        ratio = f_final / max(abs(f_init), 1e-9)
        sign = "OK" if f_final < f_init else "WARN"
        print(
            f"[{sign}] {s.meta['plan']:>10s}  f(x_init)={f_init:.4f}  "
            f"f_final={f_final:.4f}  ratio={ratio:.3f}  "
            f"iters={s.meta['n_iter']:2d}  conv={s.meta['converged']}"
        )


def main() -> int:
    args = parse_args()

    print(f"Generating amortized-prior data corpus")
    print(f"  n_plans            : {args.n_plans}")
    print(f"  samples_per_plan   : {args.per_plan}")
    print(f"  total target       : {args.n_plans * args.per_plan}")
    print(f"  grid               : {tuple(args.grid)}")
    print(f"  n_beamlets         : {args.n_beamlets}")
    print(f"  n_target_voxels    : {args.n_target_voxels}")
    print(f"  max_iter           : {args.max_iter}")
    print(f"  tol                : {args.tol}")
    print(f"  trajectory         : {'no' if args.no_trajectory else 'yes'}")
    print(f"  out                : {args.out}")
    print()

    rng = np.random.default_rng(args.seed)

    print("[1/3] Building plan templates...")
    templates = [
        build_random_plan_template(
            rng,
            name=f"plan_{i:04d}",
            grid_shape=tuple(args.grid),
            n_beamlets=args.n_beamlets,
            n_target_voxels=args.n_target_voxels,
        )
        for i in range(args.n_plans)
    ]
    print(f"      {len(templates)} templates built")

    print("[2/3] Running CG to generate samples (this is the slow part)...")
    t0 = time.time()
    samples = generate_real_samples(
        templates,
        n_samples_per_template=args.per_plan,
        rng=rng,
        max_iter=args.max_iter,
        tol=args.tol,
        record_trajectory=not args.no_trajectory,
    )
    elapsed = time.time() - t0
    print(f"      {len(samples)} samples in {elapsed:.1f}s "
          f"({elapsed / len(samples) * 1000:.0f}ms/sample)")

    spot_check(samples, templates_by_name={t.name: t for t in templates})

    print("\n--- corpus summary ---")
    n_iters = np.array([s.meta["n_iter"] for s in samples])
    converged_frac = np.mean([s.meta["converged"] for s in samples])
    print(f"n_iter:   min={n_iters.min()}  median={int(np.median(n_iters))}  "
          f"max={n_iters.max()}  mean={n_iters.mean():.1f}")
    print(f"converged fraction: {converged_frac:.2%}")
    if not args.no_trajectory:
        tlens = np.array([len(s.trajectory) for s in samples])
        print(f"trajectory len: min={tlens.min()}  median={int(np.median(tlens))}  "
              f"max={tlens.max()}")

    args.out.parent.mkdir(parents=True, exist_ok=True)
    print(f"\n[3/3] Pickling to {args.out} ...")
    with open(args.out, "wb") as f:
        pickle.dump(samples, f)
    size_mb = args.out.stat().st_size / (1024 * 1024)
    print(f"      wrote {size_mb:.2f} MiB")

    print("\nDone.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
