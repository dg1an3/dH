%% Source: PenBeamEdit/PenBeamEditView.cpp

:- module(browse_ops, [
    view_state/1,
    is_created/1,
    is_plan_bound/1,
    is_created_or_bound/1
]).

view_state(pre_create).
view_state(created).
view_state(plan_bound).
view_state(closed).

is_created(State) :- State.win == created.
is_plan_bound(State) :- State.win == plan_bound.
is_created_or_bound(State) :- State.win == created.
is_created_or_bound(State) :- State.win == plan_bound.
