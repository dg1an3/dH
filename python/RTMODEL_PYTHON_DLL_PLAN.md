# Plan: RtModel as a Python-callable DLL

Goal: call the RtModel inverse-planning optimizer from Python, so plans can be
driven and inspected with the NumPy/SciPy/matplotlib ecosystem instead of the
MFC GUI.

This document records the decision, the constraints that forced it, what has
been done, and the remaining work. Companion docs:

- [`BUILD_NATIVE.md`](BUILD_NATIVE.md) — how to build the extension.
- [`../CYTHON_WRAPPER_DESIGN.md`](../CYTHON_WRAPPER_DESIGN.md) — the full
  intended Python API (class shapes apply regardless of binding tech).

---

## 1. Decision

**Bind with pybind11. Ship a Windows-only `.pyd`. Keep RtModel unchanged.**

| Option | Verdict |
|--------|---------|
| **ctypes** | Rejected. Speaks only the C ABI (flat `extern "C"`, primitive types). RtModel is C++ with templates (`CVectorN<>`), overloaded `operator()`, inheritance and MFC objects. You'd hand-write a C shim around every call first, so ctypes just adds a fragile layer. |
| **Cython** | Rejected for this job. Best when writing *new* hot-loop code; for *wrapping existing* C++ it means a `.pyx`/`.pxd` pair kept in sync with the headers by hand (the old scaffold had already drifted out of sync). |
| **pybind11** | **Chosen.** Header-only, wraps existing C++ (STL, templates, overloads) directly, first-class NumPy buffer protocol, no separate declaration file. A working wrapper already existed in the repo. |

### Why Windows-only

RtModel is not a portable numerical library. MFC/native types appear in the
**public** members and method signatures of exactly the classes to bind:

| Class | Native leakage in its interface |
|-------|---------------------------------|
| `Prescription` | `CTypedPtrMap<CMapPtrToPtr,…>`, `CArray<BOOL,BOOL>` |
| `Plan` | `CTypedPtrMap<CMapStringToOb, CString, CHistogram*>` |
| `VOITerm` | `CArray<BOOL,BOOL>` in the pure-virtual `Eval()` |
| `VectorN` | `CString ToString()`, plus Intel **IPP** (`ipp.h`) |

Plus `stdafx.h` pulls in `afx.h`/`afxwin.h`/`afxtempl.h`/`atlcoll.h`, and the
`.cpp`s lean on **VNL** and **ITK** throughout. `#include <afx.h>` cannot be
compiled by GCC/Clang — MFC is MSVC-only — so a portable `.so` is impossible
without stripping MFC out of the public surface. That refactor was explicitly
deferred; the near-term target is a Windows `.pyd`.

A **de-MFC refactor** (replace `CArray`/`CTypedPtrMap`/`CString` with
`std::vector`/`std::map`/`std::string`, add a scalar fallback for IPP) remains
the correct long-term path to Linux/macOS. It is out of scope for this plan but
noted so the door stays open.

---

## 2. Architecture

```
Python (NumPy / SciPy / matplotlib)
        │  import rtmodel_core
        ▼
rtmodel_core.pyd                     ← pybind11 module (this work)
        │  rtmodel_bindings.cpp
        ▼
RtModel.lib  (static, built by RtModel.vcxproj, unchanged)
        │
        ├─ MFC (Dynamic)     ├─ Intel IPP
        └─ ITK 5.x + VNL
```

RtModel stays a **static library**. The extension compiles only the thin
binding TU (`rtmodel_bindings.cpp`) and **links** `RtModel.lib` + ITK/VNL/IPP.
This avoids recompiling every RtModel source (each needs the `stdafx.h`
precompiled header and the full dependency stack) inside the Python build.

### Build model (two steps)

1. `msbuild Brimstone_src.sln /t:RtModel /p:Configuration=Release /p:Platform=x64`
   → `x64/Release/RtModel.lib`
2. `pip install -e .` (in `python/`) → compiles + links `rtmodel_core.pyd`

