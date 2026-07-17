%% Source: GEOM_VIEW/include/Renderable.h, GEOM_VIEW/Renderable.cpp
%%
%% LTS for CRenderable -- the base class for everything that draws into
%% a CSceneView. Concrete subclasses in the cohort: CSurfaceRenderable,
%% CDRRRenderable (here in GEOM_VIEW), CBeamRenderable +
%% CMachineRenderable (RT_VIEW), CDRRRenderer (VSIM_OGL).
%%
%% Glue-medium. State holds the model-object pointer, color/alpha/
%% modelview matrix, enabled flag, OpenGL display-list IDs (leftover
%% from pre-D3D8 era), and a D3D8 vertex buffer.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1,
    triggers_invalidate/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   detached  -- m_pView is NULL; setters that need view do nothing
%%   attached  -- m_pView is set (via CSceneView friend access; not by
%%                a public method on this class)

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   construct                          -- Renderable.cpp:38   ctor
%%   set_object(O)                      -- Renderable.cpp:82   public
%%   set_color(C)                       -- Renderable.cpp:102  invalidates
%%   set_alpha(A)                       -- Renderable.cpp:128  invalidates
%%   set_centroid(V)                    -- Renderable.cpp:155  (no invalidate)
%%   set_modelview_matrix(M)            -- Renderable.cpp:200  invalidates if view attached
%%   set_enabled(B)                     -- Renderable.cpp:228  invalidates if view attached
%%   invalidate                         -- Renderable.cpp:246  deletes display lists, invalidates view
%%   draw_opaque                        -- Renderable.cpp:308  empty body (overridden by subclasses)
%%   draw_transparent                   -- Renderable.cpp:317  empty body
%%   draw_opaque_list                   -- Renderable.cpp:326  compiles + calls display list
%%   draw_transparent_list              -- Renderable.cpp:366  compiles + calls display list
%%   destroy_buffers                    -- Renderable.cpp:402
%%   destruct                           -- Renderable.cpp:54   empty body

initial_state(state{
    win:                          pre_construct,
    pobject:                      none,
    pview:                        none,
    color:                        rgb(255, 255, 255),
    alpha:                        1.0,
    centroid:                     vec3(0.0, 0.0, 0.0),
    modelview:                    identity_4x4,
    enabled:                      true,
    %% OpenGL display lists (leftover pre-D3D8 mechanism; -1 = invalid)
    draw_list_opaque:             -1,
    draw_list_transparent:        -1,
    %% D3D8 vertex buffer
    vertex_buffer:                none,
    %% temp distance for sorting (set by CSceneView during draw)
    temp_distance:                0.0
}).

%% C++: Renderable.cpp:38-47  ctor
step(S0, S1) --> [construct],
    { S0.win == pre_construct,
      S1 = S0.put(win, detached) }.

%% C++: Renderable.cpp:82-85  SetObject (one-line wrapper)
step(S0, S1) --> [set_object(O)],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put(pobject, O) }.

%% C++: Renderable.cpp:102-109  SetColor -- always calls Invalidate
step(S0, S1) --> [set_color(C)],
    { (S0.win == detached ; S0.win == attached),
      S2 = S0.put(color, C),
      do_invalidate(S2, S1) }.

%% C++: Renderable.cpp:128-135  SetAlpha -- always calls Invalidate
step(S0, S1) --> [set_alpha(A)],
    { (S0.win == detached ; S0.win == attached),
      S2 = S0.put(alpha, A),
      do_invalidate(S2, S1) }.

%% C++: Renderable.cpp:155  SetCentroid -- does NOT invalidate
step(S0, S1) --> [set_centroid(V)],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put(centroid, V) }.

%% C++: Renderable.cpp:200-211  SetModelviewMatrix -- only invalidates
%%      if (NULL != m_pView). Calls m_pView->Invalidate(), not
%%      this->Invalidate() -- so display lists are NOT deleted.
%%      Subtle: SetColor/SetAlpha (via Invalidate) DO delete display
%%      lists; SetModelviewMatrix does NOT. Asymmetric quirk.
step(S0, S1) --> [set_modelview_matrix(M)],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put(modelview, M) }.

%% C++: Renderable.cpp:228-239  SetEnabled -- same asymmetric pattern
%%      as SetModelviewMatrix: only m_pView->Invalidate(), not this.
step(S0, S1) --> [set_enabled(B)],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put(enabled, B) }.

%% C++: Renderable.cpp:246-279  Invalidate
%%      if m_pView is NULL: early return.
%%      else: delete both display lists (if alive), reset to -1,
%%            call m_pView->Invalidate().
step(S0, S1) --> [invalidate],
    { do_invalidate(S0, S1) }.

%% C++: Renderable.cpp:308-310  DrawOpaque -- empty body in base.
%%      Overridden by subclasses (CSurfaceRenderable, CDRRRenderable,
%%      CBeamRenderable, CMachineRenderable).
step(S0, S0) --> [draw_opaque],
    { (S0.win == detached ; S0.win == attached) }.

