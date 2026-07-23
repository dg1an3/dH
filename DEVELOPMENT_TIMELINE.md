# dH/Brimstone Repository: Development History

## Overview

This document traces the evolution of a radiotherapy treatment planning system from its origins in 1988 Fortran code through to the present day. The development spans **three decades** and involves **two parallel evolution paths** that converge in the final Brimstone application.

**Key Facts:**
- **Patent:** U.S. Patent 7,369,645
- **Primary Author:** Derek G. Lane (2nd Messenger Systems)
- **Languages:** Fortran → C++ → Python
- **Core Innovation:** Variational inverse planning using KL divergence minimization

---

## Part 1: Algorithm Origins (1988-2000)

### The Wisconsin Convolution/Superposition Method

The dose calculation algorithm originates from **Rock Mackie's group at the University of Wisconsin** (1988). The key insight: radiation dose can be computed by:

1. **TERMA calculation** - Ray-trace through patient to compute Total Energy Released per unit MAss
2. **Kernel convolution** - Spread energy using pre-computed deposition kernels

**Original Fortran Code** (`PenBeam_indens/code/`):
```
ray_trace_set_up.for  - Build ray-trace lookup tables
make_vector.for       - Direction cosines → voxel locations
new_sphere_convolve.for - Spherical kernel convolution
```

The algorithm uses a **plane-crossing** technique: for each ray direction, precompute which voxel boundaries are crossed and in what order. This avoids expensive per-ray calculations.

### C++ Translation: DivFluence (~2000)

The Fortran code was translated to C++ in `DivFluence/OrigDivFluence.cpp` (~980 lines):

| Fortran | C++ | Purpose |
|---------|-----|---------|
| `mydiv_fluence_calc.for` | `div_fluence_calc()` | TERMA calculation |
| `ray_trace_set_up.for` | `ray_trace_set_up()` | Ray-trace tables |
| `make_vector.for` | `make_vector()` | Voxel traversal |
| `mynew_sphere_convolve.for` | `new_sphere_convolve()` | Kernel convolution |

This translation preserved the algorithm exactly while enabling integration with Windows/MFC applications.

---

## Part 2: Visualization Framework (2000-2002)

### Commercial Context: Siemens VSim Prototype

Around 2000-2001, development began on a **prototype for Siemens' VSim** (Virtual Simulation) product. This commercial context explains the sophisticated visualization architecture.

**Evolution of rendering:**
```
OGL_BASE (2000)     - OpenGL immediate-mode rendering
    ↓
GEOM_VIEW (2001)    - Migration to Direct3D 8 (better Windows drivers)
    ↓
RT_VIEW (2002)      - Radiotherapy-specific: beams, machines, contours
```

### The DRR Renderer: Bridging Visualization and Dose Calculation

**Key Discovery:** The DRR (Digitally Reconstructed Radiograph) renderer in `VSIM_OGL/DRRRenderer.cpp` uses the **same ray-tracing algorithm** as TERMA calculation, but for visualization:

| DRR Renderer | TERMA Calculator |
|--------------|------------------|
| Synthetic X-ray image | Dose deposition volume |
| Sum CT values along ray | Integrate attenuation along ray |
| 2D output (image) | 3D output (TERMA volume) |
| Fixed-point arithmetic (fast) | Floating-point (precise) |

**The `ClipRaster()` function** in DRRRenderer.cpp performs the same plane-crossing voxel traversal as the Fortran `make_vector()`:

```cpp
// DRRRenderer.cpp - ClipRaster()
// Uses 16.16 fixed-point for speed
nPixelValue += pppVoxels[viStart[2] >> 16][viStart[1] >> 16][viStart[0] >> 16];
viStart += viStep;
```

The DRR files were removed ~2001 when the VSim prototype transitioned to Siemens' proprietary codebase. They have been **restored from commit 40e1350a** (June 7, 2002) as historical reference:
- `VSIM_OGL/DRRRenderer.cpp` / `.h`
- `GEOM_VIEW/DRRRenderable.cpp` / `include/DRRRenderable.h`

