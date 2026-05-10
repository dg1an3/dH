%% Source: RT_VIEW/BeamRenderable.cpp

:- module(browse_ops, [
    beam_state/1,
    is_bound/1,
    is_unbound/1,
    is_cached/1,
    is_bound_or_cached/1
]).

beam_state(unbound).
beam_state(bound).
beam_state(mesh_cached).
beam_state(destroyed).

is_bound(State) :- State.win == bound.
is_unbound(State) :- State.win == unbound.
is_cached(State) :- State.win == mesh_cached.
is_bound_or_cached(State) :- State.win == bound ; State.win == mesh_cached.
