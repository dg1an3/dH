"""UMAP cascade sweep + faithfulness scoring for the inventory embeddings.

Sweeps the full 768D -> {5,8,11,19}D -> {2,3}D cascade across a range of UMAP
n_neighbors (and a small min_dist grid), and scores how well each stage preserves
the original 768D neighborhood structure:

  - trustworthiness   : low-dim neighbors that are also high-dim neighbors
  - continuity        : high-dim neighbors that stay neighbors in low-dim
  - knn_overlap@k     : mean fraction of each point's k nearest neighbors preserved
  - spearman_dist     : rank correlation of all pairwise distances (global structure)
  - silhouette        : category (library) separation in the low-dim space

For every (n_neighbors, min_dist) it scores each mid dim (768->mid) and each full
cascade (768->mid->final), plus a direct 768->final baseline. Results go to
umap_sweep.json. The viewer's layouts are built separately by umap_layout.py.

Run (after build_inventory_embeddings.py):  python umap_sweep.py
"""
import json
from pathlib import Path

import numpy as np
from scipy.stats import spearmanr
from scipy.spatial.distance import squareform, pdist
from sklearn.manifold import trustworthiness
from sklearn.metrics import silhouette_score
from sklearn.neighbors import NearestNeighbors
import umap

HERE = Path(__file__).resolve().parent
EMB_NPY = HERE / "inventory_embeddings.npy"
META_JSON = HERE / "inventory_meta.json"
OUT_JSON = HERE / "umap_sweep.json"

MID_DIMS = [5, 8, 11, 19]
FINAL_DIMS = [2, 3]
N_NEIGHBORS = [5, 8, 15, 30, 50]   # must stay < n_points
MIN_DISTS = [0.1, 0.25, 0.5]
KNN_K = 10
SEED = 42


def continuity(X_hi, X_lo, k):
    """Mirror of trustworthiness: penalizes high-dim neighbors torn apart in low-dim."""
    n = X_hi.shape[0]
    ind_hi = NearestNeighbors(n_neighbors=k + 1).fit(X_hi).kneighbors(return_distance=False)[:, 1:]
    D_lo = squareform(pdist(X_lo))
    rank_lo = D_lo.argsort(axis=1).argsort(axis=1)
    ind_lo = NearestNeighbors(n_neighbors=k + 1).fit(X_lo).kneighbors(return_distance=False)[:, 1:]
    lo_sets = [set(row) for row in ind_lo]
    total = 0.0
    for i in range(n):
        for j in ind_hi[i]:
            if j not in lo_sets[i]:
                total += rank_lo[i, j] - k
    norm = 2.0 / (n * k * (2 * n - 3 * k - 1))
    return 1.0 - norm * total


def knn_overlap(X_hi, X_lo, k):
    nn_hi = NearestNeighbors(n_neighbors=k + 1).fit(X_hi).kneighbors(return_distance=False)[:, 1:]
    nn_lo = NearestNeighbors(n_neighbors=k + 1).fit(X_lo).kneighbors(return_distance=False)[:, 1:]
    return float(np.mean([len(set(a) & set(b)) / k for a, b in zip(nn_hi, nn_lo)]))


def score(X_hi, Y, d_hi, labels, score_sil):
    return {
        "trustworthiness": round(trustworthiness(X_hi, Y, n_neighbors=KNN_K, metric="cosine"), 4),
        "continuity": round(continuity(X_hi, Y, KNN_K), 4),
        f"knn_overlap@{KNN_K}": round(knn_overlap(X_hi, Y, KNN_K), 4),
        "spearman_dist": round(float(spearmanr(d_hi, pdist(Y)).statistic), 4),
        "silhouette": round(float(silhouette_score(Y, labels)), 4) if score_sil else float("nan"),
    }


def fit(X, dim, nn, md, metric):
    nn = min(nn, X.shape[0] - 1)
    return umap.UMAP(n_components=dim, n_neighbors=nn, min_dist=md,
                     metric=metric, random_state=SEED).fit_transform(X)


def main():
    X = np.load(EMB_NPY).astype(np.float64)
    meta = json.load(open(META_JSON, encoding="utf-8"))
    n = X.shape[0]
    labels = np.array([m["category"] for m in meta])
    score_sil = len(set(labels)) > 1
    d_hi = pdist(X)
    sil_hi = silhouette_score(X, labels, metric="cosine") if score_sil else float("nan")
    print(f"Loaded {n} embeddings, dim={X.shape[1]}  "
          f"({len(set(labels))} categories, baseline silhouette@768d={sil_hi:.4f})")

    mid_results, cascade_results, baseline_results = [], [], []

    for nn in N_NEIGHBORS:
        for md in MIN_DISTS:
            print(f"\nn_neighbors={nn}  min_dist={md}")
            # direct 768 -> final (baseline for the cascade)
            for fd in FINAL_DIMS:
                Yb = fit(X, fd, nn, md, "cosine")
                s = score(X, Yb, d_hi, labels, score_sil)
                baseline_results.append({"n_neighbors": nn, "min_dist": md, "final": fd, **s})
            # 768 -> mid, then mid -> final
            for mid in MID_DIMS:
                Ymid = fit(X, mid, nn, md, "cosine")
                sm = score(X, Ymid, d_hi, labels, score_sil)
                mid_results.append({"n_neighbors": nn, "min_dist": md, "mid": mid, **sm})
                print(f"  768->{mid:2d}d  trust={sm['trustworthiness']:.3f} "
                      f"cont={sm['continuity']:.3f} sil={sm['silhouette']:.3f}")
                for fd in FINAL_DIMS:
                    Yf = fit(Ymid, fd, nn, md, "euclidean")
                    sf = score(X, Yf, d_hi, labels, score_sil)  # scored against original 768D
                    cascade_results.append(
                        {"n_neighbors": nn, "min_dist": md, "mid": mid, "final": fd, **sf})

    OUT_JSON.write_text(json.dumps({
        "k": KNN_K, "n_points": n, "source_dim": int(X.shape[1]),
        "n_neighbors_swept": N_NEIGHBORS, "min_dists_swept": MIN_DISTS,
        "mid_dims_swept": MID_DIMS, "final_dims_swept": FINAL_DIMS,
        "silhouette_baseline_768d": round(float(sil_hi), 4),
        "mid": mid_results, "cascade": cascade_results, "baseline_direct": baseline_results,
    }, indent=2), encoding="utf-8")
    print(f"\nWrote {OUT_JSON}")

    # quick best-cascade summary per final dim
    for fd in FINAL_DIMS:
        cells = [r for r in cascade_results if r["final"] == fd]
        best = max(cells, key=lambda r: r["trustworthiness"])
        print(f"  best ->{fd}d: mid={best['mid']} nn={best['n_neighbors']} "
              f"md={best['min_dist']}  trust={best['trustworthiness']}")


if __name__ == "__main__":
    main()
