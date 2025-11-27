# Variational Bayes Optimization with RtModel

This directory contains Python bindings and tools for enhancing RtModel's variational Bayes optimization with better Hessian approximations and entropy estimation.

## Overview

The current RtModel implementation uses:
- **Conjugate Gradient** optimization with adaptive variance
- **Ad-hoc covariance approximation** using orthogonalized search directions
- **Optional free energy calculation**: F = KL_divergence - Entropy

This can be significantly improved by:
1. **Using L-BFGS-B** for better Hessian approximation (scipy)
2. **Training ML models** to learn Hessian from optimization history
3. **More accurate entropy** for variational free energy

## Files

### Core Bindings

- **`rtmodel_bindings.cpp`** - pybind11 bindings exposing RtModel to Python
  - Wraps `Prescription`, `CVectorN`, `DynamicCovarianceOptimizer`
  - Provides numpy array conversion
  - Enables Python-based optimization

- **`CMakeLists.txt`** - Build configuration for pybind11 module
- **`setup.py`** - Python package setup (uses CMake)

### Optimization Methods

- **`optimize_with_lbfgs.py`** - Use scipy's L-BFGS-B optimizer
  - Better Hessian approximation than conjugate gradient
  - Comparison with current CG optimizer
  - Notes on entropy extraction

- **`learned_optimizer.py`** - ML-based optimizer learning
  - Neural network that mimics L-BFGS behavior
  - Trained on optimization trajectories
  - Learns problem-specific curvature patterns
  - Can be trained on multiple patient cases

- **`learned_hessian_entropy.py`** - ML for entropy estimation
  - **Key innovation**: Neural network outputs Cholesky factor of inverse Hessian
  - Direct entropy calculation from learned covariance
  - More accurate than hand-crafted formulas
  - Trained on numerical Hessian ground truth

## Why ML for Hessian Approximation?

### Traditional L-BFGS
- Stores last m pairs: `{(s_k, y_k)}` where `s_k = step`, `y_k = gradient change`
- Uses two-loop recursion to approximate `H^{-1} @ g`
- **Generic formula** - doesn't exploit problem structure

### Learned Approach
- **Same input**: History of `{(s_k, y_k)}` pairs from optimization
- **Neural network** learns to predict better Hessian approximation
- **Trained on radiotherapy cases** - learns DVH optimization structure
- **Better entropy** for variational free energy

### Key Advantages

| Aspect | L-BFGS | Learned Optimizer |
|--------|--------|-------------------|
| Hessian approximation | Two-loop recursion | Neural network |
| Problem-specific | ❌ Generic | ✅ Learns from data |
| Training required | ❌ No | ✅ Offline training |
| Convergence | Standard | Potentially faster |
| Entropy accuracy | Good | Better (learned from ground truth) |
| Transferability | N/A | ✅ Across similar patients |

## Usage

### 1. Build Python Bindings

```bash
cd python
pip install pybind11 numpy scipy torch

# Option A: Using setup.py
python setup.py install

# Option B: Using CMake directly
mkdir build && cd build
cmake ..
make
```

### 2. Basic L-BFGS Optimization

```python
import rtmodel_core
from optimize_with_lbfgs import optimize_with_lbfgs

# Load your prescription (from RtModel C++)
prescription = ...  # Create via bindings

# Wrap for Python optimization
wrapper = rtmodel_core.PrescriptionWrapper(prescription)

# Initial guess
x0 = np.zeros(wrapper.get_dimension())

# Optimize with L-BFGS-B
result = optimize_with_lbfgs(wrapper, x0)

print(f"Final objective: {result.fun}")
```

### 3. Train Learned Optimizer

```python
import torch
from learned_optimizer import HessianApproximatorNetwork, train_on_multiple_problems

# Create network
param_dim = 1000  # Number of beamlets
model = HessianApproximatorNetwork(param_dim=param_dim, history_length=10)

# Collect objective functions from multiple patients
objective_functions = []
initial_guesses = []

for patient_case in patient_dataset:
    prescription = load_prescription(patient_case)
    wrapper = rtmodel_core.PrescriptionWrapper(prescription)

    obj_fn = lambda x: wrapper.evaluate(x)
    x0 = np.zeros(param_dim)

    objective_functions.append(obj_fn)
    initial_guesses.append(x0)

# Train on all cases
model = train_on_multiple_problems(
    model,
    objective_functions,
    initial_guesses,
    epochs_per_problem=10
)

# Save trained model
torch.save(model.state_dict(), 'learned_optimizer.pt')
```

