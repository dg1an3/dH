# Amortized Optimization and ML-Based Sigma Estimation for dH Algorithm

This document describes how to implement amortized optimization for the dH (variational Bayes) algorithm and how to train machine learning models to estimate sigma parameters.

## Table of Contents

1. [Background](#background)
2. [Amortized Optimization](#amortized-optimization)
3. [ML-Based Sigma Estimation](#ml-based-sigma-estimation)
4. [Implementation Guide](#implementation-guide)
5. [Training Pipeline](#training-pipeline)
6. [Integration with Existing Codebase](#integration-with-existing-codebase)

---

## Background

### Current dH Algorithm Overview

The dH algorithm implements variational Bayes optimization for radiotherapy treatment planning. Key components:

**Multi-Scale Pyramid** (4 levels):
```
Level 3 (coarsest): σ = 8.0 mm
Level 2:            σ = 3.2 mm
Level 1:            σ = 1.3 mm
Level 0 (finest):   σ = 0.5 mm
```

**Optimization Pipeline**:
```
For each pyramid level (coarse → fine):
    1. Initialize beamlet weights
    2. Run conjugate gradient optimization (~500 iterations)
    3. Compute adaptive variance via Gram-Schmidt orthogonalization
    4. Transfer weights to next finer level
```

**Sigma Parameters in Current Implementation**:
| Parameter | Location | Value | Purpose |
|-----------|----------|-------|---------|
| `DEFAULT_LEVELSIGMA[]` | `PlanOptimizer.cpp:36` | 8.0, 3.2, 1.3, 0.5 | Pyramid voxel spacing |
| `GBINS_KERNEL_WIDTH` | `Histogram.h` | 8.0 | Histogram convolution kernel |
| `varMin/varMax` | `Prescription.cpp` | 0.01 | Adaptive variance bounds |
| `SIGMOID_SCALE` | `Prescription.cpp:14` | 0.2 | Parameter transformation |

---

## Amortized Optimization

### Concept

**Traditional optimization**: Each new patient requires full optimization from scratch (expensive).

**Amortized optimization**: Train a neural network on many patient examples, then use it to provide fast, patient-specific predictions that either replace or accelerate optimization.

```
Traditional:
  Patient → Initialize(zeros) → CG(500 iters) → Optimal Weights

Amortized:
  [Training Phase]
  Many Patients → Collect Trajectories → Train Network

  [Inference Phase]
  New Patient → Network Prediction → CG(50 iters) → Optimal Weights
```

### Components of Amortized Optimization for dH

#### 1. Learned Initialization Network

Instead of initializing beamlet weights to zero, predict a warm start from patient anatomy.

**Architecture**:
```python
class InitializationNetwork(nn.Module):
    """
    Predicts initial beamlet weights from patient anatomy.

    Input:
        - CT density volume: (D, H, W) tensor
        - Structure masks: (N_structures, D, H, W) binary tensors
        - Beam geometry: gantry angles, isocenter position

    Output:
        - Initial beamlet weights: (N_beams, N_beamlets_per_beam) tensor
    """

    def __init__(self, n_structures, n_beams, max_beamlets):
        super().__init__()

        # 3D encoder for anatomy
        self.ct_encoder = nn.Sequential(
            nn.Conv3d(1, 32, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv3d(32, 64, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv3d(64, 128, kernel_size=3, stride=2, padding=1),
            nn.AdaptiveAvgPool3d((4, 4, 4))
        )

        # Structure encoder
        self.structure_encoder = nn.Sequential(
            nn.Conv3d(n_structures, 32, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv3d(32, 64, kernel_size=3, stride=2, padding=1),
            nn.AdaptiveAvgPool3d((4, 4, 4))
        )

        # Beam geometry encoder
        self.beam_encoder = nn.Sequential(
            nn.Linear(n_beams * 6, 128),  # 6 params per beam
            nn.ReLU(),
            nn.Linear(128, 256)
        )

        # Output head
        combined_dim = 128 * 64 + 64 * 64 + 256
        self.output_head = nn.Sequential(
            nn.Linear(combined_dim, 1024),
            nn.ReLU(),
            nn.Linear(1024, n_beams * max_beamlets),
            nn.Softplus()  # Ensure non-negative weights
        )

    def forward(self, ct, structures, beam_params):
        ct_features = self.ct_encoder(ct).flatten(1)
        struct_features = self.structure_encoder(structures).flatten(1)
        beam_features = self.beam_encoder(beam_params)

        combined = torch.cat([ct_features, struct_features, beam_features], dim=1)
        return self.output_head(combined)
```

**Training**:
```python
def train_initialization_network(model, dataset, epochs=100):
    """
    Train initialization network to predict optimal weights from anatomy.

    Dataset: List of (anatomy, optimal_weights) pairs from full optimizations.
    Loss: MSE between predicted and optimal weights.
    """
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-4)

    for epoch in range(epochs):
        for ct, structures, beams, optimal_weights in dataset:
            predicted = model(ct, structures, beams)
            loss = F.mse_loss(predicted, optimal_weights)

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
```

#### 2. Learned Step Direction Network

Replace or augment conjugate gradient with learned search directions.

**Architecture** (from `learned_optimizer.py`):
```python
class LearnedLBFGS(nn.Module):
    """
    Neural network that learns to approximate L-BFGS search directions.

    Input:
        - History of (s_k, y_k) pairs: position changes and gradient changes
        - Current gradient g_k

    Output:
        - Search direction d_k (approximates H^{-1} @ g_k)
    """

    def __init__(self, dim, history_size=10, hidden_dim=256):
        super().__init__()
        self.history_size = history_size

        # Encode each (s, y) pair
        self.pair_encoder = nn.Sequential(
            nn.Linear(dim * 2, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU()
        )

        # Attention over history
        self.attention = nn.MultiheadAttention(hidden_dim, num_heads=4)

        # Output direction
        self.direction_head = nn.Sequential(
            nn.Linear(hidden_dim + dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, dim)
        )

    def forward(self, gradient, s_history, y_history):
        # Encode history pairs
        pairs = torch.stack([
            self.pair_encoder(torch.cat([s, y], dim=-1))
            for s, y in zip(s_history, y_history)
        ])

        # Self-attention over history
        attended, _ = self.attention(pairs, pairs, pairs)
        history_summary = attended.mean(dim=0)

        # Predict direction
        combined = torch.cat([history_summary, gradient], dim=-1)
        return self.direction_head(combined)
```

#### 3. Learned Convergence Predictor

Predict when optimization has converged sufficiently, enabling early stopping.

```python
class ConvergencePredictor(nn.Module):
    """
    Predicts remaining optimization error from current state.

    Input:
        - Current objective value
        - Recent objective history (last 10 values)
        - Current gradient norm
        - Pyramid level
        - Patient anatomy embedding

    Output:
        - Predicted final objective value
        - Confidence interval
    """

    def __init__(self, anatomy_dim=256):
        super().__init__()

        self.history_encoder = nn.LSTM(1, 64, batch_first=True)

        self.predictor = nn.Sequential(
            nn.Linear(64 + 3 + anatomy_dim, 128),
            nn.ReLU(),
            nn.Linear(128, 2)  # (predicted_final, uncertainty)
        )

    def forward(self, obj_history, grad_norm, level, anatomy_embedding):
        _, (h_n, _) = self.history_encoder(obj_history.unsqueeze(-1))

        features = torch.cat([
            h_n.squeeze(0),
            grad_norm.unsqueeze(-1),
            level.unsqueeze(-1).float(),
            anatomy_embedding
        ], dim=-1)

        output = self.predictor(features)
        return output[:, 0], F.softplus(output[:, 1])
```

---

## ML-Based Sigma Estimation

### Motivation

The sigma parameters control the smoothness of the optimization landscape:
- **Histogram sigma**: Controls DVH smoothing (affects gradient quality)
- **Pyramid sigma**: Controls spatial resolution at each level
- **Adaptive variance**: Per-parameter variance during optimization

Currently, these are either fixed constants or computed expensively during optimization. ML can predict optimal values for each patient.

### Architecture for Sigma Prediction

#### 1. Patient-Specific Histogram Sigma

```python
class HistogramSigmaPredictor(nn.Module):
    """
    Predicts optimal histogram convolution sigma for each structure.

    Theory: Optimal sigma depends on:
        - Structure volume (larger structures → can use smaller sigma)
        - DVH shape complexity (steep DVH → needs careful sigma choice)
        - Dose gradient in structure (high gradients → smaller sigma)

    Input:
        - Structure masks: (N_structures, D, H, W)
        - Dose distribution (if available): (D, H, W)
        - Target DVH points: (N_structures, N_points, 2)

    Output:
        - Per-structure sigma: (N_structures,) tensor
    """

    def __init__(self, n_structures, n_dvh_points=10):
        super().__init__()

        # Structure geometry encoder
        self.structure_encoder = nn.Sequential(
            nn.Conv3d(1, 16, 3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv3d(16, 32, 3, stride=2, padding=1),
            nn.AdaptiveAvgPool3d(1)
        )

        # DVH encoder
        self.dvh_encoder = nn.Sequential(
            nn.Linear(n_dvh_points * 2, 64),
            nn.ReLU(),
            nn.Linear(64, 32)
        )

        # Sigma predictor
        self.sigma_head = nn.Sequential(
            nn.Linear(32 + 32, 64),
            nn.ReLU(),
            nn.Linear(64, 1),
            nn.Softplus()  # Ensure positive sigma
        )

        # Learned bounds
        self.sigma_min = nn.Parameter(torch.tensor(0.01))
        self.sigma_max = nn.Parameter(torch.tensor(1.0))

    def forward(self, structure_masks, target_dvhs):
        sigmas = []
        for i in range(structure_masks.shape[0]):
            mask = structure_masks[i:i+1].unsqueeze(0)
            dvh = target_dvhs[i]

            geom_features = self.structure_encoder(mask).flatten()
            dvh_features = self.dvh_encoder(dvh.flatten())

            combined = torch.cat([geom_features, dvh_features])
            raw_sigma = self.sigma_head(combined)

            # Bound sigma to learned range
            sigma = self.sigma_min + raw_sigma * (self.sigma_max - self.sigma_min)
            sigmas.append(sigma)

        return torch.stack(sigmas)
```

#### 2. Pyramid Level Sigma Schedule

```python
class PyramidSigmaScheduler(nn.Module):
    """
    Learns optimal sigma schedule across pyramid levels for each patient.

    Theory: Optimal coarse-to-fine schedule depends on:
        - Anatomy complexity (complex anatomy → more gradual refinement)
        - Structure spacing (close structures → finer resolution needed earlier)
        - Beam arrangement (more beams → can be more aggressive)

    Input:
        - Patient anatomy features: (anatomy_dim,)
        - Number of beams: int
        - Number of pyramid levels: int (default 4)

    Output:
        - Sigma schedule: (n_levels,) tensor
    """

    def __init__(self, anatomy_dim=256, n_levels=4):
        super().__init__()
        self.n_levels = n_levels

        # Base schedule (learnable)
        self.base_schedule = nn.Parameter(
            torch.tensor([8.0, 3.2, 1.3, 0.5])  # DEFAULT_LEVELSIGMA
        )

        # Patient-specific adjustment
        self.adjustment_net = nn.Sequential(
            nn.Linear(anatomy_dim + 1, 64),
            nn.ReLU(),
            nn.Linear(64, n_levels),
            nn.Tanh()  # Bounded adjustment
        )

        self.adjustment_scale = nn.Parameter(torch.tensor(0.3))

    def forward(self, anatomy_features, n_beams):
        inputs = torch.cat([anatomy_features, n_beams.unsqueeze(-1).float()])
        adjustment = self.adjustment_net(inputs) * self.adjustment_scale

        # Apply multiplicative adjustment to base schedule
        schedule = self.base_schedule * (1 + adjustment)

        # Ensure monotonically decreasing
        schedule = torch.sort(schedule, descending=True)[0]

        return schedule
```

#### 3. Adaptive Variance Predictor

This is the most impactful component: predicting the per-parameter adaptive variance without running the expensive Gram-Schmidt orthogonalization.

```python
class AdaptiveVariancePredictor(nn.Module):
    """
    Predicts per-beamlet adaptive variance from anatomy and beam geometry.

    Theory: The adaptive variance captures local curvature of the objective:
        - High curvature dimensions → low variance (sharp optimization)
        - Low curvature dimensions → high variance (smooth optimization)

    The curvature depends on:
        - Beamlet's contribution to target/OAR overlap
        - Dose gradients in the beamlet's path
        - Proximity to other beamlets (correlation)

    Input:
        - CT density along beam path: (N_beamlets, depth)
        - Structure intersections: (N_beamlets, N_structures)
        - Current pyramid level: int

    Output:
        - Adaptive variance: (N_beamlets,) tensor
    """

    def __init__(self, max_depth=100, n_structures=10, n_levels=4):
        super().__init__()

        # Beamlet path encoder
        self.path_encoder = nn.Sequential(
            nn.Conv1d(1, 32, kernel_size=5, padding=2),
            nn.ReLU(),
            nn.Conv1d(32, 64, kernel_size=5, stride=2, padding=2),
            nn.AdaptiveAvgPool1d(1)
        )

        # Structure intersection encoder
        self.struct_encoder = nn.Sequential(
            nn.Linear(n_structures, 32),
            nn.ReLU()
        )

        # Level embedding
        self.level_embedding = nn.Embedding(n_levels, 16)

        # Variance predictor
        self.variance_head = nn.Sequential(
            nn.Linear(64 + 32 + 16, 64),
            nn.ReLU(),
            nn.Linear(64, 1),
            nn.Softplus()
        )

        # Variance bounds (per level)
        self.var_min = nn.Parameter(torch.full((n_levels,), 0.001))
        self.var_max = nn.Parameter(torch.full((n_levels,), 0.1))

    def forward(self, beam_paths, struct_intersections, level):
        batch_size = beam_paths.shape[0]

        # Encode beam paths
        path_features = self.path_encoder(beam_paths.unsqueeze(1)).squeeze(-1)

        # Encode structure intersections
        struct_features = self.struct_encoder(struct_intersections)

        # Level embedding
        level_features = self.level_embedding(
            torch.full((batch_size,), level, dtype=torch.long)
        )

        # Predict variance
        combined = torch.cat([path_features, struct_features, level_features], dim=-1)
        raw_variance = self.variance_head(combined).squeeze(-1)

        # Bound to level-specific range
        variance = self.var_min[level] + raw_variance * (
            self.var_max[level] - self.var_min[level]
        )

        return variance
```

---

## Implementation Guide

### Step 1: Data Collection

Collect optimization trajectories from full optimizations on diverse patient cases.

```python
# data_collection.py

import json
from pathlib import Path

class OptimizationTrajectoryCollector:
    """
    Collects optimization trajectories for training amortized models.
    """

    def __init__(self, output_dir: str):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def collect_trajectory(self, patient_id: str, optimizer, prescription):
        """
        Run full optimization and collect trajectory data.
        """
        trajectory = {
            'patient_id': patient_id,
            'anatomy': self._extract_anatomy(prescription),
            'iterations': [],
            'final_weights': None,
            'final_objective': None
        }

        # Hook into optimizer to collect per-iteration data
        original_callback = optimizer.callback

        def collection_callback(iter_data):
            trajectory['iterations'].append({
                'iteration': iter_data['iteration'],
                'objective': iter_data['objective'],
                'gradient_norm': iter_data['gradient_norm'],
                'weights': iter_data['weights'].copy(),
                'search_direction': iter_data['direction'].copy(),
                'step_size': iter_data['step_size'],
                'adaptive_variance': iter_data.get('adaptive_variance', None)
            })
            if original_callback:
                original_callback(iter_data)

        optimizer.callback = collection_callback

        # Run optimization
        result = optimizer.minimize()

        trajectory['final_weights'] = result['weights']
        trajectory['final_objective'] = result['objective']

        # Save trajectory
        output_path = self.output_dir / f'{patient_id}_trajectory.json'
        with open(output_path, 'w') as f:
            json.dump(trajectory, f)

        return trajectory

    def _extract_anatomy(self, prescription):
        """Extract anatomy features for training."""
        return {
            'ct_density': prescription.get_ct_array(),
            'structure_masks': {
                name: prescription.get_structure_mask(name)
                for name in prescription.get_structure_names()
            },
            'beam_geometry': prescription.get_beam_parameters(),
            'target_dvhs': prescription.get_target_dvhs()
        }
```

### Step 2: Dataset Preparation

```python
# dataset.py

import torch
from torch.utils.data import Dataset, DataLoader
import numpy as np

class OptimizationDataset(Dataset):
    """
    Dataset of optimization trajectories for training amortized models.
    """

    def __init__(self, trajectory_dir: str, mode='initialization'):
        """
        Args:
            trajectory_dir: Directory containing trajectory JSON files
            mode: 'initialization' | 'sigma' | 'convergence'
        """
        self.trajectories = self._load_trajectories(trajectory_dir)
        self.mode = mode

    def _load_trajectories(self, trajectory_dir):
        trajectories = []
        for path in Path(trajectory_dir).glob('*_trajectory.json'):
            with open(path) as f:
                trajectories.append(json.load(f))
        return trajectories

    def __len__(self):
        return len(self.trajectories)

    def __getitem__(self, idx):
        traj = self.trajectories[idx]

        if self.mode == 'initialization':
            return self._get_initialization_sample(traj)
        elif self.mode == 'sigma':
            return self._get_sigma_sample(traj)
        elif self.mode == 'convergence':
            return self._get_convergence_sample(traj)

    def _get_initialization_sample(self, traj):
        """Sample for training initialization network."""
        return {
            'ct': torch.tensor(traj['anatomy']['ct_density']),
            'structures': torch.tensor(
                np.stack(list(traj['anatomy']['structure_masks'].values()))
            ),
            'beam_params': torch.tensor(traj['anatomy']['beam_geometry']),
            'optimal_weights': torch.tensor(traj['final_weights'])
        }

    def _get_sigma_sample(self, traj):
        """Sample for training sigma predictor."""
        # Extract optimal sigma from convergence analysis
        optimal_sigma = self._analyze_optimal_sigma(traj)

        return {
            'structures': torch.tensor(
                np.stack(list(traj['anatomy']['structure_masks'].values()))
            ),
            'target_dvhs': torch.tensor(traj['anatomy']['target_dvhs']),
            'optimal_sigma': optimal_sigma
        }

    def _get_convergence_sample(self, traj):
        """Sample for training convergence predictor."""
        # Random point in trajectory
        n_iters = len(traj['iterations'])
        point = np.random.randint(10, n_iters)

        history = [traj['iterations'][i]['objective'] for i in range(point-10, point)]

        return {
            'objective_history': torch.tensor(history),
            'gradient_norm': torch.tensor(traj['iterations'][point]['gradient_norm']),
            'final_objective': torch.tensor(traj['final_objective'])
        }

    def _analyze_optimal_sigma(self, traj):
        """
        Analyze trajectory to determine optimal sigma.

        Heuristic: Optimal sigma minimizes gradient variance while
        maintaining convergence speed.
        """
        iterations = traj['iterations']

        # Look at gradient smoothness vs convergence rate tradeoff
        # This is a simplified heuristic - real implementation would be more sophisticated

        grad_norms = [it['gradient_norm'] for it in iterations]
        objectives = [it['objective'] for it in iterations]

        # Convergence rate (objective decrease per iteration)
        conv_rate = -(objectives[-1] - objectives[0]) / len(objectives)

        # Gradient variance (indicator of landscape smoothness)
        grad_var = np.var(grad_norms)

        # Optimal sigma balances these
        # Higher variance → need larger sigma for smoothing
        optimal_sigma = 0.01 + 0.1 * np.sqrt(grad_var / (conv_rate + 1e-6))

        return torch.tensor(optimal_sigma).clamp(0.001, 1.0)
```

### Step 3: Training Loop

```python
# train.py

import torch
import torch.nn.functional as F
from torch.utils.data import DataLoader

def train_sigma_predictor(model, dataset, epochs=100, lr=1e-4):
    """
    Train the sigma prediction network.
    """
    dataloader = DataLoader(dataset, batch_size=16, shuffle=True)
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, epochs)

    for epoch in range(epochs):
        total_loss = 0
        for batch in dataloader:
            structures = batch['structures']
            target_dvhs = batch['target_dvhs']
            optimal_sigma = batch['optimal_sigma']

            # Forward pass
            predicted_sigma = model(structures, target_dvhs)

            # Loss: MSE on log-sigma (scale-invariant)
            loss = F.mse_loss(
                torch.log(predicted_sigma + 1e-6),
                torch.log(optimal_sigma + 1e-6)
            )

            # Backward pass
            optimizer.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0)
            optimizer.step()

            total_loss += loss.item()

        scheduler.step()

        if epoch % 10 == 0:
            print(f"Epoch {epoch}: Loss = {total_loss/len(dataloader):.6f}")

    return model


def train_adaptive_variance_predictor(model, dataset, epochs=100, lr=1e-4):
    """
    Train the adaptive variance prediction network.

    Ground truth: Adaptive variance computed from GSO during full optimization.
    """
    dataloader = DataLoader(dataset, batch_size=32, shuffle=True)
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)

    for epoch in range(epochs):
        total_loss = 0
        for batch in dataloader:
            beam_paths = batch['beam_paths']
            struct_intersections = batch['struct_intersections']
            level = batch['level']
            true_variance = batch['adaptive_variance']

            # Forward pass
            predicted_variance = model(beam_paths, struct_intersections, level)

            # Loss: MSE on log-variance (handles scale differences)
            loss = F.mse_loss(
                torch.log(predicted_variance + 1e-8),
                torch.log(true_variance + 1e-8)
            )

            # Regularization: encourage smoothness across beamlets
            smoothness_loss = torch.mean(
                torch.abs(predicted_variance[1:] - predicted_variance[:-1])
            )
            loss = loss + 0.01 * smoothness_loss

            # Backward pass
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            total_loss += loss.item()

        if epoch % 10 == 0:
            print(f"Epoch {epoch}: Loss = {total_loss/len(dataloader):.6f}")

    return model
```

---

## Training Pipeline

### Complete Training Workflow

```python
# training_pipeline.py

from pathlib import Path
import torch

def full_training_pipeline(
    trajectory_dir: str,
    output_dir: str,
    n_patients_train: int = 80,
    n_patients_val: int = 20
):
    """
    Complete pipeline for training amortized optimization models.

    Steps:
        1. Load and split trajectory data
        2. Train initialization network
        3. Train sigma predictor
        4. Train adaptive variance predictor
        5. Train convergence predictor
        6. Evaluate on validation set
        7. Save models
    """
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # Step 1: Load data
    print("Loading trajectory data...")
    init_dataset = OptimizationDataset(trajectory_dir, mode='initialization')
    sigma_dataset = OptimizationDataset(trajectory_dir, mode='sigma')
    conv_dataset = OptimizationDataset(trajectory_dir, mode='convergence')

    # Split datasets
    train_size = int(0.8 * len(init_dataset))

    # Step 2: Train initialization network
    print("\n=== Training Initialization Network ===")
    init_model = InitializationNetwork(
        n_structures=10,
        n_beams=5,
        max_beamlets=100
    )
    init_model = train_initialization_network(init_model, init_dataset)
    torch.save(init_model.state_dict(), output_path / 'init_network.pt')

    # Step 3: Train sigma predictor
    print("\n=== Training Sigma Predictor ===")
    sigma_model = HistogramSigmaPredictor(n_structures=10)
    sigma_model = train_sigma_predictor(sigma_model, sigma_dataset)
    torch.save(sigma_model.state_dict(), output_path / 'sigma_predictor.pt')

    # Step 4: Train adaptive variance predictor
    print("\n=== Training Adaptive Variance Predictor ===")
    av_model = AdaptiveVariancePredictor(n_structures=10)
    av_model = train_adaptive_variance_predictor(av_model, init_dataset)
    torch.save(av_model.state_dict(), output_path / 'adaptive_variance.pt')

    # Step 5: Train convergence predictor
    print("\n=== Training Convergence Predictor ===")
    conv_model = ConvergencePredictor()
    conv_model = train_convergence_predictor(conv_model, conv_dataset)
    torch.save(conv_model.state_dict(), output_path / 'convergence_predictor.pt')

    # Step 6: Evaluation
    print("\n=== Evaluation ===")
    evaluate_amortized_optimization(
        init_model, sigma_model, av_model, conv_model,
        trajectory_dir, n_patients_val
    )

    print(f"\nModels saved to {output_path}")


def evaluate_amortized_optimization(
    init_model, sigma_model, av_model, conv_model,
    test_trajectory_dir, n_patients
):
    """
    Evaluate amortized optimization vs traditional optimization.

    Metrics:
        - Time to convergence
        - Final objective value
        - Number of iterations needed
        - Sigma prediction accuracy
    """
    results = {
        'traditional': {'times': [], 'objectives': [], 'iterations': []},
        'amortized': {'times': [], 'objectives': [], 'iterations': []}
    }

    test_dataset = OptimizationDataset(test_trajectory_dir, mode='initialization')

    for i, sample in enumerate(test_dataset):
        if i >= n_patients:
            break

        # Traditional optimization (from trajectory)
        traj = test_dataset.trajectories[i]
        results['traditional']['iterations'].append(len(traj['iterations']))
        results['traditional']['objectives'].append(traj['final_objective'])

        # Amortized optimization
        with torch.no_grad():
            # Predict initial weights
            init_weights = init_model(
                sample['ct'].unsqueeze(0),
                sample['structures'].unsqueeze(0),
                sample['beam_params'].unsqueeze(0)
            ).squeeze()

            # Predict sigma
            sigma = sigma_model(
                sample['structures'].unsqueeze(0),
                sample['target_dvhs'].unsqueeze(0)
            )

        # Count iterations to converge with warm start
        # (simulated - would need actual optimizer integration)
        warm_start_iters = estimate_iterations_with_warmstart(
            init_weights, traj
        )

        results['amortized']['iterations'].append(warm_start_iters)

    # Print summary
    print("\nResults Summary:")
    print(f"Traditional avg iterations: {np.mean(results['traditional']['iterations']):.1f}")
    print(f"Amortized avg iterations: {np.mean(results['amortized']['iterations']):.1f}")
    print(f"Speedup: {np.mean(results['traditional']['iterations']) / np.mean(results['amortized']['iterations']):.2f}x")
```

---

## Integration with Existing Codebase

### C++ Integration Points

#### 1. Modify `DynamicCovarianceOptimizer` to Accept Predicted Variance

```cpp
// ConjGradOptimizer.h - Add method to set predicted variance

class DynamicCovarianceOptimizer {
public:
    // Existing methods...

    // New: Set predicted adaptive variance (bypasses GSO computation)
    void SetPredictedAdaptiveVariance(const vnl_vector<REAL>& predictedAV) {
        m_bUsePredictedAV = true;
        m_vPredictedAV = predictedAV;
    }

    bool UsePredictedVariance() const { return m_bUsePredictedAV; }

private:
    bool m_bUsePredictedAV = false;
    vnl_vector<REAL> m_vPredictedAV;
};
```

```cpp
// ConjGradOptimizer.cpp - Use predicted variance when available

void DynamicCovarianceOptimizer::ComputeAdaptiveVariance() {
    if (m_bUsePredictedAV && m_vPredictedAV.size() == m_vAdaptVariance.size()) {
        // Use ML-predicted variance
        m_vAdaptVariance = m_vPredictedAV;
        return;
    }

    // Existing GSO-based computation...
    // (lines 280-367 in current implementation)
}
```

#### 2. Modify `Prescription` to Accept Custom Sigma

```cpp
// Prescription.h - Add sigma customization

class Prescription {
public:
    // New: Set per-structure histogram sigma
    void SetStructureSigma(const CString& structName, REAL sigma) {
        m_mapStructureSigma[structName] = sigma;
    }

    REAL GetStructureSigma(const CString& structName) const {
        auto it = m_mapStructureSigma.find(structName);
        if (it != m_mapStructureSigma.end())
            return it->second;
        return m_defaultSigma;  // Fall back to default
    }

private:
    std::map<CString, REAL> m_mapStructureSigma;
    REAL m_defaultSigma = 0.01;
};
```

### Python Bindings

```python
# python/rtmodel_bindings.py

import ctypes
from pathlib import Path

class RtModelWrapper:
    """
    Python wrapper for RtModel C++ library.
    Enables ML model integration with optimization.
    """

    def __init__(self, dll_path: str):
        self.lib = ctypes.CDLL(dll_path)
        self._setup_functions()

    def _setup_functions(self):
        # SetPredictedAdaptiveVariance
        self.lib.SetPredictedAdaptiveVariance.argtypes = [
            ctypes.c_void_p,  # optimizer handle
            ctypes.POINTER(ctypes.c_double),  # variance array
            ctypes.c_int  # array length
        ]

        # SetStructureSigma
        self.lib.SetStructureSigma.argtypes = [
            ctypes.c_void_p,  # prescription handle
            ctypes.c_char_p,  # structure name
            ctypes.c_double   # sigma value
        ]

    def set_predicted_variance(self, optimizer_handle, variance_array):
        """Set ML-predicted adaptive variance."""
        arr = (ctypes.c_double * len(variance_array))(*variance_array)
        self.lib.SetPredictedAdaptiveVariance(
            optimizer_handle, arr, len(variance_array)
        )

    def set_structure_sigma(self, prescription_handle, structure_name, sigma):
        """Set ML-predicted sigma for a structure."""
        self.lib.SetStructureSigma(
            prescription_handle,
            structure_name.encode('utf-8'),
            sigma
        )


def amortized_optimize(patient_data, models, rtmodel):
    """
    Run amortized optimization using ML predictions.

    Args:
        patient_data: Dict with CT, structures, beam geometry
        models: Dict with trained ML models
        rtmodel: RtModelWrapper instance

    Returns:
        Optimized plan
    """
    # Load patient into RtModel
    prescription = rtmodel.create_prescription(patient_data)
    optimizer = rtmodel.create_optimizer(prescription)

    # ML predictions
    with torch.no_grad():
        # Predict initial weights
        init_weights = models['initialization'](
            patient_data['ct'],
            patient_data['structures'],
            patient_data['beams']
        )

        # Predict sigma for each structure
        sigmas = models['sigma'](
            patient_data['structures'],
            patient_data['target_dvhs']
        )

        # Predict adaptive variance
        av = models['adaptive_variance'](
            patient_data['beam_paths'],
            patient_data['struct_intersections'],
            level=0
        )

    # Apply predictions
    rtmodel.set_initial_weights(optimizer, init_weights.numpy())

    for i, struct_name in enumerate(patient_data['structure_names']):
        rtmodel.set_structure_sigma(prescription, struct_name, sigmas[i].item())

    rtmodel.set_predicted_variance(optimizer, av.numpy())

    # Run optimization (should converge much faster)
    result = rtmodel.optimize(optimizer)

    return result
```

### Configuration File

```yaml
# config/amortized_optimization.yaml

# Model paths
models:
  initialization: "models/init_network.pt"
  sigma_predictor: "models/sigma_predictor.pt"
  adaptive_variance: "models/adaptive_variance.pt"
  convergence_predictor: "models/convergence_predictor.pt"

# Training configuration
training:
  trajectory_dir: "data/trajectories"
  output_dir: "models"
  batch_size: 16
  learning_rate: 0.0001
  epochs: 100

# Sigma prediction bounds
sigma:
  histogram:
    min: 0.001
    max: 1.0
    default: 0.01
  pyramid:
    levels: [8.0, 3.2, 1.3, 0.5]
    adjustment_range: 0.3

# Optimization settings
optimization:
  use_predicted_variance: true
  use_predicted_sigma: true
  use_warm_start: true
  max_iterations_amortized: 100  # vs 500 for traditional
  convergence_threshold: 1e-6
```

---

## Summary

### Key Benefits of Amortized Optimization

| Aspect | Traditional | Amortized |
|--------|-------------|-----------|
| **Iterations** | ~500 | ~50-100 |
| **Per-patient compute** | Full GSO | Forward pass |
| **Sigma selection** | Fixed/manual | Patient-specific |
| **Variance estimation** | GSO (expensive) | ML prediction (fast) |
| **Convergence detection** | Fixed tolerance | Learned predictor |

### Implementation Checklist

- [ ] Collect optimization trajectories from 100+ patients
- [ ] Train initialization network
- [ ] Train sigma predictor
- [ ] Train adaptive variance predictor
- [ ] Train convergence predictor
- [ ] Add C++ integration points
- [ ] Create Python bindings
- [ ] Validate on held-out patients
- [ ] Profile speedup vs accuracy tradeoff

### References

1. Kingma & Welling, "Auto-Encoding Variational Bayes" (2013) - VAE foundation
2. Andrychowicz et al., "Learning to Learn" (2016) - Learned optimizers
3. Chen et al., "Learning to Optimize" (2017) - Amortized optimization
4. Existing codebase:
   - `python/learned_optimizer.py` - Learned L-BFGS implementation
   - `python/learned_hessian_entropy.py` - Cholesky entropy estimation
   - `RtModel/ConjGradOptimizer.cpp` - Current CG implementation
