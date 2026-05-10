# CBeam Model Summary

`CBeam` is the **most important glue-light target** because it is the only one in this cohort that **inherits `CModelObject` and is observable**. The `CSimView` LTS at [`../sim_view/`](../sim_view/) has a `[beam_changed]` step rule whose firing condition is precisely "any of the `CBeam` setters that calls `GetChangeEvent().Fire()` was invoked." Without a `CBeam` model summary, multi-class composition queries like *what user action triggered this CSimView redraw* have no source-side anchor.

This deliverable adds one extra Prolog predicate beyond the standard glue-light template: `fires_change_event/1`, listing exactly which mutators propagate observer notifications. That predicate is the composition surface the upstream LTSs reference.

## Record schema

![CBeam record schema](diagrams/bdd_record.svg)

> Source: [`diagrams/bdd_record.puml`](diagrams/bdd_record.puml)

| Field | Type | C++ source | Writers |
|---|---|---|---|
| `machine` | `CTreatmentMachine` (composed, not pointer) | [`Beam.h:91`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L91) | `serialize_load` |
| `collim_angle` | `double` | [`Beam.h:94`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L94) | `set_collim_angle`, `set_beam_to_patient_xform`, `serialize_load` |
| `gantry_angle` | `double` | [`Beam.h:95`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L95) | `set_gantry_angle`, `set_beam_to_patient_xform`, `serialize_load` |
| `couch_angle` | `double` | [`Beam.h:96`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L96) | `set_couch_angle`, `set_beam_to_patient_xform`, `serialize_load` |
| `table_offset` | `CVector<3>` | [`Beam.h:99`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L99) | `set_table_offset`, `serialize_load` |
| `collim_min` | `CVector<2>` | [`Beam.h:102`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L102) | `set_collim_min`, `serialize_load` |
| `collim_max` | `CVector<2>` | [`Beam.h:103`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L103) | `set_collim_max`, `serialize_load` |
| `beam_to_fixed_xform` | `mutable CMatrix<4>` | [`Beam.h:107`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L107) | `recompute_in_getter` (lazy cache) |
| `beam_to_patient_xform` | `mutable CMatrix<4>` | [`Beam.h:111`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L111) | `recompute_in_getter`, `set_beam_to_patient_xform` |
| `has_shielding_blocks` | `BOOL` | [`Beam.h:115`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L115) | `serialize_load` |
| `blocks` | `CObArray of CPolygon*` | [`Beam.h:118`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L118) | `add_block`, `serialize_load` |
| `weight` | `double` | [`Beam.h:121`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L121) | `set_weight`, `serialize_load` |
| `dose_valid` | `BOOL` | [`Beam.h:124`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L124) | `set_dose_valid`, `serialize_load` |
| `dose_matrix` | `CVolume<double>` | [`Beam.h:127`](../../../../VSIM_OGL/VSIM_MODEL/Beam.h#L127) | `serialize_load` |

The two `mutable CMatrix<4>` fields are **lazy caches**: they are recomputed inside the `const` getters on demand. The C++ uses `mutable` to escape const-correctness. In the LTS this manifests as `get_beam_to_fixed_xform` and `get_beam_to_patient_xform` being modeled as **state-mutating events** (via `apply_mutator/3`) even though they look like read accessors at the call site.

## Mutator alphabet — fires_change_event split

The mutators split by whether they propagate the observer notification:

| Mutator | Fires `GetChangeEvent()`? | C++ source |
|---|---|---|
| `set_collim_angle(A)` | ✓ | [`Beam.cpp:67-71`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L67) |
| `set_gantry_angle(A)` | ✓ | [`Beam.cpp:78-82`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L78) |
| `set_couch_angle(A)` | ✓ | [`Beam.cpp:89-93`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L89) |
| `set_table_offset(V)` | ✓ | [`Beam.cpp:101-105`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L101) |
| `set_collim_min(V)` | ✓ | [`Beam.cpp:114-118`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L114) |
| `set_collim_max(V)` | ✓ | [`Beam.cpp:126-130`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L126) |
| `set_beam_to_patient_xform(M)` | ✓ | [`Beam.cpp:~178-208`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L178) (decomposes into 3 angles, then fires) |
| `add_block(B)` | ✓ | [`Beam.cpp:228-235`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L228) |
| `set_weight(W)` | ✓ | [`Beam.cpp:243-248`](../../../../VSIM_OGL/VSIM_MODEL/Beam.cpp#L243) |
| `set_dose_valid(V)` | ✗ | (no `Fire()` call) |
| `serialize_load` / `serialize_save` | ✗ | IO boundary |
| `get_beam_to_*_xform` (cache recompute) | ✗ | silent |

**`set_dose_valid` not firing the observer is a known anti-pattern in this source.** Views observing dose validity must poll. Preserved verbatim — a reviewer who wants the LTS to track dose-validity propagation has to add a separate event terminal in the upstream view. The modern `dH::Beam` (see below) fixes this by using ITK's universal `Modified()` chain.

**`set_beam_to_patient_xform` fires the observer once** — even though it mutates *three* angle fields plus the cached transform. So a downstream observer sees one notification regardless of how many angles changed. This is intentional batching.

## Source mapping

| Prolog construct | C++ source |
|---|---|
| `c_beam_record:c_beam_schema/1` | `Beam.h:22-128` |
| `c_beam_record:c_beam_initial/1` | `Beam.cpp` ctor (default-constructed fields) |
| `c_beam_record:apply_mutator/3` (per setter) | individual `Set*` methods, lines 67-247 |
| `c_beam_record:fires_change_event/1` | every `GetChangeEvent().Fire();` line in `Beam.cpp` |

## Cross-language references

The modern descendant is **`dH::Beam` in [`RtModel/include/Beam.h`](../../../../RtModel/include/Beam.h)**. Inherits `itk::DataObject` (not `CModelObject`); the observer pattern is replaced by ITK's universal `Modified()`/`Update()` mechanism.

| `CBeam` mutator | `dH::Beam` counterpart | Classification |
|---|---|---|
| `set_gantry_angle(A)` | `dH::Beam::SetGantryAngle(REAL)` | bisimilar |
| `set_couch_angle(A)` | `dH::Beam::SetCouchAngle(REAL)` | bisimilar |
| `set_collim_angle(A)` | `dH::Beam::SetCollimAngle(REAL)` | bisimilar |
| `set_table_offset(V)` | (modeled implicitly via isocenter on `dH::Plan`) | divergent — intentional |
| `set_weight(W)` | `dH::Beam::SetWeight(REAL)` | bisimilar |
| `add_block(B)` | (absent — block geometry moved to `dH::Structure` as RTSTRUCT contour) | divergent — intentional |
| `set_beam_to_patient_xform(M)` | (absent — derived from angles, never set directly) | divergent — intentional |
| `set_dose_valid(V)` | (modeled implicitly via pyramid level state) | divergent — intentional |
| `GetChangeEvent().Fire()` | `this->Modified()` | tau-related |

**Critical bisimilarity nuance:** modern `dH::Beam`'s `Modified()` is called by **all** field setters automatically (ITK convention), unlike the historical `CBeam` where `set_dose_valid` is the lone exception. So the modern descendant is *more uniform* in observer propagation than the historical antecedent. The bisimulation relation has to enumerate which historical setters fire and which don't.

## Why this matters for the cohort

The CSimView LTS has a guarded `[beam_changed]` step rule. The guard is `state.current_beam != none`, but the *cause* is opaque at the CSimView level. With this CBeam model summary in place, a multi-class query like:

```prolog
?- caused_by(beam_changed, Cause, c_beam),
   c_beam_record:fires_change_event(Cause).
```

enumerates the nine setters that are candidate causes, separating them from the four mutators that are silent. That is the kind of compositional query the user asked about when challenging whether glue-light LTSs are worth doing — and CBeam is the proof case where the answer is yes.
