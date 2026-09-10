%% Source: GEOM_VIEW/include/Light.h, GEOM_VIEW/Light.cpp
%%
%% LTS for CLight (CObject + D3DLIGHT8 multi-inheritance). Glue-light:
%% a thin wrapper around D3D8's D3DLIGHT8 struct exposing position
%% (mapped to D3DLIGHT8::Direction) and diffuse color (mapped to
%% D3DLIGHT8::Diffuse). No observable change event.
%%
%% Multiple inheritance from D3DLIGHT8 (a POD struct) means the
%% accessors read/write D3DLIGHT8 fields directly via `this`-as-
%% D3DLIGHT8*. ZeroMemory in the ctor at cpp:34 clears the base
%% struct, then Type is set to D3DLIGHT_DIRECTIONAL.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   active  -- ctor ran; D3DLIGHT8 zeroed + Type set

initial_state(state{
    win:            pre_construct,
    %% D3DLIGHT8 fields exposed via CLight accessors:
    type:           none,                   %% cleared by ZeroMemory; set to D3DLIGHT_DIRECTIONAL in ctor
    direction:      vec3(0.0, 0.0, 0.0),
    diffuse:        rgb_float(0.0, 0.0, 0.0)
}).

step(S0, S1) --> [construct],
    { S0.win == pre_construct,
      S1 = S0.put([
          win - active,
          type - d3dlight_directional,            %% cpp:37
          direction - vec3(0.0, 0.0, 0.0),
          diffuse - rgb_float(0.0, 0.0, 0.0)
      ]) }.

%% C++: Light.cpp:69-76  SetPosition
%%      Direction.x/y/z := vPos[0/1/2]
%% NOTE: the C++ accessors map "position" -> D3DLIGHT8::Direction.
%% Since Type is D3DLIGHT_DIRECTIONAL, Direction is the right field
%% (directional lights have no position; the "position" is the
%% direction vector). The name "position" is misleading.
step(S0, S1) --> [set_position(V)],
    { S0.win == active,
      S1 = S0.put(direction, V) }.

%% C++: Light.cpp:55-61  GetPosition  -- read-only
step(S0, S0) --> [get_position(_)],
    { S0.win == active }.

%% C++: Light.cpp:98-104  SetDiffuseColor
%%      Diffuse.r/g/b := GetRValue/GetGValue/GetBValue(color) / 255.0
step(S0, S1) --> [set_diffuse_color(C)],
    { S0.win == active,
      S1 = S0.put(diffuse, C) }.

step(S0, S0) --> [get_diffuse_color(_)],
    { S0.win == active }.

step(S0, S1) --> [destruct],
    { S0.win == active,
      S1 = S0.put(win, destroyed) }.

valid_next(State, Event) :-
    member(Event, [
        construct, set_position(_), get_position(_),
        set_diffuse_color(_), get_diffuse_color(_), destruct
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
%%   1. Multi-inheritance from D3DLIGHT8 (a POD struct) plus CObject.
%%      The ctor casts `this` to D3DLIGHT8* and ZeroMemorys it. Brittle:
%%      assumes CObject has no virtual table or vptr that the ZeroMemory
%%      would clobber. MFC's CObject DOES have a vtbl, so this might
%%      actually corrupt it depending on the layout. Preserved verbatim.
%%
%%   2. "Position" accessors map to D3DLIGHT8::Direction. For a
%%      directional light, "position" doesn't really exist -- the
%%      vector is the LIGHT DIRECTION. The misleading name is
%%      preserved.
%%
%%   3. The #ifdef D3D8 guard at multiple sites suggests an OpenGL
%%      fallback path that was abandoned. The #else branches return
%%      default values (empty vector at cpp:60, white color at cpp:89)
%%      but the D3D8 macro is defined at Light.h:16 unconditionally,
%%      so the fallback is dead.
%%
%%   4. NO observable change event -- unlike CCamera which fires
%%      m_eventChange on every setter, CLight is a pure record. Views
%%      that need to react to light changes must poll.
