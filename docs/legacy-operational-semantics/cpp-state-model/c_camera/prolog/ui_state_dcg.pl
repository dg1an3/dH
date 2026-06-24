%% Source: GEOM_VIEW/include/Camera.h, GEOM_VIEW/Camera.cpp
%%
%% LTS for CCamera (CObject subclass) -- the viewpoint for a CSceneView.
%% OBSERVABLE: owns an m_eventChange (CObservableEvent) that fires on
%% every setter. 7 setters fire directly; SetFieldOfView fires it twice
%% indirectly via SetClippingPlanes + SetDistance.
%%
%% Pairs with c_beam in the observer-fan-out style: just as
%% c_beam_record:fires_change_event/1 enumerates the 9 CBeam setters
%% that propagate, c_camera here enumerates the 7+1 CCamera setters.
%% The CSceneView's render loop observes these events (per CSimView's
%% camera setup at simview.cpp:189-193).

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1,
    fires_change_event/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   active  -- camera exists; setters can mutate

%% ============================================================================
%% Event alphabet (10 events; 7+1 fire change)
%% ============================================================================
%%   construct                         -- Camera.cpp:32  ctor
%%   set_target(V)                     -- Camera.cpp:67  fires change
%%   set_direction(V)                  -- Camera.cpp:84  fires change
%%   set_distance(D)                   -- Camera.cpp:114 fires change
%%   set_up_direction(V)               -- Camera.cpp:132 fires change
%%   set_viewing_angle(A)              -- Camera.cpp:175 fires change
%%   set_aspect_ratio(R)               -- Camera.cpp:201 fires change
%%   set_clipping_planes(N, F)         -- Camera.cpp:240 fires change
%%   set_field_of_view(MaxSize)        -- Camera.cpp:258 convenience -- fires TWICE
%%   recalc_xform                       -- Camera.cpp:280 const helper (mutates mutable cache)
%%   recalc_projection                  -- Camera.cpp:310 const helper (mutates mutable cache)
%%   destruct                           -- Camera.cpp:57  empty body

initial_state(state{
    win:                  pre_construct,
    target:               vec3(0.0, 0.0, 0.0),
    direction:            vec3(0.0, 0.0, -1.0),
    distance:             1.0,
    up_direction:         vec3(0.0, 1.0, 0.0),
    viewing_angle:        0.0,                  %% 0 means orthographic
    aspect_ratio:         1.0,
    near_plane:           0.1,
    far_plane:            100.0,
    %% mutable caches recomputed in const getters
    xform:                identity_4x4,
    projection:           identity_4x4
}).

step(S0, S1) --> [construct],
    { S0.win == pre_construct,
      S1 = S0.put(win, active) }.

step(S0, S1) --> [set_target(V)],
    { S0.win == active,
      S1 = S0.put([target - V]) }.

step(S0, S1) --> [set_direction(V)],
    { S0.win == active,
      S1 = S0.put([direction - V]) }.

step(S0, S1) --> [set_distance(D)],
    { S0.win == active,
      S1 = S0.put([distance - D]) }.

step(S0, S1) --> [set_up_direction(V)],
    { S0.win == active,
      S1 = S0.put([up_direction - V]) }.

step(S0, S1) --> [set_viewing_angle(A)],
    { S0.win == active,
      S1 = S0.put([viewing_angle - A]) }.

step(S0, S1) --> [set_aspect_ratio(R)],
    { S0.win == active,
      S1 = S0.put([aspect_ratio - R]) }.

step(S0, S1) --> [set_clipping_planes(N, F)],
    { S0.win == active,
      S1 = S0.put([near_plane - N, far_plane - F]) }.

%% C++: Camera.cpp:258-263  SetFieldOfView(MaxSize)
%%      SetClippingPlanes(near, near + MaxSize * 2.5)   <- fires change
%%      SetDistance(near + MaxSize / 1.2)                <- fires change
%% Fires GetChangeEvent TWICE indirectly. Modeled as two state updates.
step(S0, S1) --> [set_field_of_view(MaxSize)],
    { S0.win == active,
      N is S0.near_plane,
      NewFar is N + MaxSize * 2.5,
      NewDist is N + MaxSize / 1.2,
      S1 = S0.put([
          near_plane - N,
          far_plane - NewFar,
          distance - NewDist
      ]) }.

%% const helpers that mutate the mutable cache. Modeled as state events.
step(S0, S1) --> [recalc_xform],
    { S0.win == active,
      S1 = S0.put(xform, recomputed_from_target_direction_distance) }.

step(S0, S1) --> [recalc_projection],
    { S0.win == active,
      S1 = S0.put(projection, recomputed_from_viewing_angle_aspect_planes) }.

step(S0, S1) --> [destruct],
    { S0.win == active,
      S1 = S0.put(win, destroyed) }.

%% ============================================================================
%% Which setters fire the observable change event
%% ============================================================================
fires_change_event(set_target(_)).
fires_change_event(set_direction(_)).
fires_change_event(set_distance(_)).
fires_change_event(set_up_direction(_)).
fires_change_event(set_viewing_angle(_)).
fires_change_event(set_aspect_ratio(_)).
fires_change_event(set_clipping_planes(_, _)).
fires_change_event(set_field_of_view(_)).      %% fires TWICE indirectly

valid_next(State, Event) :-
    member(Event, [
        construct, set_target(_), set_direction(_), set_distance(_),
        set_up_direction(_), set_viewing_angle(_), set_aspect_ratio(_),
        set_clipping_planes(_, _), set_field_of_view(_),
        recalc_xform, recalc_projection, destruct
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
%%   1. m_mXform and m_mProjection are `mutable` -- recomputed lazily
%%      inside const getters (GetXform calls RecalcCameraToModel;
%%      GetProjection has its own lazy path). Pattern similar to
%%      CBeam's mutable m_beamToFixedXform / m_beamToPatientXform.
%%
%%   2. SetFieldOfView (cpp:258-263) is a CONVENIENCE that calls
%%      SetClippingPlanes + SetDistance -- so calling it fires the
%%      change event TWICE. Observers attached to the camera see
%%      two distinct notifications. Quirk preserved.
%%
%%   3. Comment at Camera.h:78 // const CMatrixD<4>& GetProjection() const;
%%      is a duplicate declaration commented out. Dead.
