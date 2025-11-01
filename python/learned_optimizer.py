"""
Learned Optimizer: Using ML to approximate Hessian from L-BFGS-style history

This explores using neural networks trained on optimization trajectories
to learn better Hessian approximations than hand-crafted L-BFGS formulas.

Key Idea:
---------
L-BFGS stores history: {(s_k, y_k)} where s_k = x_{k+1} - x_k, y_k = g_{k+1} - g_k
Instead of the two-loop recursion formula, train a neural network to:
    Input: History of (s_k, y_k) pairs + current gradient
    Output: Search direction (or Hessian approximation)

Advantages:
-----------
1. Learn problem-specific structure (radiotherapy DVH patterns)
2. Potentially capture non-local curvature information
3. Can be trained offline on many patient optimization runs
4. May converge faster than vanilla L-BFGS for similar problems

References:
-----------
- "Learning to Learn by Gradient Descent by Gradient Descent" (Andrychowicz et al., 2016)
- "Learned Optimizers that Scale and Generalize" (Metz et al., 2022)
- "Meta-Learning with Implicit Gradients" (Rajeswaran et al., 2019)
"""

import torch
import torch.nn as nn
import numpy as np
from collections import deque
from typing import List, Tuple, Optional


class HessianApproximatorNetwork(nn.Module):
    """
    Neural network that learns to approximate inverse Hessian-vector products

    Input: Concatenated history of (s_k, y_k) pairs + current gradient
    Output: Search direction (H^{-1} @ g)
    """

    def __init__(self, param_dim: int, history_length: int = 10, hidden_dim: int = 256):
        super().__init__()

        self.param_dim = param_dim
        self.history_length = history_length

        # Input: history_length * (2 * param_dim) + param_dim for current gradient
        input_dim = history_length * 2 * param_dim + param_dim

        # Architecture: MLP with skip connections
        self.encoder = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
        )

        self.decoder = nn.Sequential(
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.ReLU(),
            nn.Linear(hidden_dim // 2, param_dim),
        )

        # Skip connection to ensure we at least do gradient descent
        self.skip_weight = nn.Parameter(torch.ones(1))

    def forward(self, history: torch.Tensor, gradient: torch.Tensor) -> torch.Tensor:
        """
        Args:
            history: (batch, history_length, 2*param_dim) - stacked (s_k, y_k)
            gradient: (batch, param_dim) - current gradient

        Returns:
            direction: (batch, param_dim) - search direction
        """
        batch_size = gradient.shape[0]

        # Flatten history
        history_flat = history.reshape(batch_size, -1)

        # Concatenate with gradient
        x = torch.cat([history_flat, gradient], dim=1)

        # Encode
        h = self.encoder(x)

        # Decode to search direction
        direction = self.decoder(h)

        # Add skip connection (ensures at least gradient descent)
        direction = direction - self.skip_weight * gradient

        return direction


class LearnedLBFGS:
    """
    Learned optimizer that mimics L-BFGS but uses a neural network
    to compute search directions from history
    """

    def __init__(
        self,
        model: HessianApproximatorNetwork,
        history_length: int = 10,
        device: str = 'cpu'
    ):
        self.model = model.to(device)
        self.history_length = history_length
        self.device = device

        # Storage for optimization history
        self.reset()

    def reset(self):
        """Reset optimization history"""
        self.s_history = deque(maxlen=self.history_length)  # steps
        self.y_history = deque(maxlen=self.history_length)  # gradient changes
        self.prev_x = None
        self.prev_grad = None

    def step(self, x: np.ndarray, grad: np.ndarray) -> np.ndarray:
        """
        Compute next step using learned Hessian approximation

        Args:
            x: Current parameters
            grad: Current gradient

        Returns:
            search_direction: Proposed search direction
        """
        # Update history
        if self.prev_x is not None:
            s_k = x - self.prev_x
            y_k = grad - self.prev_grad

            self.s_history.append(s_k)
            self.y_history.append(y_k)

        # Prepare input for neural network
        if len(self.s_history) > 0:
            # Pad history if needed
            s_list = list(self.s_history)
            y_list = list(self.y_history)

            while len(s_list) < self.history_length:
                s_list.insert(0, np.zeros_like(x))
                y_list.insert(0, np.zeros_like(grad))

            # Stack into history tensor
            history_np = np.stack([
                np.concatenate([s_list[i], y_list[i]])
                for i in range(self.history_length)
            ], axis=0)

            history = torch.from_numpy(history_np).float().unsqueeze(0).to(self.device)
            grad_tensor = torch.from_numpy(grad).float().unsqueeze(0).to(self.device)

            # Get search direction from neural network
            with torch.no_grad():
                direction = self.model(history, grad_tensor)

            search_direction = direction.cpu().numpy().squeeze()
        else:
            # First iteration: just use negative gradient
            search_direction = -grad

        # Store for next iteration
        self.prev_x = x.copy()
        self.prev_grad = grad.copy()

        return search_direction


class LearnedOptimizerTrainer:
    """
    Trains the learned optimizer on trajectories from traditional optimizers
    """

    def __init__(
        self,
        model: HessianApproximatorNetwork,
        learning_rate: float = 1e-3
    ):
        self.model = model
        self.optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)

    def collect_lbfgs_trajectory(
        self,
        objective_fn,
        x0: np.ndarray,
        max_iters: int = 100
    ) -> List[Tuple[np.ndarray, np.ndarray, np.ndarray]]:
        """
        Run L-BFGS and collect trajectory: (x_k, g_k, x_{k+1})

        This creates training data where:
            Input: History up to k + g_k
            Target: x_{k+1} - x_k (the step L-BFGS took)
        """
        from scipy.optimize import minimize

        trajectory = []

        def callback(xk):
            # scipy doesn't easily give us gradient, so we compute it
            _, grad = objective_fn(xk)
            trajectory.append((xk.copy(), grad.copy()))

        result = minimize(
            lambda x: objective_fn(x)[0],
            x0,
            method='L-BFGS-B',
            jac=lambda x: objective_fn(x)[1],
            callback=callback,
            options={'maxiter': max_iters}
        )

        # Convert to (x_k, g_k, step_k) format
        training_data = []
        for i in range(len(trajectory) - 1):
            x_k, g_k = trajectory[i]
            x_next, _ = trajectory[i + 1]
            step_k = x_next - x_k
            training_data.append((x_k, g_k, step_k))

        return training_data

    def train_on_trajectory(
        self,
        trajectory: List[Tuple[np.ndarray, np.ndarray, np.ndarray]],
        epochs: int = 10
    ) -> float:
        """
        Train model on a single optimization trajectory

        Loss: MSE between predicted and actual L-BFGS steps
        """
        total_loss = 0.0

        for epoch in range(epochs):
            epoch_loss = 0.0

            # Process trajectory with sliding window
            for i in range(len(trajectory)):
                # Get history window
                start = max(0, i - self.model.history_length)
                history_slice = trajectory[start:i] if i > 0 else []

                x_k, g_k, target_step = trajectory[i]

                # Build history tensor
                s_list = []
                y_list = []

                for j in range(len(history_slice) - 1):
                    x_j, g_j, _ = history_slice[j]
                    x_jp1, g_jp1, _ = history_slice[j + 1]
                    s_list.append(x_jp1 - x_j)
                    y_list.append(g_jp1 - g_j)

                # Pad if needed
                while len(s_list) < self.model.history_length:
                    s_list.insert(0, np.zeros_like(x_k))
                    y_list.insert(0, np.zeros_like(g_k))

                history_np = np.stack([
                    np.concatenate([s_list[k], y_list[k]])
                    for k in range(self.model.history_length)
                ], axis=0)

                # Convert to tensors
                history = torch.from_numpy(history_np).float().unsqueeze(0)
                gradient = torch.from_numpy(g_k).float().unsqueeze(0)
                target = torch.from_numpy(target_step).float().unsqueeze(0)

                # Forward pass
                predicted_direction = self.model(history, gradient)

                # Loss: MSE between predicted and actual step direction
                loss = nn.MSELoss()(predicted_direction, target)

                # Backward pass
                self.optimizer.zero_grad()
                loss.backward()
                self.optimizer.step()

                epoch_loss += loss.item()

            total_loss = epoch_loss / len(trajectory)

            if (epoch + 1) % 5 == 0:
                print(f"  Epoch {epoch+1}/{epochs}, Loss: {total_loss:.6f}")

        return total_loss


