%% Source: VSIM_OGL/VSIM_MODEL/Beam.h, VSIM_OGL/VSIM_MODEL/Beam.cpp
%%
%% Model summary for CBeam (CModelObject subclass -- OBSERVABLE).
%% Glue-light deliverable per DEVELOPMENT_TIMELINE.md Part 7.
%%
%% This is the most important glue-light target because the CSimView LTS
%% (sim_view/) references CBeam's GetChangeEvent() as its `beam_changed`
%% observer source. Every setter that fires GetChangeEvent().Fire() is a
%% potential cause of `beam_changed` propagating into CSimView.

:- module(c_beam_record, [
    c_beam_schema/1,
    c_beam_initial/1,
    c_beam_mutator_event/1,
    apply_mutator/3,
    fires_change_event/1                %% +Event -- which mutators fire the observer
]).

%% ============================================================================
%% Schema
%% ============================================================================
%%
%% C++: VSIM_MODEL/Beam.h:91   CTreatmentMachine m_Machine     (composed, not pointer)
%% C++: VSIM_MODEL/Beam.h:94   double m_collimAngle
%% C++: VSIM_MODEL/Beam.h:95   double m_gantryAngle
%% C++: VSIM_MODEL/Beam.h:96   double m_couchAngle
%% C++: VSIM_MODEL/Beam.h:99   CVector<3> m_vTableOffset
%% C++: VSIM_MODEL/Beam.h:102  CVector<2> m_vCollimMin
%% C++: VSIM_MODEL/Beam.h:103  CVector<2> m_vCollimMax
%% C++: VSIM_MODEL/Beam.h:107  mutable CMatrix<4> m_beamToFixedXform
%% C++: VSIM_MODEL/Beam.h:111  mutable CMatrix<4> m_beamToPatientXform
%% C++: VSIM_MODEL/Beam.h:115  BOOL m_bHasShieldingBlocks
%% C++: VSIM_MODEL/Beam.h:118  CObArray m_arrBlocks  (CPolygon*)
%% C++: VSIM_MODEL/Beam.h:121  double m_weight
%% C++: VSIM_MODEL/Beam.h:124  BOOL m_bDoseValid
%% C++: VSIM_MODEL/Beam.h:127  CVolume<double> m_dose
%%
%% OBSERVABLE -- inherits CModelObject which provides GetChangeEvent().
%% Many setters fire the change event; see fires_change_event/1 below.
c_beam_schema(c_beam_schema{
    fields: [
        field{name: machine,            type: 'CTreatmentMachine', writers: [serialize_load], readers: []},
        field{name: collim_angle,       type: 'double', writers: [set_collim_angle, set_beam_to_patient_xform, serialize_load], readers: [get_beam_to_fixed_xform]},
        field{name: gantry_angle,       type: 'double', writers: [set_gantry_angle, set_beam_to_patient_xform, serialize_load], readers: [get_beam_to_fixed_xform]},
        field{name: couch_angle,        type: 'double', writers: [set_couch_angle, set_beam_to_patient_xform, serialize_load], readers: [get_beam_to_fixed_xform]},
        field{name: table_offset,       type: 'CVector<3>', writers: [set_table_offset, serialize_load], readers: [get_beam_to_patient_xform]},
        field{name: collim_min,         type: 'CVector<2>', writers: [set_collim_min, serialize_load], readers: []},
        field{name: collim_max,         type: 'CVector<2>', writers: [set_collim_max, serialize_load], readers: []},
        field{name: beam_to_fixed_xform,   type: 'CMatrix<4>', writers: [recompute_in_getter], readers: [get_beam_to_fixed_xform]},
        field{name: beam_to_patient_xform, type: 'CMatrix<4>', writers: [recompute_in_getter, set_beam_to_patient_xform], readers: [get_beam_to_patient_xform]},
        field{name: has_shielding_blocks,  type: 'BOOL', writers: [serialize_load], readers: []},
        field{name: blocks,             type: 'CObArray of CPolygon*', writers: [add_block, serialize_load], readers: []},
        field{name: weight,             type: 'double', writers: [set_weight, serialize_load], readers: []},
        field{name: dose_valid,         type: 'BOOL', writers: [set_dose_valid, serialize_load], readers: []},
        field{name: dose_matrix,        type: 'CVolume<double>', writers: [serialize_load], readers: []}
    ],
    observable:        true,                  %% inherits CModelObject -- GetChangeEvent()
    inherits:          'CModelObject',
    serializable:      true,                  %% DECLARE_SERIAL(CBeam)
    %% Note: m_beamToFixedXform and m_beamToPatientXform are `mutable`
    %% in C++ so const-correct getters can lazily recompute them.
    has_mutable_cache: [beam_to_fixed_xform, beam_to_patient_xform]
}).