---

## Part 3: Optimization Framework Development (2003-2006)

### PenBeamEdit (~2004)

Before the full optimization system existed, `PenBeamEdit` provided a standalone tool for:
- Visualizing beamlet intensity maps
- Manual editing of beamlet weights
- Testing TERMA and convolution calculations

This served as a **testbed** for the dose calculation pipeline.

### Core Algorithm Crystallization (2003-2006)

The variational inverse planning algorithm took shape:

1. **Objective Function:** Minimize KL divergence between calculated and target DVHs
2. **Optimization:** Conjugate gradient with adaptive variance
3. **Multi-scale:** Coarse-to-fine pyramid (4 levels: 8mm → 0.5mm)
4. **Parameterization:** Sigmoid transform for bounded beamlet weights

**April 2006: CVS/Git Migration**

The first git commits (April 1-2, 2006) represent migration from CVS, not project start:
- 41 commits in 2 days
- Thread-safety fixes for multi-core optimization
- VS2005 secure CRT migration
- 4-level pyramid finalized

---

## Part 4: Production System (2006-2009)

### RtModel Library

The production dose calculation lives in `RtModel/`:

| File | CVS Rev | Function |
|------|---------|----------|
| `BeamDoseCalc.cpp` | r602 | TERMA ray-tracing |
| `SphereConvolve.cpp` | - | Kernel convolution |
| `KLDivTerm.cpp` | r647 | KL divergence computation |
| `ConjGradOptimizer.cpp` | r644 | Conjugate gradient optimizer |
| `PlanPyramid.cpp` | r647 | Multi-scale optimization |

**Key improvement over DivFluence:** `BeamDoseCalc::TraceRayTerma()` uses **trilinear interpolation** with neighborhood updates, vs. nearest-neighbor in the original:

```cpp
// BeamDoseCalc.cpp - TraceRayTerma()
REAL fluenceInc = fluence0 * exp(-mu * path) * mu * deltaPath;
UpdateTermaNeighborhood(nNdx, weights, fluenceInc);  // Trilinear splatting
```

### Brimstone Application

MFC-based GUI integrating all components:
- DICOM CT import
- Structure (ROI) definition
- Prescription/constraint setup
- Multi-threaded optimization
- Real-time DVH visualization

---

## Part 5: Algorithm Lineage Summary

```
1988  Wisconsin Fortran (Rock Mackie)
      │
      │   ray_trace_set_up.for
      │   make_vector.for
      │   new_sphere_convolve.for
      │
      ▼
~2000 DivFluence (C++ translation)
      │
      │   div_fluence_calc()      ─────────────────┐
      │   ray_trace_set_up()                       │
      │   new_sphere_convolve()                    │
      │                                            │
      ▼                                            │ Same algorithm,
2001  DRRRenderer.cpp (visualization variant)      │ different purpose
      │                                            │
      │   ClipRaster() - fixed-point ray-trace     │
      │   ComputeDRR() - accumulate CT values      │
      │                                            │
      ▼                                            │
2004  PenBeamEdit (beamlet editing testbed)        │
      │                                            │
      ▼                                            │
2008  RtModel/BeamDoseCalc.cpp (production)  ◄─────┘
      │
      │   TraceRayTerma() - floating-point
      │   TrilinearInterpDensity() - interpolation
      │   UpdateTermaNeighborhood() - splatting
      │
      ▼
2025  python/read_process_series.py (ITK port)
```

---

## Part 6: Later Development (2010-2026)

### Maintenance Phase (2010-2015)
- **2010:** Code reorganization, moved files to RtModel
- **2014:** ITK 4.3 upgrade, VS2013 support
- **2015:** Open source publication on GitHub, BSD license

### Machine Learning Experiments (2019-2020)
- TensorFlow 2.0 exploration of variational approaches
- `entropy_max` notebooks investigating entropy gradients
- Connection to variational Bayes formalization

