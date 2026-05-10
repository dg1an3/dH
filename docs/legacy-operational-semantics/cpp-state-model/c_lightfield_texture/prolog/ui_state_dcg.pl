%% Source: RT_VIEW/include/LightfieldTexture.h, RT_VIEW/LightfieldTexture.cpp
%%
%% LTS for CLightfieldTexture (CTexture subclass). The smallest of the
%% RT_VIEW renderables -- ~150 lines, one external method (SetBeam),
%% one observer callback (OnBeamChanged). Glue-medium-light.
%%
%% Behavior: when SetBeam binds a CBeam, the texture renders a
%% diagrammatic representation of the beam's collimator field +
%% blocks + central-axis crosshair onto a 512x512 GDI surface,
%% then computes a texture-space projection matrix that combines
%% the texture-size scaling, the treatment machine's projection,
%% and the inverse beam-to-patient transform.

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
%%   unbound  -- m_pBeam is NULL; texture is empty
%%   bound    -- m_pBeam attached; observer registered; texture rendered

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   set_beam(B)               -- LightfieldTexture.cpp:44   public method
%%   on_beam_changed           -- LightfieldTexture.cpp:68   observer callback
%%
%% Both events trigger the same render pipeline -- SetBeam calls
%% OnBeamChanged explicitly at cpp:64 to force the initial render.

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: ctor at cpp:28-32  m_pBeam(NULL)
initial_state(state{
    win:                       unbound,
    pbeam:                     none,
    %% texture surface state (set by OnBeamChanged via SetWidthHeight)
    texture_width:             0,
    texture_height:            0,
    texture_rendered:          false,
    %% rendered elements (in OnBeamChanged)
    field_rect_drawn:          false,
    blocks_drawn_count:        0,                  %% cpp:100-115 loop; body block-commented
    central_axis_drawn:        false,
    border_drawn:              false,
    %% computed projection matrix (cpp:135-156)
    projection_set:            false,
    %% observer registration (cpp:46-65)
    observer_attached:         false
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: LightfieldTexture.cpp:44-65  SetBeam(CBeam* pBeam)
%%      Sequence:
%%        1. if (m_pBeam != NULL): RemoveObserver from old beam
%%        2. m_pBeam = pBeam
%%        3. if (m_pBeam != NULL): AddObserver to new beam
%%        4. OnBeamChanged(GetBeam()->GetChangeEvent(), NULL)
%%      Step 4 forces an initial render. NOTE: it dereferences GetBeam()
%%      unconditionally at cpp:64 -- if pBeam is NULL, this crashes.
%%      OnBeamChanged itself has an early return for null beam at cpp:70-73,
%%      but the AddObserver caller doesn't guard the .GetChangeEvent() deref.
step(S0, S1) --> [set_beam(B)],
    { edit_ops:set_beam(S0, B, S1) }.

%% C++: LightfieldTexture.cpp:68-157  OnBeamChanged
%%      if (m_pBeam == NULL): return.
%%      else: render the lightfield diagram (collimator rectangle +
%%      blocks + central-axis crosshair + border) and compute the
%%      projection matrix.
step(S0, S1) --> [on_beam_changed],
    { S0.win == bound,
      edit_ops:on_beam_changed(S0, S1) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [set_beam(_), on_beam_changed]),
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
%%   1. cpp:64  OnBeamChanged is called with `&GetBeam()->GetChangeEvent()`
%%      AFTER the m_pBeam = pBeam assignment. If the new pBeam is NULL,
%%      GetBeam()->GetChangeEvent() dereferences NULL. The OnBeamChanged
%%      handler itself guards for null at cpp:70-73, but the caller does
%%      not. NULL-deref hazard preserved verbatim.
%%
%%   2. cpp:102-114  The block-rendering loop body is ENTIRELY block-
%%      commented:
%%        for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++) {
%%        /*  ...polygon-block rendering... */
%%        }
%%      The loop iterates GetBlockCount() times but does nothing per
%%      iteration. Dead code preserved.
%%
%%   3. cpp:147  // glMultMatrix(m_pBeam->GetTreatmentMachine()->m_projection);
%%      Commented out. The replacement at cpp:144 uses
%%      GetTreatmentMachine()->GetProjection() (with the projection-
%%      stale quirk inherited from c_treatment_machine).
%%
%%   4. cpp:154  // glMultMatrix(mXform);
%%      Commented out. The replacement uses mTex *= mB2PXform.
%%
%%   5. PEN_THICKNESS at cpp:21 is TEX_RESOLUTION / 100 = 5 pixels for
%%      the default 512-resolution texture. Hard-coded constant.
