%% Source: VSIM_OGL/simview.cpp
%%
%% Observer-pattern boundary declarations. CSimView attaches and detaches
%% itself as a listener on CBeam::GetChangeEvent() in SetCurrentBeam (cpp:87).
%% These predicates fail by default; a future test harness mocks them to
%% drive simulated `beam_changed` events at deterministic points in the LTS.

:- module(observer_boundary, [
    add_observer/3,
    remove_observer/3
]).

%% C++: simview.cpp:102-103  ::AddObserver<CSimView>(&GetCurrentBeam()->GetChangeEvent(),
%%                                                    this, OnBeamChanged)
%% kind: in-process callback registration (template helper)
%% tables: none (no DB)
add_observer(_Event, _Listener, _Method) :- fail.

%% C++: simview.cpp:92-93  ::RemoveObserver<CSimView>(&GetCurrentBeam()->GetChangeEvent(),
%%                                                     this, OnBeamChanged)
%% kind: in-process callback deregistration
%% tables: none
remove_observer(_Event, _Listener, _Method) :- fail.