c_beam_initial(c_beam{
    machine:                  default_machine,
    collim_angle:             0.0,
    gantry_angle:             0.0,
    couch_angle:              0.0,
    table_offset:             vec3(0.0, 0.0, 0.0),
    collim_min:               vec2(0.0, 0.0),
    collim_max:               vec2(0.0, 0.0),
    beam_to_fixed_xform:      identity_4x4,
    beam_to_patient_xform:    identity_4x4,
    has_shielding_blocks:     false,
    blocks:                   [],
    weight:                   1.0,
    dose_valid:               false,
    dose_matrix:              empty_volume
}).

%% ============================================================================
%% Mutator events
%% ============================================================================
c_beam_mutator_event(set_collim_angle(_)).         %% Beam.cpp:67-71
c_beam_mutator_event(set_gantry_angle(_)).         %% Beam.cpp:78-82
c_beam_mutator_event(set_couch_angle(_)).          %% Beam.cpp:89-93
c_beam_mutator_event(set_table_offset(_)).         %% Beam.cpp:101-105
c_beam_mutator_event(set_collim_min(_)).           %% Beam.cpp:114-118
c_beam_mutator_event(set_collim_max(_)).           %% Beam.cpp:126-130
c_beam_mutator_event(set_beam_to_patient_xform(_)).%% Beam.cpp:~178-208 (decomposes matrix into angles)
c_beam_mutator_event(add_block(_)).                %% Beam.cpp:228-235
c_beam_mutator_event(set_weight(_)).               %% Beam.cpp:243-248
c_beam_mutator_event(set_dose_valid(_)).
c_beam_mutator_event(serialize_load).
c_beam_mutator_event(serialize_save).
c_beam_mutator_event(get_beam_to_fixed_xform).     %% recomputes the cache; OBSERVED via cache mutation
c_beam_mutator_event(get_beam_to_patient_xform).   %% recomputes the cache

%% ============================================================================
%% Which mutators fire the observable change event
%% ============================================================================
%% C++: every "GetChangeEvent().Fire();" line in Beam.cpp:
%%   :70 set_collim_angle  :81 set_gantry_angle  :92 set_couch_angle
%%   :104 set_table_offset :117 set_collim_min   :129 set_collim_max
%%   :207 set_beam_to_patient_xform               :232 add_block
%%   :247 set_weight
fires_change_event(set_collim_angle(_)).
fires_change_event(set_gantry_angle(_)).
fires_change_event(set_couch_angle(_)).
fires_change_event(set_table_offset(_)).
fires_change_event(set_collim_min(_)).
fires_change_event(set_collim_max(_)).
fires_change_event(set_beam_to_patient_xform(_)).
fires_change_event(add_block(_)).
fires_change_event(set_weight(_)).

%% Notably DO NOT fire change:
%%   set_dose_valid (no Fire) -- a setter without observer notification
%%   serialize_load / save  -- IO boundaries
%%   get_beam_to_*_xform  -- the cache update is silent
%%
%% The set_dose_valid case is a known anti-pattern in the source; views
%% observing dose-validity must poll. Preserved verbatim.

%% ============================================================================
%% Mutator semantics
%% ============================================================================

apply_mutator(R0, set_collim_angle(A), R1) :-
    R1 = R0.put(collim_angle, A).
apply_mutator(R0, set_gantry_angle(A), R1) :-
    R1 = R0.put(gantry_angle, A).
