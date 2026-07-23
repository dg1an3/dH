%% Source: GEOM_VIEW/include/Tracker.h, GEOM_VIEW/Tracker.cpp
%%
%% LTS for CTracker (abstract-ish base class for mouse-drag trackers
%% on a CSceneView). Glue-light: all five virtual methods have empty
%% bodies in the base implementation -- they are template methods
%% intended for subclasses to override.
%%
%% Concrete subclasses in this cohort: CRotateTracker, CZoomTracker.
%% Plus referenced from VSIM_OGL/CSimView::OnCreate at cpp:198-199:
%%   m_wndREV.AddLeftTracker(new CRotateTracker(&m_wndREV));
%%   m_wndREV.AddMiddleTracker(new CZoomTracker(&m_wndREV));

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   active   -- attached to a SceneView; ready to receive mouse events

%% ============================================================================
%% Event alphabet (all five base bodies are EMPTY)
%% ============================================================================
%%   construct(View)              -- Tracker.cpp:19  ctor: m_pView(pView)
%%   is_inside(Point)              -- Tracker.cpp:30  returns TRUE unconditionally
%%   on_button_down(Flags, Point) -- Tracker.cpp:35  empty body
%%   on_button_up(Flags, Point)   -- Tracker.cpp:39  empty body
%%   on_mouse_move(Flags, Point)  -- Tracker.cpp:43  empty body
%%   on_mouse_drag(Flags, Point)  -- Tracker.cpp:47  empty body
%%   destruct                      -- Tracker.cpp:25  empty body

initial_state(state{
    win:       pre_construct,
    pview:     none
}).

step(S0, S1) --> [construct(View)],
    { S0.win == pre_construct,
      S1 = S0.put([win - active, pview - View]) }.

%% is_inside is a query, not a mutator; in the base it always returns
%% TRUE. Modeled as event terminal with no state change.
step(S0, S0) --> [is_inside(_Point)],
    { S0.win == active }.

%% All four mouse handlers have empty bodies in the base; subclasses
%% override. Modeled as event terminals with no state change.
step(S0, S0) --> [on_button_down(_Flags, _Point)],
    { S0.win == active }.
step(S0, S0) --> [on_button_up(_Flags, _Point)],
    { S0.win == active }.
step(S0, S0) --> [on_mouse_move(_Flags, _Point)],
    { S0.win == active }.
step(S0, S0) --> [on_mouse_drag(_Flags, _Point)],
    { S0.win == active }.

step(S0, S1) --> [destruct],
    { S0.win == active,
      S1 = S0.put(win, destroyed) }.

valid_next(State, Event) :-
    member(Event, [
        construct(_), is_inside(_),
        on_button_down(_, _), on_button_up(_, _),
        on_mouse_move(_, _), on_mouse_drag(_, _),
        destruct
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ============================================================================
%% Source quirks
%% ============================================================================
%%
%%   1. Every method body except the ctor is empty (Tracker.cpp:23-49).
%%      This is a template-method pattern -- the class exists to
%%      define the virtual interface that CRotateTracker, CZoomTracker,
%%      and any future subclass override.
%%
%%   2. IsInside (cpp:30-33) returns TRUE unconditionally. Subclasses
%%      can override to implement hit testing (e.g. "only respond to
%%      clicks inside a specific region"); the base accepts all clicks.
