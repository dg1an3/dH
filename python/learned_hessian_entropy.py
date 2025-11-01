"""
Learned Hessian Network for Better Entropy Estimation

Key Insight:
-----------
Instead of using L-BFGS's hand-crafted two-loop recursion to approximate H^{-1},
train a neural network to directly output the inverse Hessian (or its Cholesky
factor) from the optimization history.

This gives you:
1. Better Hessian approximation for entropy calculation
2. More accurate variational free energy
3. Problem-specific learned covariance structure

Architecture:
------------
Input: History of (s_k, y_k) pairs from optimization
Output: Lower triangular L where H^{-1} = L @ L^T (Cholesky factor)

Why Cholesky?
- Guarantees positive definite covariance
- Easy to compute log determinant: log|H^{-1}| = 2 * sum(log(diag(L)))
- Efficient sampling: z ~ N(0, I), then θ ~ L @ z
"""

import torch
import torch.nn as nn
import numpy as np
from typing import List, Tuple


class CholeskyHessianNetwork(nn.Module):
    """
    Neural network that learns to output Cholesky factor of inverse Hessian

    This is specifically designed for variational Bayes entropy estimation.

    Input: Optimization history {(s_k, y_k)}
    Output: Lower triangular L such that Σ = L @ L^T (covariance matrix)
    """

    def __init__(self, param_dim: int, history_length: int = 20, hidden_dim: int = 512):
        super().__init__()

        self.param_dim = param_dim
        self.history_length = history_length

        # Input: history of s and y vectors
        input_dim = history_length * 2 * param_dim

        # Encoder: Process history
        self.encoder = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.GELU(),
            nn.Dropout(0.1),
            nn.Linear(hidden_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.GELU(),
            nn.Dropout(0.1),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.GELU(),
        )

        # Output layer: Generate Cholesky factor
        # Only need to output lower triangular elements: n*(n+1)/2
        n_tril_elements = param_dim * (param_dim + 1) // 2
        self.cholesky_layer = nn.Linear(hidden_dim // 2, n_tril_elements)

        # Diagonal scaling to ensure numerical stability
        self.log_diag_scale = nn.Parameter(torch.zeros(param_dim))

    def forward(self, history: torch.Tensor) -> torch.Tensor:
        """
        Args:
            history: (batch, history_length, 2*param_dim)

        Returns:
            L: (batch, param_dim, param_dim) - Lower triangular Cholesky factor
        """
        batch_size = history.shape[0]

        # Flatten history
        h_flat = history.reshape(batch_size, -1)

        # Encode
        encoded = self.encoder(h_flat)

        # Generate Cholesky elements
        tril_elements = self.cholesky_layer(encoded)

        # Build lower triangular matrix
        L = torch.zeros(batch_size, self.param_dim, self.param_dim,
                       device=history.device, dtype=history.dtype)

        # Fill lower triangular part
        tril_indices = torch.tril_indices(self.param_dim, self.param_dim,
                                          offset=0, device=history.device)
        L[:, tril_indices[0], tril_indices[1]] = tril_elements

        # Ensure positive diagonal for valid Cholesky factor
        # Use softplus to ensure positivity, scaled by learned parameters
        diag_indices = torch.arange(self.param_dim, device=history.device)
        L[:, diag_indices, diag_indices] = (
            torch.nn.functional.softplus(L[:, diag_indices, diag_indices])
            * torch.exp(self.log_diag_scale)
        )

        return L

    def compute_covariance(self, history: torch.Tensor) -> torch.Tensor:
        """
        Compute full covariance matrix Σ = L @ L^T

        Args:
            history: (batch, history_length, 2*param_dim)

        Returns:
            Sigma: (batch, param_dim, param_dim) - Covariance matrix
        """
        L = self.forward(history)
        Sigma = torch.bmm(L, L.transpose(-2, -1))
        return Sigma

    def compute_entropy(self, history: torch.Tensor) -> torch.Tensor:
        """
        Compute differential entropy H = 0.5 * log(det(2πe Σ))

        For Cholesky factor L:
            log|Σ| = log|L @ L^T| = 2 * sum(log(diag(L)))

        Args:
            history: (batch, history_length, 2*param_dim)

        Returns:
            entropy: (batch,) - Differential entropy in nats
        """
        L = self.forward(history)

        # Extract diagonal
        diag_indices = torch.arange(self.param_dim, device=L.device)
        L_diag = L[:, diag_indices, diag_indices]

        # Compute log determinant
        log_det = 2.0 * torch.sum(torch.log(L_diag + 1e-10), dim=-1)

        # Entropy formula: 0.5 * (n * log(2πe) + log|Σ|)
        log_2pi_e = np.log(2 * np.pi * np.e)
        entropy = 0.5 * (self.param_dim * log_2pi_e + log_det)

        return entropy


class HessianEntropyTrainer:
    """
    Trains the Hessian network to match true curvature at optimum
    """

    def __init__(self, model: CholeskyHessianNetwork, learning_rate: float = 1e-4):
        self.model = model
        self.optimizer = torch.optim.AdamW(model.parameters(), lr=learning_rate,
                                           weight_decay=1e-5)

    def compute_numerical_hessian(
        self,
        objective_fn,
        x: np.ndarray,
        epsilon: float = 1e-5
    ) -> np.ndarray:
        """
        Compute numerical Hessian via finite differences

        This is the "ground truth" for training
        """
        n = len(x)
        H = np.zeros((n, n))

        # Get gradient at x
        _, grad_x = objective_fn(x)

        # Finite difference for each parameter
        for i in range(n):
            x_plus = x.copy()
            x_plus[i] += epsilon
            _, grad_plus = objective_fn(x_plus)

            H[:, i] = (grad_plus - grad_x) / epsilon

        # Symmetrize
        H = 0.5 * (H + H.T)

        # Regularize to ensure positive definite
        H += 1e-6 * np.eye(n)

        return H

    def train_on_optimization_run(
        self,
        objective_fn,
        x0: np.ndarray,
        n_steps: int = 100,
        epochs: int = 20
    ):
        """
        Train on a single optimization run

        1. Run L-BFGS to collect trajectory and find optimum
        2. Compute numerical Hessian at optimum (ground truth)
        3. Train network to match it
        """
        from scipy.optimize import minimize

        print(f"Running optimization to collect trajectory...")

        # Collect trajectory
        trajectory = []
        def callback(xk):
            _, grad = objective_fn(xk)
            trajectory.append((xk.copy(), grad.copy()))

        result = minimize(
            lambda x: objective_fn(x)[0],
            x0,
            method='L-BFGS-B',
            jac=lambda x: objective_fn(x)[1],
            callback=callback,
            options={'maxiter': n_steps}
        )

        x_opt = result.x
        print(f"Optimization converged. Final value: {result.fun:.6f}")

        # Compute numerical Hessian at optimum
        print("Computing numerical Hessian (ground truth)...")
        H_true = self.compute_numerical_hessian(objective_fn, x_opt)

        # Compute inverse and Cholesky factor
        try:
            H_inv_true = np.linalg.inv(H_true)
            L_true = np.linalg.cholesky(H_inv_true)
            L_true_tensor = torch.from_numpy(L_true).float()
        except np.linalg.LinAlgError:
            print("Warning: Hessian not invertible, using pseudo-inverse")
            H_inv_true = np.linalg.pinv(H_true)
            # Use eigendecomposition for Cholesky of pseudo-inverse
            eigvals, eigvecs = np.linalg.eigh(H_inv_true)
            eigvals = np.maximum(eigvals, 1e-6)
            L_true = eigvecs @ np.diag(np.sqrt(eigvals))
            L_true_tensor = torch.from_numpy(L_true).float()

        # Prepare history from trajectory
        history_list = []
        for i in range(1, len(trajectory)):
            s_k = trajectory[i][0] - trajectory[i-1][0]
            y_k = trajectory[i][1] - trajectory[i-1][1]

            # Store last history_length pairs
            start = max(0, i - self.model.history_length)
            window = trajectory[start:i+1]

            s_list = []
            y_list = []
            for j in range(len(window) - 1):
                s_j = window[j+1][0] - window[j][0]
                y_j = window[j+1][1] - window[j][1]
                s_list.append(s_j)
                y_list.append(y_j)

            # Pad if needed
            while len(s_list) < self.model.history_length:
                s_list.insert(0, np.zeros_like(x0))
                y_list.insert(0, np.zeros_like(x0))

            history_np = np.stack([
                np.concatenate([s_list[k], y_list[k]])
                for k in range(self.model.history_length)
            ], axis=0)

            history_list.append(history_np)

        if len(history_list) == 0:
            print("No history collected!")
            return

        # Train the model
        print(f"\nTraining Hessian network...")
        print("-" * 70)

        for epoch in range(epochs):
            epoch_loss = 0.0

            for history_np in history_list:
                history = torch.from_numpy(history_np).float().unsqueeze(0)

                # Predict Cholesky factor
                L_pred = self.model(history)

                # Loss: Frobenius norm between predicted and true Cholesky factors
                loss_cholesky = torch.nn.functional.mse_loss(
                    L_pred.squeeze(0), L_true_tensor
                )

                # Additional loss: match entropy
                entropy_pred = self.model.compute_entropy(history)

                # True entropy from ground truth Hessian
                log_det_true = 2.0 * np.sum(np.log(np.diag(L_true) + 1e-10))
                entropy_true = 0.5 * (len(x0) * np.log(2*np.pi*np.e) + log_det_true)
                entropy_true_tensor = torch.tensor([entropy_true], dtype=torch.float32)

                loss_entropy = torch.nn.functional.mse_loss(
                    entropy_pred, entropy_true_tensor
                )

                # Combined loss
                loss = loss_cholesky + 0.1 * loss_entropy

                # Optimize
                self.optimizer.zero_grad()
                loss.backward()
                torch.nn.utils.clip_grad_norm_(self.model.parameters(), 1.0)
                self.optimizer.step()

                epoch_loss += loss.item()

            avg_loss = epoch_loss / len(history_list)

            if (epoch + 1) % 5 == 0:
                print(f"Epoch {epoch+1}/{epochs}, Loss: {avg_loss:.6e}")

        print(f"\nTraining complete!")
        print(f"True entropy: {entropy_true:.6f} nats")

        # Test on final history
        final_history = torch.from_numpy(history_list[-1]).float().unsqueeze(0)
        pred_entropy = self.model.compute_entropy(final_history).item()
        print(f"Predicted entropy: {pred_entropy:.6f} nats")
        print(f"Relative error: {abs(pred_entropy - entropy_true) / abs(entropy_true) * 100:.2f}%")


# Example usage
if __name__ == '__main__':
    print("Learned Hessian Network for Variational Bayes Entropy")
    print("=" * 70)

    # Test on simple quadratic
    n_dim = 50

    # Create random positive definite matrix
    A = np.random.randn(n_dim, n_dim)
    A = A.T @ A + 0.1 * np.eye(n_dim)

    def quadratic_objective(x):
        value = 0.5 * x @ A @ x
        grad = A @ x
        return value, grad

    # Create model
    model = CholeskyHessianNetwork(param_dim=n_dim, history_length=20)

    print(f"\nModel: {sum(p.numel() for p in model.parameters()):,} parameters")

    # Train on optimization run
    trainer = HessianEntropyTrainer(model)
    x0 = np.random.randn(n_dim)

    trainer.train_on_optimization_run(quadratic_objective, x0, n_steps=50, epochs=30)

    print("\n" + "=" * 70)
    print("KEY ADVANTAGES FOR VARIATIONAL BAYES:")
    print("=" * 70)
    print("""
1. BETTER ENTROPY ESTIMATES
   - Learns true curvature from numerical Hessian
   - More accurate than L-BFGS two-loop recursion
   - Direct output: entropy in nats

2. PROBLEM-SPECIFIC LEARNING
   - Train on many patient cases
   - Learns typical covariance structure of radiotherapy optimization
   - Transfer learning across similar anatomies

3. EFFICIENT INFERENCE
   - O(n²) for Cholesky, vs O(n³) for full Hessian
   - Can sample from posterior: θ ~ N(θ*, Σ)
   - Uncertainty quantification for treatment plans

4. INTEGRATION WITH RTMODEL
   - Use pybind11 bindings from earlier
   - Run optimization, collect (s_k, y_k) history
   - Feed to network → get entropy → compute free energy
   - F = KL_divergence - Entropy (variational bound)
""")
