%% Source: GEOM_VIEW/include/RotateTracker.h, GEOM_VIEW/RotateTracker.cpp
%%
%% LTS for CRotateTracker (CTracker subclass). Glue-medium. Overrides
%% OnButtonDown to capture a starting point and OnMouseDrag to rotate
%% the view's camera around the camera target by the displacement.
%%
%% NOTABLE BUG preserved verbatim: the rotation matrix is declared
%% but never initialized:
%%   RotateTracker.cpp:89  CMatrixD<3> mRotate; // = CreateRotate(...);
%% The CreateRotate call is commented out, so mRotate is default-
%% constructed. Whether the rotation works at all depends on
%% CMatrixD<3>::CMatrixD() -- if it initializes to identity, the
%% rotation is a no-op (camera direction unchanged on every drag tick).
%% If it leaves memory uninitialized, vNewDir at cpp:92 is garbage.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   pre_construct  -- ctor not yet
%%   ready          -- attached to SceneView; no drag in progress
%%   dragging       -- between OnButtonDown and OnButtonUp (sub-state of ready)
%%
%% Note: OnButtonUp is NOT overridden in this subclass -- the base's
%% empty body handles it. So "dragging" doesn't have a clean exit
%% event; in practice the drag ends when no more OnMouseDrag fires.
%% Modeled as a sub-state with implicit termination.

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   construct(View)                -- RotateTracker.cpp:33
%%   on_button_down(Flags, Point)   -- RotateTracker.cpp:53  override
%%   on_mouse_drag(Flags, Point)    -- RotateTracker.cpp:74  override
%%   destruct                       -- RotateTracker.cpp:44  empty body

initial_state(state{
    win:               pre_construct,
    pview:             none,
    %% captured at OnButtonDown, updated at OnMouseDrag
    v_prev_point:      vec3(0.0, 0.0, 0.0)
}).

%% C++: RotateTracker.cpp:33-36  ctor: CTracker(pView).
step(S0, S1) --> [construct(View)],
    { S0.win == pre_construct,
      S1 = S0.put([win - ready, pview - View]) }.

%% C++: RotateTracker.cpp:53-66  OnButtonDown
%%      AssertValid camera (DEBUG only)
%%      m_vPrevPoint = m_pView->ModelPtFromWndPt(point) - camera.GetTarget()
step(S0, S1) --> [on_button_down(_Flags, Point)],
    { (S0.win == ready ; S0.win == dragging),
      S1 = S0.put([
          win - dragging,
          v_prev_point - model_pt_minus_target(Point)
      ]) }.

%% C++: RotateTracker.cpp:74-103  OnMouseDrag
%%      vCurrPoint = ModelPtFromWndPt(point) - target
%%      mRotate = (uninitialized CMatrixD<3>)
%%      vNewDir = mRotate * camera.GetDirection()
%%      camera.SetDirection(vNewDir)            <-- effect depends on
%%                                                  CMatrixD default ctor!
%%      m_vPrevPoint = new value
%%      RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW)
step(S0, S1) --> [on_mouse_drag(_Flags, Point)],
    { S0.win == dragging,
      S1 = S0.put([
          v_prev_point - model_pt_minus_target(Point)
          %% NOTE: in a working implementation we'd also encode the
          %% camera.SetDirection side-effect, but because mRotate is
          %% uninitialized in source, that side-effect is undefined.
          %% Preserved verbatim.
      ]) }.

step(S0, S1) --> [destruct],
    { (S0.win == ready ; S0.win == dragging),
      S1 = S0.put(win, destroyed) }.

valid_next(State, Event) :-
    member(Event, [
        construct(_), on_button_down(_, _), on_mouse_drag(_, _), destruct
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ============================================================================
%% Source quirks preserved verbatim
%% ============================================================================
%%
%%   1. RotateTracker.cpp:89  CMatrixD<3> mRotate; (// = CreateRotate(...));
%%      The rotation matrix is declared but the CreateRotate
%%      initialization is commented out. mRotate is default-
%%      constructed -- behavior depends on CMatrixD<3>::CMatrixD().
%%      If identity-init, every drag tick multiplies camera.GetDirection
%%      by identity -> no rotation. If zero-init, camera direction
%%      becomes zero vector -> camera breaks. Preserved verbatim.
%%
%%   2. NO override of OnButtonUp -- inherits the base's empty body.
%%      The drag-end is therefore not explicitly modeled; "dragging"
%%      sub-state implicitly ends when OnMouseDrag stops being called.
%%
%%   3. NO override of OnMouseMove (only OnMouseDrag).
%%
%%   4. Each OnMouseDrag tick redraws via RedrawWindow with
%%      RDW_INVALIDATE | RDW_UPDATENOW -- synchronous redraw, not
%%      observer-based. Causes per-tick framebuffer thrashing during
%%      a continuous drag.
