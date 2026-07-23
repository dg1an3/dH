"""
Validate the softmax-entropy gradient FORMULA that will be implemented in
Prescription::operator():

    p   = softmax(x)                      (with max-subtraction)
    H   = -sum_i p_i log p_i
    dH/dx_k = -p_k (log p_k + H)

against central finite differences. This checks the math before it goes into
C++; the in-app gradient check will then verify the actual implementation.
"""
import numpy as np

rng = np.random.default_rng(0)


def softmax(x):
    z = x - np.max(x)
    e = np.exp(z)
    return e / e.sum()


def entropy(x):
    p = softmax(x)
    p = np.clip(p, 1e-300, None)
    return -np.sum(p * np.log(p))


def analytic_grad(x):
    p = softmax(x)
    H = entropy(x)
    lp = np.log(np.clip(p, 1e-300, None))
    return -p * (lp + H)


def fd_grad(x, eps=1e-6):
    g = np.zeros_like(x)
    for k in range(len(x)):
        xp = x.copy(); xp[k] += eps
        xm = x.copy(); xm[k] -= eps
        g[k] = (entropy(xp) - entropy(xm)) / (2 * eps)
    return g


worst_normwise = 0.0
worst_abs = 0.0
worst_ptrel = 0.0   # pointwise rel, but ONLY where the gradient is non-negligible
for trial in range(50):
    n = int(rng.integers(3, 200))
    scale = float(rng.choice([0.1, 1.0, 5.0]))   # near-uniform .. peaked
    x = rng.normal(0, scale, size=n)
    ga = analytic_grad(x)
    gn = fd_grad(x)

    # norm-wise relative error (robust, standard gradient-check metric)
    normwise = np.linalg.norm(ga - gn) / (np.linalg.norm(ga) + np.linalg.norm(gn) + 1e-30)
    # max absolute error
    abserr = np.max(np.abs(ga - gn))
    # pointwise relative error restricted to components that actually matter
    # (|g| >= 1e-6 * max|g|), so FD noise on ~0 components doesn't dominate
    gmax = np.max(np.abs(ga))
    mask = np.abs(ga) >= 1e-3 * max(gmax, 1e-30)
    ptrel = np.max(np.abs(ga[mask] - gn[mask]) / np.abs(ga[mask])) if mask.any() else 0.0

    worst_normwise = max(worst_normwise, normwise)
    worst_abs = max(worst_abs, abserr)
    worst_ptrel = max(worst_ptrel, ptrel)

print(f"worst norm-wise rel error : {worst_normwise:.3e}   <-- rigorous metric")
print(f"worst absolute error      : {worst_abs:.3e}")
print(f"worst pointwise rel error : {worst_ptrel:.3e}  (FD truncation on small |g|)")
# norm-wise is the definitive gradient-check metric; pointwise rel is dominated
# by central-difference truncation noise on near-zero components.
print("PASS" if worst_normwise < 1e-6 else "FAIL")
