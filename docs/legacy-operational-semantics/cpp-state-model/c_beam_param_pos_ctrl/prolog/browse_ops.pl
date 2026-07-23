%% Source: RT_VIEW/BeamParamPosCtrl.cpp

:- module(browse_ops, [
    dialog_state/1,
    is_ready/1,
    is_bound/1,
    is_ready_or_bound/1
]).

dialog_state(pre_init).
dialog_state(ready).
dialog_state(bound).

is_ready(State) :- State.win == ready.
is_bound(State) :- State.win == bound.
is_ready_or_bound(State) :- State.win == ready ; State.win == bound.
