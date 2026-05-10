%% Source: RT_VIEW/include/BeamParamCollimCtrl.h, RT_VIEW/BeamParamCollimCtrl.cpp
%%
%% LTS for CBeamParamCollimCtrl (CDialog subclass) -- the twin of
%% CBeamParamPosCtrl. Glue-medium. Same MFC dialog pattern (DDX-bound
%% int fields with paired ON_EN_CHANGE / ON_NOTIFY UDN_DELTAPOS handlers
%% that funnel into OnApplyChanges -> UpdateData(TRUE) -> DDX save).
%%
%% IMPORTANT distinguishing feature: this class has an explicit
%% **re-entrancy guard** that c_beam_param_pos_ctrl does NOT:
%%   BeamParamCollimCtrl.cpp:31  BOOL m_bUpdatingData = FALSE;
%%   cpp:65   m_bUpdatingData = TRUE;  -- before SetCollim* calls
%%   cpp:77   m_bUpdatingData = FALSE; -- after SetCollim* calls
%%   cpp:86   if (IsWindow && !m_bUpdatingData)  UpdateData(FALSE);
%%
%% This guard prevents a re-entrancy loop when a CBeam observer
%% reacts to a SetCollimAngle/SetCollimMin/SetCollimMax change event
%% (per c_beam_record:fires_change_event/1) by calling SetBeam, which
%% would otherwise re-enter DoDataExchange in load mode while the save
%% is still in progress.
%%
%% The fact that c_beam_param_pos_ctrl LACKS this guard is suspect --
%% the same observer scenario applies to SetGantryAngle/SetCouchAngle/
%% SetTableOffset. The PosCtrl version has commented-out RemoveObserver/
%% AddObserver pairs (cpp:63-69) that suggest an earlier observer
%% management strategy that was abandoned. The Collim version solves
%% the same problem with the m_bUpdatingData flag instead.

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
%%   pre_init       -- ctor ran; OnInitDialog not yet
%%   ready          -- OnInitDialog succeeded
%%   bound          -- SetBeam(B) attached non-NULL CBeam
%%
%% Note: NO modal/scan/confirm tiers; m_bUpdatingData is a TRANSIENT
%% scope-local flag (true only inside the Save branch of DoDataExchange),
%% modeled here as a state-dict bool rather than a sub-state.

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   on_init_dialog                -- BeamParamCollimCtrl.cpp:110  OnInitDialog
%%   set_beam(B)                   -- BeamParamCollimCtrl.cpp:82   SetBeam
%%   on_apply_changes              -- BeamParamCollimCtrl.cpp:94-102 ON_EN_CHANGE x5
%%   on_apply_spin_changes(NMHDR)  -- BeamParamCollimCtrl.cpp:95-103 ON_NOTIFY UDN_DELTAPOS x5
%%   do_data_exchange(Save)        -- BeamParamCollimCtrl.cpp:33   DDX

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: ctor at cpp:18-29  m_pBeam(NULL); five m_n* fields all 0.
%%      module-global BOOL m_bUpdatingData at cpp:31 = FALSE.
initial_state(state{
    win:                pre_init,
    pbeam:              none,
    %% DDX-bound fields
    n_collim_angle:     0,                          %% degrees
    n_jaw_x1:           0,
    n_jaw_x2:           0,
    n_jaw_y1:           0,
    n_jaw_y2:           0,
    %% spin button ranges (set in OnInitDialog)
    collim_range:       none,
    jaw_x1_range:       none,
    jaw_x2_range:       none,
    jaw_y1_range:       none,
    jaw_y2_range:       none,
    %% re-entrancy guard (module-global in source; modeled in state)
    updating_data:      false
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: BeamParamCollimCtrl.cpp:110-122  OnInitDialog
%%      collim:          [0, 359]
%%      jawX1/X2/Y1/Y2:  [-20, 20]
step(S0, S1) --> [on_init_dialog],
    { S0.win == pre_init,
      edit_ops:on_init_dialog(S0, S1) }.

%% C++: BeamParamCollimCtrl.cpp:82-90  SetBeam
%%      m_pBeam = pBeam; if (IsWindow && !m_bUpdatingData) UpdateData(FALSE).
%%      The !m_bUpdatingData guard is the re-entrancy break.
step(S0, S1) --> [set_beam(B)],
    { browse_ops:is_ready_or_bound(S0),
      edit_ops:set_beam(S0, B, S1) }.

%% C++: ON_EN_CHANGE x5 (collim, jawX1, jawX2, jawY1, jawY2) at cpp:94-102.
%%      All call OnApplyChanges -> UpdateData(TRUE).
step(S0, S1) --> [on_apply_changes],
    { browse_ops:is_bound(S0),
      edit_ops:apply_changes(S0, S1) }.

step(S0, S1) --> [on_apply_spin_changes(_NMHDR)],
    { browse_ops:is_bound(S0),
      edit_ops:apply_changes(S0, S1) }.

step(S0, S1) --> [do_data_exchange(Save)],
    { browse_ops:is_ready_or_bound(S0),
      edit_ops:do_data_exchange(S0, Save, S1) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        on_init_dialog, set_beam(_),
        on_apply_changes, on_apply_spin_changes(_),
        do_data_exchange(true), do_data_exchange(false)
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
%%   1. m_bUpdatingData at cpp:31 is a MODULE-GLOBAL BOOL, not a member
%%      variable. This means two CBeamParamCollimCtrl instances would
%%      share the flag -- if both are alive simultaneously and one is
%%      doing a save while the other receives a SetBeam, the second
%%      would skip its UpdateData(FALSE) refresh. Bug-prone but
%%      preserved verbatim.
%%
%%   2. cpp:66-67  Three commented-out lines:
%%        forBeam->SetAngles(((double)m_nCollimAngle) * PI / 180.0,
%%                            forBeam->myGantryAngle.Get(),
%%                            forBeam->myCouchAngle.Get());
%%      An earlier batched-setter pattern; replaced by the three
%%      separate SetCollimAngle/SetCollimMin/SetCollimMax calls.
%%
%%   3. The asymmetry with c_beam_param_pos_ctrl is itself a quirk:
%%      PosCtrl has commented-out RemoveObserver/AddObserver pairs;
%%      CollimCtrl uses the m_bUpdatingData flag. They are addressing
%%      the same re-entrancy concern with different (incomplete and
%%      complete, respectively) solutions.
