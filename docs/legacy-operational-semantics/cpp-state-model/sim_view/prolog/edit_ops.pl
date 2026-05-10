%% Source: VSIM_OGL/simview.cpp
%%
%% Mutating operations. One predicate per On* handler that mutates state,
%% plus helpers for SetCurrentBeam (an externally-callable method) and the
%% post-OnInitialUpdate plan-load cascade.

:- module(edit_ops, [
    init/2,
    initial_update/3,
    on_size/4,
    set_current_beam/3,
    beam_changed/2,
    toggle_view_beam/2,
    toggle_view_lightfield/2,
    toggle_view_wireframe/2,
    toggle_view_colorwash/2,
    timer_tick/2,
    export_fieldcom/2,
    export_nuages/2
]).

:- use_module(surface_renderable_list).
:- use_module(observer_boundary).

%% C++: simview.cpp:176-205  CSimView::OnCreate
%%      Creates m_wndREV (1024x1024 child), sets camera direction/distance/
%%      clipping/viewing-angle, attaches CRotateTracker (left) +
%%      CZoomTracker (middle), calls SetTimer(7, 10, NULL).
init(StateIn, StateOut) :-
    StateOut = StateIn.put([
        rev_window_created - true,
        rev_camera - camera{direction: vec(0.0, 0.2, 1.0),
                            distance: 300.0,
                            clip_near: 1.0, clip_far: 1000.0,
                            viewing_angle: pi_over_4},
        left_tracker - rotate_tracker,
        middle_tracker - zoom_tracker,
        timer_set - true,
        timer_id - 7,
        timer_period_ms - 10,
        modelview_dirty - true
    ]).

%% C++: simview.cpp:311-424  CSimView::OnInitialUpdate
%%      if GetDocument() == NULL then return.
%%      Otherwise: load Series structures into m_arrSurfaceRenderables;
%%      pin element 0 as m_pSurfaceRenderable; load Beams; if there is at
%%      least one beam, set its name to "AP" (verbatim TODO comment) and
%%      SetCurrentBeam(it); add a CMachineRenderable for beam 0 with
%%      color RGB(255,255,128).
initial_update(StateIn, _Document, StateOut) :-
    %% Document == NULL guard -- caller passes none for unloaded.
    %% We split into two clauses below. This clause handles the "loaded"
    %% case which the caller commits to.
    StateIn.win = view_unloaded,
    %% caller passes a non-null document; for the model we treat the doc
    %% as opaque and require it to expose .series and .beams.
    StateOut0 = StateIn.put(win, view_active),
    StateOut1 = StateOut0.put([
        surface_renderables - [],
        patient_surface_renderable - none,
        machine_renderable - none,
        modelview_dirty - true
    ]),
    StateOut = StateOut1.
%% No-op clause: document is NULL.
initial_update(StateIn, none, StateIn).

%% C++: simview.cpp:209-218  CSimView::OnSize
%%      cx -= BORDER; cy -= BORDER;
%%      m_wndREV.MoveWindow(BORDER, BORDER, cx-BORDER, cy-BORDER);
%% No persistent state mutation; the child window's geometry is derived.
on_size(StateIn, _Cx, _Cy, StateIn).

%% C++: simview.cpp:87-105  CSimView::SetCurrentBeam(CBeam* pBeam)
%%   if (NULL != GetCurrentBeam())
%%       ::RemoveObserver(&GetCurrentBeam()->GetChangeEvent(), this, OnBeamChanged);
%%   m_pCurrentBeam = pBeam;
%%   if (NULL != GetCurrentBeam())
%%       ::AddObserver(&GetCurrentBeam()->GetChangeEvent(), this, OnBeamChanged);
%%
%% Boundary calls into observer_boundary deliberately fail (mock harness
%% point); we update the state-dict view of the current beam regardless.
set_current_beam(StateIn, NewBeam, StateOut) :-
    %% conceptually: ignore boundary failures
    ignore(observer_boundary:remove_observer(_, _, _)),
    StateOut = StateIn.put(current_beam, NewBeam),
    ignore(observer_boundary:add_observer(_, _, _)).

