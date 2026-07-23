%% Source: VSIM_OGL/MainFrm.cpp
%%
%% Mutating operations for CMainFrame.

:- module(edit_ops, [
    on_create/2,
    on_close/2,
    toggle_bar/3
]).

%% C++: MainFrm.cpp:55-149  CMainFrame::OnCreate
%%      All bar/explorer creation + tree-item class registration +
%%      docking + LoadBarState.
on_create(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - created,
        toolbar_visible - true,                   %% cpp:60-62  CreateEx + WS_VISIBLE
        status_bar_visible - true,                %% cpp:68
        beam_param_bar_visible - true,            %% cpp:76-79
        explorer_bar_visible - true,              %% cpp:90-93
        pos_ctrl_created - true,                  %% cpp:84-85
        collim_ctrl_created - true,               %% cpp:87-88
        object_explorer_created - true,           %% cpp:104-110
        explorer_font - arial_18,                 %% cpp:116-123
        mesh_renderable_registered - true,        %% cpp:127-128
        beam_renderable_registered - true,        %% cpp:129-130
        bar_state_loaded - true                   %% cpp:146  LoadBarState("ControlBars")
    ]).

%% C++: MainFrm.cpp:180-186  CMainFrame::OnClose
%%      SaveBarState("ControlBars") then CFrameWnd::OnClose.
on_close(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - closing,
        bar_state_loaded - false                  %% bar state saved (boundary write)
    ]).

%% C++: ON_COMMAND_EX(ID_VIEW_BEAMPARAM, OnBarCheck)  (framework default)
%%      Toggles the named bar's visibility flag. Modeled here per-bar
%%      so cross-class queries can distinguish "user toggled toolbar"
%%      from "user toggled beam param bar".
toggle_bar(StateIn, toolbar, StateOut) :-
    flip(StateIn.toolbar_visible, V),
    StateOut = StateIn.put(toolbar_visible, V).
toggle_bar(StateIn, beam_param_bar, StateOut) :-
    flip(StateIn.beam_param_bar_visible, V),
    StateOut = StateIn.put(beam_param_bar_visible, V).
toggle_bar(StateIn, explorer_bar, StateOut) :-
    flip(StateIn.explorer_bar_visible, V),
    StateOut = StateIn.put(explorer_bar_visible, V).
toggle_bar(StateIn, status_bar, StateOut) :-
    flip(StateIn.status_bar_visible, V),
    StateOut = StateIn.put(status_bar_visible, V).

flip(false, true).
flip(true, false).
