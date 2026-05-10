%% Source: VSIM_OGL/MainFrm.cpp
%%
%% Read-only operations for CMainFrame.

:- module(browse_ops, [
    frame_state/1,
    is_created/1,
    is_closing/1
]).

frame_state(pre_create).
frame_state(created).
frame_state(closing).
frame_state(closed).

is_created(State) :- State.win == created.
is_closing(State) :- State.win == closing.
