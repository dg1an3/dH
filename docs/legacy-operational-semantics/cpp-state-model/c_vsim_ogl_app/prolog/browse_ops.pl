%% Source: VSIM_OGL/VSIM_OGL.cpp
%%
%% Read-only operations for CVSIM_OGLApp.

:- module(browse_ops, [
    app_state/1,
    is_initialized/1,
    is_modal/1,
    is_terminal/1
]).

app_state(pre_init).
app_state(initialized).
app_state(showing_about).
app_state(exiting).
app_state(exited).

%% C++: post-InitInstance return TRUE state.
is_initialized(State) :- State.win == initialized.

%% C++: VSIM_OGL.cpp:194  CAboutDlg.DoModal() in progress.
is_modal(State) :- State.win == showing_about.

is_terminal(State) :- State.win == exited.
