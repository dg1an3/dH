%% Source: VSIM_OGL/VSIM_MODEL/Plan.h, VSIM_OGL/VSIM_MODEL/Plan.cpp
%%
%% Model summary for CPlan (CDocument subclass). Glue-light deliverable
%% per DEVELOPMENT_TIMELINE.md Part 7 -- schema + mutator alphabet, no
%% step//2 LTS. CPlan composes a CSeries reference and a list of CBeams,
%% with a dose-valid flag and a precomputed dose matrix.

:- module(c_plan_record, [
    c_plan_schema/1,
    c_plan_initial/1,
    c_plan_mutator_event/1,
    apply_mutator/3
]).

%% ============================================================================
%% Schema
%% ============================================================================
%%
%% C++: VSIM_MODEL/Plan.h:85  CSeries * m_pSeries
%% C++: VSIM_MODEL/Plan.h:88  CObArray m_arrBeams
%% C++: VSIM_MODEL/Plan.h:91  BOOL m_bDoseValid
%% C++: VSIM_MODEL/Plan.h:92  CVolume<double> m_dose
%% C++: VSIM_MODEL/Plan.h:99  CString m_strSeriesFilename
%% C++: VSIM_MODEL/Plan.h:96  static CDocTemplate* m_pSeriesDocTemplate
%%
%% NOT observable -- CPlan is a CDocument; mutations call UpdateAllViews(NULL)
%% explicitly (see AddBeam at Plan.cpp:69) instead of firing a change event.
c_plan_schema(c_plan_schema{
    fields: [
        field{name: series_ref,        type: 'CSeries*',          writers: [set_series, on_open_document], readers: [serialize_save]},
        field{name: beams,             type: 'CObArray of CBeam*', writers: [add_beam, serialize_load, on_new_document], readers: [serialize_save]},
        field{name: dose_valid,        type: 'BOOL',               writers: [set_dose_valid, serialize_load], readers: [serialize_save, dose_clear_check]},
        field{name: dose_matrix,       type: 'CVolume<double>',    writers: [serialize_load, dose_clear_on_invalid], readers: [serialize_save]},
        field{name: series_filename,   type: 'CString',            writers: [serialize_load], readers: [on_open_document]}
    ],
    static_fields: [
        field{name: series_doc_template,  type: 'CDocTemplate*', writers: [set_series_doc_template], readers: [on_open_document]}
    ],
    observable: false,
    inherits: 'CDocument',
    serializable: true,
    triggers_view_update: [add_beam]            %% Plan.cpp:69 calls UpdateAllViews(NULL)
}).

c_plan_initial(c_plan{
    series_ref:       none,                    %% Plan.cpp:30  m_pSeries(NULL)
    beams:            [],
    dose_valid:       false,                   %% Plan.cpp:31  m_bDoseValid(FALSE)
    dose_matrix:      empty_volume,
    series_filename:  ''
}).

%% ============================================================================
%% Mutator events
%% ============================================================================
c_plan_mutator_event(set_series(_)).            %% Plan.cpp:49
c_plan_mutator_event(add_beam(_)).              %% Plan.cpp:64
c_plan_mutator_event(set_dose_valid(_)).        %% Plan.h:37 (inline)
c_plan_mutator_event(on_new_document).          %% Plan.cpp:156
c_plan_mutator_event(on_open_document(_)).      %% Plan.cpp:170
c_plan_mutator_event(serialize_load).           %% Plan.cpp:88-110
c_plan_mutator_event(serialize_save).           %% Plan.cpp:83-87,113-117
c_plan_mutator_event(set_series_doc_template(_)). %% Plan.cpp:122 (static)

%% ============================================================================
%% Mutator semantics
%% ============================================================================

apply_mutator(R0, set_series(S), R1) :-
    R1 = R0.put(series_ref, S).

%% C++: Plan.cpp:64-72  AddBeam appends and calls UpdateAllViews(NULL).
%%      View-update side-effect is a boundary primitive; not modeled here.
apply_mutator(R0, add_beam(B), R1) :-
    append(R0.beams, [B], NewBeams),
    R1 = R0.put(beams, NewBeams).

apply_mutator(R0, set_dose_valid(V), R1) :-
    R1 = R0.put(dose_valid, V).

apply_mutator(R0, on_new_document, R1) :-
    %% Plan.cpp:156-168 -- delegates to CDocument::OnNewDocument; resets
    %% the document. The actual reset behavior depends on the framework
    %% (CDocument::DeleteContents) which is a boundary; we model the
    %% post-condition: beams cleared, dose invalid.
    R1 = R0.put([
        beams - [],
        dose_valid - false
    ]).

%% C++: Plan.cpp:170-213  OnOpenDocument
%%      Calls CDocument::OnOpenDocument first, then opens the associated
%%      CSeries via the static doc template using m_strSeriesFilename.
apply_mutator(R0, on_open_document(_Path), R1) :-
    %% After OnOpenDocument completes, series_ref is hydrated.
    R1 = R0.put(series_ref, hydrated_from_template).

apply_mutator(R0, serialize_load, R1) :-
    %% Plan.cpp:92  ar >> m_strSeriesFilename
    %% Plan.cpp:95  m_arrBeams.RemoveAll()
    %% Plan.cpp:98  m_arrBeams.Serialize(ar)
    %% Plan.cpp:101 SERIALIZE_VALUE(ar, m_bDoseValid)
    %% Plan.cpp:110 m_dose.Serialize(ar)
    R1 = R0.put([
        series_filename - loaded_from_archive,
        beams - loaded_from_archive,
        dose_valid - loaded_from_archive,
        dose_matrix - loaded_from_archive
    ]).

%% C++: Plan.cpp:104-107  if storing and !m_bDoseValid: m_dose.SetDimensions(0,0,0)
%%      Side effect during save: dose matrix is shrunk to 0x0x0 if invalid.
%%      This is observable (the post-save in-memory state has empty dose).
apply_mutator(R0, serialize_save, R1) :-
    ( R0.dose_valid == false
    -> R1 = R0.put(dose_matrix, empty_volume)
    ;  R1 = R0
    ).

apply_mutator(R0, set_series_doc_template(_T), R0).
%% Static field; doesn't mutate any per-instance record.

%% ============================================================================
%% Cross-deliverable composition note
%% ============================================================================
%%
%% Modern descendant: dH::Plan in RtModel/include/Plan.h. The 2008+ rewrite
%% drops CDocument inheritance entirely (dH::Plan is an itk::DataObject)
%% and replaces CObArray of raw CBeam* with vector<dH::Beam::Pointer>.
%%
%% Bisimilarity candidates:
%%   add_beam(B)        ~~~  dH::Plan::AddBeam(Beam*)
%%   set_dose_valid(V)  ~~~  dH::Plan -- absent; dose validity is implicit
%%                            in the multi-scale pyramid state (see Brimstone
%%                            Part 4 of DEVELOPMENT_TIMELINE.md)
%%   on_open_document   ~~~  dH::PlanXmlReader::ReadFile  (PlanXmlFile.cpp)
%%   on_new_document    ~~~  dH::Plan::Plan() default ctor + explicit clear
%%
%% Three divergences:
%%   1. CDocument::Serialize -> dH::PlanXmlReader/Writer (XML, not binary)
%%   2. m_strSeriesFilename / m_pSeriesDocTemplate -> direct CSeries
%%      pointer ownership in modern; the indirection through doc template
%%      is dropped.
%%   3. UpdateAllViews(NULL) -> ITK observer pattern
%%      (DataObject::Modified() chain).
