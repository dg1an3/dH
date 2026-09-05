"""Build the viewer's data.json: every swept (mid-dim, n_neighbors) cascade cell.

For each intermediate dim in {5,8,11,19} and each n_neighbors, projects
768D -> mid (cosine) -> 3D and 2D (euclidean), ball-warps the cloud into a tidy
sphere/disk, and stores both layouts so the viewer can switch between them live.
Each node also carries its mid-D vectors (v5/v8/v11/v19, taken at the default
n_neighbors) plus the raw v768 latent, feeding the viewer's proximity highlight.

Writes umap_viewer/data.json. Run after build_inventory_embeddings.py.
"""
import json
from pathlib import Path

import numpy as np
from sklearn.manifold import trustworthiness
from sklearn.neighbors import NearestNeighbors
import umap

HERE = Path(__file__).resolve().parent
EMB_NPY = HERE / "inventory_embeddings.npy"
META_JSON = HERE / "inventory_meta.json"
OUT_DIR = HERE / "umap_viewer"
OUT_JSON = OUT_DIR / "data.json"

MID_DIMS = [5, 8, 11, 19]
FINAL_DIMS = [2, 3]
N_NEIGHBORS = [5, 8, 15, 30, 50]   # must stay < n_points
DEFAULT_NN = 30
DEFAULT_MID = 5
MIN_DIST = 0.25
SEED = 42
SCALE = 100.0       # world half-extent for the Three.js scene
SPHERE_WARP = 0.6   # 0 = raw UMAP shape, 1 = fully uniform ball/disk
KNN_K = 10


def fit(X, dim, nn, metric):
    nn = min(nn, X.shape[0] - 1)
    return umap.UMAP(n_components=dim, n_neighbors=nn, min_dist=MIN_DIST,
                     metric=metric, random_state=SEED).fit_transform(X.astype(np.float64))


def ball_warp(Y, strength):
    """Redistribute radii by rank so the cloud fills a ball (3D) / disk (2D) evenly.

    Direction from the centroid is preserved; radius is remapped toward the uniform
    d-ball CDF r ∝ (rank/N)^(1/d), removing UMAP's empty core and corner outliers.
    Output radius is SCALE.
    """
    d = Y.shape[1]
    c = Y.mean(axis=0)
    V = Y - c
    r = np.linalg.norm(V, axis=1)
    dirs = V / np.maximum(r, 1e-9)[:, None]
    ranks = r.argsort().argsort()
    target = ((ranks + 0.5) / len(r)) ** (1.0 / d)
    rn = r / r.max()
    new_rn = (1.0 - strength) * rn + strength * target
    return dirs * (new_rn * SCALE)[:, None]


def center_scale(Y):
    Y = Y - Y.mean(axis=0)
    return Y / np.abs(Y).max() * SCALE


def knn_overlap(X_hi, Y, k):
    nn_hi = NearestNeighbors(n_neighbors=k + 1).fit(X_hi).kneighbors(return_distance=False)[:, 1:]
    nn_lo = NearestNeighbors(n_neighbors=k + 1).fit(Y).kneighbors(return_distance=False)[:, 1:]
    return float(np.mean([len(set(a) & set(b)) / k for a, b in zip(nn_hi, nn_lo)]))


def main():
    X = np.load(EMB_NPY).astype(np.float64)
    meta = json.load(open(META_JSON, encoding="utf-8"))
    n = X.shape[0]
    print(f"Projecting {n} nodes  768 -> {MID_DIMS} -> {FINAL_DIMS}")

    # per-node mid-D vectors at the default n_neighbors (for proximity highlight)
    vspaces = {"v768": X}
    for mid in MID_DIMS:
        vspaces[f"v{mid}"] = fit(X, mid, DEFAULT_NN, "cosine")

    layouts = {}
    for mid in MID_DIMS:
        for nn in N_NEIGHBORS:
            Ymid = fit(X, mid, nn, "cosine")
            cell = {}
            for fd, key in [(3, "3d"), (2, "2d")]:
                Yf = fit(Ymid, fd, nn, "euclidean")
                tw = round(trustworthiness(X, Yf, n_neighbors=KNN_K, metric="cosine"), 4)
                ov = round(knn_overlap(X, Yf, KNN_K), 4)
                W = ball_warp(center_scale(Yf), SPHERE_WARP)
                coords = [[round(float(v), 2) for v in row] for row in W]
                cell[key] = coords
                cell[f"metrics_{key}"] = {"trustworthiness": tw, f"knn_overlap@{KNN_K}": ov}
            layouts[f"mid{mid}_nn{nn}"] = cell
            print(f"  mid={mid:2d} nn={nn:2d}  "
                  f"trust3d={layouts[f'mid{mid}_nn{nn}']['metrics_3d']['trustworthiness']:.3f} "
                  f"trust2d={layouts[f'mid{mid}_nn{nn}']['metrics_2d']['trustworthiness']:.3f}")

    nodes = []
    for i, m in enumerate(meta):
        rec = {
            "name": m["name"], "description": m["description"],
            "category": m["category"], "tier": m["tier"], "kind": m["kind"],
            "salience": m["salience"],
        }
        for k, V in vspaces.items():
            rec[k] = [round(float(x), 4) for x in V[i]]
        nodes.append(rec)

    cats = sorted({m["category"] for m in meta})
    tiers = sorted({m["tier"] for m in meta})
    OUT_DIR.mkdir(exist_ok=True)
    OUT_JSON.write_text(json.dumps({
        "meta": {
            "n_points": n, "source_dim": int(X.shape[1]),
            "mid_dims": MID_DIMS, "final_dims": FINAL_DIMS,
            "n_neighbors": N_NEIGHBORS, "default_mid": DEFAULT_MID,
            "default_nn": DEFAULT_NN, "min_dist": MIN_DIST, "knn_k": KNN_K,
        },
        "categories": cats, "tiers": tiers,
        "nodes": nodes, "layouts": layouts,
    }, indent=1), encoding="utf-8")
    print(f"\nWrote {OUT_JSON}  ({n} nodes, {len(layouts)} layout cells, "
          f"{len(cats)} categories)")


if __name__ == "__main__":
    main()
