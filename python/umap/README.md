# Inventory Embedding UMAP Viewer

Visualizes the component + class/algorithm embeddings (`../component_embeddings.json`,
`../class_embeddings.json`) as an interactive 3D/2D UMAP, with a swept cascade
**768D → {5,8,11,19}D → {2,3}D** across UMAP `n_neighbors`.

Modeled on the `semantic_notes` `umap_3d_viewer` pattern.

> **Note:** the generated data artifacts (`*_embeddings.json`, `inventory_embeddings.npy`,
> `inventory_meta.json`, `umap_sweep.json`, `umap_viewer/data.json`) are **not checked in** —
> they are `.gitignore`d and must be built by running the pipeline below. The viewer
> fetches `data.json` at load, so run steps 1–3 before opening it.

## Pipeline

```bash
# 1. combine the two Ollama embedding files -> matrix + metadata (87 nodes x 768d)
python build_inventory_embeddings.py     # -> inventory_embeddings.npy, inventory_meta.json

# 2. faithfulness sweep: scores every 768->mid and 768->mid->final cell vs the
#    original 768D (trustworthiness, continuity, knn overlap, spearman, silhouette)
python umap_sweep.py                      # -> umap_sweep.json

# 3. build the viewer's data.json: all (mid-dim x n_neighbors) layout cells,
#    each with a 3D and 2D projection + per-node v5/v8/v11/v19/v768 vectors
python umap_layout.py                     # -> umap_viewer/data.json

# 4. serve + open
python umap_viewer/serve.py 8778          # http://localhost:8778/
```

Re-run steps 1–3 whenever the upstream embeddings change (e.g. after
`../embed_components.py` / `../embed_classes.py`).

## Viewer controls

- **intermediate dim** `{5,8,11,19}` and **n_neighbors** `{5,8,15,30,50}` — switch
  the precomputed cascade cell live; the metrics badge shows that cell's
  trustworthiness and kNN-overlap vs the 768D source.
- **projection** 3D / 2D · **color by** tier / library / kind.
- **click a node** — highlights its nearest neighbors in the **proximity space**
  selector (`v768` raw, or any mid-D projection); hover for name + description.
- legend swatches toggle categories; the search box filters labels.

## Sweep findings (n = 87)

Faithfulness rises with `n_neighbors` and `min_dist`; intermediate dim matters
little above 8D. Best mid-D fidelity ≈ `trust 0.95` at `nn=50, md=0.5, mid=11`.
The full grid is in `umap_sweep.json` (`mid`, `cascade`, `baseline_direct`).
