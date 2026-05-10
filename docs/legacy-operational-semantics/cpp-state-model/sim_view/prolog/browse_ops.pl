%% Source: VSIM_OGL/simview.h, VSIM_OGL/simview.cpp
%%
%% Read-only operations: state classifiers and ON_UPDATE_COMMAND_UI handlers.
%% Mirrors the OnUpdate* methods in simview.cpp which compute a CCmdUI's
%% Enable/Check shape from current state without mutating the state.

:- module(browse_ops, [
    view_state/1,
    is_loaded/1,
    update_view_beam/2,
    update_view_lightfield/2,
    update_view_wireframe/2,
    update_view_colorwash/2
]).

%% State-tier classifier. CSimView has only one tier (browse), with two
%% sub-states distinguished by whether OnInitialUpdate has populated the
%% renderables.
view_state(view_unloaded).
view_state(view_active).

%% C++: simview.cpp:317-320  if (GetDocument() == NULL) return;
%% Plan loaded iff initial_update completed and series was non-null.
is_loaded(State) :- State.win == view_active.

%% C++: simview.cpp:225-232  void CSimView::OnUpdateViewBeam(CCmdUI* pCmdUI)
%%      pCmdUI->Enable(m_pBeamRenderable != NULL);
%%      if (m_pBeamRenderable != NULL)
%%          pCmdUI->SetCheck(m_pBeamRenderable->IsEnabled() ? 1 : 0);
update_view_beam(State, ui{enable: Enable, check: Check}) :-
    ( State.beam_renderable_pointer == none
    -> Enable = false, Check = 0
    ;  Enable = true,  ( State.beam_renderable_enabled == true -> Check = 1 ; Check = 0 )
    ).

%% C++: simview.cpp:254-261  void CSimView::OnUpdateViewLightfield(CCmdUI* pCmdUI)
%%      pCmdUI->Enable(m_pSurfaceRenderable != NULL);
%%      if (m_pSurfaceRenderable != NULL)
%%          pCmdUI->SetCheck(m_pSurfaceRenderable->GetTexture() ? 1 : 0);
update_view_lightfield(State, ui{enable: Enable, check: Check}) :-
    ( State.patient_surface_renderable == none
    -> Enable = false, Check = 0
    ;  Enable = true,  ( State.surface_textured == true -> Check = 1 ; Check = 0 )
    ).

%% C++: simview.cpp:274-281  void CSimView::OnUpdateViewWireframe(CCmdUI* pCmdUI)
%%      pCmdUI->Enable(m_pSurfaceRenderable != NULL);
%%      if (m_pSurfaceRenderable != NULL)
%%          pCmdUI->SetCheck(m_bWireFrame ? 1 : 0);
update_view_wireframe(State, ui{enable: Enable, check: Check}) :-
    ( State.patient_surface_renderable == none
    -> Enable = false, Check = 0
    ;  Enable = true,  ( State.wireframe == true -> Check = 1 ; Check = 0 )
    ).

%% C++: simview.cpp:288-291  void CSimView::OnUpdateViewColorwash(CCmdUI* pCmdUI)
%%      pCmdUI->SetCheck(m_bColorWash ? 1 : 0);
%% NOTE: This handler does NOT call pCmdUI->Enable(...) -- the menu item is
%% enabled at all times. Preserved verbatim.
update_view_colorwash(State, ui{enable: true, check: Check}) :-
    ( State.colorwash == true -> Check = 1 ; Check = 0 ).
