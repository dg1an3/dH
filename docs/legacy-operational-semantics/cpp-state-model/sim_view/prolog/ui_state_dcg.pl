%% Source: VSIM_OGL/simview.cpp, VSIM_OGL/simview.h
%%
%% Labeled-transition system for CSimView (CView subclass, MFC SDI).
%% Single-tier (browse) state machine with two sub-states distinguished by
%% whether OnInitialUpdate has populated the renderables.
%%
%% Event terminals follow the BEGIN_MESSAGE_MAP entries in simview.cpp:107-124,
%% plus the externally-callable SetCurrentBeam method and the OnBeamChanged
%% observer event. ON_UPDATE_COMMAND_UI handlers are read-only and live in
%% browse_ops.pl rather than as step//2 rules.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    derive_state/2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).
:- use_module(surface_renderable_list).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%
%%   view_unloaded -- pre-OnInitialUpdate or document is NULL
%%   view_active   -- plan loaded, renderables populated
%%
%% No scan tier (no AfxBeginThread / std::thread).
%% No modal tier (no DoModal()).
%% No confirm tier (no MessageBox Yes/No).
%% No terminal tier (WM_DESTROY inherited from CView; not in this msg map).

%% ============================================================================
%% Event alphabet (one row per BEGIN_MESSAGE_MAP entry + lifecycle/observer)
%% ============================================================================
%%   init                     -- ON_WM_CREATE        simview.cpp:109,176
%%   initial_update(Doc)      -- virtual override    simview.cpp:311  (NOT in msg map)
%%   on_size(Cx, Cy)          -- ON_WM_SIZE          simview.cpp:110,209
%%   erase_bkgnd              -- ON_WM_ERASEBKGND    simview.cpp:111,263
%%   view_beam                -- ON_COMMAND          simview.cpp:112,220
%%   view_lightfield          -- ON_COMMAND          simview.cpp:114,234
%%   view_wireframe           -- ON_COMMAND          simview.cpp:116,269
%%   view_colorwash           -- ON_COMMAND          simview.cpp:118,283
%%   timer(IDEvent)           -- ON_WM_TIMER         simview.cpp:120,428
%%   export_fieldcom          -- ON_COMMAND          simview.cpp:121,458
%%   export_nuages            -- ON_COMMAND          simview.cpp:122,542
%%   set_current_beam(Beam)   -- public method       simview.cpp:87
%%   beam_changed             -- observer fire       simview.cpp:293
%%
%% ON_UPDATE_COMMAND_UI handlers (113,115,117,119) are read-only;
%% see browse_ops:update_view_*/2.

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: simview.cpp:68-76  CSimView::CSimView()
%%      : m_pCurrentBeam(NULL),
%%        m_pBeamRenderable(NULL),
%%        m_pSurfaceRenderable(NULL),
%%        m_bPatientEnabled(TRUE),
%%        m_bWireFrame(FALSE),
%%        m_bColorWash(FALSE)
%%
%% Plus the .h-declared but uninitialized fields:
%%   m_arrSurfaceRenderables (CObArray) -- empty array
%%   m_sliderPos (double)               -- WARNING: stale field, never read/written
initial_state(state{
    win:                          view_unloaded,

    %% --- selection / observation ---
    current_beam:                 none,

    %% --- renderable handles ---
    beam_renderable_pointer:      none,    %% WARNING: never assigned anywhere -- OnViewBeam dereferences NULL
    patient_surface_renderable:   none,    %% set in OnInitialUpdate, not afterward
    surface_renderables:          [],      %% populated in OnInitialUpdate
    machine_renderable:           none,    %% added in OnInitialUpdate iff GetBeamCount() > 0

    %% --- view-mode toggles (the four ON_COMMAND togglers) ---
    beam_renderable_enabled:      false,   %% reflects m_pBeamRenderable->IsEnabled()
    surface_textured:             false,   %% reflects m_pSurfaceRenderable->GetTexture() != NULL
    wireframe:                    false,   %% m_bWireFrame
    colorwash:                    false,   %% m_bColorWash

    %% --- stale fields preserved verbatim (memory-layout fidelity) ---
    patient_enabled:              true,    %% WARNING: stale field (m_bPatientEnabled, init TRUE, never read)
    slider_pos:                   0.0,     %% WARNING: stale field (m_sliderPos, never accessed)

    %% --- child window state ---
    rev_window_created:           false,
    rev_camera:                   none,
    left_tracker:                 none,
    middle_tracker:               none,

    %% --- timer ---
    timer_set:                    false,
    timer_id:                     0,
    timer_period_ms:              0,

    %% --- derived bookkeeping ---
    modelview_dirty:              true
}).

%% ============================================================================
%% Step rules. One per event terminal. Body delegates to browse_ops/edit_ops/
%% surface_renderable_list -- never mutates state inline.
%% ============================================================================

%% C++: simview.cpp:109,176  ON_WM_CREATE -> OnCreate
%% Lifecycle: pre-OnInitialUpdate but after the window exists.
step(S0, S1) --> [init],
    { S0.win = view_unloaded,
      edit_ops:init(S0, Sa),
      derive_state(Sa, S1) }.

