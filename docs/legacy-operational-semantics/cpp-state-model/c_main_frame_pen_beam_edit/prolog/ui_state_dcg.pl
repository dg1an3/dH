%% Source: PenBeamEdit/MainFrm.h, PenBeamEdit/MainFrm.cpp
%%
%% LTS for CMainFrame (PenBeamEdit's CFrameWnd subclass). Glue-medium
%% but on the lighter side: only ON_WM_CREATE in the message map, just
%% toolbar + status bar (no docking control bars, no object explorer,
%% no beam-param tab control). About 1/3 the OnCreate complexity of
%% VSIM_OGL's MainFrame.

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
%%   pre_create  -- ctor ran; OnCreate not yet
%%   created     -- OnCreate succeeded; toolbar+status created and docked
%%   closed      -- frame destroyed
%%
%% Note: NO ON_WM_CLOSE handler in this MainFrame (compare VSIM_OGL's
%% which saves bar state on close). The framework's default OnClose
%% handles it; no custom save logic.

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   pre_create_window      -- MainFrm.cpp:79  PreCreateWindow virtual
%%   on_create              -- MainFrm.cpp:24  ON_WM_CREATE -> OnCreate
%%   destruct               -- ~CMainFrame (cpp:45-47, empty body)

%% ============================================================================
%% Initial state
%% ============================================================================
initial_state(state{
    win:                 pre_create,
    %% PenBeamEdit MainFrame has only two control bars (no object
    %% explorer, no beam param tab control).
    toolbar_created:     false,
    status_bar_created:  false,
    docking_enabled:     false
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: MainFrm.cpp:79-87  PreCreateWindow
step(S0, S0) --> [pre_create_window],
    { S0.win == pre_create }.

%% C++: MainFrm.cpp:49-77  OnCreate
%%      CFrameWnd::OnCreate; m_wndToolBar.CreateEx + LoadToolBar(IDR_MAINFRAME);
%%      m_wndStatusBar.Create + SetIndicators; EnableDocking; DockControlBar.
step(S0, S1) --> [on_create],
    { S0.win == pre_create,
      edit_ops:on_create(S0, S1) }.

%% C++: MainFrm.cpp:45-47  ~CMainFrame (empty body)
step(S0, S1) --> [destruct],
    { browse_ops:is_created(S0),
      S1 = S0.put(win, closed) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [pre_create_window, on_create, destruct]),
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
%%   1. No ON_WM_CLOSE handler. VSIM_OGL's MainFrame saves bar state in
%%      OnClose; PenBeamEdit's does not. The framework default OnClose
%%      runs, and bar state is NOT persisted. Preserved verbatim.
%%
%%   2. Empty constructor and destructor (cpp:39-43, 45-47). The
%%      embedded m_wndToolBar/m_wndStatusBar own their resources via
%%      stack-style composition; the dtor is empty.