### Modern Era (2025)
- **January:** Python/ITK port of TERMA calculation
- **October:** Repository consolidation (pheonixrt merge)
- **October:** Variational Bayes documentation, optional free energy calculation
- **November:** Docker builds for Fortran code, EGSnrc integration

### Build Resurrection (May 2026)

After several years of toolchain drift the C++ projects no longer built on a stock VS2022 install. The May 2026 effort restored Debug|x64 builds for the three solution projects (`RtModel`, `Graph`, `Brimstone`) end-to-end, producing a launchable `Brimstone.exe` again.

**Toolchain & dependency migrations:**
- **ITK 4.3 → ITK 5.4** (via vcpkg `x64-windows`). The `.vcxproj` files referenced an ITK *source-tree* layout (`$(ITK_DIR)\Modules\Core\SpatialObjects\include\…`); collapsed to vcpkg's flat `include\ITK-5.4` plus VXL/VNL paths. ITK 5.4 requires C++17 for inline variables and `[[maybe_unused]]`, so `<LanguageStandard>stdcpp17</LanguageStandard>` added project-wide.
- **Intel IPP 5.0 → oneAPI IPP 2022.** The `ippm` matrix module was removed entirely in modern IPP — the `ippmDotProduct_*` and `ippmL2Norm_*` template specializations in `VectorOps.h` were dropped (the templated fallbacks remain). `ippsConv_64f` short-form was replaced by `ippsConvolve_64f` with a different buffer-passing signature; rather than rewrite for the new API, the two call sites in `Histogram::ConvGauss` and `HistogramGradient::Conv_dGauss` got a hand-rolled 1-D linear convolution loop. `ippiWarpAffineBack_32f_C1R` (image affine warp, used for slice-view rendering) was stubbed pending an `itk::ResampleImageFilter` replacement.
- **MFC component reinstalled** in VS2022 BuildTools (Community had only ATL); the project explicitly requires `<UseOfMfc>Dynamic</UseOfMfc>`.
- **DCMTK 3.6.9 installed via vcpkg** to satisfy `SeriesDicomImporter` (DICOM RT structure-set import). DCMTK's `osconfig.h` C++11 detection requires MSVC's opt-in `/Zc:__cplusplus` flag — without it, `__cplusplus` reports `199711L` and the header errors out.

**ITK 5.x API churn surfaced by the rebuild:**
- `itk::SpatialObjectPoint::GetPosition()` renamed to `GetPositionInObjectSpace()` (8 sites in `PlanarView.cpp`, 2 in `Structure.cpp`).
- `itk::PolygonSpatialObject::AddPoint(Point)` now requires a wrapping `SpatialObjectPoint` instead of a bare `itk::Point` (call sites in `PlanarView`, `SeriesDicomImporter`).
- `itk::PolygonSpatialObject::ReplacePoint(...)` removed entirely; vertex moves now mutate the existing point in place via `SetPositionInObjectSpace`.
- `vector<itk::SmartPointer<>>::push_back(NULL)` no longer compiles (ambiguous `int → SmartPointer` conversion); `nullptr` required throughout.

**C++17 / modern-MSVC adjustments:**
- `auto_ptr` (removed in C++17) → `std::unique_ptr` (`BrimstoneDoc::m_pOptimizer`).
- Legacy MS-only header `<typeinfo.h>` → standard `<typeinfo>` (`XMLLogging`).
- `WINVER` and `_WIN32_WINNT` bumped from `0x0400` (Windows 95!) to `0x0601` (Windows 7) — modern MFC errors below `0x0501`.
- `NOMINMAX` defined to prevent `windows.h`'s `max`/`min` macros from clashing with `vnl_sse::max`/`min` member functions.

