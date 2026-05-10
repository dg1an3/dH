%% Source: RT_VIEW/MachineRenderable.cpp

:- module(browse_ops, [
    machine_state/1,
    is_bound/1,
    is_unbound/1,
    is_destroyed/1
]).

machine_state(unbound).
machine_state(bound).
machine_state(destroyed).

is_bound(State) :- State.win == bound.
is_unbound(State) :- State.win == unbound.
is_destroyed(State) :- State.win == destroyed.
