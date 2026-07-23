%% Source: VSIM_OGL/DRRRenderer.cpp, VSIM_OGL/DRRRenderer.h
%%
%% Labeled-transition system for CDRRRenderer (a COpenGLRenderer subclass).
%% Unlike CSimView this control has NO BEGIN_MESSAGE_MAP -- its event
%% surface comes from public methods (set_volume, ComputeDRR, DrawScene),
%% an observer callback (OnChange from camera.projection), and a
%% background-thread completion event (BackgroundComputeDRR ending).
%%
%% State alphabet:
%%   unbound       -- forVolume is NULL; nothing to render
%%   dirty         -- volume associated; needs recompute
%%   computed      -- pixel array valid; ready to draw
%%   bg_computing  -- high-res background pass in progress
%%
%% NOTE: bg_computing is reachable only when the AfxBeginThread spawn at
%% DRRRenderer.cpp:539 is uncommented. In the source as preserved, the
%% spawn is commented out, so bg_computing is unreachable in practice.
%% The LTS includes the state for completeness so a future un-comment
%% does not change the surface.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).
:- use_module(compute_drr_pipeline).

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   init(View)                   -- ctor                  cpp:59
%%   set_volume(V)                -- CAssociation set       cpp:34, used by client
%%   set_volume_transform(M)      -- CValue.Set             cpp:36
%%   compute_drr                  -- public method         cpp:242
%%   draw_scene                   -- render path           cpp:460
%%   render_scene                 -- empty                 cpp:544
%%   camera_projection_change     -- observer fire         cpp:548 (pSource = camera.projection)
%%   bg_thread_complete           -- thread end-of-body    cpp:39-53
%%   destruct                     -- dtor                  cpp:71
%%
%% Note: NO BEGIN_MESSAGE_MAP block in DRRRenderer.h. ATL/Win32 message
%% machinery omitted -- this is a renderable participant in an OpenGL
%% scene, not a Win32 widget.

%% ============================================================================
%% Initial state
%% ============================================================================
initial_state(state{
    win:                                       unbound,
    view:                                      none,
    for_volume:                                none,
    volume_transform:                          identity,
    recompute:                                 true,
    n_steps:                                   128,        %% RAY_TRACE_RESOLUTION
    n_shift:                                   7,          %% RAY_TRACE_RES_LOG2
    n_res_div:                                 4,
    image_width:                               0,
    image_height:                              0,
    viewport:                                  [0, 0, 0, 0],
    pixels:                                    [],
    n_max:                                     0,
    n_min:                                     0,
    over_max:                                  false,
    thread:                                    none,
    thread_running:                            false,
    observer_attached_to_camera_projection:    false,

    %% --- stale / module-globals preserved verbatim ---
    %% C++ has `static bool bOverMax = false;` at cpp:240 -- module-global
    %% used as a one-shot warning latch. We preserve the field; the
    %% over_max bool above mirrors it.
    use_tandem_rays:                           false       %% #define USE_TANDEM_RAYS commented out at cpp:35
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: DRRRenderer.cpp:59  CDRRRenderer::CDRRRenderer(COpenGLView *pView)
step(S0, S1) --> [init(View)],
    { S0.win == unbound,
      edit_ops:init(S0, View, S1) }.

%% C++: DRRRenderer.cpp:34,247  forVolume = X (CAssociation assignment)
%% Externally-driven; transitions unbound -> dirty (or stays unbound on NULL).
step(S0, S1) --> [set_volume(V)],
    { edit_ops:set_volume(S0, V, S1) }.

%% C++: DRRRenderer.cpp:36,259  volumeTransform.Set(M) (CValue.Set)
step(S0, S1) --> [set_volume_transform(M)],
    { edit_ops:set_volume_transform(S0, M, S1) }.

%% C++: DRRRenderer.cpp:242  CDRRRenderer::ComputeDRR()
%% Precondition: forVolume != NULL.
step(S0, S1) --> [compute_drr],
    { S0.for_volume \= none,
      edit_ops:compute_drr(S0, S1) }.

%% C++: DRRRenderer.cpp:460  CDRRRenderer::DrawScene()
%% Conditional behavior: short-circuits if forVolume == NULL OR isEnabled
%% is false. Internal cascade may invoke compute_drr.
step(S0, S1) --> [draw_scene],
    { edit_ops:draw_scene(S0, S1) }.

%% C++: DRRRenderer.cpp:544  CDRRRenderer::OnRenderScene() {}
%% Empty body, preserved verbatim.
step(S0, S0) --> [render_scene].

%% C++: DRRRenderer.cpp:548  CDRRRenderer::OnChange(pSource, pOldValue)
%%      Source-discriminated: only the camera.projection branch mutates state.
step(S0, S1) --> [camera_projection_change],
    { edit_ops:on_camera_projection_change(S0, S1) }.

%% C++: DRRRenderer.cpp:39-53  BackgroundComputeDRR
%%      Pre: thread_running. Post: pixels valid; thread cleared.
%% Currently UNREACHABLE in source (AfxBeginThread spawn at cpp:539 is
%% commented out). Preserved for completeness.
step(S0, S1) --> [bg_thread_complete],
    { S0.thread_running == true,
      compute_drr_pipeline:bg_thread_complete(S0, S1) }.

%% C++: DRRRenderer.cpp:71  CDRRRenderer::~CDRRRenderer()
step(S0, S1) --> [destruct],
    { edit_ops:destruct(S0, S1) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================

valid_next(State, Event) :-
    member(Event, [
        init(_), set_volume(_), set_volume_transform(_),
        compute_drr, draw_scene, render_scene,
        camera_projection_change, bg_thread_complete, destruct
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ATL COM machinery omitted -- not applicable; CDRRRenderer is a plain
%% C++ class with no message map / no COM_INTERFACE_ENTRY blocks.

%% ============================================================================
%% Source quirks preserved verbatim
%% ============================================================================
%%
%%   1. AfxBeginThread spawn at cpp:539 is commented out:
%%        // m_pThread = ::AfxBeginThread(BackgroundComputeDRR, ...);
%%      So bg_computing is structurally reachable but never reached at
%%      runtime in this build. The LTS keeps the state to surface a future
%%      un-comment.
%%
%%   2. #define USE_TANDEM_RAYS commented out at cpp:35.
%%      Feature toggle that didn't survive; preserved as state field.
%%
%%   3. static bool bOverMax = false at cpp:240.
%%      Module-global one-shot warning latch; mirrored as state.over_max.
%%      The AfxMessageBox warning at cpp:494 fires on bOverMax==true.
%%
%%   4. Multiple ASSERT(nDelta == nDelta2) lines are commented out
%%      (cpp:111, 151, 195, 220) -- they were debug-time cross-checks
%%      against the original floating-point implementation. Preserved
%%      as comments only; not in the LTS.
%%
%%   5. The OnRenderScene() override has an empty body (cpp:544-546).
%%      Preserved verbatim as a state-pure no-op.
%%
%%   6. The whole file carries a "HISTORICAL FILE - Restored from commit
%%      40e1350a (June 7, 2002) - This file is NOT built" header. The
%%      LTS treats it as live source despite the comment, since the
%%      operational semantics are what matters for the bisimilarity
%%      story per DEVELOPMENT_TIMELINE.md Part 7.