**Other small fixes:**
- `DCMTK::findAndGetSint32` argument tightened from `long&` to `Sint32&` — three call sites in `SeriesDicomImporter`.
- `Log` macro (`#define Log OutputDebugString`, single-arg) was being called printf-style in newly-added free-energy logging code; wrapped with `CString::Format`.
- vcpkg auto-link mismatch on Brimstone Debug|x64 (vcpkg defaulted to release libs, MFC uses `MultiThreadedDebugDLL`); resolved by setting `<UseDebugLibraries>true</UseDebugLibraries>` so vcpkg picks debug variants.
- `PrescriptionBar.cpp` is C++/CLI managed code (System.Windows.Forms references); excluded from x64 build via `<ExcludedFromBuild>` since modern MSVC has no x64 C++/CLI toolchain for legacy WinForms references.

**Verification:** A standalone smoke test (`RtModelSmokeTest/smoke_test.cpp`) was written against the hand-rolled convolution. It checks dimensional correctness (`N + K − 1`), sum-preservation (`Σ out = Σ in × Σ k`), delta-input recovery, symmetry, and orientation across four input shapes — 27 checks all pass.

The Win32 configurations were not migrated — vcpkg only has the `x64-windows` triplet installed, and resurrecting Win32 would require a parallel `x86-windows` triplet plus restoring the C++/CLI WinForms toolchain. Release|x64 was mirrored for `RtModel` only; `Graph` and `Brimstone` Release|x64 still need the same treatment.

---

---

## Part 7: Formal Modeling Direction (2026 →)

With the Brimstone application running again, the next direction is **bringing it under the same formal-verification lens that the algorithm itself was originally evaluated through**. This is not a new technique for this codebase — it is the same methodology returning home.

### Lineage: from CRUTPr (2003) to ALGT (2026)

The original Siemens COHERENCE Dosimetrist Workspace 2.0 (2003) was verified using a Prolog-based procedure called **CRUTPr** — *executable requirements* expressed as predicates over inputs and outputs, run in SWI-Prolog. Derek's recollection of that work is preserved in [docs/wiki/VerificationExpertSystem.md](docs/wiki/VerificationExpertSystem.md): rather than re-implementing each algorithm in MATLAB to compare against, you describe declaratively *what relationships must hold* between inputs and outputs, and check those.

That pattern was extended at `d:/MUSIQ/ALGT/` (the *Algorithm Logic verification, Clarion simulator, MUZAQ demonstrator* platform) into three coordinated threads sharing a SWI-Prolog + Logtalk foundation:

1. **ALGT** — formal verification of geometric RTP algorithms (descendant of CRUTPr). The reference test corpus at `d:/MUSIQ/ALGT/algt_tests/` covers `ALGT_BEAM_VOLUME`, `ALGT_MARGIN3D`, `ALGT_MESH_PLANE_INTERSECTION`, `ALGT_SSD`, `ALGT_ISODENSITY`, `ALGT_STRUCT_PROJ`, and more — each a `.pl` module of declarative invariants over the algorithm's IO.
2. **Clarion simulator** — a DCG parser and modular execution engine for the Clarion 4GL language used in MOSAIQ, with execution-trace comparison against the compiled DLL.
3. **MUZAQ modernization demo** — translation of a FreePascal/Lazarus treatment-management app to a Prolog **labeled transition system**, then onward to an Elixir actor system, with each form becoming an actor and every GUI event + DB command surfacing on an interleavable event stream.

