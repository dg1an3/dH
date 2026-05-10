%% Source: RT_VIEW/LightfieldTexture.cpp

:- module(edit_ops, [
    set_beam/3,
    on_beam_changed/2
]).

%% C++: LightfieldTexture.cpp:44-65  SetBeam
%%      RemoveObserver from old beam (if any), assign m_pBeam, AddObserver
%%      to new beam (if any), call OnBeamChanged for initial render.
set_beam(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        pbeam - none,
        win - unbound,
        observer_attached - false,
        texture_rendered - false
    ]).
set_beam(StateIn, Beam, StateOut) :-
    %% Boundary: RemoveObserver+AddObserver (kind: in-process callback registration).
    S1 = StateIn.put([
        pbeam - Beam,
        win - bound,
        observer_attached - true
    ]),
    %% Force initial render via the explicit OnBeamChanged call at cpp:64.
    on_beam_changed(S1, StateOut).

%% C++: LightfieldTexture.cpp:68-157  OnBeamChanged
%%      Renders: SetWidthHeight(512,512); collimator rectangle; blocks
%%      (loop body commented out); central axis crosshair; border;
%%      computes projection matrix combining texture-size scaling +
%%      machine projection + inverse beam-to-patient xform.
on_beam_changed(StateIn, StateOut) :-
    ( StateIn.pbeam == none
    -> StateOut = StateIn          %% early return at cpp:70-73
    ;  StateOut = StateIn.put([
           texture_width - 512,
           texture_height - 512,
           texture_rendered - true,
           field_rect_drawn - true,
           blocks_drawn_count - 0,    %% loop runs but body is commented out
           central_axis_drawn - true,
           border_drawn - true,
           projection_set - true
       ])
    ).
