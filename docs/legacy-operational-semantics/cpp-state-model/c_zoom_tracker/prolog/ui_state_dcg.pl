%% Source: GEOM_VIEW/include/ZoomTracker.h, GEOM_VIEW/ZoomTracker.cpp
%%
%% LTS for CZoomTracker (CTracker subclass). Glue-medium. Overrides
%% OnButtonDown to capture initial zoom + Y position, OnMouseDrag to
%% adjust the camera distance based on Y-displacement.

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
%%   ready          -- attached to SceneView
%%   dragging       -- between OnButtonDown and (implicit) drag-end

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   construct(View)               -- ZoomTracker.cpp:33
%%   on_button_down(Flags, Point)  -- ZoomTracker.cpp:54
%%   on_mouse_drag(Flags, Point)   -- ZoomTracker.cpp:78
%%   destruct                       -- ZoomTracker.cpp:44

initial_state(state{
    win:               pre_construct,
    pview:             none,
    %% captured at OnButtonDown
    init_zoom:         0.0,           %% m_initZoom = camera.GetDistance()
    init_y:            0.0            %% m_initY = point.y / rect.Height
}).

%% C++: ZoomTracker.cpp:33-36
step(S0, S1) --> [construct(View)],
    { S0.win == pre_construct,
      S1 = S0.put([win - ready, pview - View]) }.

%% C++: ZoomTracker.cpp:54-70  OnButtonDown
%%      m_initZoom = camera.GetDistance()
%%      m_initY = (double)point.y / (double)rect.Height()
step(S0, S1) --> [on_button_down(_Flags, _Point)],
    { (S0.win == ready ; S0.win == dragging),
      S1 = S0.put([
          win - dragging,
          init_zoom - camera_distance,
          init_y - point_y_over_height
      ]) }.

%% C++: ZoomTracker.cpp:78-102  OnMouseDrag
%%      currY = point.y / rect.Height
%%      zoom = exp(2.0 * (m_initY - currY))         <-- m_initZoom * prefix commented out
%%      camera.SetDistance(m_initZoom / zoom)
%%      RedrawWindow(...)
step(S0, S0) --> [on_mouse_drag(_Flags, _Point)],
    { S0.win == dragging }.

step(S0, S1) --> [destruct],
    { (S0.win == ready ; S0.win == dragging),
      S1 = S0.put(win, destroyed) }.

valid_next(State, Event) :-
    member(Event, [construct(_), on_button_down(_, _), on_mouse_drag(_, _), destruct]),
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
%%   1. ZoomTracker.cpp:93  // m_initZoom * is commented out:
%%        double zoom = // m_initZoom *
%%            exp(2.0 * (m_initY - currY));
%%      So zoom = exp(2.0 * dY) instead of zoom = m_initZoom * exp(...).
%%      Without the m_initZoom multiplier, zoom is centered around 1.0
%%      regardless of starting distance, which gives "relative" zoom
%%      behavior. With m_initZoom prefix it would have been scaled
%%      to the absolute starting distance. Either way functions, but
%%      the commented-out version is the intended behavior.
%%
%%   2. cpp:98  // m_pView->GetCamera().SetZoom(zoom); commented out.
%%      The replacement at cpp:97 uses SetDistance(m_initZoom / zoom)
%%      instead -- the API renamed Zoom to Distance at some point.
%%
%%   3. NO override of OnButtonUp -- same pattern as RotateTracker.
%%
%%   4. Per-tick RedrawWindow with synchronous flush -- same per-tick
%%      framebuffer thrash issue as RotateTracker.
