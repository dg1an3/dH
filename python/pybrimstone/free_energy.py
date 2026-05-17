"""
Variational free-energy diagnostics for the hierarchical-Bayes driver.

Implements Step 4 of HIERARCHICAL_BAYES_DESIGN.md: validates that the
coordinate-ascent updates in HierarchicalBayes monotonically decrease the
total free energy

    F_total = sum_p F_p

where each phase contributes the brimstone-style free energy

    F_p = data_cost_p(mu_p) + prior_pull(mu_p, mu_eta, precision_eta) - H(q_p)

with data_cost_p the inner-optimization data term (analog of the C++
m_FinalValue minus the prior cost), prior_pull = 0.5 * sum(precision_eta *
(mu_p - mu_eta)^2) the CoursePriorTerm cost, and H(q_p) the Gaussian
posterior entropy 0.5 * sum(log(2*pi*e*var_p_i)).

In C++ this would correspond to summing each phase's
    DynamicCovarianceOptimizer::GetFreeEnergy()  (ConjGradOptimizer.h:54)
after SetComputeFreeEnergy(true), plus the Course-level prior cost
between phases. This module provides the Python-side reference for the
same quantity so the math is testable with the Step-3 driver before
the C++ wrapper rounds out free-energy exposure.

Monotonicity holds when var_p is the *data-only* posterior variance.
If var_p includes the prior (the double-counting case from Step 3),
F_total can drift; the test suite documents this with a contrasting
case.
"""

from __future__ import annotations

from typing import Callable, List, Sequence

import numpy as np


DataCostFn = Callable[[np.ndarray], float]


def gaussian_entropy_diag(variance: np.ndarray) -> float:
    """Entropy of a diagonal Gaussian: 0.5 * sum(log(2*pi*e*var_i))."""
    variance = np.asarray(variance, dtype=np.float64)
    if np.any(variance <= 0):
        raise ValueError("variance must be strictly positive")
    return 0.5 * float(np.sum(np.log(2.0 * np.pi * np.e * variance)))


def phase_free_energy(
    data_cost: float,
    mu_p: np.ndarray,
    var_p: np.ndarray,
    mu_eta: np.ndarray,
    precision_eta: np.ndarray,
) -> float:
    """
    Single-phase variational free energy under the hierarchical prior.

    F_p = data_cost(mu_p)
        + 0.5 * sum(precision_eta_i * (mu_p_i - mu_eta_i)^2)    # prior pull
        - 0.5 * sum(log(2*pi*e*var_p_i))                         # entropy

    Args:
        data_cost: scalar inner-optimization data cost evaluated at mu_p
            (i.e., the inner J_p with the prior term removed).
        mu_p, var_p: phase-level posterior mean and variance (per-beamlet).
        mu_eta, precision_eta: Course-level prior parameters (per-beamlet).
    """
    diff = mu_p - mu_eta
    prior_pull = 0.5 * float(np.sum(precision_eta * diff * diff))
    entropy = gaussian_entropy_diag(var_p)
    return float(data_cost) + prior_pull - entropy


def total_free_energy(
    history_entry: dict,
    data_cost_fns: Sequence[DataCostFn],
) -> float:
    """
    Sum F_p across all phases for one entry of HierarchicalBayes.history.

    Args:
        history_entry: a single dict from HierarchicalBayes.run()['history'].
            Expects keys: mu_eta, var_eta, phase_mus, phase_vars.
        data_cost_fns: P callables, one per phase, each taking a beamlet
            vector and returning the scalar data-only cost.
    """
    mu_eta = history_entry["mu_eta"]
    var_eta = history_entry["var_eta"]
    precision_eta = 1.0 / var_eta

    phase_mus = history_entry["phase_mus"]
    phase_vars = history_entry["phase_vars"]
    if len(data_cost_fns) != len(phase_mus):
        raise ValueError(
            f"data_cost_fns ({len(data_cost_fns)}) must match phase count "
            f"({len(phase_mus)})"
        )

    total = 0.0
    for data_cost_fn, mu_p, var_p in zip(data_cost_fns, phase_mus, phase_vars):
        total += phase_free_energy(
            data_cost=data_cost_fn(mu_p),
            mu_p=mu_p,
            var_p=var_p,
            mu_eta=mu_eta,
            precision_eta=precision_eta,
        )
    return total


def free_energy_trajectory(
    history: Sequence[dict],
    data_cost_fns: Sequence[DataCostFn],
) -> List[float]:
    """F_total evaluated at every outer iteration in driver.history."""
    return [total_free_energy(h, data_cost_fns) for h in history]
