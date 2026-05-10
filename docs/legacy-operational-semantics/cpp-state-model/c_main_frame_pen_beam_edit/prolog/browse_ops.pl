%% Source: PenBeamEdit/MainFrm.cpp

:- module(browse_ops, [
    frame_state/1,
    is_created/1
]).

frame_state(pre_create).
frame_state(created).
frame_state(closed).

is_created(State) :- State.win == created.