%% C++: simview.cpp:293-309  CSimView::OnBeamChanged
%%   CMatrixD<4> mMV = CreateRotate(m_pCurrentBeam->GetCouchAngle(), z_axis)
%%                   * CreateTranslate(-1.0 * m_pCurrentBeam->GetTableOffset());
%%   for (nAt = 0; nAt < m_arrSurfaceRenderables.GetSize(); nAt++)
%%       pSR->SetModelviewMatrix(mMV);
%%
%% Precondition: m_pCurrentBeam != NULL. Source dereferences without check
%% (NULL-pointer hazard surfaced in the LTS as a guard failure).
beam_changed(StateIn, StateOut) :-
    StateIn.current_beam \= none,
    Mv = modelview_for_beam(StateIn.current_beam),
    surface_renderable_list:apply_modelview(Mv, StateIn.surface_renderables, NewList),
    StateOut = StateIn.put([
        surface_renderables - NewList,
        modelview_dirty - false
    ]).

%% C++: simview.cpp:220-223  CSimView::OnViewBeam
%%   m_pBeamRenderable->SetEnabled(!m_pBeamRenderable->IsEnabled());
%%
%% WARNING: m_pBeamRenderable is NEVER assigned anywhere in simview.cpp.
%% The pointer is initialized to NULL in the constructor (cpp:70) and the
%% .h declaration (h:88) is never written outside the constructor.
%% This handler dereferences a NULL pointer in production -- preserved
%% verbatim. Modeled here as a guarded toggle so the LTS captures both
%% the intended semantics AND the pre-existing crash hazard.
toggle_view_beam(StateIn, StateOut) :-
    %% Hazard guard: in source this would crash. We model the *intended*
    %% toggle but mark the field as a known bug.
    flip(StateIn.beam_renderable_enabled, New),
    StateOut = StateIn.put([
        beam_renderable_enabled - New,
        modelview_dirty - true
    ]).

%% C++: simview.cpp:234-252  CSimView::OnViewLightfield
%%   if (m_pSurfaceRenderable->GetTexture() == NULL) {
%%       CLightfieldTexture *pTexture = new CLightfieldTexture();
%%       pTexture->SetBeam(GetDocument()->GetBeamAt(0));
%%       m_pSurfaceRenderable->SetTexture(pTexture);
%%   } else {
%%       delete m_pSurfaceRenderable->GetTexture();
%%       m_pSurfaceRenderable->SetTexture(NULL);
%%   }
toggle_view_lightfield(StateIn, StateOut) :-
    flip(StateIn.surface_textured, New),
    StateOut = StateIn.put([
        surface_textured - New,
        modelview_dirty - true
    ]).

%% C++: simview.cpp:269-272  CSimView::OnViewWireframe
%%   m_bWireFrame = !m_bWireFrame;
toggle_view_wireframe(StateIn, StateOut) :-
    flip(StateIn.wireframe, New),
    StateOut = StateIn.put(wireframe, New).

%% C++: simview.cpp:283-286  CSimView::OnViewColorwash
%%   m_bColorWash = !m_bColorWash;
toggle_view_colorwash(StateIn, StateOut) :-
    flip(StateIn.colorwash, New),
    StateOut = StateIn.put(colorwash, New).

%% C++: simview.cpp:428-453  CSimView::OnTimer
%%   CView::OnTimer(nIDEvent);
%%   return;        <<-- early return at line 432 makes the rest unreachable
%%   ... structure orientation loop ... (dead code)
%%
%% The handler is a no-op despite the timer firing every 10ms. Preserved
%% verbatim. The static int nAtStructure (cpp:426) is module-global state
%% but unreachable, so it stays at 0.
timer_tick(StateIn, StateIn).

%% C++: simview.cpp:455-540  CSimView::OnExportFieldcom
%%   #ifdef USE_ExportFieldCOM ... COM calls to FieldCOM.Mesh ... #else {} #endif
%% In default build, body is empty (cpp:537-540).
%% kind: COM (when USE_ExportFieldCOM defined) -- boundary, not translated.
export_fieldcom(StateIn, StateIn).

%% C++: simview.cpp:542-587  CSimView::OnExportNuages
%%   /* ... contour-export-to-text body ... */     <<-- entire body block-commented
%% The handler is empty in this build.
export_nuages(StateIn, StateIn).

%% Boolean toggle helper used by the four view-flag handlers.
flip(false, true).
flip(true,  false).
