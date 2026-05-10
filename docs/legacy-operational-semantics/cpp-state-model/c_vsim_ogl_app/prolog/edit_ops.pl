%% Source: VSIM_OGL/VSIM_OGL.cpp
%%
%% Mutating operations for CVSIM_OGLApp.

:- module(edit_ops, [
    init_instance/2,
    exit_instance/2,
    open_document_file/3,
    on_file_open/2
]).

%% C++: VSIM_OGL.cpp:52-141  CVSIM_OGLApp::InitInstance
%%      Documented as the heaviest method in the class. Sequence:
%%        1. AfxEnableControlContainer
%%        2. CoInitialize(NULL) -- abort with FALSE on failure
%%        3. Enable3dControls or Enable3dControlsStatic
%%        4. SetRegistryKey
%%        5. LoadStdProfileSettings (MRU)
%%        6. Construct primary CSingleDocTemplate (Plan + MainFrame + SimView)
%%        7. AddDocTemplate
%%        8. Construct series doc template (NOT added globally; bound via
%%           CPlan::SetSeriesDocTemplate)
%%        9. ParseCommandLine + ProcessShellCommand
%%       10. Get main frame, ShowWindow + UpdateWindow
%%       11. Sync beam pointers into m_wndPosCtrl + m_wndCollimCtrl
%%
%% Modeled as one composite transition. Boundary primitives
%% (CoInitialize, Win32 windowing) are listed but not translated.
init_instance(StateIn, StateOut) :-
    %% Boundary: CoInitialize. Modeled as success.
    StateOut = StateIn.put([
        win - initialized,
        com_initialized - true,
        registry_key_set - true,
        profile_loaded - true,
        plan_doc_template - registered,
        series_doc_template - bound_to_cplan,
        cmd_line_processed - true,
        main_frame_visible - true,
        beam_pointers_synced - true
    ]).

%% C++: VSIM_OGL.cpp:201-214  ExitInstance
%%      CoUninitialize -> CWinApp::ExitInstance -> close all series docs
%%      -> delete the series doc template.
exit_instance(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - exited,
        com_initialized - false,
        series_doc_template - none,
        open_documents - []
    ]).

%% C++: VSIM_OGL.cpp:226-246  OpenDocumentFile(LPCTSTR)
%%      pPlan = CWinApp::OpenDocumentFile(lpszFileName)  (boundary)
%%      if (pPlan): ASSERT IsKindOf(CPlan); CreateSphereStructure("Sphere")
%%      else: AfxMessageBox; return NULL.
%%
%% Quirk: CreateSphereStructure is called on EVERY successful open --
%% testing scaffolding shipped to production. Preserved verbatim; the
%% LTS records sphere_structure_added := true on success.
open_document_file(StateIn, _Path, StateOut) :-
    %% Modeled as success path; failure path is a separate clause.
    %% Boundary returns the CPlan*; we treat it opaquely.
    NewDocs = [opened_plan | StateIn.open_documents],
    StateOut = StateIn.put([
        open_documents - NewDocs,
        sphere_structure_added - true               %% scaffolding side-effect
    ]).

%% C++: VSIM_OGL.cpp:216-224  OnFileOpen
%%      m_pDocManager->CloseAllDocuments(FALSE);   <- order: standard first
%%      m_pSeriesDocTemplate->CloseAllDocuments(FALSE);  <- then series
%%      m_pDocManager->OnFileOpen();
%%
%% Boundary: framework dialog and document closure. Modeled as a
%% post-condition: open_documents := [] (after the two CloseAllDocuments
%% boundary calls).
on_file_open(StateIn, StateOut) :-
    StateOut = StateIn.put(open_documents, []).
