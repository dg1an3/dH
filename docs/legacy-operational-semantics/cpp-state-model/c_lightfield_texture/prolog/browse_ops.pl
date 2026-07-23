%% Source: RT_VIEW/LightfieldTexture.cpp

:- module(browse_ops, [
    texture_state/1,
    is_unbound/1,
    is_bound/1
]).

texture_state(unbound).
texture_state(bound).

is_unbound(State) :- State.win == unbound.
is_bound(State) :- State.win == bound.
