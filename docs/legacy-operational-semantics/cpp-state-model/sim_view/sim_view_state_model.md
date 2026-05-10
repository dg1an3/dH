# CSimView State Model

`CSimView` is a `CView` subclass in the VSIM_OGL CT-simulator prototype (~2001), rendering the patient's structures, beam geometry, and treatment-machine geometry in an OpenGL "Room's Eye View" inside the document/view frame. The DCG translation reproduces every observable transition driven by the `BEGIN_MESSAGE_MAP` block plus the externally-callable `SetCurrentBeam` method and the `OnBeamChanged` observer event. This is a plain MFC SDI view — no ATL COM plumbing, no DoModal cascades, no background threads — so the state machine is a single browse tier with two sub-states (unloaded / active).

## 1. Primary State Machine

**13 event terminals across 2 states.** The diagram below captures the lifecycle (init → initial_update → active) and the cluster of in-state transitions corresponding to the four ON_COMMAND view-toggles, the observer, and the lifecycle/timer/export events.

![CSimView Primary State Machine](diagrams/stm_primary.svg)

> Source: [`diagrams/stm_primary.puml`](diagrams/stm_primary.puml)

The control has only the **browse** tier. There is no `scan` tier (no `AfxBeginThread` or `std::thread`), no `modal` tier (no `DoModal` calls), no `confirm` tier (no Yes/No `MessageBox`). `initial_update(none)` is a self-loop — the framework's first activation can deliver a NULL document, in which case `OnInitialUpdate` returns early at `simview.cpp:317-320` and leaves the state at `view_unloaded`. The `Closed` terminal is a placeholder for the inherited `WM_DESTROY`; it is not dispatched by this control's own message map.

## 2. State Dict Schema

### 2.1 Schema diagram

The state dict has tag `state` and a single root composing several auxiliary records. The mauve **Display Rows** package mirrors `m_arrSurfaceRenderables` items (one record per `CSurfaceRenderable`); the green **UI Cascade** package mirrors the `CCmdUI` shape `OnUpdate*` would write into the Win32 menu/toolbar.

![CSimView state{} dict schema](diagrams/bdd_state_dict.svg)

> Source: [`diagrams/bdd_state_dict.puml`](diagrams/bdd_state_dict.puml)

**Key insight:** `m_pBeamRenderable` (state field `beam_renderable_pointer`) is **never assigned** anywhere in `simview.cpp`. The constructor at `simview.cpp:70` initializes it to `NULL`; no `OnInitialUpdate` block, no helper, and no public method ever writes it. `OnViewBeam` at `simview.cpp:222` dereferences this pointer unconditionally — a NULL-pointer crash in production. The LTS preserves the field verbatim as `none` and models the *intended* toggle of `beam_renderable_enabled` so reachable states reflect the operational expectation, while marking the hazard explicitly in `edit_ops.pl::toggle_view_beam`.

### 2.2 Truncated record details (grep-able fallback)

The `state{}` root holds 5 fields the BDD truncated for legibility. They split into **stale-preserved** (initialized in the C++ constructor but never read or written outside it) and **child-window-config** (set once in `OnCreate`):

| Field | Type | Notes |
|---|---|---|
| `patient_enabled` | `bool` | ⚠ Stale: initialized to TRUE at `simview.cpp:72`, never read or written. Preserved verbatim per the cpp-state-model fidelity rule. |
| `slider_pos` | `double` | ⚠ Stale: declared at `simview.h:103`, never accessed. |
| `rev_camera` | `camera{...}` \| `none` | Set in `OnCreate` (`simview.cpp:189-194`) — direction, distance, clipping planes, viewing angle. Read by the renderer only; not mutated by any `step//2` rule after `init`. |
| `left_tracker` | `atom` \| `none` | Set to `rotate_tracker` in `OnCreate` (`simview.cpp:198`). Boundary handle. |
| `middle_tracker` | `atom` \| `none` | Set to `zoom_tracker` in `OnCreate` (`simview.cpp:199`). Boundary handle. |

### 2.3 State{} write authority