The unifying claim — captured in `d:/MUSIQ/ALGT/docs/compositional-semantics.md` — is **bisimilarity as a release criterion**: a modernized component is safe to substitute when no external observer (including the database, including the user's mouse) can behaviorally distinguish it from the original.

### Direction A: Brimstone GUI as a labeled transition system

The MUZAQ pattern lifts cleanly onto Brimstone because Brimstone is structurally the same kind of system — an MFC SDI application (`CBrimstoneApp` / `CBrimstoneDoc` / `CBrimstoneView` / `CMainFrame`) coordinating dialog edits, file I/O, and a long-running optimization thread (`OptThread`). What MUZAQ does for Lazarus forms, the `cpp-state-model` skill lineage does for `CDialog`/`CView` subclasses: each MFC entity becomes a state machine, GUI events become labeled transitions, and side-effects (DICOM reads, kernel-data loads, plan saves, optimizer ticks) surface on an interleavable event stream.

A first-pass slice of the Brimstone LTS:

| MFC entity | DCG / Prolog module | Key labeled transitions |
|---|---|---|
| `CBrimstoneDoc` | `doc_state_dcg.pl` | `dicom_imported(series)`, `plan_loaded(path)`, `plan_saved(path)`, `series_set(series)`, `plan_set(plan)` |
| `CBrimstoneView` + `CPlanarView` | `view_state_dcg.pl` | `slice_changed(z)`, `lut_updated(idx,lut)`, `contour_drawn(struct,poly)`, `vertex_dragged(struct,idx,Δ)` |
| `CMainFrame` + dialogs | `frame_state_dcg.pl` | `prescription_opened`, `plan_setup_opened`, `beamlets_generated`, `optimization_started`, `optimization_finished` |
| `OptThread` | `optimizer_op_dcg.pl` | `pyramid_level_advanced(L)`, `cg_iteration(k)`, `kl_evaluated(value)`, `free_energy_evaluated(F)`, `converged` |
| `SeriesDicomImporter` | `dicom_boundary.pl` | `read_series(uid)`, `read_struct_set(roi*)`, `read_dose(plan)` |

Each transition carries enough payload to reconstruct the visible state for trace comparison against a recorded session of the live MFC app. The interleavable event stream is the surface where bisimilarity gets checked.

This work is the natural follow-on to the **Build Resurrection** above: now that Brimstone runs again, recording reference event traces becomes feasible.

### Direction B: Prolog verification predicates for the dose-calc & optimizer kernels

The ALGT tradition models numerical kernels not as state machines but as **pure functions with declarative IO predicates** — the `python-algorithm-verification` skill descended from `algt_tests/`. Brimstone's RtModel library has six obvious candidate kernels, each with invariants the existing C++ code relies on but does not formally check:

| Kernel | Source | Invariants worth encoding |
|---|---|---|
| `Histogram::ConvGauss` / `HistogramGradient::Conv_dGauss` | [RtModel/Histogram.cpp](RtModel/Histogram.cpp), [RtModel/HistogramGradient.cpp](RtModel/HistogramGradient.cpp) | Output dim = `N+K−1`; sum-preservation `Σ out = Σ in × Σ k`; delta-input recovers kernel; symmetry; linearity |
| `KLDivTerm::Eval` | [RtModel/KLDivTerm.cpp](RtModel/KLDivTerm.cpp) | KL ≥ 0 (Gibbs); KL = 0 iff target = calculated; gradient-of-KL agrees with finite-difference at random probes |
| `DynamicCovarianceOptimizer` (CG) | [RtModel/ConjGradOptimizer.cpp](RtModel/ConjGradOptimizer.cpp) | Sufficient-decrease per iteration; conjugacy of search directions in the quadratic limit; covariance Σ positive-definite |
| `SphereConvolve` | [RtModel/SphereConvolve.cpp](RtModel/SphereConvolve.cpp) | Energy conservation under convolution; spherical-kernel rotational invariance modulo sampling |
| `BeamDoseCalc::TraceRayTerma` | [RtModel/BeamDoseCalc.cpp](RtModel/BeamDoseCalc.cpp) | TERMA ≥ 0; attenuation monotone in path-length; voxel traversal exhaustive (no skipped voxels along the ray) |
| `PlanPyramid` (down/upsample) | [RtModel/PlanPyramid.cpp](RtModel/PlanPyramid.cpp) | Information-preserving Galerkin condition for the inverse-filter step; weight transfer between levels approximately commutes with the cost function |

The Build Resurrection smoke test ([RtModelSmokeTest/smoke_test.cpp](RtModelSmokeTest/smoke_test.cpp)) is in fact a *baby CRUTPr* for the convolution kernel — it encodes four of the invariants in the first row of the table above, just in C++ rather than Prolog. Promoting it to Prolog predicates over a tolerance-based equality assertion (the ALGT pattern) would generalize the approach to the rest of the kernels and let the same predicates run against either the C++ implementation or the [python/](python/) ITK port.

### Direction C: Historical LTSs for the visualization lineage

The same modeling technique applied to *current* Brimstone (Direction A) is equally valuable applied retrospectively to the **older** sister projects that this repository still carries — `VSIM_OGL`, `VSIM_MODEL`, `PenBeamEdit`, `RT_VIEW`, `GEOM_VIEW`, `OGL_BASE`. These are the visualization stack and the bridging beamlet-editing testbed documented in **Parts 2–3** above and in the standalone [RT_VIEW_HISTORY.md](RT_VIEW_HISTORY.md). Their code is frozen; their algorithms (ray-traced DRR generation, marching-squares contour extraction, slice+dose LUT blending, observable-pattern model/view binding) are exactly the kind of stable, well-bounded targets that ALGT models well. Building LTSs for them turns the prose lineage in this document into something *executable* — and lets the bisimilarity argument span the full chain *2001 VSim → 2004 PenBeamEdit → 2008+ RtModel/Brimstone → 2026 modernized build*, not just "old Brimstone vs. modernized Brimstone."

**VSIM_OGL (~2001, Siemens VSim CT-sim prototype):**

| Entity | DCG / Prolog module | Key labeled transitions |
|---|---|---|
| `CVSIM_OGLApp` (`CWinApp`) | `vsim_ogl_app_dcg.pl` | `app_started`, `doc_template_registered(series\|plan)`, `cmdline_opened(path)` |
| `CMainFrame` (`CFrameWnd`) | `vsim_ogl_frame_dcg.pl` | `view_swapped(simview)`, `menu_invoked(id)`, `tracker_attached(camera)` |
| `CSimView` (`CView`, observer) | `vsim_ogl_simview_dcg.pl` | `plan_observed(plan)`, `beam_added(beam)`, `surface_added(struct)`, `redraw_requested`, `mouse_drag(Δ)` |
| `CDRRRenderer` (`COpenGLRenderer`) | `drr_renderer_dcg.pl` | `scene_changed`, `frame_rendered`, `clip_raster_setup(beam)`, `drr_accumulated(slab)` |

**VSIM_MODEL** (the model side that `simview` observes):

| Entity | DCG / Prolog module | Key labeled transitions |
|---|---|---|
| `CSeries` (`CDocument`) | `vsim_series_dcg.pl` | `volume_loaded(dim,spacing)`, `slice_indexed(z)` |
| `CPlan` (`CDocument`) | `vsim_plan_dcg.pl` | `beam_added(b)`, `beam_removed(b)`, `prescription_set(dose)` |
| `CBeam` (`CModelObject`, observable) | `vsim_beam_dcg.pl` | `gantry_set(°)`, `couch_set(°)`, `collim_set(jaw,jaw)`, `change_fired(field)` |
| `CTreatmentMachine` | `vsim_machine_dcg.pl` | `geometry_loaded(model)`, `lightfield_textured(id)` |

**PenBeamEdit (~2004, beamlet-editing testbed — the bridge between VSim visualization and production dose):**

PenBeamEdit is the architecturally pivotal intermediate point in the lineage. Its [TransitionPlan.txt](PenBeamEdit/TransitionPlan.txt) reads as a literal migration diary — `CImage → CVolume`, `CDib from GUI_BASE`, extending `CPlan` to carry beam doses + beam weights + total dose, an `OpenGLRenderer` for slice rendering with LUT-blending of two images, and a DVH graph view. Modeling it as an LTS captures the *moment* the visualization stack and the dose model first interleaved on a shared event surface.

| Entity | DCG / Prolog module | Key labeled transitions |
|---|---|---|
| `CPenBeamEditApp` (`CWinApp`) | `pbe_app_dcg.pl` | `app_started`, `doc_template_registered(plan)`, `cmdline_opened(path)` |
| `CMainFrame` (`CFrameWnd`) | `pbe_frame_dcg.pl` | `view_attached(pbeview)`, `menu_invoked(id)` |
| `CPenBeamEditView` (`CView`) | `pbe_view_dcg.pl` | `view_drawn(slice)`, `view_resized(w,h)`, `lbutton_down(pt)`, `update_received(hint)`, `slice_changed(z)`, `lut_blend_alpha_set(α)`, `dvh_redrawn` |
| Migration steps from `TransitionPlan.txt` | `pbe_transition_dcg.pl` | `cimage_replaced_by_cvolume`, `gui_base_cdib_adopted`, `plan_extended_with_beam_doses`, `gl_slice_blender_added`, `dvh_view_attached` |

The last row above is unusual — it makes the *transition itself* a transcribed LTS, so the architectural moves recorded in the plain-text plan become checkable artifacts in the same Prolog model. The prefix matches the cpp-state-model skill convention (`pbe_*`).

**Algorithm verification predicates for the DRR / geometric stack:**

The `algt_tests/` corpus already covers some of these — `ALGT_BEAM_CAX_ISOCENTER.pl`, `ALGT_BEAM_VOLUME_PLANAR.pl`, `ALGT_STRUCT_PROJ.pl`, `ALGT_MESH_PLANE_INTERSECTION.pl` — and the dH visualization stack is a natural place to *re-run* those predicates against historical implementations rather than only against modern ones. Specifically:

| Algorithm | Historical site | Modern site | Invariant family |
|---|---|---|---|
| Ray-traced DRR | `VSIM_OGL/DRRRenderer.cpp::ClipRaster` / `ComputeDRR` | `RtModel/BeamDoseCalc.cpp::TraceRayTerma` | Voxel-traversal exhaustiveness; line-integral ≥ 0; rotation invariance up to sampling |
| Marching squares (contour extraction) | `GEOM_VIEW/MarchingSquares.cpp` | (only here historically) | Topological consistency; closed-loop preservation; isovalue monotonicity |
| Beam-volume rendering | `RT_VIEW/BeamRenderable.cpp` | (no modern equivalent) | Beam frustum geometry matches `ALGT_BEAM_VOLUME_PLANAR` |
| Structure projection onto BEV | `RT_VIEW/MachineRenderable.cpp` | (Brimstone uses 2D contours instead) | `ALGT_STRUCT_PROJ` predicates |
| Camera + light primitives | `OGL_BASE/OpenGLCamera.cpp`, `OpenGLLight.cpp` | (replaced in modern era) | Projection-matrix invariants; viewport ↔ world-space round-trip |
| Slice + dose blending (LUT, α) | `PenBeamEdit/PenBeamEditView.cpp` (per `TransitionPlan.txt` step 4) | `Brimstone/PlanarView.cpp::DrawImages` | Blended pixel = `α·LUT₁(slice) + (1−α)·LUT₂(dose)`; clamping; idempotence at α∈{0,1} |
| Beamlet-edit ↔ dose-recalc loop | `PenBeamEdit/PenBeamEditView.cpp::OnLButtonDown` | `Brimstone/OptThread.cpp` (now optimizer-driven) | Edit-then-recalc commutes; modified beam weights monotone in dose at the affected voxels |

**Why bother modeling frozen code?**

Three reasons make this more than a curiosity:

1. **Provenance for the modernization claim.** The bisimilarity criterion in `d:/MUSIQ/ALGT/docs/compositional-semantics.md` becomes much stronger when you can demonstrate it across the *historical* commit-graph as well as the modern one — i.e. show that the 2026-era kernels are bisimilar to the 2008 RtModel kernels, which are themselves bisimilar (modulo recorded differences) to the 2001 VSIM_OGL prototype kernels. The lineage in **Parts 1–5** of this document becomes a chain of formal-equivalence claims, not just a story.
2. **The `cpp-state-model` skill is already shaped for this.** That skill's deliverable list — `ui_state_dcg.pl` + `browse_ops.pl` + `edit_ops.pl` + `<data>_list.pl` + `<display>_list.pl` + scan/op pipeline modules + `<control>_state_model.md` + diagrams + rendered SVGs, with original C++ preserved as anchor comments — is designed for exactly this kind of MFC/ATL legacy code. Each of the historical projects above is a single skill-invocation away from a first-pass model.
3. **Diff-able diagrams.** The PlantUML/SVG diagrams emitted by the cpp-state-model skill are themselves comparable across versions — visual diffs of the LTS for `VSIM_OGL/CSimView` vs. modern `Brimstone/CBrimstoneView` make architectural drift legible at a glance, the way commit logs do not.

The historical projects do not need to *build* under modern toolchains for this to work — they just need to *parse*. Their source is preserved in this repository precisely so the lineage in Parts 1–5 stays groundable.

### Why this direction now

Four reasons converge on this being the right next step rather than a "someday":

1. **The build is alive.** Reference traces and reference outputs can be generated again from the modern Brimstone — necessary for Directions A and B.
2. **The historical code does not need to build.** Direction C only requires the older sources to *parse*; their source is preserved in this repository precisely so that lineage claims stay groundable. No toolchain effort is gated on Parts 1–5 being runnable.
3. **The methodology already exists in this author's hands.** ALGT is not speculative — it is a working SWI-Prolog/Logtalk platform with a test corpus, currently being shaped into an [FDA MDDT submission](https://d:/MUSIQ/ALGT/docs/FDA_MDDT_Proposal.md) as ALGT-FMEA.
4. **The C++ kernels (modern *and* historical) are stable.** With ITK 4.3 → 5.4 and IPP 5.0 → oneAPI behind us on the modern side, and the legacy projects frozen by definition, kernels in scope for modeling are unlikely to churn under our feet during model construction.

The deliverable shape — `ui_state_dcg.pl` + per-class operation modules + `<entity>_state_model.md` + PlantUML/SVG diagrams for the GUI threads (modern *and* historical); and `<algo>_verification.pl` + `<algo>_invariants.pl` + a pytest-style runner harness for the kernel thread — is the same shape `d:/MUSIQ/ALGT` already produces for Clarion forms and pymedphys kernels.

---

## Key Insights

### 1. The Algorithm is Older Than the Repository
The git history starts in 2006, but the algorithm dates to 1988 (Fortran) with C++ translation around 2000.

### 2. Visualization and Dose Calculation Share Common Roots
DRRRenderer (2001) and BeamDoseCalc (2008) use the same ray-tracing technique. The DRR code is the "missing link" showing how visualization expertise informed dose calculation.

### 3. Commercial Origins Explain the Architecture
The Siemens VSim prototype work (2001) drove professional-grade visualization that seems over-engineered for a research project.

### 4. DivFluence is the Rosetta Stone
`DivFluence/OrigDivFluence.cpp` is a direct C++ translation of the Fortran, making it the key reference for understanding the algorithm's structure.

---

## Intellectual Property

- **U.S. Patent 7,369,645** - Radiotherapy treatment planning
- **Copyright:** 2000-2021, Derek G. Lane
- **License:** Modified BSD with Hippocratic and Anti-996 clauses
- **Company:** 2nd Messenger Systems

---

## Source Attribution

This history is based on:
1. Git commit history (2006-2025)
2. CVS $Id$ tags in source files (r602-r647, 2008-2009)
3. Copyright headers in source files (2000-2002)
4. Oral history from the original developer
5. Fortran code comments (Rock Mackie, 1988)

---

*Document revised: May 9, 2026*
