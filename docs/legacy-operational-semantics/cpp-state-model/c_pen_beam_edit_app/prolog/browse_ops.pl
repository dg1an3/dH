%% Source: PenBeamEdit/PenBeamEdit.cpp
%%
%% Read-only operations for CPenBeamEditApp.

:- module(browse_ops, [
    app_state/1,
    is_initialized/1,
    is_modal/1,
    is_importing/1,
    is_terminal/1
]).

app_state(pre_init).
app_state(initialized).
app_state(showing_about).
app_state(importing).
app_state(exited).

is_initialized(State) :- State.win == initialized.
is_modal(State)       :- State.win == showing_about.
is_importing(State)   :- State.win == importing.
is_terminal(State)    :- State.win == exited.
