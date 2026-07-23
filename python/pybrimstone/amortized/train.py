"""
Training loop for the amortized course-prior network.

Phase 1a: real training on the toy generator. MSE on delta_x with
Adam, mini-batches, an input scaler fit on the training split, and a
per-epoch train/val log. The shape-validating stub from Phase 0
became train_check() and still runs first so misconfigured corpora
fail before any work happens.

Output: a TrainResult bundling the trained model, the fitted input
scaler, and the per-epoch metrics. The notebook unpacks this and
plots the curve.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Optional, Tuple

import numpy as np
import torch
import torch.nn as nn

from .config import PrototypeConfig
from .data import samples_to_arrays
from .network import AmortizedCoursePrior
from .types import Sample


# ---------------------------------------------------------------------------
# Input scaling
# ---------------------------------------------------------------------------


@dataclass
class StandardScaler:
    """Per-feature mean/std normalization, fit on a training array.

    No external dependency on sklearn — the prototype is small enough
    that a 10-line scaler is cleaner than pulling another package.
    Floors std at 1e-8 so constant columns don't divide by zero.
    """

    mean_x: Optional[np.ndarray] = None
    std_x: Optional[np.ndarray] = None
    mean_c: Optional[np.ndarray] = None
    std_c: Optional[np.ndarray] = None

    def fit(self, x: np.ndarray, c: np.ndarray) -> "StandardScaler":
        self.mean_x = x.mean(axis=0)
        self.std_x = np.maximum(x.std(axis=0), 1e-8)
        self.mean_c = c.mean(axis=0)
        self.std_c = np.maximum(c.std(axis=0), 1e-8)
        return self

    def transform(
        self, x: np.ndarray, c: np.ndarray
    ) -> Tuple[np.ndarray, np.ndarray]:
        if self.mean_x is None:
            raise RuntimeError("StandardScaler.transform called before fit")
        return (x - self.mean_x) / self.std_x, (c - self.mean_c) / self.std_c


# ---------------------------------------------------------------------------
# Phase 0 shape-only check (still useful)
# ---------------------------------------------------------------------------


def train_check(samples: List[Sample], config: PrototypeConfig) -> None:
    """Shape-validate a sample list against the config. Raises on mismatch."""
    if not samples:
        raise ValueError("train() needs at least one Sample")
    for i, s in enumerate(samples):
        if s.x_init.shape != (config.phase_dim,):
            raise ValueError(
                f"sample[{i}].x_init shape {s.x_init.shape} != "
                f"(phase_dim={config.phase_dim},)"
            )
        if s.delta_x.shape != (config.phase_dim,):
            raise ValueError(
                f"sample[{i}].delta_x shape {s.delta_x.shape} != "
                f"(phase_dim={config.phase_dim},)"
            )
        course_flat_dim = int(np.asarray(s.course_state.flatten()).shape[0])
        if course_flat_dim != config.course_dim:
            raise ValueError(
                f"sample[{i}].course_state.flatten() length {course_flat_dim} != "
                f"course_dim={config.course_dim}"
            )


# ---------------------------------------------------------------------------
# Training result
# ---------------------------------------------------------------------------


@dataclass
class TrainResult:
    model: AmortizedCoursePrior
    scaler: StandardScaler
    train_loss: List[float] = field(default_factory=list)
    val_loss: List[float] = field(default_factory=list)
    config: Optional[PrototypeConfig] = None
    baseline_zero_val_mse: Optional[float] = None
    baseline_linear_val_mse: Optional[float] = None
    # Best-snapshot val MSE (under keep_best=True this equals the val MSE of
    # the returned weights; under keep_best=False it equals val_loss[-1]).
    best_val_mse: Optional[float] = None


# ---------------------------------------------------------------------------
# Baselines (for the eval comparison)
# ---------------------------------------------------------------------------


def _zero_predictor_mse(delta_val: np.ndarray) -> float:
    """Constant-zero prediction MSE — the trivial lower bar."""
    return float(np.mean(delta_val ** 2))


def _linear_baseline_mse(
    x_train: np.ndarray,
    c_train: np.ndarray,
    d_train: np.ndarray,
    x_val: np.ndarray,
    c_val: np.ndarray,
    d_val: np.ndarray,
) -> float:
    """Linear least-squares baseline: delta ≈ W [x; c; 1] via lstsq.

    Per the analysis in notes/coursepriorterm_audit.md the optimal
    predictor has a bilinear lam*mu term that linear regression cannot
    fit — so this baseline should be strictly easier to beat than the
    MLP for a sufficiently expressive MLP. If the MLP cannot beat this,
    something is wrong.
    """
    n_train = x_train.shape[0]
    n_val = x_val.shape[0]
    A_train = np.concatenate([x_train, c_train, np.ones((n_train, 1))], axis=1)
    A_val = np.concatenate([x_val, c_val, np.ones((n_val, 1))], axis=1)
    W, *_ = np.linalg.lstsq(A_train, d_train, rcond=None)
    pred = A_val @ W
    return float(np.mean((pred - d_val) ** 2))


# ---------------------------------------------------------------------------
# Training
# ---------------------------------------------------------------------------


def train(
    samples: List[Sample],
    config: PrototypeConfig,
    *,
    verbose: bool = False,
) -> TrainResult:
    """Train an AmortizedCoursePrior on a list of Samples.

    Splits into train/val per config.val_fraction, fits a StandardScaler
    on the training inputs, runs Adam for config.epochs, logs MSE per
    epoch, and computes two baselines (constant-zero, linear regression)
    on the val split for the eval table.
    """
    train_check(samples, config)

    rng = np.random.default_rng(config.seed)
    torch.manual_seed(config.seed)

    arr = samples_to_arrays(samples)
    n = arr["x_init"].shape[0]
    perm = rng.permutation(n)
    n_val = max(1, int(round(n * config.val_fraction)))
    val_idx = perm[:n_val]
    train_idx = perm[n_val:]

    x_train, c_train, d_train = (
        arr["x_init"][train_idx],
        arr["course"][train_idx],
        arr["delta_x"][train_idx],
    )
    x_val, c_val, d_val = (
        arr["x_init"][val_idx],
        arr["course"][val_idx],
        arr["delta_x"][val_idx],
    )

    scaler = StandardScaler().fit(x_train, c_train)
    xs_train, cs_train = scaler.transform(x_train, c_train)
    xs_val, cs_val = scaler.transform(x_val, c_val)

    # Baselines computed on UNscaled inputs — the MSE target space is the
    # raw delta_x, and the baselines should consume the same input the
    # MLP gets (post-scale) so this is apples-to-apples.
    baseline_zero = _zero_predictor_mse(d_val)
    baseline_linear = _linear_baseline_mse(
        xs_train, cs_train, d_train, xs_val, cs_val, d_val
    )

    model = AmortizedCoursePrior(
        phase_dim=config.phase_dim,
        course_dim=config.course_dim,
        hidden=config.hidden,
    )
    opt = torch.optim.Adam(
        model.parameters(),
        lr=config.lr,
        weight_decay=getattr(config, "weight_decay", 0.0),
    )
    loss_fn = nn.MSELoss()

    xs_train_t = torch.from_numpy(xs_train.astype(np.float32))
    cs_train_t = torch.from_numpy(cs_train.astype(np.float32))
    d_train_t = torch.from_numpy(d_train.astype(np.float32))
    xs_val_t = torch.from_numpy(xs_val.astype(np.float32))
    cs_val_t = torch.from_numpy(cs_val.astype(np.float32))
    d_val_t = torch.from_numpy(d_val.astype(np.float32))

    result = TrainResult(
        model=model,
        scaler=scaler,
        config=config,
        baseline_zero_val_mse=baseline_zero,
        baseline_linear_val_mse=baseline_linear,
    )

    n_train = xs_train_t.shape[0]
    batch_size = max(1, int(config.batch_size))
    keep_best = bool(getattr(config, "keep_best", True))
    best_val = float("inf")
    best_state = None

    for epoch in range(int(config.epochs)):
        model.train()
        idx = torch.randperm(n_train)
        epoch_losses: List[float] = []
        for start in range(0, n_train, batch_size):
            sl = idx[start:start + batch_size]
            xb, cb, db = xs_train_t[sl], cs_train_t[sl], d_train_t[sl]
            pred = model(xb, cb)
            loss = loss_fn(pred, db)
            opt.zero_grad()
            loss.backward()
            opt.step()
            epoch_losses.append(float(loss.item()))

        train_mse = float(np.mean(epoch_losses)) if epoch_losses else float("nan")

        model.eval()
        with torch.no_grad():
            val_pred = model(xs_val_t, cs_val_t)
            val_mse = float(loss_fn(val_pred, d_val_t).item())

        result.train_loss.append(train_mse)
        result.val_loss.append(val_mse)

        if keep_best and val_mse < best_val:
            best_val = val_mse
            best_state = {k: v.detach().clone() for k, v in model.state_dict().items()}

        if verbose:
            print(
                f"epoch {epoch:3d}  train {train_mse:.4f}  val {val_mse:.4f}  "
                f"baselines: zero={baseline_zero:.4f}  linear={baseline_linear:.4f}"
            )

    if keep_best and best_state is not None:
        model.load_state_dict(best_state)
        result.best_val_mse = best_val
    else:
        result.best_val_mse = result.val_loss[-1] if result.val_loss else None

    return result
