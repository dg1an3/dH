# CPlan Model Summary

`CPlan` is the second `CDocument` in the VSIM_OGL prototype — it composes a `CSeries` reference (the CT volume + structures) with a list of `CBeam`s and a precomputed dose matrix. Glue-light deliverable per `DEVELOPMENT_TIMELINE.md` Part 7: schema + mutator alphabet, no `step//2` LTS.

## Record schema

![CPlan record schema](diagrams/bdd_record.svg)

> Source: [`diagrams/bdd_record.puml`](diagrams/bdd_record.puml)

| Field | Type | C++ source | Writers | Readers |
|---|---|---|---|---|
| `series_ref` | `CSeries*` (nullable) | [`Plan.h:85`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L85) | `set_series`, `on_open_document` | `serialize_save` |
| `beams` | `CObArray of CBeam*` | [`Plan.h:88`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L88) | `add_beam`, `serialize_load`, `on_new_document` | `serialize_save` |
| `dose_valid` | `BOOL` | [`Plan.h:91`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L91) | `set_dose_valid`, `serialize_load` | `serialize_save` |
| `dose_matrix` | `CVolume<double>` | [`Plan.h:92`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L92) | `serialize_load`, `dose_clear_on_invalid` | `serialize_save` |
| `series_filename` | `CString` | [`Plan.h:99`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L99) | `serialize_load` | `on_open_document` |
| `series_doc_template` (static) | `CDocTemplate*` | [`Plan.h:96`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L96) | `set_series_doc_template` | `on_open_document` |

**Not observable** — `CPlan` is a `CDocument`, not a `CModelObject`. Instead of an explicit observable change event, [`AddBeam`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L64) calls `UpdateAllViews(NULL)` directly to refresh attached views. That MFC framework method is a boundary primitive; it is named in the schema as `triggers_view_update: [add_beam]` rather than modeled.

## Mutator alphabet

| Event | C++ source | Effect |
|---|---|---|
| `set_series(S)` | [`Plan.cpp:49`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L49) | sets `series_ref` |
| `add_beam(B)` | [`Plan.cpp:64`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L64) | appends to `beams`; calls `UpdateAllViews(NULL)` |
| `set_dose_valid(V)` | [`Plan.h:37`](../../../../VSIM_OGL/VSIM_MODEL/Plan.h#L37) (inline) | sets `dose_valid` |
| `on_new_document` | [`Plan.cpp:156`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L156) | clears `beams`, sets `dose_valid=false` |
| `on_open_document(Path)` | [`Plan.cpp:170`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L170) | hydrates `series_ref` via the static doc template + `series_filename` |
| `serialize_load` | [`Plan.cpp:88-110`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L88) | hydrates `series_filename`, `beams`, `dose_valid`, `dose_matrix` |
| `serialize_save` | [`Plan.cpp:104-107`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L104) | side-effect: if `dose_valid == false`, dose matrix is shrunk to 0×0×0 in-memory |
| `set_series_doc_template(T)` | [`Plan.cpp:122`](../../../../VSIM_OGL/VSIM_MODEL/Plan.cpp#L122) | static; no per-instance state change |

**Note on `serialize_save`:** unlike most `serialize_save` events which are read-only, `CPlan`'s save handler **mutates the in-memory dose matrix** when the dose is invalid (it calls `m_dose.SetDimensions(0, 0, 0)`). That's an unusual side-effect — preserved verbatim because it's load-bearing for memory footprint. A reviewer driving `serialize_save` against an invalid plan should observe the dose-matrix shrinkage.

## Source mapping

| Prolog construct | C++ source |
|---|---|
| `c_plan_record:c_plan_schema/1` | `Plan.h:18-100` |
| `c_plan_record:c_plan_initial/1` | `Plan.cpp:29-33` (default ctor) |
| `c_plan_record:apply_mutator(_, add_beam(_), _)` | `Plan.cpp:64-72` |
| `c_plan_record:apply_mutator(_, on_new_document, _)` | `Plan.cpp:156-168` |
| `c_plan_record:apply_mutator(_, on_open_document(_), _)` | `Plan.cpp:170-213` |
| `c_plan_record:apply_mutator(_, serialize_load, _)` | `Plan.cpp:88-110` |
| `c_plan_record:apply_mutator(_, serialize_save, _)` | `Plan.cpp:83-87, 104-117` (incl. dose-clear side-effect) |

## Cross-language references

The modern descendant **`dH::Plan` in [`RtModel/include/Plan.h`](../../../../RtModel/include/Plan.h)** drops `CDocument` entirely (it's an `itk::DataObject`). The mutator alphabet diverges in three structured ways:

| `CPlan` mutator | `dH::Plan` counterpart | Classification |
|---|---|---|
| `add_beam(B)` | `dH::Plan::AddBeam(Beam*)` | bisimilar |
| `set_dose_valid(V)` | (absent — implicit in pyramid state) | divergent — intentional |
| `on_open_document(Path)` | `dH::PlanXmlReader::ReadFile(Path)` | tau-related (XML vs binary serialization) |
| `serialize_load` / `serialize_save` | `dH::PlanXmlWriter::WriteFile` / `ReadFile` | tau-related |
| `serialize_save` dose-shrinkage side-effect | (absent) | divergent — intentional |
| `set_series_doc_template` (static) | (absent — direct ownership) | divergent — intentional |
| `UpdateAllViews(NULL)` boundary | `itk::DataObject::Modified()` chain | tau-related |

The most interesting bisimilarity question: the implicit `dose_valid` flag in `dH::Plan`. The 2008+ rewrite tracks dose validity as part of the multi-scale pyramid state (per `DEVELOPMENT_TIMELINE.md` Part 4) — invalidation is propagated through pyramid level transitions rather than via an explicit `BOOL`. A formal pairing has to treat the `dose_valid` field as one half of a tau-step pair with the pyramid's level-state.
