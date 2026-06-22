"""Combine the component + class Ollama embeddings into one matrix for UMAP.

Reads ../component_embeddings.json (26 components) and ../class_embeddings.json
(61 classes/algorithms), and writes, into this directory:
  - inventory_embeddings.npy   : float32 matrix [N, 768], components then classes
  - inventory_meta.json        : per-node metadata aligned row-for-row with the .npy
                                 {name, description, category, tier, kind, salience}

`category` is the owning module/library (e.g. "RtModel") so a component clusters
with its own classes; `tier` is the high-level grouping (production / foundation /
component / legacy-rt / utility); `kind` is component / class / algorithm / function.
"""
import json
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
PY_DIR = HERE.parent                       # the repo's python/ folder
COMP_JSON = PY_DIR / "component_embeddings.json"
CLASS_JSON = PY_DIR / "class_embeddings.json"
OUT_NPY = HERE / "inventory_embeddings.npy"
OUT_META = HERE / "inventory_meta.json"

# salience (node + label size in the viewer) by kind — components stand tallest.
SALIENCE = {"component": 0.95, "algorithm": 0.7, "class": 0.5, "function": 0.4}


def main():
    comp = json.load(open(COMP_JSON, encoding="utf-8"))["components"]
    cls = json.load(open(CLASS_JSON, encoding="utf-8"))["classes"]

    # component name -> tier (its own coarse category), for tagging classes.
    tier_of = {c["name"]: c["category"] for c in comp}

    rows, meta = [], []

    # --- components: category == their own name so classes nest under them ---
    for c in comp:
        rows.append(c["embedding"])
        meta.append({
            "name": c["name"],
            "description": c["text"],
            "category": c["name"],
            "tier": c["category"],
            "kind": "component",
            "salience": SALIENCE["component"],
        })

    # --- classes / algorithms: category == owning library ---
    for c in cls:
        lib = c["library"]
        rows.append(c["embedding"])
        meta.append({
            "name": f"{lib}.{c['name']}",
            "description": c["text"],
            "category": lib,
            "tier": tier_of.get(lib, "component"),
            "kind": c["kind"],
            "salience": SALIENCE.get(c["kind"], 0.5),
        })

    X = np.asarray(rows, dtype=np.float32)
    np.save(OUT_NPY, X)
    OUT_META.write_text(json.dumps(meta, indent=1), encoding="utf-8")

    cats = sorted({m["category"] for m in meta})
    print(f"Wrote {OUT_NPY.name}  shape={X.shape}")
    print(f"Wrote {OUT_META.name}  ({len(meta)} nodes, {len(cats)} categories)")
    by_tier = {}
    for m in meta:
        by_tier.setdefault(m["tier"], 0)
        by_tier[m["tier"]] += 1
    for t, n in sorted(by_tier.items()):
        print(f"  {n:3d}  {t}")


if __name__ == "__main__":
    main()
