%% Source: RT_VIEW/include/BeamParamPosCtrl.h, RT_VIEW/BeamParamPosCtrl.cpp
%%
%% LTS for CBeamParamPosCtrl (CDialog subclass). Glue-medium.
%% Pairs with CBeamParamCollimCtrl (the twin sibling for collimator+
%% jaw settings). Both controls follow the same MFC dialog pattern:
%% DDX-bound int fields plus paired ON_EN_CHANGE / ON_NOTIFY UDN_DELTAPOS
%% handlers that call OnApplyChanges -> UpdateData(TRUE) -> writes
%% angle/offset back into the bound CBeam.
%%
%% IMPORTANT: this dialog illustrates the MFC DDX pattern explicitly.
%% DoDataExchange has bidirectional behavior:
%%   - When NOT validating (UpdateData(FALSE)), values are read FROM
%%     the bound CBeam, converted from radians to degrees for angles,
%%     and stored in m_n* fields.
%%   - When validating (UpdateData(TRUE)), values are written FROM
%%     the m_n* fields back into the CBeam, converted from degrees to
%%     radians for angles.
%% This is the canonical pattern referenced by the cpp-state-model
%% skill's "MFC extras" section.

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
%%   ready          -- OnInitDialog succeeded; spin ranges set
%%   bound          -- SetBeam(B) attached a non-NULL CBeam; UpdateData(FALSE) fired

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   on_init_dialog                -- BeamParamPosCtrl.cpp:104  OnInitDialog
%%   set_beam(B)                   -- BeamParamPosCtrl.cpp:76   SetBeam
%%   on_apply_changes              -- BeamParamPosCtrl.cpp:88-96 ON_EN_CHANGE x5
%%   on_apply_spin_changes(NMHDR)  -- BeamParamPosCtrl.cpp:89-97 ON_NOTIFY UDN_DELTAPOS x5
%%   do_data_exchange(Save)        -- BeamParamPosCtrl.cpp:32  DDX entry; Save in {true, false}
%%
%% NOTE: ON_EN_CHANGE/ON_NOTIFY entries are repeated in the message map
%% across 5 control IDs each (gantry/couch/tableX/tableY/tableZ). They
%% all map to the same OnApplyChanges/OnApplySpinChanges handler, so
%% the LTS uses a single event terminal per shape.

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: BeamParamPosCtrl.cpp:18-29  CBeamParamPosCtrl::CBeamParamPosCtrl
%%      m_pBeam(NULL); m_n* fields all 0 (AFX_DATA_INIT block).
initial_state(state{
    win:                pre_init,
    pbeam:              none,
    %% DDX-bound fields (copies of CBeam values during edit)
    n_gantry_angle:     0,                          %% degrees
    n_couch_angle:      0,                          %% degrees
    n_table_x:          0,
    n_table_y:          0,
    n_table_z:          0,
    %% spin button ranges (set in OnInitDialog at cpp:109-113)
    gantry_range:       none,
    couch_range:        none,
    table_x_range:      none,
    table_y_range:      none,
    table_z_range:      none
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: BeamParamPosCtrl.cpp:104-117  OnInitDialog
%%      CDialog::OnInitDialog; sets spin ranges:
%%        gantry/couch:  [0, 359]
%%        tableX/Y/Z:    [-200, 200]
step(S0, S1) --> [on_init_dialog],
    { S0.win == pre_init,
      edit_ops:on_init_dialog(S0, S1) }.

%% C++: BeamParamPosCtrl.cpp:76-84  SetBeam
%%      m_pBeam = pBeam; if (IsWindow(m_hWnd)) UpdateData(FALSE).
%% UpdateData(FALSE) calls DoDataExchange with bSaveAndValidate = false,
%% which loads angle/offset values FROM pBeam into the m_n* fields.
step(S0, S1) --> [set_beam(B)],
    { browse_ops:is_ready_or_bound(S0),
      edit_ops:set_beam(S0, B, S1) }.

%% C++: BeamParamPosCtrl.cpp:119-128  OnApplyChanges
%%      if (IsWindow && m_pBeam != NULL): UpdateData(TRUE).
%%      UpdateData(TRUE) calls DoDataExchange with bSaveAndValidate = true,
%%      which validates and writes m_n* values back to pBeam (calling
%%      pBeam->SetGantryAngle, SetCouchAngle, SetTableOffset).
%%      Each of those CBeam setters fires GetChangeEvent (per c_beam fan-out).
step(S0, S1) --> [on_apply_changes],
    { browse_ops:is_bound(S0),
      edit_ops:apply_changes(S0, S1) }.

%% C++: BeamParamPosCtrl.cpp:130-136  OnApplySpinChanges
%%      Forwards directly to OnApplyChanges; LRESULT *pResult set to 0.
step(S0, S1) --> [on_apply_spin_changes(_NMHDR)],
    { browse_ops:is_bound(S0),
      edit_ops:apply_changes(S0, S1) }.

%% C++: BeamParamPosCtrl.cpp:32-74  DoDataExchange
%%      Public-callable via UpdateData. Modeled as an explicit event so
%%      cross-class queries can distinguish FROM-beam-load vs TO-beam-save.
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
%%   1. BeamParamPosCtrl.cpp:63-65, 67-69  Three commented-out
%%      pBeam->gantryAngle.RemoveObserver / AddObserver pairs around
%%      the angle setters. Looks like a removed direct-observer pattern
%%      that was replaced with the broader CBeam GetChangeEvent. Dead
%%      code preserved.
%%
%%   2. BeamParamPosCtrl.cpp:126  // AfxGetMainWnd()->RedrawWindow(...)
%%      OnApplyChanges has a commented-out RedrawWindow call. The
%%      change-event fan-out from CBeam should drive observer-based
%%      redraws instead.
%%
%%   3. The five DDX_Text bindings + five DDV_MinMaxInt validations
%%      at cpp:48-58 are 1:1 with the five m_n* fields.
%%
%%   4. The five ON_EN_CHANGE entries at cpp:88-96 all map to the same
%%      OnApplyChanges. The five ON_NOTIFY UDN_DELTAPOS entries at
%%      cpp:89-97 all map to OnApplySpinChanges -- which itself just
%%      calls OnApplyChanges. There is no per-field discrimination at
%%      the handler level.
