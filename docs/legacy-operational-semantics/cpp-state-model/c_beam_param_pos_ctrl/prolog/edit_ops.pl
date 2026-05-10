%% Source: RT_VIEW/BeamParamPosCtrl.cpp

:- module(edit_ops, [
    on_init_dialog/2,
    set_beam/3,
    apply_changes/2,
    do_data_exchange/3
]).

%% C++: BeamParamPosCtrl.cpp:104-117  OnInitDialog
%%      CDialog::OnInitDialog (boundary); set spin-button ranges:
%%        gantry/couch:  [0, 359] degrees
%%        tableX/Y/Z:    [-200, 200] mm (assumed mm; comment-free)
on_init_dialog(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - ready,
        gantry_range - range(0, 359),
        couch_range - range(0, 359),
        table_x_range - range(-200, 200),
        table_y_range - range(-200, 200),
        table_z_range - range(-200, 200)
    ]).

%% C++: BeamParamPosCtrl.cpp:76-84  SetBeam
%%      m_pBeam := pBeam; if window valid: UpdateData(FALSE) -> loads
%%      values FROM the bound CBeam into m_n* fields.
set_beam(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        pbeam - none,
        win - ready          %% no beam -> back to ready
    ]).
set_beam(StateIn, Beam, StateOut) :-
    %% UpdateData(FALSE) cascades into DoDataExchange with Save=false.
    do_data_exchange(StateIn.put(pbeam, Beam), false, S1),
    StateOut = S1.put(win, bound).

%% C++: BeamParamPosCtrl.cpp:119-128  OnApplyChanges
%%      Calls UpdateData(TRUE) -> DoDataExchange with Save=true ->
%%      writes values back into pBeam via SetGantryAngle/SetCouchAngle/
%%      SetTableOffset.
%%
%% Each of those three CBeam setters fires GetChangeEvent. So
%% on_apply_changes is one of the upstream causes of c_beam's
%% fires_change_event/1 fan-out.
apply_changes(StateIn, StateOut) :-
    do_data_exchange(StateIn, true, StateOut).

%% C++: BeamParamPosCtrl.cpp:32-74  DoDataExchange
%%      Save = false:
%%        m_nGantryAngle := (int)(pBeam->GetGantryAngle() * 180 / PI)
%%        m_nCouchAngle  := (int)(pBeam->GetCouchAngle()  * 180 / PI)
%%        vOffset := pBeam->GetTableOffset()
%%        m_nTableX/Y/Z := (int) vOffset[0]/[1]/[2]
%%      DDX_Text + DDV_MinMaxInt for each (always runs, regardless of Save).
%%      Save = true:
%%        pBeam->SetGantryAngle(m_nGantryAngle * PI / 180)
%%        pBeam->SetCouchAngle (m_nCouchAngle  * PI / 180)
%%        pBeam->SetTableOffset(CVectorD<3>(m_nTableX, m_nTableY, m_nTableZ))
do_data_exchange(StateIn, false, StateOut) :- !,
    %% Read FROM bound CBeam. Modeled symbolically (degrees-from-radians
    %% conversion handled by the boundary).
    ( StateIn.pbeam == none
    -> StateOut = StateIn          %% no-op: no bound beam
    ;  StateOut = StateIn.put([
           n_gantry_angle - degrees_from_beam(StateIn.pbeam, gantry),
           n_couch_angle - degrees_from_beam(StateIn.pbeam, couch),
           n_table_x - mm_from_beam(StateIn.pbeam, x),
           n_table_y - mm_from_beam(StateIn.pbeam, y),
           n_table_z - mm_from_beam(StateIn.pbeam, z)
       ])
    ).
do_data_exchange(StateIn, true, StateOut) :-
    %% Write TO bound CBeam. The boundary CBeam:: setters each fire
    %% GetChangeEvent (per c_beam_record:fires_change_event/1).
    %% State-local: nothing changes here -- the dialog's m_n* are the
    %% source of truth during edit.
    StateOut = StateIn.
