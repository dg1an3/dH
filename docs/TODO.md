# brimstone — Modernization Roadmap

A cross-cutting TODO for modernizing the brimstone codebase (RtModel, Brimstone,
Graph) and related repositories. Ordered by dependency: earlier phases unblock
later ones.

## Phase 1 — Understand & restructure (foundation)

- [ ] **Reconstruct version history from originals.**
  Diff `VecMat` vs. `VecMat_original` and `Brimstone` vs. `Brimstone_original`
  to produce a coherent, documented version history. Do this *first* — capture
  provenance before moving files around.
- [ ] **Reorganize source directories** into `Applications`, `Libraries`, and
  `Third Party`.
- [ ] **Implement a full CMake-based build.** Precondition for all
  cross-platform work in Phase 2.
- [ ] **Replace `XMLLogging` with structured logging.**

## Phase 2 — Decouple from Windows/MFC
*(depends on CMake)*

- [ ] **Remove `WINDOWS` dependence from `RtModel`** so the core library builds
  and runs cross-platform.
- [ ] **Implement a DICOM-native data layer** (combines former items 8 + 9):
  - Non-MFC persistence built around DICOM objects (preferred over ad-hoc JSON).
  - Remote data access via DicomWeb — use a pymedphys DicomWeb server to host
    binary data store.
  - Shared object model / serialization so local persistence and DicomWeb access
    are the same operation over different transports.
- [ ] **Port the MFC GUI to WebView2** with vtk.js (or an alternative graph
  library) for visualization.

## Phase 3 — New capabilities

- [ ] **Add RTK support and CUDA-based optimization for `RtModel`.**

---
*Ordering is a suggestion based on what unblocks what. Update freely; ask me to
re-sync or re-prioritize any time.*