apply_mutator(R0, set_couch_angle(A), R1) :-
    R1 = R0.put(couch_angle, A).
apply_mutator(R0, set_table_offset(V), R1) :-
    R1 = R0.put(table_offset, V).
apply_mutator(R0, set_collim_min(V), R1) :-
    R1 = R0.put(collim_min, V).
apply_mutator(R0, set_collim_max(V), R1) :-
    R1 = R0.put(collim_max, V).

%% C++: Beam.cpp:~178-208  SetBeamToPatientXform decomposes the matrix
%% into gantry/couch/collim angles via inverse-kinematics. Side-effect
%% mutates THREE angle fields plus the cached transform.
apply_mutator(R0, set_beam_to_patient_xform(M), R1) :-
    %% Decomposition is opaque at the LTS level. We model the post-
    %% condition: the matrix is stored and the angles are re-derived
    %% (placeholder atoms; reviewer can substitute a numeric model).
    R1 = R0.put([
        beam_to_patient_xform - M,
        gantry_angle - derived_from(M),
        couch_angle  - derived_from(M),
        collim_angle - derived_from(M)
    ]).

apply_mutator(R0, add_block(B), R1) :-
    append(R0.blocks, [B], NewBlocks),
    R1 = R0.put(blocks, NewBlocks).

apply_mutator(R0, set_weight(W), R1) :-
    R1 = R0.put(weight, W).

apply_mutator(R0, set_dose_valid(V), R1) :-
    R1 = R0.put(dose_valid, V).

apply_mutator(R0, get_beam_to_fixed_xform, R1) :-
    %% Lazy recompute in the const getter; mutates the mutable cache.
    R1 = R0.put(beam_to_fixed_xform, recomputed_from_angles(R0.gantry_angle, R0.couch_angle, R0.collim_angle)).

apply_mutator(R0, get_beam_to_patient_xform, R1) :-
    %% Same pattern; depends on table_offset too.
    R1 = R0.put(beam_to_patient_xform, recomputed(R0.table_offset, R0.gantry_angle, R0.couch_angle, R0.collim_angle)).

apply_mutator(R0, serialize_load, R1) :-
    R1 = R0.put([
        collim_angle - loaded_from_archive,
        gantry_angle - loaded_from_archive,
        couch_angle - loaded_from_archive,
        table_offset - loaded_from_archive,
        collim_min - loaded_from_archive,
        collim_max - loaded_from_archive,
        weight - loaded_from_archive,
        dose_valid - loaded_from_archive,
        dose_matrix - loaded_from_archive,
        blocks - loaded_from_archive
    ]).

apply_mutator(R0, serialize_save, R0).

%% ============================================================================
%% Cross-deliverable composition note
%% ============================================================================
%%
%% Modern descendant: dH::Beam in RtModel/include/Beam.h. The 2008+ rewrite
%% is itk::DataObject-based and uses the ITK Modified() observer pattern
%% in place of CObservableObject::GetChangeEvent(). The mutator alphabet
%% aligns very closely:
%%
%%   set_gantry_angle(A)        ~~~  dH::Beam::SetGantryAngle(REAL)
%%   set_couch_angle(A)         ~~~  dH::Beam::SetCouchAngle(REAL)
%%   set_collim_angle(A)        ~~~  dH::Beam::SetCollimAngle(REAL)
%%   set_table_offset(V)        ~~~  (modeled implicitly via isocenter)
%%   set_weight(W)              ~~~  dH::Beam::SetWeight(REAL)
%%   add_block(B)               ~~~  dH::Beam -- absent (block geometry
%%                                    moved to dH::Structure as RTSTRUCT
%%                                    contour)
%%   GetChangeEvent().Fire()    ~~~  this->Modified()
%%
%% Bisimilarity holds for the angle/weight setters and their downstream
%% observer fan-out into views (CSimView -> Brimstone/CPlanarView).
%%
%% IMPORTANT for CSimView composition: every fires_change_event/1 fact
%% above is a candidate cause of CSimView's [beam_changed] step.
%% A multi-class query "what CBeam mutation triggered this CSimView
%% redraw" enumerates fires_change_event/1.
