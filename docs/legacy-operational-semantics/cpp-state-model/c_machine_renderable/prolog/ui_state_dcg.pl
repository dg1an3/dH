%% Source: RT_VIEW/include/MachineRenderable.h, RT_VIEW/MachineRenderable.cpp
%%
%% LTS for CMachineRenderable (CRenderable subclass). Renders the
%% treatment machine (table + gantry + collimator) in a CSceneView,
%% driven by the gantry/couch/collimator angles of a bound CBeam.
%%
%% Glue-medium with one significant preserved quirk: m_bWireFrame
%% is initialized to FALSE in the ctor and NEVER written elsewhere,
%% making the entire DrawOpaque body unreachable. The "wireframe vs
%% solid" rendering mode advertised by the implementation is dead code.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   unbound  -- m_pObject is NULL (no CBeam attached)
%%   bound    -- CBeam attached; observer registered on its GetChangeEvent

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   set_beam(B)              -- MachineRenderable.cpp:63   delegates to SetObject
%%   set_object(O)            -- MachineRenderable.cpp:73   public override
%%   draw_opaque              -- MachineRenderable.cpp:104  CRenderable hook (dead code -- m_bWireFrame is always FALSE)
%%   draw_transparent         -- MachineRenderable.cpp:137  CRenderable hook (always renders)
%%   on_invalidate            -- inherited from CRenderable; observer callback fires this
%%   destruct                 -- ~CMachineRenderable

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: ctor at cpp:34-37  m_bWireFrame(FALSE).
initial_state(state{
    win:                       unbound,
    pbeam:                     none,
    %% rendering mode -- DEAD CODE; never written
    wireframe_mode:            false,
    %% observer status
    observer_attached:         false,
    %% rendering bookkeeping (true after the most recent draw)
    table_drawn:               false,
    gantry_drawn:              false,
    collimator_drawn:          false,
    %% derived rendering parameters (computed in Draw* from CBeam)
    sad_cached:                none,
    scd_cached:                none,
    axis_to_collim_cached:     none,
    gantry_angle_cached:       none
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: MachineRenderable.cpp:63-66  SetBeam(CBeam* pBeam)
%%      Just delegates to SetObject(pBeam). One-line wrapper.
step(S0, S1) --> [set_beam(B)],
    { edit_ops:set_object(S0, B, S1) }.

%% C++: MachineRenderable.cpp:73-97  SetObject(CObject* pObject)
%%      ASSERT pObject IsKindOf(CBeam).
%%      RemoveObserver from old beam, CRenderable::SetObject, AddObserver
%%      to new beam, Invalidate.
step(S0, S1) --> [set_object(O)],
    { edit_ops:set_object(S0, O, S1) }.

%% C++: MachineRenderable.cpp:104-130  DrawOpaque
%%      if (m_bWireFrame): SetupLines + DrawTable + Rotate + DrawGantry +
%%      DrawCollimator. ELSE: NOTHING.
%%      Since m_bWireFrame is initialized FALSE and never written, the
%%      else (no-op) branch always runs. Preserved verbatim.
step(S0, S1) --> [draw_opaque],
    { browse_ops:is_bound(S0),
      edit_ops:draw_opaque(S0, S1) }.

%% C++: MachineRenderable.cpp:137-156  DrawTransparent
%%      Unconditionally: DrawTable + Rotate + DrawGantry + DrawCollimator.
%%      The "table is also drawn during transparent pass" bit overlaps
%%      with the wireframe-only path -- if m_bWireFrame were ever TRUE,
%%      the table would be rendered TWICE per frame. Preserved.
step(S0, S1) --> [draw_transparent],
    { browse_ops:is_bound(S0),
      edit_ops:draw_transparent(S0, S1) }.

%% C++: CRenderable::Invalidate (inherited)
%%      Marks this renderable as needing redraw. The observer registered
%%      in SetObject calls Invalidate() on every CBeam GetChangeEvent fire.
step(S0, S1) --> [on_invalidate],
    { S1 = S0.put([
          table_drawn - false,
          gantry_drawn - false,
          collimator_drawn - false
      ]) }.

%% C++: ~CMachineRenderable (cpp:44-46, empty body)
step(S0, S1) --> [destruct],
    { S1 = S0.put([
          win - destroyed,
          observer_attached - false
      ]) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        set_beam(_), set_object(_),
        draw_opaque, draw_transparent, on_invalidate, destruct
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
%%   1. m_bWireFrame at MachineRenderable.h:53 is initialized to FALSE
%%      in the ctor (cpp:35) and NEVER written elsewhere. The
%%      DrawOpaque body's if (m_bWireFrame) branch is therefore dead
%%      code. The wireframe rendering path declared by the public
%%      interface is non-functional in this build.
%%
%%   2. DrawTransparent (cpp:137-156) AND the dead DrawOpaque branch
%%      (cpp:107-129) both call DrawTable + DrawGantry + DrawCollimator.
%%      If m_bWireFrame were ever flipped TRUE, every frame would
%%      render the machine twice -- once as wireframe lines, once as
%%      filled polygons. Z-fighting hazard preserved verbatim.
%%
%%   3. SetObject (cpp:73-97) registers Invalidate (a CRenderable
%%      method) as the observer callback rather than a custom handler.
%%      Compare to CLightfieldTexture which registers OnBeamChanged.
%%      Both work -- Invalidate just triggers a redraw on next frame,
%%      while OnBeamChanged re-renders synchronously.
%%
%%   4. Both DrawOpaque (when reachable) and DrawTransparent compute
%%      `axis_to_collim = SAD - SCD` and rotate by gantry angle. The
%%      axis_to_collim caching is implicit -- there's no flag, so
%%      every frame recomputes the same value from the (immutable
%%      between SetBeam calls) CTreatmentMachine.
