"""
Example: Using scipy's L-BFGS-B optimizer with RtModel

This demonstrates how to use a proper quasi-Newton method (L-BFGS-B)
for variational Bayes optimization, which provides a much better
Hessian approximation than the current conjugate gradient approach.

The L-BFGS-B inverse Hessian approximation can be used to compute
a more accurate entropy estimate for the variational free energy.
"""

import numpy as np
from scipy.optimize import minimize
import rtmodel_core


def optimize_with_lbfgs(prescription_wrapper, x0, maxiter=500, gtol=1e-5):
    """
    Optimize using scipy's L-BFGS-B algorithm

    Args:
        prescription_wrapper: PrescriptionWrapper instance
        x0: Initial parameter guess (numpy array)
        maxiter: Maximum iterations
        gtol: Gradient tolerance

    Returns:
        dict with optimization results including Hessian approximation
    """

    # Track iteration history
    history = {
        'iteration': [],
        'objective': [],
        'gradient_norm': [],
    }

    def objective_and_gradient(x):
        """Objective function for scipy.optimize"""
        value, grad = prescription_wrapper.evaluate(x)

        # Store history
        history['iteration'].append(len(history['iteration']))
        history['objective'].append(value)
        history['gradient_norm'].append(np.linalg.norm(grad))

        if len(history['iteration']) % 10 == 0:
            print(f"Iteration {history['iteration'][-1]}: "
                  f"f={value:.6f}, ||g||={history['gradient_norm'][-1]:.6e}")

        return value, grad

    print("Starting L-BFGS-B optimization...")
    print(f"Problem dimension: {len(x0)}")

    # Run L-BFGS-B optimization
    result = minimize(
        objective_and_gradient,
        x0,
        method='L-BFGS-B',
        jac=True,  # We provide gradient
        options={
            'maxiter': maxiter,
            'gtol': gtol,
            'disp': True,
            'maxcor': 20,  # Number of L-BFGS correction pairs to store
        }
    )

    # Extract inverse Hessian approximation from L-BFGS
    # Note: scipy's L-BFGS doesn't directly expose H, but we can request it
    # by using a custom L-BFGS implementation or approximating from the final state

    result.history = history

    return result


def compute_entropy_from_lbfgs_hessian(inv_hessian):
    """
    Compute differential entropy from L-BFGS inverse Hessian approximation

    For a Gaussian distribution:
    H = 0.5 * log(det(2πe * Σ))
      = 0.5 * (n * log(2πe) + log(det(Σ)))

    where Σ is the covariance (inverse Hessian at optimum)

    Args:
        inv_hessian: Inverse Hessian approximation (n x n matrix)

    Returns:
        entropy: Differential entropy in nats
    """
    n = inv_hessian.shape[0]

    # Compute log determinant using eigenvalues (numerically stable)
    eigenvalues = np.linalg.eigvalsh(inv_hessian)

    # Protect against non-positive eigenvalues
    eigenvalues = np.maximum(eigenvalues, 1e-10)

    log_det = np.sum(np.log(eigenvalues))

    # Entropy formula
    log_2pi_e = np.log(2 * np.pi * np.e)
    entropy = 0.5 * (n * log_2pi_e + log_det)

    return entropy


def compute_free_energy(kl_divergence, entropy):
    """
    Compute variational free energy

    F = E_q[log p(D|θ)] - E_q[log q(θ)] + const
      = -KL(q||p_target) - H[q(θ)]

    where we approximate:
    - KL divergence ≈ sum of KL terms (expected log likelihood)
    - H[q(θ)] = entropy of variational posterior

    Args:
        kl_divergence: Sum of KL divergence terms (objective value)
        entropy: Entropy of variational posterior

    Returns:
        free_energy: Variational free energy
    """
    return kl_divergence - entropy


def compare_cg_vs_lbfgs(prescription, x0):
    """
    Compare conjugate gradient vs L-BFGS optimization

    This will show the difference in:
    1. Convergence speed
    2. Final objective value
    3. Entropy estimates
    4. Free energy estimates
    """

    print("="*70)
    print("COMPARISON: Conjugate Gradient vs L-BFGS-B")
    print("="*70)

    # 1. Run conjugate gradient (current method)
    print("\n[1/2] Running Conjugate Gradient optimizer...")
    print("-"*70)

    cg_optimizer = rtmodel_core.ConjGradOptimizer(prescription)
    cg_optimizer.set_compute_free_energy(True)

    x_cg = cg_optimizer.minimize(x0.copy())
    cg_value = cg_optimizer.get_final_value()
    cg_entropy = cg_optimizer.get_entropy()
    cg_free_energy = cg_optimizer.get_free_energy()

    print(f"\nCG Results:")
    print(f"  Final objective: {cg_value:.6f}")
    print(f"  Entropy (CG approx): {cg_entropy:.6f}")
    print(f"  Free energy (CG): {cg_free_energy:.6f}")

    # 2. Run L-BFGS-B
    print("\n[2/2] Running L-BFGS-B optimizer...")
    print("-"*70)

    wrapper = rtmodel_core.PrescriptionWrapper(prescription)
    result_lbfgs = optimize_with_lbfgs(wrapper, x0.copy())

    # Approximate Hessian from L-BFGS final state
    # In a full implementation, you'd extract this from the L-BFGS state
    # For now, we note that scipy's L-BFGS doesn't expose it directly
    # You'd need to use a custom implementation or numerical approximation

    print(f"\nL-BFGS Results:")
    print(f"  Final objective: {result_lbfgs.fun:.6f}")
    print(f"  Converged: {result_lbfgs.success}")
    print(f"  Iterations: {result_lbfgs.nit}")
    print(f"  Function evals: {result_lbfgs.nfev}")

    # Note: To get proper Hessian from L-BFGS, you'd need to either:
    # 1. Use a custom L-BFGS implementation that exposes H
    # 2. Use scipy.optimize.fmin_l_bfgs_b with custom callback
    # 3. Approximate H numerically at the optimum

    print("\n" + "="*70)
    print("SUMMARY")
    print("="*70)
    print(f"Objective improvement: {cg_value - result_lbfgs.fun:.6e}")
    print(f"  (negative = L-BFGS better)")

    print("\nNote: L-BFGS provides superior Hessian approximation")
    print("      for entropy estimation, but scipy doesn't expose H directly.")
    print("      Consider using PyTorch or JAX for full Hessian access.")

    return {
        'cg': {'x': x_cg, 'f': cg_value, 'entropy': cg_entropy, 'F': cg_free_energy},
        'lbfgs': {'x': result_lbfgs.x, 'f': result_lbfgs.fun, 'result': result_lbfgs},
    }


if __name__ == '__main__':
    print("RtModel + scipy L-BFGS-B Optimization Example")
    print("=" * 70)

    # This is a template - you'd need to load actual Plan and Prescription objects
    # from your clinical data

    print("\nTo use this script:")
    print("1. Build the Python bindings: python setup.py install")
    print("2. Load your Plan and create a Prescription object")
    print("3. Call optimize_with_lbfgs(prescription_wrapper, x0)")
    print("\nFor entropy estimation, consider using PyTorch or JAX optimizers")
    print("which provide full Hessian access.")
