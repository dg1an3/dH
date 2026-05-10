# CDRRRenderer State Model

`CDRRRenderer` is a `COpenGLRenderer` subclass from the VSIM_OGL CT-simulator prototype (~2001) that produces a Digitally Reconstructed Radiograph (DRR) from a CT volume by ray-tracing per output pixel through the volume and accumulating voxel values along each ray. It is structurally distinct from `CSimView` in two important ways: (1) **it has no `BEGIN_MESSAGE_MAP` block** — it is a renderable participant in an OpenGL scene rather than a Win32 widget, and its event surface comes from public methods, an observer callback, and a background-thread completion event; and (2) **the heavy lifting is algorithmic** — `ClipRaster` (16.16 fixed-point ray-volume clipping) and `ComputeDRR` (project corners → unproject anchors → ray-march → post-process) are both translated as pure Prolog pipeline modules in addition to the LTS itself.

The source file carries a verbatim header marking it `HISTORICAL FILE - Restored from commit 40e1350a (June 7, 2002) - This file is NOT built`. Per `DEVELOPMENT_TIMELINE.md` Part 7 Direction C, frozen historical sources are legitimate LTS targets — the bisimilarity claim against modern descendants (`Brimstone/CPlanarView` and the Python ITK-based DRR port) depends on having a parsable, modeled antecedent.

## 1. Primary State Machine

**9 event terminals across 4 states + 1 terminal.** The browse tier (`unbound | dirty | computed`) covers the synchronous render path; the scan tier (`bg_computing`) is structurally present but unreachable in source as preserved (the `AfxBeginThread` spawn at `DRRRenderer.cpp:539` is commented out).

![CDRRRenderer Primary State Machine](diagrams/stm_primary.svg)

> Source: [`diagrams/stm_primary.puml`](diagrams/stm_primary.puml)

The four substates of the browse tier reflect the dirty-flag discipline maintained around `m_bRecomputeDRR`. `set_volume(none)` returns the renderer to `unbound`, where `DrawScene` short-circuits at `cpp:466`. Any of `set_volume(V)` (with `V != none`), `set_volume_transform(M)`, or `camera_projection_change` (the observer callback from `m_pView->camera.projection`) drives back to `dirty`. Only `compute_drr` or `draw_scene` (when `recompute == true`) advance to `computed`.

## 2. State Dict Schema

### 2.1 Schema diagram

![CDRRRenderer state{} dict schema](diagrams/bdd_state_dict.svg)

> Source: [`diagrams/bdd_state_dict.puml`](diagrams/bdd_state_dict.puml)

**Key insight:** `m_pThread` is the only handle on the background hi-res pass. The `AfxBeginThread` spawn at `cpp:539` is commented out in source as preserved, so `thread_running` stays `false` and the `bg_computing` state is unreachable. The LTS retains the state for completeness — a future un-comment of that one line should not change the model surface.

### 2.2 Truncated record details

| Field | Type | Notes |
|---|---|---|
| `use_tandem_rays` | `bool` | ⚠ Stale: from `// #define USE_TANDEM_RAYS` commented out at `DRRRenderer.cpp:35`. A feature toggle that did not survive. Preserved as a state field with default `false` for fidelity. |

### 2.3 State{} write authority

| Field | Written By |
|---|---|
| `win` | `init` (unbound), `set_volume`, `compute_drr` (computed), `on_camera_projection_change` (dirty), `bg_thread_complete`, `bg_thread_aborted` |
| `view` | `init` (one-shot) |
| `for_volume` | `set_volume` |
| `volume_transform` | `set_volume_transform` |
| `recompute` | `init` (true), `set_volume`, `set_volume_transform`, `compute_drr` (false), `on_camera_projection_change`, `bg_thread_complete` (false) |
| `n_steps`, `n_shift`, `n_res_div` | `init` (defaults from `RAY_TRACE_RESOLUTION` macros), `draw_scene` Phase 3 (low-res reset), Phase 5 (hi-res reset) |
| `image_width`, `image_height` | `draw_scene` (derived from `rect.Width / n_res_div`) |
| `viewport` | `draw_scene` (`glGetIntegerv` snapshot) |
| `n_max`, `n_min`, `over_max` | `compute_drr_pipeline:compute_drr_tick` (per-pixel update) |
| `pixels` | `compute_drr_pipeline` (loaded), `set_volume(none)` (cleared) |
| `thread`, `thread_running` | `bg_thread_started`, `bg_thread_complete`, `bg_thread_aborted` (always via `abort_thread_if_running` helper) |

**Key insight:** Three duplicate "if (m_pThread != NULL) suspend + delete + clear" blocks in source (`cpp:75-77` destructor, `cpp:470-472` DrawScene, `cpp:553-556` OnChange) all map to the single `edit_ops:abort_thread_if_running/2` predicate. The LTS factors duplicate C++ code into one Prolog clause without losing fidelity.

### 2.4 Enumerations

`DRRMode = unbound | dirty | computed | bg_computing`. The `bg_computing` value is reachable only when the commented spawn at `cpp:539` is restored.

### 2.5 Database Surface

