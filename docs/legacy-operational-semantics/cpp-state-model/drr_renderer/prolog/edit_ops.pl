%% Source: VSIM_OGL/DRRRenderer.cpp, VSIM_OGL/DRRRenderer.h
%%
%% Mutating operations. One predicate per externally-callable method on
%% CDRRRenderer that mutates state; the heavy work lives in
%% compute_drr_pipeline.pl (the ray-trace) and clip_raster_pipeline.pl.

:- module(edit_ops, [
    init/3,
    set_volume/3,
    set_volume_transform/3,
    compute_drr/2,
    draw_scene/2,
    on_render_scene/2,
    on_camera_projection_change/2,
    destruct/2
]).

:- use_module(compute_drr_pipeline).

%% C++: DRRRenderer.cpp:59-69  CDRRRenderer::CDRRRenderer(COpenGLView *pView)
%%      : COpenGLRenderer(pView), forVolume(NULL), m_bRecomputeDRR(TRUE),
%%        m_nSteps(RAY_TRACE_RESOLUTION), m_nShift(RAY_TRACE_RES_LOG2),
%%        m_nResDiv(4), m_pThread(NULL)
%%      m_pView->camera.projection.AddObserver(this, OnChange);
init(StateIn, View, StateOut) :-
    StateOut = StateIn.put([
        win - unbound,
        view - View,
        for_volume - none,
        volume_transform - identity,
        recompute - true,
        n_steps - 128,                  %% RAY_TRACE_RESOLUTION
        n_shift - 7,                    %% RAY_TRACE_RES_LOG2
        n_res_div - 4,
        thread - none,
        thread_running - false,
        pixels - [],
        n_max - 0,
        n_min - 0,
        over_max - false,
        observer_attached_to_camera_projection - true
    ]).

%% C++: DRRRenderer.cpp:34  CAssociation< CVolume< short > > forVolume;
%%      The CAssociation pattern allows the volume to be set via assignment
%%      operator; we model this as an explicit set_volume event.
set_volume(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        for_volume - none,
        win - unbound,
        recompute - true,
        pixels - []
    ]).
set_volume(StateIn, Volume, StateOut) :-
    StateOut = StateIn.put([
        for_volume - Volume,
        win - dirty,
        recompute - true,
        pixels - []
    ]).

%% C++: DRRRenderer.cpp:36  CValue< CMatrix<4> > volumeTransform;
%%      Set via volumeTransform.Set(M); changes invalidate the DRR.
set_volume_transform(StateIn, Matrix, StateOut) :-
    StateOut = StateIn.put([
        volume_transform - Matrix,
        recompute - true
    ]).

%% C++: DRRRenderer.cpp:242-458  void CDRRRenderer::ComputeDRR()
%%      Snapshot matrices+viewport, project corners, march rays, post-process.
%%      Sets m_bRecomputeDRR = FALSE on exit (cpp:457).
compute_drr(StateIn, StateOut) :-
    StateIn.for_volume \= none,
    %% State-level: just transition to `computed`. The actual ray-trace
    %% pipeline (project corners, unproject anchors, ray-march, post-
    %% process) lives in compute_drr_pipeline.pl and is invoked
    %% separately when stepwise trace output is wanted.
    StateOut = StateIn.put([
        win - computed,
        recompute - false
    ]).

%% C++: DRRRenderer.cpp:460-542  void CDRRRenderer::DrawScene()
%%      if (!isEnabled.Get()) return;
%%      if (forVolume != NULL) {
%%          if (m_pThread != NULL) suspend+delete;
%%          if (m_bRecomputeDRR) ComputeDRR();
%%          glDrawPixels(...);
%%          if (m_nResDiv > 1) start hi-res bg thread (currently commented out);
%%      }
%% Returns updated state without doing the GL boundary calls.
draw_scene(StateIn, StateOut) :-
    ( StateIn.for_volume == none
    -> StateOut = StateIn          %% short-circuits at cpp:466
    ;  abort_thread_if_running(StateIn, S1),
       ( S1.recompute == true
       -> compute_drr(S1, S2)
       ;  S2 = S1
       ),
       %% glDrawPixels is the boundary; state-pure here.
       %% bg-thread spawn at cpp:539 is COMMENTED OUT in source -- preserved
       %% verbatim. We leave thread_running = false.
       StateOut = S2
    ).

%% C++: DRRRenderer.cpp:544-546  void CDRRRenderer::OnRenderScene() { }
%% Empty body. State-pure no-op preserved verbatim.
on_render_scene(StateIn, StateIn).

%% C++: DRRRenderer.cpp:548-562  void CDRRRenderer::OnChange(...)
%%      if (pSource == &m_pView->camera.projection) {
%%          if (m_pThread != NULL) suspend+delete;
%%          m_bRecomputeDRR = TRUE;
%%      }
%%      COpenGLRenderer::OnChange(pSource, pOldValue);
on_camera_projection_change(StateIn, StateOut) :-
    abort_thread_if_running(StateIn, S1),
    StateOut = S1.put([
        recompute - true,
        win - dirty
    ]).

%% C++: DRRRenderer.cpp:71-79  destructor
destruct(StateIn, StateOut) :-
    abort_thread_if_running(StateIn, StateOut).

%% Helper -----------------------------------------------------------------

%% C++: DRRRenderer.cpp:468-473, 552-557  if (m_pThread != NULL) {
%%                                            m_pThread->SuspendThread();
%%                                            delete m_pThread;
%%                                            m_pThread = NULL; }
%% Repeated three times in the source -- factor into one predicate.
abort_thread_if_running(StateIn, StateOut) :-
    ( StateIn.thread_running == true
    -> compute_drr_pipeline:bg_thread_aborted(StateIn, suspend_and_delete, StateOut)
    ;  StateOut = StateIn
    ).
