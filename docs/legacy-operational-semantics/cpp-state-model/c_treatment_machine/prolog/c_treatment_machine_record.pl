%% Source: VSIM_OGL/VSIM_MODEL/TreatmentMachine.h, VSIM_OGL/VSIM_MODEL/TreatmentMachine.cpp
%%
%% Model summary for CTreatmentMachine. Glue-light per
%% DEVELOPMENT_TIMELINE.md Part 7. The trivial case of the cohort -- 7
%% public fields, no setters, projection matrix derived in ctor only.

:- module(c_treatment_machine_record, [
    c_treatment_machine_schema/1,
    c_treatment_machine_initial/1,
    c_treatment_machine_mutator_event/1,
    apply_mutator/3
]).

%% ============================================================================
%% Schema
%% ============================================================================
%%
%% C++: VSIM_MODEL/TreatmentMachine.h:26-28  CString manufacturer, model, serial
%% C++: VSIM_MODEL/TreatmentMachine.h:31-33  double SAD, SCD, SID
%% C++: VSIM_MODEL/TreatmentMachine.h:36     CMatrix<4> projection
%%
%% OBSERVABLE in name (inherits CModelObject) but does NOT fire any
%% change events. All seven fields are public; clients mutate directly
%% without going through accessors.
%%
%% Anti-pattern preserved verbatim: m_projection is computed in the
%% constructor from m_SCD and m_SID. If a client mutates either of those
%% public fields after construction, m_projection is NOT recomputed --
%% it goes stale. Serialize() does not save or load m_projection at all
%% (cpp:37-45 omits it from the SERIALIZE_VALUE block), so on load the
%% post-deserialization projection is the constructor's default
%% (700.0/1400.0 reciprocals), not whatever was saved.
c_treatment_machine_schema(c_treatment_machine_schema{
    fields: [
        field{name: manufacturer, type: 'CString',    writers: [direct_field_write, serialize_load], readers: [serialize_save]},
        field{name: model,        type: 'CString',    writers: [direct_field_write, serialize_load], readers: [serialize_save]},
        field{name: serial_number, type: 'CString',   writers: [direct_field_write, serialize_load], readers: [serialize_save]},
        field{name: sad,          type: 'double',     writers: [direct_field_write, serialize_load], readers: [serialize_save, ctor_projection_compute]},
        field{name: scd,          type: 'double',     writers: [direct_field_write, serialize_load], readers: [serialize_save, ctor_projection_compute]},
        field{name: sid,          type: 'double',     writers: [direct_field_write, serialize_load], readers: [serialize_save, ctor_projection_compute]},
        field{name: projection,   type: 'CMatrix<4>', writers: [ctor_only], readers: []}
    ],
    observable_inherits: 'CModelObject',
    observable_fires:    [],                  %% nothing fires GetChangeEvent
    serializable: true,
    serialize_omits: [projection],            %% bug or feature: projection not persisted
    public_field_layout: true,                 %% all fields are public; no encapsulation
    quirks: [
        projection_stale_after_field_mutation,
        projection_resets_on_load_to_default
    ]
}).

c_treatment_machine_initial(c_treatment_machine{
    manufacturer:   '',
    model:          '',
    serial_number:  '',
    sad:            700.0,                    %% TreatmentMachine.cpp:24
    scd:            300.0,                    %% TreatmentMachine.cpp:25
    sid:            1400.0,                   %% TreatmentMachine.cpp:26  (m_SAD * 2.0)
    projection:     create_projection(300.0, 1400.0)  %% TreatmentMachine.cpp:28
}).

%% ============================================================================
%% Mutator events
%% ============================================================================
c_treatment_machine_mutator_event(set_manufacturer(_)).
c_treatment_machine_mutator_event(set_model(_)).
c_treatment_machine_mutator_event(set_serial_number(_)).
c_treatment_machine_mutator_event(set_sad(_)).
c_treatment_machine_mutator_event(set_scd(_)).
c_treatment_machine_mutator_event(set_sid(_)).
c_treatment_machine_mutator_event(serialize_load).
c_treatment_machine_mutator_event(serialize_save).

%% NOTE: the "set_*" mutators are NOT actual C++ methods -- they correspond
%% to direct field writes by clients. The model normalizes them as named
%% events so cross-class queries can reference them.

%% ============================================================================
%% Mutator semantics
%% ============================================================================
apply_mutator(R0, set_manufacturer(S), R1)  :- R1 = R0.put(manufacturer, S).
apply_mutator(R0, set_model(S), R1)         :- R1 = R0.put(model, S).
apply_mutator(R0, set_serial_number(S), R1) :- R1 = R0.put(serial_number, S).

%% Quirk: setting SAD/SCD/SID does NOT update projection in source.
%% Preserved verbatim -- LTS reflects the staleness.
apply_mutator(R0, set_sad(V), R1) :- R1 = R0.put(sad, V).
apply_mutator(R0, set_scd(V), R1) :- R1 = R0.put(scd, V).
apply_mutator(R0, set_sid(V), R1) :- R1 = R0.put(sid, V).

apply_mutator(R0, serialize_load, R1) :-
    %% TreatmentMachine.cpp:37-45 -- 6 SERIALIZE_VALUE calls;
    %% projection is NOT in the list, so it stays at whatever it was.
    R1 = R0.put([
        manufacturer - loaded_from_archive,
        model - loaded_from_archive,
        serial_number - loaded_from_archive,
        sad - loaded_from_archive,
        scd - loaded_from_archive,
        sid - loaded_from_archive
        %% projection deliberately not in list -- stays stale on load!
    ]).

apply_mutator(R0, serialize_save, R0).

%% ============================================================================
%% Cross-deliverable composition note
%% ============================================================================
%%
%% Modern descendant: there is no direct dH::TreatmentMachine class. The
%% modern Brimstone uses BeamDoseCalc + EnergyDepKernel which encode the
%% machine geometry implicitly via the kernel data files (6MV_kernel.dat,
%% 15MV_kernel.dat, 2MV_kernel.dat). The historical CTreatmentMachine
%% with hardcoded SAD/SCD/SID is one of the divergence points where the
%% 2008+ rewrite did NOT preserve the abstraction.
%%
%% The two projection-stale quirks above are absent from the modern stack
%% precisely because there's no separate machine object -- the kernel
%% encodes the geometry monolithically.