### 4. Better Entropy with Learned Hessian

```python
from learned_hessian_entropy import CholeskyHessianNetwork

# Create Hessian network (outputs covariance Cholesky factor)
hessian_model = CholeskyHessianNetwork(param_dim=param_dim, history_length=20)

# Train on optimization runs
trainer = HessianEntropyTrainer(hessian_model)

for patient_case in patient_dataset:
    prescription = load_prescription(patient_case)
    wrapper = rtmodel_core.PrescriptionWrapper(prescription)
    obj_fn = lambda x: wrapper.evaluate(x)
    x0 = np.zeros(param_dim)

    trainer.train_on_optimization_run(obj_fn, x0, n_steps=100, epochs=20)

# Use for entropy estimation
import torch
history = collect_optimization_history(...)  # from L-BFGS run
history_tensor = torch.from_numpy(history).float().unsqueeze(0)

# Get entropy directly
entropy = hessian_model.compute_entropy(history_tensor)

# Compute variational free energy
kl_divergence = prescription.final_objective_value
free_energy = kl_divergence - entropy.item()

print(f"Free Energy: {free_energy:.6f}")
```

## Comparison: CG vs L-BFGS vs Learned

### Current (Conjugate Gradient + Ad-hoc Covariance)
```
Σ = Q^T · D · Q
```
- **Q**: Orthogonalized CG directions (Gram-Schmidt)
- **D**: Diagonal with `pow(4.0, i)` scaling ❌ **No theoretical basis**
- **Entropy**: From this ad-hoc covariance

### L-BFGS-B (scipy)
```
H^{-1} ≈ Two-loop recursion on {(s_k, y_k)}
```
- ✅ **Theoretically grounded**: H ≈ ∇²f at convergence
- ✅ **Superlinear convergence**
- ⚠️ scipy doesn't expose H directly (need custom impl)

### Learned Hessian Network
```
L = NeuralNet({(s_k, y_k)})
Σ = L @ L^T
```
- ✅ **Trained on ground truth**: Numerical Hessian
- ✅ **Direct entropy**: H = 0.5 * (n log(2πe) + 2 Σ log(diag(L)))
- ✅ **Problem-specific**: Learns radiotherapy structure
- ✅ **Transfer learning**: Across patients with similar anatomy

## Theoretical Foundation

### Variational Free Energy
For variational Bayes with Gaussian posterior q(θ):

```
F = E_q[log p(D|θ)] - KL[q(θ) || p(θ)]
  ≈ -KL_divergence - Entropy[q(θ)]
```

where:
- **KL_divergence**: Sum of DVH KL terms (from RtModel `Prescription`)
- **Entropy[q(θ)]**: `0.5 * log(det(2πe Σ))`
- **Σ**: Covariance of variational posterior ≈ H^{-1} (inverse Hessian)

### Why Better Hessian → Better Free Energy

1. **Current method**: Ad-hoc scaling → incorrect Σ → wrong entropy
2. **L-BFGS**: Principled H^{-1} approximation → better entropy
3. **Learned**: Trained on numerical Hessian → best entropy

More accurate entropy → more accurate free energy → better optimization objective for variational Bayes.

## Next Steps

1. **Build and test bindings** on actual RtModel data
2. **Collect training dataset** - Run L-BFGS on 50-100 patient cases
3. **Train learned Hessian network** on collected trajectories
4. **Compare convergence**: CG vs L-BFGS vs Learned
5. **Validate entropy estimates** against numerical Hessian
6. **Deploy** for clinical planning

## References

### L-BFGS Theory
- Nocedal & Wright, "Numerical Optimization" (2006), Chapter 7

### Learned Optimizers
- Andrychowicz et al., "Learning to Learn by Gradient Descent by Gradient Descent" (NeurIPS 2016)
- Metz et al., "Learned Optimizers that Scale and Generalize" (ICML 2022)

### Variational Inference
- Blei et al., "Variational Inference: A Review for Statisticians" (JASA 2017)
- Bishop, "Pattern Recognition and Machine Learning" (2006), Chapter 10

## License

See main LICENSE file. U.S. Patent 7,369,645 applies.

Copyright (c) 2007-2025, Derek G. Lane