**No DB access.** `CDRRRenderer` reads voxels exclusively from the in-memory `CVolume<short>` referenced by `forVolume`; there is no SQL or persistent-store interaction. The output is rendered into the OpenGL framebuffer via `glDrawPixels`, which is also not a DB.

## 3. Event → Predicate Transformation Map

| Event | Guard | Transformation Predicates | State Fields Affected | Tables Read/Written |
|---|---|---|---|---|
| `init(View)` | `win == unbound` | `edit_ops:init` | every initial-state field; attaches camera-projection observer | (none) |
| `set_volume(V)` | (none) | `edit_ops:set_volume` | `for_volume`, `win` (`unbound`/`dirty`), `recompute`, `pixels` | (none) |
| `set_volume_transform(M)` | (none) | `edit_ops:set_volume_transform` | `volume_transform`, `recompute` | (none) |
| `compute_drr` | `for_volume != none` | `edit_ops:compute_drr` (LTS-level); `compute_drr_pipeline:compute_drr_init`/`tick`/`complete` (algorithmic) | `win` (computed), `recompute` (false), pipeline writes `pixels`/`n_max`/`n_min`/`over_max` | (none — voxel array is in-memory) |
| `draw_scene` | (none) | `edit_ops:draw_scene` (cascades into `compute_drr` if dirty); `clip_raster_pipeline:clip_all_axes` (per-pixel) | depends on cascade; eventually framebuffer via `glDrawPixels` boundary | (none) |
| `render_scene` | (none) | `edit_ops:on_render_scene` (empty body) | (none) | (none) |
| `camera_projection_change` | (none) | `edit_ops:on_camera_projection_change`, `abort_thread_if_running` | `recompute` (true), `win` (dirty), `thread*` | (none) |
| `bg_thread_complete` | `thread_running == true` | `compute_drr_pipeline:bg_thread_complete` | `thread_running` (false), `recompute` (false) | (none) |
| `destruct` | (none) | `edit_ops:destruct`, `abort_thread_if_running` | `thread*` cleared | (none) |

There are no `ON_UPDATE_COMMAND_UI` handlers because there is no Win32 menu/toolbar surface — `CDRRRenderer` is a sub-renderable, not a top-level UI element.

## 4. Data Pipeline (Compute)

The compute pipeline is the algorithmic heart of the deliverable — a 5-phase translation of `ComputeDRR` (`DRRRenderer.cpp:242-458`) decomposed by procedural block.

![CDRRRenderer Compute Pipeline](diagrams/act_data_pipeline.svg)

> Source: [`diagrams/act_data_pipeline.puml`](diagrams/act_data_pipeline.puml)

**Phase 1 (DCG dispatch):** `step//2` matches `[compute_drr]`, guards on `for_volume != none`, calls `edit_ops:compute_drr/2`. **Phase 2 (project corners):** the 8 volume corners are `gluProject`'d into window space and the min/max projected `z` define the near/far planes that bracket the volume. **Phase 3 (unproject anchors):** at both `z_min` and `z_max`, three `gluUnProject` calls produce a starting point and two step vectors (one per pixel right, one per pixel down); these are converted to 16.16 fixed-point with a `+32768` sub-pixel center offset. **Phase 4 (ray march):** for each output pixel `(nY, nX)`, compute `viStep = (viEnd - viStart) >> n_shift`, run `clip_raster_pipeline:clip_all_axes` to confine the ray to the `[0,W) x [0,H) x [0,D)` voxel box, then accumulate voxel values along the clipped ray. **Phase 5 (post-process):** under `#ifdef POST_PROCESS`, window/level the accumulated `int` values into `[0, 255]` and pack to RGBA.

The fidelity-critical part is Phase 4. `clip_raster_pipeline:clip_raster/7` reproduces all four corner cases of the C++ original (start-below-min × step-sign, start-above-max × step-sign, plus the symmetric pair on the END), including the four `ASSERT` post-conditions at `cpp:231-234` that bound the surviving start/end indices. The `nDelta == nDelta2` cross-checks against the floating-point implementation are commented out in source and preserved as comments only.

## 5. Save Pipeline

The "save" path is `DrawScene` (`DRRRenderer.cpp:460-542`). The durable side-effect is the framebuffer update via `glDrawPixels` — there is no persistent file or database write.

![CDRRRenderer DrawScene Pipeline](diagrams/act_save_pipeline.svg)

> Source: [`diagrams/act_save_pipeline.puml`](diagrams/act_save_pipeline.puml)

Phase 5 of the diagram captures the **commented-out** background-thread spawn at `cpp:539`. The source as preserved leaves it disabled; the LTS retains the structural state (`bg_computing`) so that future un-comment does not break compatibility with extant traces.

## 6. Domain Constraints and Invariants

![CDRRRenderer Constraints and Invariants](diagrams/par_invariants.svg)

> Source: [`diagrams/par_invariants.puml`](diagrams/par_invariants.puml)

### 6.1 Constraints (validation-time gates)

