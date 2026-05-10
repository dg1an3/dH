%% Source: RT_VIEW/BeamParamCollimCtrl.cpp

:- module(edit_ops, [
    on_init_dialog/2,
    set_beam/3,
    apply_changes/2,
    do_data_exchange/3
]).

%% C++: BeamParamCollimCtrl.cpp:110-122  OnInitDialog
%%      Set spin ranges:
%%        collim:        [0, 359] degrees
%%        jawX1/X2/Y1/Y2: [-20, 20] cm (jaw is in cm; CBeam stores cm too)
on_init_dialog(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - ready,
        collim_range - range(0, 359),
        jaw_x1_range - range(-20, 20),
        jaw_x2_range - range(-20, 20),
        jaw_y1_range - range(-20, 20),
        jaw_y2_range - range(-20, 20)
    ]).

%% C++: BeamParamCollimCtrl.cpp:82-90  SetBeam
%%      m_pBeam = pBeam;
%%      if (IsWindow && !m_bUpdatingData) UpdateData(FALSE);
%% Re-entrancy guard: when m_bUpdatingData is true (set during the Save
%% branch of DoDataExchange), SetBeam DOES NOT call UpdateData(FALSE).
%% This breaks the re-entrancy loop: a CBeam observer reacting to
%% SetCollimAngle/SetCollimMin/SetCollimMax would otherwise re-enter
%% DoDataExchange in load mode mid-save.
set_beam(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        pbeam - none,
        win - ready
    ]).
set_beam(StateIn, Beam, StateOut) :-
    ( StateIn.updating_data == true
    -> %% guarded: skip the UpdateData(FALSE) refresh
       StateOut = StateIn.put(pbeam, Beam)
    ;  %% normal path: load FROM beam
       do_data_exchange(StateIn.put(pbeam, Beam), false, S1),
       StateOut = S1.put(win, bound)
    ).

%% C++: BeamParamCollimCtrl.cpp  OnApplyChanges -> UpdateData(TRUE)
apply_changes(StateIn, StateOut) :-
    do_data_exchange(StateIn, true, StateOut).

%% C++: BeamParamCollimCtrl.cpp:33-80  DoDataExchange
%%      Load (Save = false) at cpp:38-48:
%%        m_nCollimAngle := degrees from pBeam->GetCollimAngle()
%%        m_nJawX1/X2 := pBeam->GetCollimMin()[0] / GetCollimMax()[0]
%%        m_nJawY1/Y2 := pBeam->GetCollimMin()[1] / GetCollimMax()[1]
%%      Save (Save = true) at cpp:63-78:
%%        m_bUpdatingData = TRUE
%%        pBeam->SetCollimAngle(rad)
%%        pBeam->SetCollimMin(CVectorD<2>(m_nJawX1, m_nJawY1))
%%        pBeam->SetCollimMax(CVectorD<2>(m_nJawX2, m_nJawY2))
%%        m_bUpdatingData = FALSE
do_data_exchange(StateIn, false, StateOut) :- !,
    ( StateIn.pbeam == none
    -> StateOut = StateIn
    ;  StateOut = StateIn.put([
           n_collim_angle - degrees_from_beam(StateIn.pbeam, collim),
           n_jaw_x1 - cm_from_beam(StateIn.pbeam, collim_min, x),
           n_jaw_x2 - cm_from_beam(StateIn.pbeam, collim_max, x),
           n_jaw_y1 - cm_from_beam(StateIn.pbeam, collim_min, y),
           n_jaw_y2 - cm_from_beam(StateIn.pbeam, collim_max, y)
       ])
    ).
do_data_exchange(StateIn, true, StateOut) :-
    %% Save: set the re-entrancy flag, fan out to CBeam, clear the flag.
    %% Each SetCollim* fires GetChangeEvent (per c_beam fires_change_event).
    %% Modeled as a transient toggle on updating_data.
    S1 = StateIn.put(updating_data, true),
    %% (boundary CBeam writes happen here; observer-driven SetBeam calls
    %%  during this window would skip UpdateData(FALSE) due to the guard)
    StateOut = S1.put(updating_data, false).