| Field | Written By |
|---|---|
| `win` | `init`, `initial_update` |
| `current_beam` | `set_current_beam` |
| `beam_renderable_pointer` | (never written — see Key insight) |
| `patient_surface_renderable` | `initial_update` (one-shot) |
| `surface_renderables` | `initial_update`, `beam_changed` (via `apply_modelview`) |
| `machine_renderable` | `initial_update` (one-shot, when `GetBeamCount() > 0`) |
| `beam_renderable_enabled` | `toggle_view_beam` |
| `surface_textured` | `toggle_view_lightfield` |
| `wireframe` | `toggle_view_wireframe` |
| `colorwash` | `toggle_view_colorwash` |
| `modelview_dirty` | `init`, `initial_update`, `toggle_view_beam`, `toggle_view_lightfield` (set true); `beam_changed` (cleared) |
| `rev_window_created`, `rev_camera`, `left_tracker`, `middle_tracker`, `timer_set`, `timer_id`, `timer_period_ms` | `init` (one-shot) |
| `ui_cascade` | `derive_state` (after every state-mutating step) |
| `patient_enabled`, `slider_pos` | (never written) |

**Key insight:** The four `m_b*` toggle flags (`m_bWireFrame`, `m_bColorWash`, plus the renderable-property reads `m_pBeamRenderable->IsEnabled()` and `m_pSurfaceRenderable->GetTexture()`) are the only mutators in the entire control's runtime. Everything else is set once during `init`/`initial_update` and never touched again.

### 2.4 Enumerations

`ViewMode = view_unloaded | view_active` is the only enum. No further enumeration mappings are needed.

### 2.5 Database Surface

**No DB access.** `CSimView` does not call `CPreferDB`, `CRecordset`, ADO, or raw ODBC anywhere. The persistent state of the visualization (CT volume, structures, beam parameters, machine geometry) is owned by the `CPlan` document and read by `CSimView` via `GetDocument()` accessors only — there is no SQL surface in this control.

The `OnExportFieldcom` handler does call into a COM server (`FieldCOM.Mesh`, `FieldCOM.FileStorage`) under `#ifdef USE_ExportFieldCOM`, but those calls write to the file system via the COM server, not to a database. The default build of this code excludes that `#ifdef`, so even the file-system side effect is absent in practice.

## 3. Event → Predicate Transformation Map

| Event | Guard | Transformation Predicates | State Fields Affected | Tables Read/Written |
|---|---|---|---|---|
| `init` | `win == view_unloaded` | `edit_ops:init`, `derive_state` | `rev_window_created`, `rev_camera`, `left_tracker`, `middle_tracker`, `timer_*`, `modelview_dirty`, `ui_cascade` | (none) |
| `initial_update(Doc)` | `win == view_unloaded` | `edit_ops:initial_update`, `surface_renderable_list:load_surface_renderable_list`, `derive_state` | `win`, `surface_renderables`, `patient_surface_renderable`, `machine_renderable`, `modelview_dirty`, `ui_cascade` | (none) |
| `on_size(Cx, Cy)` | (none) | `edit_ops:on_size` | (none — boundary `MoveWindow`) | (none) |
| `erase_bkgnd` | (none) | (none — returns TRUE to suppress default erasure) | (none) | (none) |
| `view_beam` | `is_loaded` | `edit_ops:toggle_view_beam`, `derive_state` | `beam_renderable_enabled`, `modelview_dirty`, `ui_cascade` | (none) |
| `view_lightfield` | `is_loaded` ∧ `patient_surface_renderable != none` | `edit_ops:toggle_view_lightfield`, `derive_state` | `surface_textured`, `modelview_dirty`, `ui_cascade` | (none) |
| `view_wireframe` | (none) | `edit_ops:toggle_view_wireframe` | `wireframe` | (none) |
| `view_colorwash` | (none) | `edit_ops:toggle_view_colorwash` | `colorwash` | (none) |
| `timer(IDEvent)` | (none) | `edit_ops:timer_tick` (no-op due to early `return`) | (none) | (none) |
| `export_fieldcom` | (none) | `edit_ops:export_fieldcom` (state-pure) | (none) | external file via COM (when `USE_ExportFieldCOM` is defined) |
| `export_nuages` | (none) | `edit_ops:export_nuages` (state-pure; body block-commented) | (none) | (none — body disabled) |
| `set_current_beam(Beam)` | (none) | `edit_ops:set_current_beam`, `observer_boundary:remove_observer`, `observer_boundary:add_observer`, `derive_state` | `current_beam`, `ui_cascade` | (none) |
| `beam_changed` | `current_beam != none` | `edit_ops:beam_changed`, `surface_renderable_list:apply_modelview` | `surface_renderables` (per-record `modelview` field), `modelview_dirty` | (none) |

ON_UPDATE_COMMAND_UI handlers (`OnUpdateViewBeam`, `OnUpdateViewLightfield`, `OnUpdateViewWireframe`, `OnUpdateViewColorwash`) are read-only and live in `browse_ops.pl::update_view_*/2`; they do not produce events but are folded into `state.ui_cascade` by `derive_state/2` after every state-mutating step.

