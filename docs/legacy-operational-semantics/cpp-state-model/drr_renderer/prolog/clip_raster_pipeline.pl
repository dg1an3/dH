%% Source: VSIM_OGL/DRRRenderer.cpp
%%
%% Pure model of the ClipRaster() 16.16 fixed-point clipping algorithm
%% (DRRRenderer.cpp:88-238). For each axis nD in {0,1,2}, ClipRaster
%% adjusts the raster start point and the destination length so the
%% ray fragment stays within the [nSourceMinD, nSourceMaxD-1] range
%% along that axis. Returns false if the ray does not intersect.
%%
%% The algorithm has four cases per axis:
%%   case A: start below min, step <= 0          -> no intersection
%%   case B: start below min, step >  0          -> advance start, shrink length
%%   case C: start above max, step >= 0          -> no intersection
%%   case D: start above max, step <  0          -> advance start, shrink length
%% then re-checks the END for two more cases on the length adjustment.

:- module(clip_raster_pipeline, [
    clip_raster/7,                  % +Axis, +SrcMin, +SrcMax, +StartIn, +Step, +DestLenIn, -StartOut/-DestLenOut/-Ok
    clip_all_axes/8                 % +VolDims (W,H,D), +StartIn, +Step, +DestLenIn, -StartOut, -DestLenOut, -Ok
]).

%% Vector accessors (fixed-point 16.16 means: physical = raw >> 16).
fx_to_int(Fx, I) :- I is Fx >> 16.

%% C++: DRRRenderer.cpp:88-238  bool ClipRaster(...)
%% Single-axis clipper. StartIn/StartOut and Step are 3-vectors of int (16.16).
%% DestLenIn/DestLenOut are integer step counts.
clip_raster(Axis, SrcMin, SrcMax, StartIn, Step, DestLenIn, [StartOut, DestLenOut, Ok]) :-
    nth0(Axis, StartIn, S0Fx),
    nth0(Axis, Step, StepFx),
    fx_to_int(S0Fx, S0Int),
    %% Phase 1: adjust the raster start.
    ( S0Int < SrcMin
    ->  ( StepFx =< 0
        -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
        ;  Delta is ((SrcMin << 16) - S0Fx) // StepFx + 1,
           DestLen1 is DestLenIn - Delta,
           ( DestLen1 < 0
           -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
           ;  advance(StartIn, Step, Delta, StartMid),
              clip_raster_end(Axis, SrcMin, SrcMax, StartMid, Step, DestLen1, [StartOut, DestLenOut, Ok]))
        )
    ; S0Int > SrcMax - 1
    ->  ( StepFx >= 0
        -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
        ;  Delta is (((SrcMax - 1) << 16) - S0Fx) // StepFx + 1,
           DestLen1 is DestLenIn - Delta,
           ( DestLen1 < 0
           -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
           ;  advance(StartIn, Step, Delta, StartMid),
              clip_raster_end(Axis, SrcMin, SrcMax, StartMid, Step, DestLen1, [StartOut, DestLenOut, Ok]))
        )
    ;   clip_raster_end(Axis, SrcMin, SrcMax, StartIn, Step, DestLenIn, [StartOut, DestLenOut, Ok])
    ).

%% C++: DRRRenderer.cpp:174-229  Phase 2: adjust the raster length.
clip_raster_end(Axis, SrcMin, SrcMax, StartIn, Step, DestLenIn, [StartOut, DestLenOut, Ok]) :-
    nth0(Axis, StartIn, S0Fx),
    nth0(Axis, Step, StepFx),
    EndFx is S0Fx + DestLenIn * StepFx,
    fx_to_int(EndFx, EndInt),
    ( EndInt > SrcMax - 1
    ->  ( StepFx =< 0
        -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
        ;  DestLen1 is (((SrcMax - 1) << 16) - S0Fx) // StepFx,
           ( DestLen1 < 0
           -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
           ;  Ok = true, StartOut = StartIn, DestLenOut = DestLen1)
        )
    ; EndInt < SrcMin
    ->  ( StepFx >= 0
        -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
        ;  DestLen1 is ((SrcMin << 16) - S0Fx) // StepFx,
           ( DestLen1 < 0
           -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
           ;  Ok = true, StartOut = StartIn, DestLenOut = DestLen1)
        )
    ;   Ok = true, StartOut = StartIn, DestLenOut = DestLenIn
    ).

%% advance(In, Step, K, Out): Out[i] = In[i] + K * Step[i].
advance([X, Y, Z], [Sx, Sy, Sz], K, [Xo, Yo, Zo]) :-
    Xo is X + K * Sx,
    Yo is Y + K * Sy,
    Zo is Z + K * Sz.

%% C++: DRRRenderer.cpp:385-387  if (ClipRaster(... 0, ...) && ClipRaster(... 1, ...) && ClipRaster(... 2, ...))
%%      All three axes must succeed for the ray to contribute a pixel.
clip_all_axes(W, H, D, StartIn, Step, DestLenIn, [StartOut, DestLenOut], Ok) :-
    clip_raster(0, 0, W, StartIn, Step, DestLenIn, [S1, L1, Ok1]),
    ( Ok1 == false
    -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
    ;  clip_raster(1, 0, H, S1, Step, L1, [S2, L2, Ok2]),
       ( Ok2 == false
       -> Ok = false, StartOut = StartIn, DestLenOut = DestLenIn
       ;  clip_raster(2, 0, D, S2, Step, L2, [S3, L3, Ok3]),
          Ok = Ok3, StartOut = S3, DestLenOut = L3
       )
    ).