%% C++: simview.cpp:311  CSimView::OnInitialUpdate (virtual override; NOT in msg map)
%% Treated as event terminal because the framework dispatches it deterministically
%% after WM_CREATE on first activation.
step(S0, S1) --> [initial_update(Doc)],
    { S0.win = view_unloaded,
      edit_ops:initial_update(S0, Doc, Sa),
      derive_state(Sa, S1) }.

%% C++: simview.cpp:110,209  ON_WM_SIZE -> OnSize
step(S0, S1) --> [on_size(Cx, Cy)],
    { edit_ops:on_size(S0, Cx, Cy, S1) }.

%% C++: simview.cpp:111,263  ON_WM_ERASEBKGND -> OnEraseBkgnd
%% Returns TRUE to suppress default erasure -- no state change.
step(S0, S0) --> [erase_bkgnd].

%% C++: simview.cpp:112,220  ON_COMMAND(ID_VIEW_BEAM) -> OnViewBeam
%% WARNING: dereferences m_pBeamRenderable, which is never assigned (see edit_ops).
step(S0, S1) --> [view_beam],
    { browse_ops:is_loaded(S0),
      edit_ops:toggle_view_beam(S0, Sa),
      derive_state(Sa, S1) }.

%% C++: simview.cpp:114,234  ON_COMMAND(ID_VIEW_LIGHTPATCH) -> OnViewLightfield
step(S0, S1) --> [view_lightfield],
    { browse_ops:is_loaded(S0),
      S0.patient_surface_renderable \= none,
      edit_ops:toggle_view_lightfield(S0, Sa),
      derive_state(Sa, S1) }.

%% C++: simview.cpp:116,269  ON_COMMAND(ID_VIEW_WIREFRAME) -> OnViewWireframe
step(S0, S1) --> [view_wireframe],
    { edit_ops:toggle_view_wireframe(S0, S1) }.

%% C++: simview.cpp:118,283  ON_COMMAND(ID_VIEW_COLORWASH) -> OnViewColorwash
step(S0, S1) --> [view_colorwash],
    { edit_ops:toggle_view_colorwash(S0, S1) }.

%% C++: simview.cpp:120,428  ON_WM_TIMER -> OnTimer
%% Body has early `return` (cpp:432) -- no-op despite firing every 10ms.
step(S0, S1) --> [timer(_IDEvent)],
    { edit_ops:timer_tick(S0, S1) }.

%% C++: simview.cpp:121,458  ON_COMMAND(ID_EXPORT_FIELDCOM) -> OnExportFieldcom
%% kind: COM boundary (when USE_ExportFieldCOM defined); empty body otherwise.
step(S0, S1) --> [export_fieldcom],
    { edit_ops:export_fieldcom(S0, S1) }.

%% C++: simview.cpp:122,542  ON_COMMAND(ID_EXPORT_NUAGES) -> OnExportNuages
%% Entire body block-commented; effectively empty.
step(S0, S1) --> [export_nuages],
    { edit_ops:export_nuages(S0, S1) }.

%% C++: simview.cpp:87  CSimView::SetCurrentBeam(CBeam* pBeam)
%% Externally-callable method; treated as event terminal.
step(S0, S1) --> [set_current_beam(Beam)],
    { edit_ops:set_current_beam(S0, Beam, Sa),
      derive_state(Sa, S1) }.

%% C++: simview.cpp:293  CSimView::OnBeamChanged(CObservableEvent*, void*)
%% Observer firing from CBeam::GetChangeEvent(). Precondition encoded:
%% m_pCurrentBeam != NULL (else dereferences NULL at cpp:297).
step(S0, S1) --> [beam_changed],
    { S0.current_beam \= none,
      edit_ops:beam_changed(S0, S1) }.

%% ============================================================================
%% derive_state/2 -- re-applies button-enable / check derivation after every
%% state change that could affect ON_UPDATE_COMMAND_UI output.
%%
%% Per the cpp-state-model audit form: every step//2 rule that mutates the
%% current_beam, surface_textured, wireframe, colorwash, beam_renderable_enabled
%% or patient_surface_renderable must end in derive_state.
%% ============================================================================
derive_state(SIn, SOut) :-
    browse_ops:update_view_beam(SIn,       UB),
    browse_ops:update_view_lightfield(SIn, UL),
    browse_ops:update_view_wireframe(SIn,  UW),
    browse_ops:update_view_colorwash(SIn,  UC),
    SOut = SIn.put(ui_cascade, ui_cascade{
        view_beam:       UB,
        view_lightfield: UL,
        view_wireframe:  UW,
        view_colorwash:  UC
    }).

%% ============================================================================
%% Sanity helpers
%% ============================================================================

%% valid_next(+State, -Event) succeeds for every event whose step//2 rule
%% would fire from the given state. Useful for exhaustive state-space walks.
valid_next(State, Event) :-
    member(Event, [
        init, initial_update(_), on_size(_,_), erase_bkgnd,
        view_beam, view_lightfield, view_wireframe, view_colorwash,
        timer(_), export_fieldcom, export_nuages,
        set_current_beam(_), beam_changed
    ]),
    phrase(step(State, _), [Event]).

%% valid_sequence(+EventList) checks that the sequence is dispatchable from
%% the initial state.
valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ATL COM machinery omitted -- not applicable; CSimView is a plain MFC
%% CView subclass with no COM_INTERFACE_ENTRY / BEGIN_PROP_MAP / SINK_MAP
%% blocks.
