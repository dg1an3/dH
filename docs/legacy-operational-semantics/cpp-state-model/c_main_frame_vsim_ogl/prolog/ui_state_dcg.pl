%% Source: VSIM_OGL/MainFrm.h, VSIM_OGL/MainFrm.cpp
%%
%% LTS for CMainFrame (CFrameWnd subclass) -- the VSIM_OGL main frame.
%% Glue-medium target: small message map (4 entries), heavy OnCreate
%% wiring up the docking control bars (toolbar, status, beam param tab
%% control with two embedded controls, object explorer).

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
%%   created     -- OnCreate succeeded; all bars created and docked
%%   closing     -- OnClose entered (saves bar state)
%%   closed      -- frame destroyed

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   on_create                  -- MainFrm.cpp:23  ON_WM_CREATE -> OnCreate
%%   pre_create_window          -- MainFrm.cpp:151 PreCreateWindow virtual
%%   on_close                   -- MainFrm.cpp:26  ON_WM_CLOSE -> OnClose
%%   on_bar_check(BarId)        -- MainFrm.cpp:24  ON_COMMAND_EX(ID_VIEW_BEAMPARAM, OnBarCheck)
%%   on_update_control_bar_menu -- MainFrm.cpp:25  ON_UPDATE_COMMAND_UI (read-only)
%%
%% NOTE: OnBarCheck and OnUpdateControlBarMenu are framework-provided
%% on CFrameWnd; this class does not override them. The events are still
%% in the alphabet so cross-class queries (e.g. "user toggled the beam
%% param bar from the View menu") have a name to reference.

%% ============================================================================
%% Initial state
%% ============================================================================
%%
%% C++: MainFrm.cpp:41-48  CMainFrame::CMainFrame()
%%      : m_bViewBeam(TRUE), m_bViewLightpatch(FALSE), m_pExplorerFont(NULL)
initial_state(state{
    win:                          pre_create,
    %% docking control bars; visibility tracked at the LTS surface
    toolbar_visible:              false,
    status_bar_visible:           false,
    beam_param_bar_visible:       false,
    explorer_bar_visible:         false,
    %% sub-controls inside the docking bars
    pos_ctrl_created:             false,
    collim_ctrl_created:          false,
    object_explorer_created:      false,
    explorer_font:                none,
    %% tree-item class registrations (MainFrm.cpp:127-130)
    mesh_renderable_registered:   false,
    beam_renderable_registered:   false,
    %% bar state restored from registry
    bar_state_loaded:             false,
    %% --- stale fields preserved verbatim (initialized in ctor, never read/written) ---
    %% C++: MainFrm.h:64  BOOL m_bViewLightpatch
    view_lightpatch:              false,
    %% C++: MainFrm.h:65  BOOL m_bViewBeam (init TRUE)
    view_beam:                    true
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: MainFrm.cpp:151-159  PreCreateWindow
%%      Calls CFrameWnd::PreCreateWindow; no state mutation.
step(S0, S0) --> [pre_create_window],
    { S0.win == pre_create }.

%% C++: MainFrm.cpp:55-149  OnCreate
%%      Heavy wiring: toolbar.CreateEx, status bar Create, BeamParam
%%      tab control bar Create, embed PosCtrl + CollimCtrl, ExplorerCtrl
%%      Create, ObjectExplorer Create + font, register CMesh + CBeam
%%      tree-item classes, EnableDocking + DockControlBar x3,
%%      LoadBarState("ControlBars").
step(S0, S1) --> [on_create],
    { S0.win == pre_create,
      edit_ops:on_create(S0, S1) }.

%% C++: MainFrm.cpp:24  ON_COMMAND_EX(ID_VIEW_BEAMPARAM, OnBarCheck)
%%      Framework-provided handler that toggles the docking bar's
%%      visibility. Event terminal carries the bar id so cross-class
%%      queries can distinguish toolbar vs beam-param vs explorer toggles.
step(S0, S1) --> [on_bar_check(BarId)],
    { browse_ops:is_created(S0),
      edit_ops:toggle_bar(S0, BarId, S1) }.

%% C++: MainFrm.cpp:25  ON_UPDATE_COMMAND_UI(ID_VIEW_BEAMPARAM, OnUpdateControlBarMenu)
%%      Read-only handler; framework-provided. Computes Enable/Check.
%%      No state mutation; sits in browse_ops.
step(S0, S0) --> [on_update_control_bar_menu(_BarId)].

%% C++: MainFrm.cpp:180-186  OnClose
%%      SaveBarState("ControlBars") then CFrameWnd::OnClose.
step(S0, S1) --> [on_close],
    { browse_ops:is_created(S0),
      edit_ops:on_close(S0, S1) }.

%% Implicit destruction (WM_DESTROY -> dtor at MainFrm.cpp:50).
%% Modeled as a separate event to keep the LTS terminal explicit.
step(S0, S1) --> [destruct],
    { S0.win == closing,
      S1 = S0.put([
          win - closed,
          explorer_font - none      %% delete m_pExplorerFont (cpp:52)
      ]) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        pre_create_window, on_create,
        on_bar_check(toolbar), on_bar_check(beam_param_bar), on_bar_check(explorer_bar),
        on_update_control_bar_menu(_),
        on_close, destruct
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
%%   1. m_bViewLightpatch (FALSE) and m_bViewBeam (TRUE) at MainFrm.h:64-65
%%      are initialized in the ctor and NEVER read or written elsewhere.
%%      Stale fields preserved for memory-layout fidelity. The "view
%%      lightpatch / view beam" command IDs are handled by CSimView, not
%%      here -- these flags appear to be left-over from an earlier design
%%      where the frame owned the toggle state.
%%
%%   2. The OnBarCheck/OnUpdateControlBarMenu pair handles the standard
%%      MFC docking-bar visibility toggle; both are framework-provided
%%      base class methods. This class does not override either.