- **`compute_drr_gate`** — `state.for_volume != none` is required for the `[compute_drr]` step rule. Reads `forVolume->width.Get()` unconditionally at `cpp:247`; the gate prevents a NULL deref at the LTS level.
- **`draw_scene_gate`** — `DrawScene` short-circuits when `for_volume == none` at `cpp:466`. Modeled as a first-branch in `edit_ops:draw_scene/2` rather than a precondition gate, mirroring the C++ control flow.

### 6.2 Invariants (state-time)

- **`clip_raster_correctness`** — On `Ok==true`, the surviving `Start[axis] >> 16` and `(Start + DestLen*Step)[axis] >> 16` are both within `[SrcMin, SrcMax-1]`. Documents the four `ASSERT` post-conditions at `cpp:231-234`.
- **`thread_lifecycle`** — `thread_running == true` iff `thread != none` and the `CWinThread` is suspended-or-running. The three duplicated "suspend + delete + clear" blocks in source map to the single `abort_thread_if_running/2` predicate.
- **`recompute_dirty_discipline`** — `win == dirty` iff `recompute == true`; `win == computed` implies `recompute == false`. Maintained by every state-mutating predicate.
- **`historical_marker`** — the file's `HISTORICAL FILE - This file is NOT built` header is preserved verbatim; the LTS treats it as a live source for bisimilarity purposes.

## Source Mapping

| Event | C++ Source |
|---|---|
| `init(View)` | `VSIM_OGL/DRRRenderer.cpp:59` (`CDRRRenderer::CDRRRenderer`) |
| `set_volume(V)` | `VSIM_OGL/DRRRenderer.h:34` (`CAssociation< CVolume< short > > forVolume`); externally driven assignment |
| `set_volume_transform(M)` | `VSIM_OGL/DRRRenderer.h:36` (`CValue< CMatrix<4> > volumeTransform`); externally driven `Set` |
| `compute_drr` | `VSIM_OGL/DRRRenderer.cpp:242` (`void CDRRRenderer::ComputeDRR()`) |
| `draw_scene` | `VSIM_OGL/DRRRenderer.cpp:460` (`void CDRRRenderer::DrawScene()`) |
| `render_scene` | `VSIM_OGL/DRRRenderer.cpp:544` (`void CDRRRenderer::OnRenderScene() {}`) — empty body |
| `camera_projection_change` | `VSIM_OGL/DRRRenderer.cpp:548` (`OnChange`); registered at `cpp:68` (`m_pView->camera.projection.AddObserver(this, OnChange)`) |
| `bg_thread_complete` | `VSIM_OGL/DRRRenderer.cpp:39-53` (`UINT BackgroundComputeDRR(LPVOID)`); spawn at `cpp:539` is currently commented out |
| `destruct` | `VSIM_OGL/DRRRenderer.cpp:71` (`CDRRRenderer::~CDRRRenderer`) |

Algorithmic predicates and their C++ originals:

| Prolog predicate | C++ Source |
|---|---|
| `clip_raster_pipeline:clip_raster/7` | `DRRRenderer.cpp:88-238` (`bool ClipRaster(...)`) |
| `clip_raster_pipeline:clip_all_axes/8` | `DRRRenderer.cpp:385-387` (the three-axis `&&` chain) |
| `compute_drr_pipeline:compute_drr_init/2` | `DRRRenderer.cpp:242-349` (Phases 1-3 of `ComputeDRR`) |
| `compute_drr_pipeline:compute_drr_tick/3` | `DRRRenderer.cpp:374-426` (inner X-loop body) |
| `compute_drr_pipeline:compute_drr_complete/2` | `DRRRenderer.cpp:432-455` (`#ifdef POST_PROCESS` block) |
| `compute_drr_pipeline:bg_thread_started/2` | `DRRRenderer.cpp:539` (spawn line, commented out) |
| `compute_drr_pipeline:bg_thread_complete/2` | `DRRRenderer.cpp:39-53` (`BackgroundComputeDRR` body) |

### Cross-language references

The natural cross-deliverable pairing for `CDRRRenderer` is the modern `Brimstone/RtModel/BeamDoseCalc.cpp::TraceRayTerma` (timeline Part 4). Both functions trace rays through a CT volume and accumulate values along each ray; `CDRRRenderer` is the visualization-side version, `BeamDoseCalc` the dose-calculation-side. The bisimilarity claim worth pursuing first is around their **ray-traversal algorithms**: both must visit every voxel intersected by a ray exactly once, modulo their respective sampling granularities. The historical CDRRRenderer used 16.16 fixed-point with `ClipRaster` for performance; `BeamDoseCalc` uses double-precision `TraceRayTerma`. A formal pairing would establish that the set of voxels visited by `clip_raster_pipeline + ray_march` matches the set visited by the `BeamDoseCalc` traversal modulo numeric precision.

The Python port at `python/read_process_series.py::terma_from_density` (timeline Part 6 Modern Era 2025) is the third leg — same algorithm via ITK. A complete Direction B verification package would have one CRUTPr-style invariant predicate set running against all three implementations.

No counterpart deliverables in `clarion-state-model` or `cs-ui-state-model` exist for `CDRRRenderer`. MOSAIQ does not have a DRR rendering counterpart in those ecosystems.