def train_on_multiple_problems(
    model: HessianApproximatorNetwork,
    objective_functions: List,
    initial_guesses: List[np.ndarray],
    epochs_per_problem: int = 10
):
    """
    Train learned optimizer on multiple radiotherapy optimization problems

    This is the key: train on many patient cases to learn the common
    structure of radiotherapy DVH optimization
    """
    trainer = LearnedOptimizerTrainer(model)

    print("Training learned optimizer on multiple problems...")
    print("=" * 70)

    for prob_idx, (obj_fn, x0) in enumerate(zip(objective_functions, initial_guesses)):
        print(f"\nProblem {prob_idx + 1}/{len(objective_functions)}")
        print("-" * 70)

        # Collect L-BFGS trajectory
        print("  Collecting L-BFGS trajectory...")
        trajectory = trainer.collect_lbfgs_trajectory(obj_fn, x0, max_iters=50)
        print(f"  Collected {len(trajectory)} steps")

        # Train on this trajectory
        print("  Training on trajectory...")
        loss = trainer.train_on_trajectory(trajectory, epochs=epochs_per_problem)
        print(f"  Final loss: {loss:.6f}")

    print("\n" + "=" * 70)
    print("Training complete!")

    return model


# Example usage
if __name__ == '__main__':
    print("Learned Optimizer for Radiotherapy Planning")
    print("=" * 70)

    # Example: Simple quadratic objective (replace with actual RtModel Prescription)
    def quadratic_objective(x):
        """Simple test objective"""
        A = np.random.randn(len(x), len(x))
        A = A.T @ A + np.eye(len(x))  # Make positive definite

        value = 0.5 * x @ A @ x
        grad = A @ x
        return value, grad

    # Create model
    param_dim = 100
    model = HessianApproximatorNetwork(param_dim=param_dim, history_length=10)

    print(f"\nModel architecture:")
    print(f"  Parameter dimension: {param_dim}")
    print(f"  History length: 10")
    print(f"  Total parameters: {sum(p.numel() for p in model.parameters()):,}")

    # To actually use this:
    print("\n" + "=" * 70)
    print("USAGE GUIDE")
    print("=" * 70)
    print("""
1. COLLECT TRAINING DATA:
   - Run L-BFGS on 50-100 patient treatment plans
   - Save optimization trajectories (x_k, g_k, step_k)

2. TRAIN THE MODEL:
   - Use train_on_multiple_problems() with your patient dataset
   - Model learns common structure of radiotherapy optimization

3. USE FOR NEW PATIENTS:
   - Initialize LearnedLBFGS with trained model
   - Should converge faster than vanilla L-BFGS
   - Can still compute entropy from accumulated (s_k, y_k) history

4. HESSIAN APPROXIMATION:
   - After optimization, the (s_k, y_k) history can be used
   - Feed through a separate network to estimate H^{-1}
   - Use for variational entropy calculation
""")

    print("\nAdvantages over vanilla L-BFGS:")
    print("  ✓ Learns problem-specific curvature patterns")
    print("  ✓ May converge in fewer iterations")
    print("  ✓ Can incorporate domain knowledge (DVH structure)")
    print("  ✓ Still maintains (s_k, y_k) history for Hessian estimation")
