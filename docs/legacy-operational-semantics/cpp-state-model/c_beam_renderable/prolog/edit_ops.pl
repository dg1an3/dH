%% Source: RT_VIEW/BeamRenderable.cpp

:- module(edit_ops, [
    set_object/3,
    draw_transparent/2,
    on_beam_changed/2
]).

%% C++: BeamRenderable.cpp:154-183  SetObject
set_object(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        pbeam - none,
        win - unbound,
        observer_attached - false
    ]).
set_object(StateIn, Beam, StateOut) :-
    %% RemoveObserver from old beam, AddObserver on new beam,
    %% trigger initial OnBeamChanged at cpp:176.
    S1 = StateIn.put([
        pbeam - Beam,
        win - bound,
        observer_attached - true
    ]),
    on_beam_changed(S1, StateOut).

%% C++: BeamRenderable.cpp:438-457  DrawTransparent
%%      Live code: SetRenderState(FLAT) -> DrawBlockDivergenceSurfaces ->
%%                 SetRenderState(GOURAUD).
%%      DrawBlockDivergenceSurfaces lazy-generates the mesh on first
%%      call when m_pBlockDivSurfMesh == NULL.
draw_transparent(StateIn, StateOut) :-
    %% Lazy mesh generation: if not cached and the toggle is on,
    %% advance to mesh_cached.
    ( StateIn.block_divergence_surfaces_enabled == true,
      StateIn.block_div_surf_mesh == none
    -> StateOut = StateIn.put([
           block_div_surf_mesh - generated_from_block_zero,
           win - mesh_cached
       ])
    ;  StateOut = StateIn       %% already cached or toggle off
    ).

%% C++: BeamRenderable.cpp:249-265  OnBeamChanged
%%      pEvent == &GetBeam()->GetChangeEvent() check; recompute
%%      modelview matrix; Invalidate.
%%
%% NOTE: this does NOT regenerate the cached m_pBlockDivSurfMesh, so
%% if the beam's block 0 vertices change, the cached mesh becomes
%% stale. The cache is only refreshed on destruct or never. Quirk.
on_beam_changed(StateIn, StateOut) :-
    StateOut = StateIn.put(modelview_matrix, recomputed_from_beam_xform).
