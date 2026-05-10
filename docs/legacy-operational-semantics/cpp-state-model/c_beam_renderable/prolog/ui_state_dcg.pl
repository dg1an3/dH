%% Source: RT_VIEW/include/BeamRenderable.h, RT_VIEW/BeamRenderable.cpp
%%
%% LTS for CBeamRenderable (CRenderable subclass). Renders the
%% treatment beam's geometry -- collimator field rectangle,
%% shielding-block divergence surfaces, central axis, field
%% divergence lines/surfaces -- in a CSceneView.
%%
%% Heaviest of the RT_VIEW renderables (~600 lines source) BUT most
%% of the bulk is **dead code from a partial API migration**: the
%% original implementation used CRenderContext (OpenGL idioms);
%% the surviving code uses LPDIRECT3DDEVICE8 (D3D8). The migration
%% left two large rendering paths block-commented:
%%
%%   1. DrawOpaque body (cpp:192-241) -- ENTIRELY commented out.
%%      Originally rendered: graticule, field rectangle (at SCD
%%      and SID planes), blocks, central axis, field divergence
%%      lines. None of it runs.
%%
%%   2. DrawFieldDivergenceSurfaces body (cpp:469-478) -- ENTIRELY
%%      commented out. The if (m_bFieldDivergenceSurfacesEnabled)
%%      branch is unreachable in practice (flag defaults FALSE)
%%      and even if reached, does nothing.
%%
%% Only DrawBlockDivergenceSurfaces actually renders -- it lazily
%% generates a D3DXMesh from the bound CBeam's block 0 vertices,
%% caches it in m_pBlockDivSurfMesh, then calls DrawSubset(0).
%% That is the entirety of the working rendering surface in this
%% build.

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
%%   unbound       -- m_pObject is NULL
%%   bound         -- CBeam attached; observer registered
%%   mesh_cached   -- m_pBlockDivSurfMesh has been generated and cached

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   set_beam(B)                              -- BeamRenderable.cpp:143
%%   set_object(O)                            -- BeamRenderable.cpp:154
%%   draw_opaque                              -- BeamRenderable.cpp:190 (dead code)
%%   draw_transparent                         -- BeamRenderable.cpp:438
%%   on_beam_changed                          -- BeamRenderable.cpp:249
%%   destruct                                 -- ~CBeamRenderable releases mesh
%%
%% NOT in event alphabet (helper methods called only from dead code):
%%   DrawGraticule, DrawField, DrawBlocks, DrawCentralAxis,
%%   DrawFieldDivergenceLines, DrawFieldDivergenceSurfaces -- all
%%   reachable only via the commented-out DrawOpaque body. They
%%   exist as defined methods but cannot be triggered through any
%%   live event flow.

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: ctor at cpp:96-105:
%%   m_bCentralAxisEnabled(TRUE)            <- never read by live code
%%   m_bGraticuleEnabled(FALSE)             <- never read by live code
%%   m_bFieldDivergenceSurfacesEnabled(FALSE) <- read in dead body
%%   m_bBlockDivergenceSurfacesEnabled(TRUE) <- the only LIVE flag
%%   m_pBlockDivSurfMesh(NULL)
%%   SetColor(RGB(0, 255, 0)); SetAlpha(0.25);
initial_state(state{
    win:                              unbound,
    pbeam:                            none,
    color:                            rgb(0, 255, 0),
    alpha:                            0.25,
    %% --- 4 toggle flags (3 of them functionally dead) ---
    central_axis_enabled:             true,        %% only read from dead DrawOpaque
    graticule_enabled:                false,       %% only read from dead DrawGraticule (called only by dead DrawOpaque)
    field_divergence_surfaces_enabled: false,      %% read by DrawFieldDivergenceSurfaces (live) but body is empty
    block_divergence_surfaces_enabled: true,       %% the only LIVE toggle
    %% --- 4 collimator-corner cache fields ---
    %% Set in dead DrawOpaque from CBeam GetCollimMin/Max. Used by
    %% the dead Draw* helpers. In current build these stay at zero.
    v_min:                            vec2(0.0, 0.0),
    v_max:                            vec2(0.0, 0.0),
    v_min_x_max_y:                    vec2(0.0, 0.0),
    v_max_x_min_y:                    vec2(0.0, 0.0),
    %% --- D3D8 cached mesh (the LIVE rendering artifact) ---
    block_div_surf_mesh:              none,
    %% --- modelview matrix (set on every OnBeamChanged) ---
    modelview_matrix:                 identity_4x4,
    %% --- observer ---
    observer_attached:                false
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: BeamRenderable.cpp:143-147  SetBeam (one-line wrapper)
step(S0, S1) --> [set_beam(B)],
    { edit_ops:set_object(S0, B, S1) }.

%% C++: BeamRenderable.cpp:154-183  SetObject
step(S0, S1) --> [set_object(O)],
    { edit_ops:set_object(S0, O, S1) }.

%% C++: BeamRenderable.cpp:190-242  DrawOpaque  -- DEAD BODY
%%      Body entirely block-commented. Preserved verbatim.
step(S0, S0) --> [draw_opaque],
    { browse_ops:is_bound_or_cached(S0) }.

%% C++: BeamRenderable.cpp:438-457  DrawTransparent
%%      Live body: SetRenderState(D3DSHADE_FLAT);
%%                 DrawBlockDivergenceSurfaces(pd3dDev);
%%                 SetRenderState(D3DSHADE_GOURAUD);
%%      The commented-out lines (SetSmoothShading, SetLighting,
%%      DrawFieldDivergenceSurfaces) are pre-D3D-migration debris.
step(S0, S1) --> [draw_transparent],
    { browse_ops:is_bound_or_cached(S0),
      edit_ops:draw_transparent(S0, S1) }.

%% C++: BeamRenderable.cpp:249-265  OnBeamChanged
%%      Recomputes the modelview matrix:
%%        mProjInv = invert(machine.projection)
%%        SetModelviewMatrix(beam.beam_to_fixed_xform
%%                           * scale(1, 1, -1)
%%                           * mProjInv)
%%      Then calls Invalidate() to mark for redraw.
step(S0, S1) --> [on_beam_changed],
    { browse_ops:is_bound_or_cached(S0),
      edit_ops:on_beam_changed(S0, S1) }.

%% C++: BeamRenderable.cpp:112-118  ~CBeamRenderable
%%      if (m_pBlockDivSurfMesh) m_pBlockDivSurfMesh->Release()
step(S0, S1) --> [destruct],
    { S1 = S0.put([
          win - destroyed,
          block_div_surf_mesh - released,
          observer_attached - false
      ]) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        set_beam(_), set_object(_),
        draw_opaque, draw_transparent, on_beam_changed, destruct
    ]),
    phrase(step(State, _), [Event]).

valid_sequence(Events) :-
    initial_state(S0),
    phrase(step_seq(S0, _), Events).

step_seq(S, S) --> [].
step_seq(S0, S) --> step(S0, S1), step_seq(S1, S).

%% ============================================================================
%% Source quirks preserved verbatim (the dead code is the story)
%% ============================================================================
%%
%%   1. DrawOpaque body (cpp:192-241) is ENTIRELY block-commented.
%%      Originally invoked: DrawGraticule, DrawField, DrawBlocks
%%      (twice -- at SCD plane and at SID plane), DrawCentralAxis,
%%      DrawFieldDivergenceLines. None of these helpers are reachable
%%      through any live event flow.
%%
%%   2. DrawFieldDivergenceSurfaces body (cpp:469-478) is block-
%%      commented. Even though the function is called from
%%      DrawTransparent at (commented-out) cpp:445, the body is empty.
%%
%%   3. m_bCentralAxisEnabled (init TRUE) and m_bGraticuleEnabled
%%      (init FALSE) are NEVER READ by any live code path. They are
%%      stale flags preserved for memory-layout fidelity.
%%
%%   4. m_bFieldDivergenceSurfacesEnabled (init FALSE) IS read by
%%      DrawFieldDivergenceSurfaces (which is live and called from
%%      DrawTransparent), but the body is empty -- so the flag has
%%      no effect.
%%
%%   5. m_bBlockDivergenceSurfacesEnabled (init TRUE) is the ONLY
%%      LIVE flag. Setting it false would suppress the only working
%%      rendering output. There is no public setter; it can only be
%%      flipped via direct member access (private!) -- so the flag
%%      is effectively constant TRUE.
%%
%%   6. DrawBlockDivergenceSurfaces (cpp:487-541) is the only
%%      working rendering. Lazy mesh generation -- on first call,
%%      reads CBeam->GetBlock(0) (HARD-CODED INDEX 0; if the beam
%%      has multiple blocks, only block 0 renders), builds a
%%      D3DXMesh, caches it. Subsequent calls just DrawSubset(0).
%%      The cache is invalidated only on destruct (~CBeamRenderable
%%      releases m_pBlockDivSurfMesh). It does NOT regenerate when
%%      the beam's blocks change.
%%
%%   7. cpp:494-498  Three commented-out IPolygon3D / SafeArray
%%      pattern lines suggest an earlier COM-based block geometry
%%      access that was abandoned for the direct CPolygon* API.
%%
%%   8. cpp:259  CreateScale(CVectorD<3>(1.0, 1.0, -1.0)) -- the
%%      Z-flip in the modelview matrix. The beam-to-fixed transform
%%      uses the IEC convention (beam down +Z) but the rendering
%%      coordinate system uses screen-Z (out of screen +Z), so the
%%      Z component is flipped.
