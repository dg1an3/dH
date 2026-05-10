%% Source: VSIM_OGL/VSIM_OGL.cpp, VSIM_OGL/VSIM_OGL.h
%%
%% LTS for CVSIM_OGLApp (CWinApp subclass). Glue-medium target -- the
%% state surface is small but non-trivial: an MFC application has
%% pre-init / initialized / exiting / exited lifecycle states plus a
%% modal sub-state for the CAboutDlg.DoModal() cascade.
%%
%% Compared to CSimView this is a smaller deliverable because there
%% are only 7 ON_COMMAND-equivalent entry points (3 handlers + 4
%% standard CWinApp forwards) and only one modal.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   pre_init      -- CVSIM_OGLApp::CVSIM_OGLApp ran; InitInstance not yet
%%   initialized   -- InitInstance returned TRUE; main frame visible
%%   showing_about -- CAboutDlg.DoModal() in progress  (modal tier)
%%   exiting       -- ExitInstance running
%%   exited        -- ExitInstance returned

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   init_instance               -- VSIM_OGL.cpp:52 InitInstance virtual
%%   exit_instance               -- VSIM_OGL.cpp:201 ExitInstance virtual
%%   on_idle(LCount)             -- VSIM_OGL.cpp:248 OnIdle virtual
%%   on_app_about                -- VSIM_OGL.cpp:25  ON_COMMAND(ID_APP_ABOUT)
%%   about_close                 -- modal close from CAboutDlg.DoModal()
%%   on_file_open                -- VSIM_OGL.cpp:26  ON_COMMAND(ID_FILE_OPEN)
%%   on_file_save                -- VSIM_OGL.cpp:259 OnFileSave (empty body)
%%   on_file_new                 -- VSIM_OGL.cpp:29  forwarded to CWinApp::OnFileNew
%%   on_file_print_setup         -- VSIM_OGL.cpp:32  forwarded to CWinApp::OnFilePrintSetup
%%   open_document_file(Path)    -- VSIM_OGL.cpp:226 OpenDocumentFile virtual

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: VSIM_OGL.cpp:38-42  CVSIM_OGLApp::CVSIM_OGLApp() {} (empty)
%% C++: VSIM_OGL.cpp:46-47  CVSIM_OGLApp theApp;  (one global instance)
initial_state(state{
    win:                     pre_init,
    com_initialized:         false,             %% set by CoInitialize in init_instance
    registry_key_set:        false,             %% set by SetRegistryKey
    profile_loaded:          false,             %% set by LoadStdProfileSettings
    plan_doc_template:       none,              %% added in init_instance
    series_doc_template:     none,              %% the secondary template
    main_frame_visible:      false,
    cmd_line_processed:      false,
    %% boundary handles owned by base CWinApp; tracked at the LTS surface
    pdoc_manager:            none,
    %% Standard MFC framework state (CWinApp inherited)
    open_documents:          [],
    %% beam-pointer sync flag set after main-frame init
    beam_pointers_synced:    false
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: VSIM_OGL.cpp:52-141  CVSIM_OGLApp::InitInstance()
step(S0, S1) --> [init_instance],
    { S0.win == pre_init,
      edit_ops:init_instance(S0, S1) }.

%% C++: VSIM_OGL.cpp:226-246  CVSIM_OGLApp::OpenDocumentFile(LPCTSTR)
%%      Calls CWinApp::OpenDocumentFile, then on success calls
%%      pPlan->GetSeries()->CreateSphereStructure("Sphere") -- a testing
%%      scaffolding side-effect that creates a sphere structure on every
%%      plan open. Preserved verbatim.
step(S0, S1) --> [open_document_file(Path)],
    { browse_ops:is_initialized(S0),
      edit_ops:open_document_file(S0, Path, S1) }.

%% C++: VSIM_OGL.cpp:216-224  OnFileOpen
%%      Closes ALL open documents (both standard and series) before
%%      calling m_pDocManager->OnFileOpen(). Order matters: standard
%%      docs first, then series.
step(S0, S1) --> [on_file_open],
    { browse_ops:is_initialized(S0),
      edit_ops:on_file_open(S0, S1) }.

%% C++: VSIM_OGL.cpp:259-261  OnFileSave -- EMPTY BODY.
%% Preserved verbatim. ID_FILE_SAVE is in the menu but does nothing.
step(S0, S0) --> [on_file_save],
    { browse_ops:is_initialized(S0) }.

%% C++: VSIM_OGL.cpp:29  ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
%% Forwarded to base class. Boundary: framework-managed.
step(S0, S0) --> [on_file_new],
    { browse_ops:is_initialized(S0) }.

%% C++: VSIM_OGL.cpp:32  ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
step(S0, S0) --> [on_file_print_setup],
    { browse_ops:is_initialized(S0) }.

%% C++: VSIM_OGL.cpp:191-195  OnAppAbout
%%      Constructs CAboutDlg, calls DoModal(). DoModal() blocks; the
%%      LTS models entry-into-modal as a state transition.
step(S0, S1) --> [on_app_about],
    { browse_ops:is_initialized(S0),
      S1 = S0.put(win, showing_about) }.

%% C++: modal close from CAboutDlg.DoModal() returns
%%      The about box has no handlers; it just closes via OK/Cancel.
step(S0, S1) --> [about_close],
    { S0.win == showing_about,
      S1 = S0.put(win, initialized) }.

%% C++: VSIM_OGL.cpp:248-257  OnIdle(LONG lCount)
%%      Runs every 4 idle counts; gets the active CSimView.
%%      State-pure; preserved as a no-op transition.
step(S0, S0) --> [on_idle(_LCount)],
    { browse_ops:is_initialized(S0) }.

%% C++: VSIM_OGL.cpp:201-214  ExitInstance
%%      CoUninitialize, CWinApp::ExitInstance, m_pSeriesDocTemplate->
%%      CloseAllDocuments(TRUE), delete m_pSeriesDocTemplate.
step(S0, S1) --> [exit_instance],
    { browse_ops:is_initialized(S0),
      edit_ops:exit_instance(S0, S1) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        init_instance, open_document_file(_),
        on_file_open, on_file_save, on_file_new, on_file_print_setup,
        on_app_about, about_close,
        on_idle(_), exit_instance
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ============================================================================
%% Source quirks preserved verbatim
%% ============================================================================
%%
%%   1. OnFileSave at VSIM_OGL.cpp:259 has an EMPTY BODY. The save
%%      command is wired up via the menu but does nothing.
%%
%%   2. OpenDocumentFile at cpp:237 calls CreateSphereStructure("Sphere")
%%      on every successful plan open -- testing scaffolding shipped to
%%      production. Every loaded plan gets an extra sphere structure.
%%
%%   3. The DEFAULT_VIEW #ifdef block at cpp:91-97 is disabled. The
%%      original wizard-generated CVSIM_OGLDoc/CVSIM_OGLView triplet
%%      was replaced with CPlan/CSimView; the dead code is preserved.
%%
%%   4. OnIdle at cpp:248 looks up pView but discards it -- the body
%%      `return TRUE` short-circuits before doing anything with it.
%%      Looks like a left-over experiment.
%%
%%   5. The InitFieldCOMDLL call at cpp:62-67 is commented out.
%%      The CDynLinkLibrary linkage is also commented. FieldCOM
%%      integration was a dead branch.