%% C++: Renderable.cpp:317-319  DrawTransparent -- empty body.
step(S0, S0) --> [draw_transparent],
    { (S0.win == detached ; S0.win == attached) }.

%% C++: Renderable.cpp:326-364  DrawOpaqueList
%%      if (!IsEnabled()) return.
%%      Compiles a display list (lazy: only if m_nDrawListOpaque == -1)
%%      by calling DrawOpaque inside glNewList/glEndList; then calls
%%      the display list. Subsequent calls just glCallList without
%%      recompiling.
step(S0, S1) --> [draw_opaque_list],
    { (S0.win == detached ; S0.win == attached),
      ( S0.enabled == true
      -> ( S0.draw_list_opaque == -1
         -> S1 = S0.put(draw_list_opaque, compiled_with_id)
         ;  S1 = S0
         )
      ;  S1 = S0
      ) }.

step(S0, S1) --> [draw_transparent_list],
    { (S0.win == detached ; S0.win == attached),
      ( S0.enabled == true
      -> ( S0.draw_list_transparent == -1
         -> S1 = S0.put(draw_list_transparent, compiled_with_id)
         ;  S1 = S0
         )
      ;  S1 = S0
      ) }.

step(S0, S1) --> [destroy_buffers],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put([
          draw_list_opaque - -1,
          draw_list_transparent - -1,
          vertex_buffer - none
      ]) }.

step(S0, S1) --> [destruct],
    { (S0.win == detached ; S0.win == attached),
      S1 = S0.put(win, destroyed) }.

%% ============================================================================
%% Which setters trigger Invalidate (the cascading redraw)
%% ============================================================================
triggers_invalidate(set_color(_)).             %% unconditional (cpp:108)
triggers_invalidate(set_alpha(_)).             %% unconditional (cpp:134)
triggers_invalidate(set_modelview_matrix(_)).  %% only if m_pView attached (cpp:206)
triggers_invalidate(set_enabled(_)).           %% only if m_pView attached (cpp:234)

%% set_centroid does NOT invalidate -- centroid is purely for sort
%% ordering and doesn't affect the rendered output.

%% ============================================================================
%% Helper: do_invalidate
%% ============================================================================
%% C++: Renderable.cpp:246-279
do_invalidate(StateIn, StateOut) :-
    ( StateIn.pview == none
    -> StateOut = StateIn          %% early return; no view = no effect
    ;  StateOut = StateIn.put([
           draw_list_opaque - -1,
           draw_list_transparent - -1
       ])
    ).

valid_next(State, Event) :-
    member(Event, [
        construct, set_object(_), set_color(_), set_alpha(_),
        set_centroid(_), set_modelview_matrix(_), set_enabled(_),
        invalidate, draw_opaque, draw_transparent,
        draw_opaque_list, draw_transparent_list, destroy_buffers, destruct
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
%%   1. IsHighlighted/SetHighlighted DECLARED in the .h (Renderable.h:
%%      93-94) but NEVER DEFINED in the .cpp. There is no m_bHighlighted
%%      member field. Any caller would get a linker error. Likely a
%%      planned feature that was never implemented. Preserved verbatim
%%      as part of the documented public interface.
%%
%%   2. SetupRenderingContext (cpp:286-301) has its body ENTIRELY
%%      commented out. Originally set up the OpenGL rendering state
%%      (matrix, color, alpha, shading model); the D3D8 migration left
%%      it as a no-op. Preserved.
%%
%%   3. OpenGL display lists (m_nDrawListOpaque/Transparent + glNewList/
%%      glDeleteLists/glCallList) coexist with D3D8 in the same class.
%%      DrawOpaqueList compiles a GL display list around the D3D8
%%      DrawOpaque calls -- this is broken; display lists can't capture
%%      D3D8 commands. The lazy-compile logic still runs but produces
%%      empty display lists. Preserved verbatim.
%%
%%   4. Asymmetric invalidate behavior:
%%        SetColor/SetAlpha   -> Invalidate() unconditionally (cpp:108,134)
%%                              -- so calling these on a detached
%%                              renderable still runs the Invalidate body
%%                              which early-returns. Harmless but odd.
%%        SetModelviewMatrix/SetEnabled -> m_pView->Invalidate() ONLY if
%%                              view attached (cpp:206,234). So these
%%                              skip the display-list deletion that
%%                              Invalidate() does.
%%      The asymmetry means SetColor invalidates display lists but
%%      SetModelviewMatrix doesn't, even though both visibly change
%%      the rendered output.
%%
%%   5. Missing override of OnButtonUp pattern (per tracker quirk):
%%      DrawOpaque and DrawTransparent both have empty bodies in the
%%      base. Subclasses are expected to override -- if they don't,
%%      nothing renders despite being attached to a view.
%%
%%   6. m_tempDistance is PUBLIC (Renderable.h:101). Used by CSceneView
%%      for back-to-front sort during transparent rendering. Public-
%%      mutable temp variable -- unusual but preserved.
