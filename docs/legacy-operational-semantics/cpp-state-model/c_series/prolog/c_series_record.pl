%% Source: VSIM_OGL/VSIM_MODEL/Series.h, VSIM_OGL/VSIM_MODEL/Series.cpp
%%
%% Model summary for CSeries (CDocument subclass). This is a glue-light
%% target -- a pure record with no state-machine structure. Per the
%% Direction C scaling note in DEVELOPMENT_TIMELINE.md Part 7, glue-light
%% classes get a slim deliverable: schema + mutator-event list +
%% observer/serialization boundary, NOT a full step//2 LTS.
%%
%% The schema is here for two reasons:
%%   1. composition -- other LTSs (CSimView, CDRRRenderer) reference
%%      CSeries as an opaque atom, but field-level queries need the
%%      schema for semantic interpretation.
%%   2. the bisimilarity claim against the modern descendant
%%      dH::Series at RtModel/include/Series.h is *exactly* a schema
%%      + mutator alignment claim.

:- module(c_series_record, [
    c_series_schema/1,                  % -SchemaDict
    c_series_initial/1,                 % -InitialRecord
    c_series_mutator_event/1,           % -EventName
    apply_mutator/3                     % +Record, +Event, -Record'
]).

%% ============================================================================
%% Schema (mirrors the .h field declarations)
%% ============================================================================
%%
%% C++: VSIM_MODEL/Series.h:30  CMatrix<4> m_volumeTransform
%% C++: VSIM_MODEL/Series.h:33  CVolume< short > volume
%% C++: VSIM_MODEL/Series.h:36  CObArray m_arrStructures
%%
%% No m_* observable change-event in this class -- CSeries does not
%% derive from CModelObject. Mutations are silent.
c_series_schema(c_series_schema{
    fields: [
        field{name: volume_transform,  type: 'CMatrix<4>', writers: [serialize_load], readers: [serialize_save]},
        field{name: volume,            type: 'CVolume<short>', writers: [serialize_load], readers: [serialize_save]},
        field{name: structures,        type: 'CObArray', writers: [serialize_load, on_new_document], readers: [serialize_save]}
    ],
    observable: false,
    inherits: 'CDocument',
    serializable: true
}).

%% C++: VSIM_MODEL/Series.cpp:21-23  CSeries::CSeries()  (default-constructed; no field inits)
%%      Note: members are default-constructed; m_volumeTransform begins as
%%      identity (per CMatrix<4>'s default ctor); volume is empty;
%%      m_arrStructures is empty.
c_series_initial(c_series{
    volume_transform: identity_4x4,
    volume:           empty_volume,
    structures:       []
}).

%% ============================================================================
%% Mutator event list (no state machine -- just named events for composition)
%% ============================================================================
%%
%% Each event name corresponds to an externally-driven mutation. Other LTSs
%% reference these names when their step//2 rules cause CSeries side-effects.
c_series_mutator_event(on_new_document).      %% C++: Series.cpp:111
c_series_mutator_event(serialize_load).       %% C++: Series.cpp:90  ar.IsLoading() branch
c_series_mutator_event(serialize_save).       %% C++: Series.cpp:94  the else branch (read-only)
c_series_mutator_event(add_structure(_)).     %% inherent: CObArray::Add
c_series_mutator_event(remove_all_structures).%% C++: Series.cpp:103,117  m_arrStructures.RemoveAll()
c_series_mutator_event(set_volume_transform(_)). %% direct field write
c_series_mutator_event(set_volume(_)).        %% direct field write

%% ============================================================================
%% Mutator semantics (apply_mutator/3)
%% ============================================================================

%% C++: VSIM_MODEL/Series.cpp:111-120  CSeries::OnNewDocument
apply_mutator(R0, on_new_document, R1) :-
    R1 = R0.put(structures, []).

%% C++: VSIM_MODEL/Series.cpp:90-97  ar >> m_volumeTransform; volume.Serialize(ar);
apply_mutator(R0, serialize_load, R1) :-
    %% Loaded values come from the archive boundary; modeled as opaque.
    R1 = R0.put([
        volume_transform - loaded_from_archive,
        volume - loaded_from_archive,
        structures - loaded_from_archive
    ]).

%% C++: VSIM_MODEL/Series.cpp:94  ar << m_volumeTransform   (read-only)
apply_mutator(R0, serialize_save, R0).

apply_mutator(R0, add_structure(S), R1) :-
    append(R0.structures, [S], NewStructures),
    R1 = R0.put(structures, NewStructures).

apply_mutator(R0, remove_all_structures, R1) :-
    R1 = R0.put(structures, []).

apply_mutator(R0, set_volume_transform(M), R1) :-
    R1 = R0.put(volume_transform, M).

apply_mutator(R0, set_volume(V), R1) :-
    R1 = R0.put(volume, V).

%% ============================================================================
%% Cross-deliverable composition note
%% ============================================================================
%%
%% Modern descendant: dH::Series in RtModel/include/Series.h. The dH::Series
%% schema replaces CObArray with itk::SmartPointer-based aggregates and adds
%% a Density volume (the CT) plus a separate structure list. The bisimilarity
%% candidate is:
%%
%%   apply_mutator(R, set_volume(V), R')   ~~~  dH::Series::SetDensity(V)
%%   apply_mutator(R, add_structure(S),R') ~~~  dH::Series::AddStructure(S)
%%
%% Field-level: m_volumeTransform's role is taken by ITK's image origin/
%% spacing/direction matrix; m_arrStructures becomes a vector of
%% dH::Structure::Pointer. The behavioral equivalence is over the
%% mutator alphabet, not the field encoding.
