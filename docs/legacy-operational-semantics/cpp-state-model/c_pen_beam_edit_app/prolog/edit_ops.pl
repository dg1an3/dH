%% Source: PenBeamEdit/PenBeamEdit.cpp
%%
%% Mutating operations for CPenBeamEditApp.

:- module(edit_ops, [
    init_instance/2
]).

%% C++: PenBeamEdit.cpp:91-155  CPenBeamEditApp::InitInstance
%%      Sequence (simpler than CVSIM_OGLApp -- no CoInitialize, no
%%      explorer-window beam-pointer sync):
%%        1. AfxEnableControlContainer
%%        2. Enable3dControls / Enable3dControlsStatic
%%        3. SetRegistryKey
%%        4. LoadStdProfileSettings
%%        5. Construct primary CSingleDocTemplate (CPlan + CMainFrame +
%%           CPenBeamEditView). Note: #if OLD branch with CPenBeamEditDoc
%%           is dead; the #else branch is what's compiled.
%%        6. AddDocTemplate
%%        7. Construct series doc template, bind via CPlan::SetSeriesDocTemplate
%%        8. ParseCommandLine + ProcessShellCommand
%%        9. ShowWindow + UpdateWindow
init_instance(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - initialized,
        registry_key_set - true,
        profile_loaded - true,
        plan_doc_template - registered,
        series_doc_template - bound_to_cplan,
        cmd_line_processed - true,
        main_frame_visible - true
    ]).
