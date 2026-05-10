%% Source: VSIM_OGL/DRRRenderer.cpp
%%
%% Pure model of the ComputeDRR() ray-tracing pipeline (DRRRenderer.cpp:242-458).
%% Decomposed into four phases that mirror the procedural blocks in the C++:
%%
%%   1. project_volume_corners  -> [zMin, zMax]
%%      gluProject the 8 volume corners; min/max the projected z.
%%      (DRRRenderer.cpp:266-294)
%%
%%   2. compute_ray_anchors -> {viStart, viStartStepX, viStartStepY,
%%                              viEnd, viEndStepX, viEndStepY}
%%      gluUnProject (start,start,zMin), step+0.5px-grid offsets, dito for zMax.
%%      Convert to 16.16 fixed-point with +32768 sub-pixel offset.
%%      (DRRRenderer.cpp:298-349)
%%
%%   3. ray_march -> output pixel array
%%      Double-loop over (nY, nX); for each pixel:
%%        - viStep = (viEnd - viStart) >> m_nShift
%%        - run clip_all_axes; if it succeeds, accumulate voxels along the
%%          ray; else leave m_arrPixels[nPixelAt] untouched.
%%      Walks viStart by viStartStepX per X-step and viStartStepY per Y-step.
%%      (DRRRenderer.cpp:367-430)
%%
%%   4. post_process -> RGBA pixel array
%%      Window/level the accumulated values into [0,255]; pack to RGBA.
%%      (DRRRenderer.cpp:432-455 under #ifdef POST_PROCESS)

:- module(compute_drr_pipeline, [
    compute_drr_init/2,             % +RendererState, -PipelineState0
    compute_drr_tick/3,             % +PipelineStateIn, +Pixel(Y,X), -PipelineStateOut
    compute_drr_complete/2,         % +PipelineStateIn, -RendererStateOut
    bg_thread_started/2,            % +RendererStateIn, -RendererStateOut
    bg_thread_complete/2,           % +RendererStateIn, -RendererStateOut
    bg_thread_aborted/3             % +RendererStateIn, +Reason, -RendererStateOut
]).

:- use_module(clip_raster_pipeline).

%% Initial pipeline state. Captures the snapshot of viewport + matrices that
%% Phase 1-2 produced; Phase 3 advances per-tick.
compute_drr_init(R, drr_pipeline{
    width:         R.volume_width,
    height:        R.volume_height,
    depth:         R.volume_depth,
    n_steps:       R.n_steps,
    n_shift:       R.n_shift,
    image_width:   R.image_width,
    image_height:  R.image_height,
    %% phase 2 anchors -- caller passes pre-projected vectors.
    vi_start:      R.vi_start,
    vi_end:        R.vi_end,
    vi_step_x_start: R.vi_step_x_start,
    vi_step_x_end:   R.vi_step_x_end,
    vi_step_y_start: R.vi_step_y_start,
    vi_step_y_end:   R.vi_step_y_end,
    %% accumulator
    pixels:        [],
    cur_y:         0,
    cur_x:         0,
    n_max:         -2147483647,
    n_min:          2147483647,
    over_max:      false
}).

%% C++: DRRRenderer.cpp:374-426  inner X loop body
%% Tick(P, X) advances one pixel: clip + ray-march + accumulate.
compute_drr_tick(P0, pixel(Y, X), P1) :-
    %% viStep = (viEnd - viStart) >> m_nShift
    sub_vec(P0.vi_end, P0.vi_start, Diff),
    shift_right_vec(Diff, P0.n_shift, ViStep),
    %% Run the 3-axis clipper.
    clip_raster_pipeline:clip_all_axes(P0.width, P0.height, P0.depth,
                                       P0.vi_start, ViStep, P0.n_steps,
                                       [_StartClipped, LenClipped], Ok),
    ( Ok == true
    ->  %% march along the ray; here we model the OUTCOME as a
        %% non-deterministic accumulation predicate (real implementation
        %% would index the voxel grid; the LTS model leaves it abstract).
        AccumValue = ray_accumulate(P0.vi_start, ViStep, LenClipped),
        update_extrema(P0, AccumValue, NMax, NMin, OverMax),
        P1 = P0.put([
            pixels - [pixel{y: Y, x: X, value: AccumValue} | P0.pixels],
            n_max - NMax,
            n_min - NMin,
            over_max - OverMax,
            cur_y - Y,
            cur_x - X
        ])
    ;   %% No-intersection: pixel left at default. We still advance the
        %% counter so the LTS captures the "pass-through" tick.
        P1 = P0.put([cur_y - Y, cur_x - X])
    ).

%% C++: DRRRenderer.cpp:432-455  #ifdef POST_PROCESS
compute_drr_complete(P, R1) :-
    %% Window/level into [0, 255] and pack to RGBA. Modeled as a derived
    %% array of post_pixel{value: V, rgba: rgba(V,V,V,0)} entries; the real
    %% C++ writes back into m_arrPixels in place.
    NWindow is P.n_max - P.n_min,
    findall(post_pixel{y: Y, x: X, value: V8, rgba: rgba(V8, V8, V8, 0)},
            (member(pixel{y: Y, x: X, value: V}, P.pixels),
             V1 is V - P.n_min,
             V2 is V1 * 256 // NWindow,
             V8 is min(255, max(0, V2))),
            PostPixels),
    R1 = renderer{
        pixels:    PostPixels,
        n_max:     P.n_max,
        n_min:     P.n_min,
        over_max:  P.over_max,
        recompute: false
    }.

%% Background-thread coordination predicates. The actual AfxBeginThread
%% spawn is currently commented out in source (cpp:539) -- the LTS keeps
%% these for completeness so a future un-comment doesn't change the LTS
%% surface. kind: thread-spawn / thread-join boundary, not translated.

%% C++: DRRRenderer.cpp:539  m_pThread = ::AfxBeginThread(BackgroundComputeDRR, ...)
bg_thread_started(R0, R1) :-
    R1 = R0.put([thread_running, true]).

%% C++: DRRRenderer.cpp:39-53  BackgroundComputeDRR end-of-body
bg_thread_complete(R0, R1) :-
    R1 = R0.put([thread_running - false, recompute - false]).

%% C++: DRRRenderer.cpp:73-78  destructor; OnChange
%%      both call SuspendThread + delete -- we model this as an abort.
bg_thread_aborted(R0, _Reason, R1) :-
    R1 = R0.put([thread_running, false]).

%% Helpers ----------------------------------------------------------------

sub_vec([Ax,Ay,Az], [Bx,By,Bz], [Cx,Cy,Cz]) :-
    Cx is Ax - Bx, Cy is Ay - By, Cz is Az - Bz.

shift_right_vec([X,Y,Z], N, [Xo,Yo,Zo]) :-
    Xo is X >> N, Yo is Y >> N, Zo is Z >> N.

%% Updates n_max / n_min / over_max accumulators.
%% C++: DRRRenderer.cpp:410-413  if (nPixelValue > m_nSteps * (int)(nMaxVoxel) / 2) bOverMax = true;
update_extrema(P, V, NMax, NMin, OverMax) :-
    NMax is max(P.n_max, V),
    NMin is min(P.n_min, V),
    %% NOTE: real C++ checks against m_nSteps * nMaxVoxel / 2; we keep the
    %% predicate symbolic since nMaxVoxel comes from forVolume->GetMax().
    OverMax = P.over_max.
