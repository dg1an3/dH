%% Source: PenBeamEdit/PenBeamEdit.h, PenBeamEdit/PenBeamEdit.cpp
%%
%% LTS for CPenBeamEditApp (CWinApp subclass). Glue-medium.
%% Compared to CVSIM_OGLApp, the lifecycle is similar but the heavy
%% transition is OnFileImport, which has its own pipeline module.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).
:- use_module(import_pipeline).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   pre_init        -- ctor ran; InitInstance not yet
%%   initialized     -- main frame visible
%%   showing_about   -- CAboutDlg.DoModal()
%%   importing       -- OnFileImport pipeline running (driven by import_pipeline.pl)
%%   exited          -- terminal

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   init_instance              -- PenBeamEdit.cpp:91   InitInstance virtual
%%   on_app_about               -- PenBeamEdit.cpp:64   ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
%%   about_close                -- modal close from CAboutDlg.DoModal
%%   on_file_import             -- PenBeamEdit.cpp:65   ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
%%   import_phase_complete      -- pipeline tick driven by import_pipeline:import_phase/3
%%   import_done                -- pipeline finished (phase == complete)
%%   import_cancelled           -- file dialog returned non-IDOK (cpp:219-220)
%%   on_file_new                -- PenBeamEdit.cpp:68   forwarded to CWinApp::OnFileNew
%%   on_file_open               -- PenBeamEdit.cpp:69   forwarded to CWinApp::OnFileOpen
%%   on_file_print_setup        -- PenBeamEdit.cpp:71   forwarded to CWinApp::OnFilePrintSetup

%% ============================================================================
%% Initial state
%% ============================================================================
initial_state(state{
    win:                       pre_init,
    plan_doc_template:         none,
    series_doc_template:       none,             %% PenBeamEdit.cpp:135
    main_frame_visible:        false,
    cmd_line_processed:        false,
    registry_key_set:          false,
    profile_loaded:            false,
    %% Import pipeline anchor (when win == importing)
    import:                    none,
    %% Stale fields from the constructor; the ctor is empty so there's
    %% nothing else to preserve.
    aux_fields_init:           true
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: PenBeamEdit.cpp:91-155  InitInstance
step(S0, S1) --> [init_instance],
    { S0.win == pre_init,
      edit_ops:init_instance(S0, S1) }.

%% C++: PenBeamEdit.cpp:64,205-209  ON_COMMAND(ID_APP_ABOUT) -> OnAppAbout -> DoModal
step(S0, S1) --> [on_app_about],
    { browse_ops:is_initialized(S0),
      S1 = S0.put(win, showing_about) }.

step(S0, S1) --> [about_close],
    { S0.win == showing_about,
      S1 = S0.put(win, initialized) }.

%% C++: PenBeamEdit.cpp:65,215-325  ON_COMMAND(ID_FILE_IMPORT) -> OnFileImport
%% Enters the importing state; the import_pipeline drives subsequent
%% phases until phase == complete or the file dialog cancels.
step(S0, S1) --> [on_file_import],
    { browse_ops:is_initialized(S0),
      import_pipeline:import_init(IS),
      S1 = S0.put([
          win - importing,
          import - IS
      ]) }.

%% Pipeline tick: advance one phase via import_pipeline:import_phase/3.
step(S0, S1) --> [import_phase_complete],
    { S0.win == importing,
      import_pipeline:import_phase(S0.import.phase, S0.import, NewImport),
      S1 = S0.put(import, NewImport) }.

%% File dialog returned non-IDOK (cpp:219-220).
step(S0, S1) --> [import_cancelled],
    { S0.win == importing,
      S0.import.phase == file_dialog,
      S1 = S0.put([
          win - initialized,
          import - none
      ]) }.

%% Pipeline reached `complete` phase; transition back to initialized.
step(S0, S1) --> [import_done],
    { S0.win == importing,
      S0.import.phase == complete,
      S1 = S0.put([
          win - initialized,
          import - none
      ]) }.

%% C++: PenBeamEdit.cpp:68  ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
step(S0, S0) --> [on_file_new],
    { browse_ops:is_initialized(S0) }.

%% C++: PenBeamEdit.cpp:69  ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
%% Note: forwarded directly, NOT overridden (unlike CVSIM_OGLApp).
step(S0, S0) --> [on_file_open],
    { browse_ops:is_initialized(S0) }.

%% C++: PenBeamEdit.cpp:71  ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
step(S0, S0) --> [on_file_print_setup],
    { browse_ops:is_initialized(S0) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        init_instance, on_app_about, about_close,
        on_file_import, import_phase_complete, import_cancelled, import_done,
        on_file_new, on_file_open, on_file_print_setup
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ============================================================================
%% Source quirks preserved verbatim (full inventory in import_pipeline.pl)
%% ============================================================================
%%
%%   1. PenBeamEdit.cpp:117-141  #if OLD ... #else ... #endif. The OLD
%%      branch references CPenBeamEditDoc which doesn't exist anymore;
%%      the #else branch (CPlan + CPenBeamEditView) is what's compiled.
%%      The dead OLD branch is preserved.
%%
%%   2. PenBeamEdit.cpp:232  pPlan->SetPathName("ImportedPlan.pln")
%%      followed by cpp:235  pPlan->SetPathName("IMPORT.PLN") -- second
%%      call wins. The "ImportedPlan.pln" is dead.
%%
%%   3. PenBeamEdit.cpp:265  region init at "nAtX > 45 && nAtX < 55"
%%      (a 9-voxel vertical strip), NOT "all ones" as the comment claims.
%%      Comment lies; preserved verbatim.
%%
%%   4. PenBeamEdit.cpp:253  // pStructure->m_pRegion = pRegion;
%%      Commented out; the association is via GetRegion()-returned ptr.
%%
%%   5. PenBeamEdit.cpp:286  for (int nAt = 1; nAt < 100; nAt++) starts
%%      at nAt=1, NOT 0. Reads dose1.dat .. dose99.dat -- 99 beams not
%%      100, despite the variable hint "100".
%%
%%   6. PenBeamEdit.cpp:314  outer loop reuses nAtY from cpp:261's
%%      outer loop without redeclaration. Pre-C++03 MSVC for-loop scope
%%      leak. Preserved verbatim.
%%
%%   7. PenBeamEdit.cpp:298-299  pencil-beam gaussian weight uses
%%      sqrt(2*PI*sigma) instead of sqrt(2*PI*sigma^2). Off-by-a-factor;
%%      compensated by the maxDose normalization. Preserved verbatim.
