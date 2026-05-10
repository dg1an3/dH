%% Source: PenBeamEdit/MainFrm.cpp

:- module(edit_ops, [
    on_create/2
]).

%% C++: MainFrm.cpp:49-77  CMainFrame::OnCreate
%%      CFrameWnd::OnCreate; toolbar.CreateEx(TBSTYLE_FLAT, ...)
%%      + LoadToolBar(IDR_MAINFRAME); status bar Create + SetIndicators
%%      (4-element indicators array at cpp:28-34); EnableDocking
%%      (CBRS_ALIGN_ANY); DockControlBar(toolbar).
on_create(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - created,
        toolbar_created - true,
        status_bar_created - true,
        docking_enabled - true
    ]).