## 4. Data Pipeline (Load)

The load pipeline is concentrated entirely in `OnInitialUpdate` (`simview.cpp:311-424`). It reads the `CPlan` document via `GetDocument()`, iterates `pSeries->GetStructureCount()` to build per-structure renderables, and pins the patient surface (index 0) into a special pointer.

![CSimView Load Pipeline](diagrams/act_data_pipeline.svg)

> Source: [`diagrams/act_data_pipeline.puml`](diagrams/act_data_pipeline.puml)

**Phase 1 (DCG dispatch):** `step//2` matches `[initial_update(Doc)]` and calls `edit_ops:initial_update/3`. **Phase 2 (record list):** `surface_renderable_list:load_surface_renderable_list/2` produces one `surface_renderable{}` dict per `CMesh`, with `color = arrColors[i mod 13]` (`simview.cpp:30-44`) and `icon_id = arrIconResourceIDs[i mod 13]` (`simview.cpp:46-61`). Element 0 is also pinned to `state.patient_surface_renderable`. **Phase 3 (beam wiring):** for each `CBeam` in `GetDocument()->GetBeamCount()`, a `CBeamRenderable` is created via `pNewItem->CreateRenderableForObject(&m_wndREV)` (boundary call); beam 0's name is set to `"AP"` (note the verbatim `// TODO: why is this not loaded?` in source) and `SetCurrentBeam(beam[0])` is invoked, which attaches the `OnBeamChanged` observer. **Phase 4 (machine renderable):** when there is at least one beam, a `CMachineRenderable` is added with color `RGB(255,255,128)` and alpha `1.0` (the `// 0.5)` comment hints at an alpha experiment).

The load pipeline does not touch any SQL or persistent store; everything originates from the in-memory `CPlan` document.

## 5. Save Pipeline

`CSimView` has two `ON_COMMAND` export handlers, both effectively no-ops in this build:

![CSimView Save Pipeline](diagrams/act_save_pipeline.svg)

> Source: [`diagrams/act_save_pipeline.puml`](diagrams/act_save_pipeline.puml)

- **`export_fieldcom`** (`simview.cpp:458-540`) is wrapped in `#ifdef USE_ExportFieldCOM`. Under that flag it calls into a COM server (`FieldCOM.Mesh`, `FieldCOM.FileStorage`) to write `.dat` files per structure. Under the default build (line 537-540) the body is empty `void CSimView::OnExportFieldcom() { }`. The Prolog `edit_ops:export_fieldcom/2` is a state-pure no-op; the COM boundary is the durable side-effect when the flag is set.

- **`export_nuages`** (`simview.cpp:542-587`) has its entire body inside a `/* ... */` block comment. Even under any compile flag the function body is empty. The original (commented) implementation wrote `.cnt` text files per structure containing contour vertex coordinates via `fopen`/`fprintf`. The Prolog `edit_ops:export_nuages/2` is a state-pure no-op.

Neither path mutates `state{}` fields. The boundary primitives (COM and `fopen`) are not translated; the diagram marks them with explanatory `note` blocks.

## 6. Domain Constraints and Invariants

![CSimView Cross-field Invariants](diagrams/par_invariants.svg)

> Source: [`diagrams/par_invariants.puml`](diagrams/par_invariants.puml)

### 6.1 Constraints (validation-time gates)

- **`beam_changed_gate`** — `state.current_beam != none` is required for the `[beam_changed]` step rule to fire. This gate exists because `OnBeamChanged` (`simview.cpp:297`) dereferences `m_pCurrentBeam` unconditionally; a fire of the observer event before any `SetCurrentBeam` would crash the C++ control. The DCG models the precondition explicitly so unsafe sequences fail at the LTS level rather than reaching a NULL deref.

- **`is_loaded_gate`** — most ON_COMMAND view-toggle handlers assume `OnInitialUpdate` has populated the renderables. The `[view_beam]` and `[view_lightfield]` step rules guard on `is_loaded(state)` (i.e. `win == view_active`). `view_wireframe` and `view_colorwash` deliberately do NOT have this guard because their `m_b*` flags are pure booleans not tied to renderable pointers — they can fire harmlessly before initial_update.

### 6.2 Invariants (state-time, preserved by every mutating step)

- **`ui_cascade_consistency`** — `state.ui_cascade` matches the `derive_state/2` output. Re-applied at the end of every step rule that mutates `current_beam`, `beam_renderable_enabled`, `surface_textured`, `wireframe`, `colorwash`, or `patient_surface_renderable`. This is the audit form of "the menu/toolbar Enable/Check shape is always consistent with the underlying state."