All external paths come from environment variables that default to the
vcpkg/oneAPI layout already in `RtModel.vcxproj`. Details in `BUILD_NATIVE.md`.

---

## 3. Status

### Done

- [x] Chose pybind11; consolidated onto `rtmodel_bindings.cpp` as the single
      native path.
- [x] Fixed the two build/import blockers in `rtmodel_bindings.cpp`:
  - MFC headers now precede `pybind11`/`Python.h` (else `afx.h` errors because
    `Python.h` has already included `windows.h`).
  - Registered the abstract `DynamicCovarianceCostFunction` base, so the
    `Prescription` base is valid to pybind11 and a `Prescription` can be passed
    where the optimizer constructor wants a `DynamicCovarianceCostFunction*`.
- [x] Rewrote `setup.py` as a pybind11 Windows build (MFC Dynamic, `/std:c++17`,
      Unicode, `USE_IPP`/`USE_RTOPT`/`NOMINMAX`, links `RtModel.lib` + ITK/VNL/IPP,
      env-var-driven paths, non-Windows guard).
- [x] `pyproject.toml` builds against pybind11; `BUILD_NATIVE.md` written.
- [x] Marked the superseded Cython scaffold (`pybrimstone/core.pyx`/`.pxd`).

### Currently exposed to Python

- `VectorN` (`CVectorN<double>`) with NumPy round-trip.
- `Prescription` — `get_number_of_unknowns()`, `set_gbin_var()`.
- `PrescriptionWrapper` — `evaluate(x) -> (value, grad)`, `evaluate_value(x)`,
  `get_dimension()` (SciPy-friendly).
- `ConjGradOptimizer` (`DynamicCovarianceOptimizer`) — `minimize()`,
  `set_adaptive_variance()`, `set_compute_free_energy()`, `get_final_value()`,
  `get_entropy()`, `get_free_energy()`, `get_adaptive_variance()`.

### Not verified

- Not compiled: MFC/ITK/IPP require MSVC; correctness comes from
  cross-referencing headers and `RtModel.vcxproj`, not a green build. First
  build on a real Windows toolchain will shake out remaining details.

---

## 4. Roadmap to end-to-end use

Today a `Prescription` must still be constructed on the C++ side (no Python
constructors for the data model yet). To make the module usable start-to-finish
from Python:

### Phase A — Data model construction
1. Bind `dH::Plan` construction and beam / beamlet setup.
2. Bind `Series` / `Structure`, plus ITK-volume ↔ NumPy conversion
   (image buffer sharing without copies where possible).
3. Add a `Prescription(plan)` constructor and `AddStructureTerm` / `KLDivTerm`
   so objectives can be defined from Python.

### Phase B — Optimization loop
4. Wire the `OptimizerCallback` through to a Python `Callable` for live
   monitoring (iteration, cost, gradient norm, current weights, dose, sigma).
5. Expose `PlanOptimizer` (multi-scale pyramid) and return an
   `OptimizationResult`.

### Phase C — Validation & ergonomics
6. Cross-check the pybind11 path against the pure-Python `pybrimstone`
   reimplementation (same inputs → same cost/gradient) as a correctness oracle.
7. Package wheels (cibuildwheel on Windows), pin exact ITK/IPP lib sets.
8. Optional DICOM I/O and pymedphys/matplotlib visualization helpers.

### Longer term (separate track)
9. De-MFC the RtModel public surface → cross-platform (Linux/macOS) builds.

---

## 5. Risks

| Risk | Mitigation |
|------|------------|
| MFC + `Python.h` include-order / CRT conflicts | Documented; MFC-first ordering and `/MD` everywhere. |
| ITK is highly modular — many link deps | `setup.py` globs all ITK/VNL libs; `RTMODEL_ITK_LIBS` pins an exact set. |
| Runtime DLL loading (ITK/IPP/MFC) | Ship DLLs beside the `.pyd` or on `PATH`; document. |
| API drift between C++ and pure-Python port | Phase C uses the port as a numerical oracle. |
| Windows-only limits adoption | De-MFC track (Phase 9) keeps cross-platform open. |
