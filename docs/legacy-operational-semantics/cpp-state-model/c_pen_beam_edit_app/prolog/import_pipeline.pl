%% Source: PenBeamEdit/PenBeamEdit.cpp:215-325  CPenBeamEditApp::OnFileImport
%%
%% Pure-functional model of the OnFileImport pipeline -- a 6-phase
%% workflow that synthesizes a CPlan from a directory of ASCII .dat
%% files containing density + 100 pencil-beam doses. This is the
%% identity of PenBeamEdit per DEVELOPMENT_TIMELINE.md Part 3:
%% it is the "beamlet-editing testbed" that bridges VSim visualization
%% and production RtModel/Brimstone dose calculation.
%%
%% The phases mirror the procedural blocks in OnFileImport:
%%
%%   1. file_dialog -> path           cpp:218-224  CFileDialog modal
%%   2. setup_plan_paths -> plan      cpp:226-236  GetFirstDocTemplate; SetPathName twice
%%   3. create_series -> series       cpp:238-244  m_pSeriesDocTemplate->CreateNewDocument
%%   4. read_density -> volume        cpp:247-248  ReadAsciiImage(strPath + "\\density.dat", &series.volume, 1000.0)
%%   5. init_structure -> structure   cpp:251-270  CStructure with vertical-strip region
%%   6. read_pencil_beams -> dose     cpp:286-311  100-iteration loop reading dose%i.dat,
%%                                                 sets gaussian weight, sums into total dose
%%   7. normalize_dose                cpp:314-320  divide all voxels by maxDose
%%   8. update_views                  cpp:323-324  pPlan->UpdateAllViews(NULL); RedrawWindow

:- module(import_pipeline, [
    import_init/1,
    import_phase/3,
    pencil_beam_weight/3,
    is_in_initial_strip/3
]).

%% ============================================================================
%% Phase enumeration
%% ============================================================================
import_init(import_state{
    phase:           file_dialog,
    selected_path:   none,
    plan:            none,
    series:          none,
    structure:       none,
    n_pencil_beams_read: 0,
    max_dose:        0.0,
    normalized:      false,
    views_updated:   false
}).

%% C++: PenBeamEdit.cpp:218-224  CFileDialog modal entry
import_phase(file_dialog, S0, S1) :-
    S0.phase == file_dialog,
    %% Modeled as success path; failure (DoModal != IDOK) returns to caller.
    S1 = S0.put([
        phase - setup_plan_paths,
        selected_path - some_path
    ]).

%% C++: PenBeamEdit.cpp:226-236  Get current plan via doc template,
%%      then SetPathName("ImportedPlan.pln") and SetPathName("IMPORT.PLN").
%% Quirk: SetPathName called TWICE; second call wins. Preserved verbatim.
import_phase(setup_plan_paths, S0, S1) :-
    S0.phase == setup_plan_paths,
    S1 = S0.put([
        phase - create_series,
        plan - opaque_plan_with_path('IMPORT.PLN')      %% second SetPathName wins
    ]).

%% C++: PenBeamEdit.cpp:238-244  CreateNewDocument on series template,
%%      then SetPathName("IMPORT.SER"), SetModifiedFlag.
import_phase(create_series, S0, S1) :-
    S0.phase == create_series,
    S1 = S0.put([
        phase - read_density,
        series - opaque_series_with_path('IMPORT.SER')
    ]).

%% C++: PenBeamEdit.cpp:247-248  ReadAsciiImage(path + "\\density.dat",
%%                                              &pSeries->volume, 1000.0)
%% kind: file boundary (CStdioFile::ReadString line-by-line).
%% The 1000.0 weight scales density values to "density * 1000" as voxel
%% short integers.
import_phase(read_density, S0, S1) :-
    S0.phase == read_density,
    S1 = S0.put([
        phase - init_structure,
        density_loaded - true
    ]).

%% C++: PenBeamEdit.cpp:251-270  Initialize structure region.
%% Quirk: comment at cpp:260 says "initialize the region to all ones"
%% but the loop at cpp:265-269 only sets ones where (nAtX > 45 && nAtX < 55).
%% That's a 9-voxel-wide vertical strip in the X dimension, NOT all ones.
%% Comment lies. Preserved verbatim.
%%
%% Quirk: cpp:253  // pStructure->m_pRegion = pRegion;  -- commented out.
%% The pRegion is associated via GetRegion() return-by-pointer instead.
import_phase(init_structure, S0, S1) :-
    S0.phase == init_structure,
    S1 = S0.put([
        phase - read_pencil_beams,
        structure - opaque_structure_with_strip_region
    ]).

%% C++: PenBeamEdit.cpp:286-311  for (int nAt = 1; nAt < 100; nAt++)
%%      Reads dose%i.dat for nAt=1..99 (NOT 0..99; nAt starts at 1!).
%%      Computes gaussian weight = (1/sqrt(2*PI*sigma)) * exp(-(50-nAt)^2 / sigma^2)
%%      with sigma = 7.0 (cpp:276). Adds weight*pencil_dose into total dose.
import_phase(read_pencil_beams, S0, S1) :-
    S0.phase == read_pencil_beams,
    S1 = S0.put([
        phase - normalize_dose,
        n_pencil_beams_read - 99,                   %% 99 beams (1..99 inclusive)
        max_dose - some_max_dose
    ]).

%% C++: PenBeamEdit.cpp:314-320  Divide every dose voxel by maxDose.
%% Quirk: nAtY at cpp:314 is REUSED from cpp:261 outer loop without
%% redeclaration -- relies on MSVC's pre-C++ scoping that lets the
%% loop variable leak past the enclosing for-statement. Preserved
%% verbatim; assumes /Zc:forScope-.
import_phase(normalize_dose, S0, S1) :-
    S0.phase == normalize_dose,
    S1 = S0.put([
        phase - update_views,
        normalized - true
    ]).

%% C++: PenBeamEdit.cpp:323-324  pPlan->UpdateAllViews(NULL);
%%                                m_pMainWnd->RedrawWindow();
%% kind: framework boundary calls.
import_phase(update_views, S0, S1) :-
    S0.phase == update_views,
    S1 = S0.put([
        phase - complete,
        views_updated - true
    ]).

%% ============================================================================
%% Helper predicates encoding the math
%% ============================================================================

%% C++: PenBeamEdit.cpp:298-299  weight = 1/sqrt(2*PI*sigma) * exp(-(50-nAt)^2/sigma^2)
%% sigma = 7.0; nAt in [1, 99]; peak at nAt = 50.
%% NOTE: this is NOT a normalized gaussian -- the denominator under
%% sqrt should be (2 * PI * sigma^2) for a standard normal, but the
%% source uses (2 * PI * sigma). Preserved verbatim; the per-beam
%% weights are normalized again by maxDose at cpp:314-320.
pencil_beam_weight(NAt, Sigma, W) :-
    Pi is 4.0 * atan(1.0),
    Numer is (50 - NAt) * (50 - NAt),
    W is (1.0 / sqrt(2 * Pi * Sigma)) * exp(-Numer / (Sigma * Sigma)).

%% C++: PenBeamEdit.cpp:265  if (nAtX > 45 && nAtX < 55)
%% The 9-voxel-wide vertical strip used as the initial structure region.
is_in_initial_strip(_NAtY, NAtX, true) :- NAtX > 45, NAtX < 55, !.
is_in_initial_strip(_NAtY, _NAtX, false).