- **`modelview_dirty_discipline`** — `modelview_dirty == true` indicates that `beam_changed` will recompute the per-renderable matrix on next fire. Set by `init`, `initial_update`, `toggle_view_beam`, `toggle_view_lightfield`. Cleared exclusively by `beam_changed`. The C++ source has no explicit dirty flag — the LTS makes the implicit "next OnBeamChanged will recompute" relation auditable.

- **`stale_field_preservation`** — `patient_enabled`, `slider_pos`, and `beam_renderable_pointer` are never written by any step rule. Preserves memory-layout fidelity 1:1 with the C++ instance.

## Source Mapping

Every event in Section 3 mapped back to its originating C++ source line:

| Event | C++ Source |
|---|---|
| `init` | `VSIM_OGL/simview.cpp:109` (`ON_WM_CREATE`) → `simview.cpp:176` (`CSimView::OnCreate`) |
| `initial_update(Doc)` | `VSIM_OGL/simview.cpp:311` (`CSimView::OnInitialUpdate`, virtual override; not in `BEGIN_MESSAGE_MAP`) |
| `on_size(Cx, Cy)` | `simview.cpp:110` (`ON_WM_SIZE`) → `simview.cpp:209` (`CSimView::OnSize`) |
| `erase_bkgnd` | `simview.cpp:111` (`ON_WM_ERASEBKGND`) → `simview.cpp:263` (`CSimView::OnEraseBkgnd`) |
| `view_beam` | `simview.cpp:112` (`ON_COMMAND(ID_VIEW_BEAM, OnViewBeam)`) → `simview.cpp:220` |
| `view_lightfield` | `simview.cpp:114` (`ON_COMMAND(ID_VIEW_LIGHTPATCH, OnViewLightfield)`) → `simview.cpp:234` |
| `view_wireframe` | `simview.cpp:116` (`ON_COMMAND(ID_VIEW_WIREFRAME, OnViewWireframe)`) → `simview.cpp:269` |
| `view_colorwash` | `simview.cpp:118` (`ON_COMMAND(ID_VIEW_COLORWASH, OnViewColorwash)`) → `simview.cpp:283` |
| `timer(IDEvent)` | `simview.cpp:120` (`ON_WM_TIMER`) → `simview.cpp:428` (`OnTimer`; body has early `return` at line 432) |
| `export_fieldcom` | `simview.cpp:121` (`ON_COMMAND(ID_EXPORT_FIELDCOM, OnExportFieldcom)`) → `simview.cpp:458` (under `#ifdef USE_ExportFieldCOM`) or `simview.cpp:537` (empty default body) |
| `export_nuages` | `simview.cpp:122` (`ON_COMMAND(ID_EXPORT_NUAGES, OnExportNuages)`) → `simview.cpp:542` (entire body inside `/* ... */`) |
| `set_current_beam(Beam)` | `simview.cpp:87` (`CSimView::SetCurrentBeam`, public method declared in `simview.h:34`) |
| `beam_changed` | `simview.cpp:293` (`CSimView::OnBeamChanged`, observer callback registered via `::AddObserver` at `simview.cpp:102`) |

ON_UPDATE_COMMAND_UI handlers (lines 113, 115, 117, 119) are paired read-only counterparts; their cross-references are documented in `browse_ops.pl::update_view_*/2`.

### Cross-language references

No counterpart deliverables exist yet for VSIM_OGL or any of its sister classes. The dH/Brimstone repository's `DEVELOPMENT_TIMELINE.md` Part 7 (Direction C) frames the historical visualization lineage — `VSIM_OGL`, `VSIM_MODEL`, `PenBeamEdit`, `RT_VIEW`, `GEOM_VIEW`, `OGL_BASE` — as the natural cohort for a batch retrospective LTS pass. When that work proceeds, this `sim_view` deliverable becomes the reference for cross-checking against (1) the modern `Brimstone/CPlanarView` (the post-2008 successor that absorbed the slice-rendering responsibilities) and (2) any future C# WinForms or Streamlit reimplementation that targets the same visualization workflow.

The bisimilarity claim worth pursuing first: `CSimView` (2001) and `CPlanarView` (2008+) share the same load-from-`CPlan`-document → render-renderables → react-to-beam-change pattern, with the difference that `CPlanarView` is 2-D slice-based rather than 3-D OpenGL. The structural equivalence of their event alphabets (lifecycle, view toggles, observer events) is the candidate for formal pairing.
