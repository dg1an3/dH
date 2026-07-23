# CSeries Model Summary

`CSeries` is a `CDocument` subclass in the VSIM_OGL CT-simulator prototype (~2001) holding a CT volume and the anatomical structures contoured on it. This is the **glue-light** form of the formal-modeling deliverable — pure record schema + mutator alphabet, no `step//2` LTS, because there are no meaningful states-and-transitions to capture (just field assignments and serialization in/out).

See `DEVELOPMENT_TIMELINE.md` Part 7 for the rationale: the full ALGT 5-file deliverable is overkill for record-only classes, so glue-light targets get this slim format. The schema is here for **composition** (other LTSs reference `CSeries` as an opaque atom but field-level queries need the schema) and for the **bisimilarity claim** against the modern descendant `dH::Series` at [RtModel/include/Series.h](../../../../RtModel/include/Series.h).

## Record schema

![CSeries record schema](diagrams/bdd_record.svg)

> Source: [`diagrams/bdd_record.puml`](diagrams/bdd_record.puml)

| Field | Type | C++ source | Writers | Readers |
|---|---|---|---|---|
| `volume_transform` | `CMatrix<4>` | [`Series.h:30`](../../../../VSIM_OGL/VSIM_MODEL/Series.h#L30) | `serialize_load` | `serialize_save` |
| `volume` | `CVolume<short>` | [`Series.h:33`](../../../../VSIM_OGL/VSIM_MODEL/Series.h#L33) | `serialize_load` | `serialize_save` |
| `structures` | `CObArray` (heterogeneous; in practice `CSurface*`) | [`Series.h:36`](../../../../VSIM_OGL/VSIM_MODEL/Series.h#L36) | `serialize_load`, `on_new_document` | `serialize_save` |

**Not observable** — `CSeries` does not derive from `CModelObject`, so it has no `GetChangeEvent()`. Mutations are silent. That's important: any LTS that wants to react to `CSeries` changes has to re-poll.

## Mutator alphabet

These are the externally-callable named events that drive `CSeries`. Other LTSs reference these names in their `step//2` rules when an event in the upstream LTS causes a `CSeries` side-effect.

| Event | C++ source | Effect |
|---|---|---|
| `on_new_document` | [`Series.cpp:111`](../../../../VSIM_OGL/VSIM_MODEL/Series.cpp#L111) | clears `structures` |
| `serialize_load` | [`Series.cpp:90`](../../../../VSIM_OGL/VSIM_MODEL/Series.cpp#L90) (the `ar.IsLoading()` branch) | hydrates all three fields from archive |
| `serialize_save` | [`Series.cpp:94`](../../../../VSIM_OGL/VSIM_MODEL/Series.cpp#L94) (the else branch) | reads-only; emits to archive |
| `add_structure(S)` | inherent (`CObArray::Add`) | appends to `structures` |
| `remove_all_structures` | [`Series.cpp:103,117`](../../../../VSIM_OGL/VSIM_MODEL/Series.cpp#L103) | clears `structures` |
| `set_volume_transform(M)` | direct field write | sets `volume_transform` |
| `set_volume(V)` | direct field write | sets `volume` |

The first three are MFC framework hooks; the last four are direct API surface. The destructor at [`Series.cpp:25-32`](../../../../VSIM_OGL/VSIM_MODEL/Series.cpp#L25) iterates `m_arrStructures` and `delete`s each — that's a destructor-time invariant, not a mutator.

## Source mapping

| Prolog construct | C++ source |
|---|---|
| `c_series_record:c_series_schema/1` | `Series.h:21-67` |
| `c_series_record:c_series_initial/1` | `Series.cpp:21-23` (default ctor) |
| `c_series_record:apply_mutator(_, on_new_document, _)` | `Series.cpp:111-120` |
| `c_series_record:apply_mutator(_, serialize_load, _)` | `Series.cpp:90-105` `ar.IsLoading()` branch |
| `c_series_record:apply_mutator(_, serialize_save, _)` | `Series.cpp:94-105` else branch (no-op on the record dict) |

## Cross-language references

The natural bisimilarity counterpart is **`dH::Series` in [`RtModel/include/Series.h`](../../../../RtModel/include/Series.h)**. The modern version replaces `CObArray` with ITK smart-pointer aggregates, splits the volume into a `Density` (CT) volume, and adds a structures list typed as `vector<dH::Structure::Pointer>`. The mutator alphabet aligns:

| `CSeries` mutator | `dH::Series` counterpart |
|---|---|
| `set_volume(V)` | `SetDensity(VolumeReal*)` |
| `add_structure(S)` | `AddStructure(Structure*)` |
| `serialize_load` / `serialize_save` | absent — modern version uses `dH::PlanXmlFile` for IO, not `CDocument::Serialize` |
| `on_new_document` | absent — `dH::Series` is not a `CDocument`, no MFC framework hook |
| `set_volume_transform(M)` | absent — modern version stores transform implicitly via ITK origin/spacing/direction |

Two events present in `CSeries` are **divergent**: serialization moved out of class (to PlanXmlFile), and the explicit `volume_transform` matrix moved into ITK's image metadata. These are *intentional* divergences (the 2008+ rewrite chose a different IO and geometry encoding), classified as `intentional` per the cross-language conventions. Bisimilarity holds modulo the τ-steps `serialize_load` / `serialize_save` ↔ PlanXmlFile load/save, and `set_volume_transform` ↔ ITK image-metadata setter.
