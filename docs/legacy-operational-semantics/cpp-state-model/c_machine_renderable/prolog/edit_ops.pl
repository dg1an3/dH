%% Source: RT_VIEW/MachineRenderable.cpp

:- module(edit_ops, [
    set_object/3,
    draw_opaque/2,
    draw_transparent/2
]).

%% C++: MachineRenderable.cpp:73-97  SetObject
set_object(StateIn, none, StateOut) :- !,
    StateOut = StateIn.put([
        pbeam - none,
        win - unbound,
        observer_attached - false
    ]).
set_object(StateIn, Beam, StateOut) :-
    %% Boundary: RemoveObserver+AddObserver. Observer registered
    %% with `Invalidate` as the callback (cpp:91-92).
    StateOut = StateIn.put([
        pbeam - Beam,
        win - bound,
        observer_attached - true
    ]).

%% C++: MachineRenderable.cpp:104-130  DrawOpaque
%%      if (m_bWireFrame): render in line mode.
%%      else: nothing.
%%      Since m_bWireFrame is always FALSE, the else branch always
%%      runs -- this is a state-pure no-op in practice.
draw_opaque(StateIn, StateOut) :-
    ( StateIn.wireframe_mode == true
    -> StateOut = StateIn.put([
           table_drawn - true,
           gantry_drawn - true,
           collimator_drawn - true,
           sad_cached - sad_from_machine,
           scd_cached - scd_from_machine,
           axis_to_collim_cached - sad_minus_scd,
           gantry_angle_cached - gantry_from_beam
       ])
    ;  StateOut = StateIn       %% the actual runtime path: no-op
    ).

%% C++: MachineRenderable.cpp:137-156  DrawTransparent
%%      Unconditionally renders table + gantry + collimator.
draw_transparent(StateIn, StateOut) :-
    StateOut = StateIn.put([
        table_drawn - true,
        gantry_drawn - true,
        collimator_drawn - true,
        sad_cached - sad_from_machine,
        scd_cached - scd_from_machine,
        axis_to_collim_cached - sad_minus_scd,
        gantry_angle_cached - gantry_from_beam
    ]).
