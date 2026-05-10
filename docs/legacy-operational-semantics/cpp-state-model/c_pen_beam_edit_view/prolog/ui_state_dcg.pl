%% Source: PenBeamEdit/PenBeamEditView.h, PenBeamEdit/PenBeamEditView.cpp
%%
%% LTS for CPenBeamEditView (CView subclass). The "view" half of the
%% PenBeamEdit testbed -- shows a colormapped dose+density rendering
%% on the left and a cumulative DVH graph on the right (CGraph child
%% window). OnUpdate is the key path: it (re)builds the histogram
%% from CPlan's dose matrix and the structure region, then refreshes
%% the graph data series.
%%
%% Pairs with PenBeamEdit/TransitionPlan.txt (modeled separately in
%% transition_plan.pl) -- this file's adoption of CDib at OnCreate
%% is one of the X-marked items.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

:- use_module(browse_ops).
:- use_module(edit_ops).
:- use_module(transition_plan).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   pre_create    -- ctor ran; OnCreate not yet
%%   created       -- OnCreate succeeded; graph child + colormap loaded
%%   plan_bound    -- OnUpdate has built the histogram (post first plan-update)
%%   closed        -- view destroyed

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   pre_create_window     -- PenBeamEditView.cpp:45  PreCreateWindow
%%   on_create             -- PenBeamEditView.cpp:23  ON_WM_CREATE
%%   on_size(Cx, Cy)       -- PenBeamEditView.cpp:24  ON_WM_SIZE
%%   on_update(Hint)       -- PenBeamEditView.cpp:189  OnUpdate virtual
%%   on_draw               -- PenBeamEditView.cpp:56  OnDraw virtual
%%   on_file_print         -- PenBeamEditView.cpp:27  forwarded
%%   on_file_print_direct  -- PenBeamEditView.cpp:28  forwarded
%%   on_file_print_preview -- PenBeamEditView.cpp:29  forwarded
%%   on_prepare_printing   -- PenBeamEditView.cpp:113  OnPreparePrinting
%%   on_lbutton_down(Pt)   -- PenBeamEditView.h:68  DECLARED but no impl,
%%                              and not in the message map. ORPHAN.
%%   destruct              -- ~CPenBeamEditView (cpp:41-43, empty body)

%% ============================================================================
%% Initial state
%% ============================================================================
initial_state(state{
    win:                       pre_create,
    %% colormap (from IDB_RAINBOW resource), loaded in OnCreate
    colormap_loaded:           false,
    colormap_size:             0,
    %% graph child window
    graph_window_created:      false,
    graph_data_series_count:   0,
    %% histogram (composed CHistogram)
    histogram_volume_set:      none,
    histogram_region_set:      none,
    %% layout
    last_size:                 size(0, 0)
}).

%% ============================================================================
%% Step rules
%% ============================================================================

%% C++: PenBeamEditView.cpp:45-51  PreCreateWindow
step(S0, S0) --> [pre_create_window],
    { S0.win == pre_create }.

%% C++: PenBeamEditView.cpp:153-179  OnCreate
%%      Creates m_graph child window (200x200, ID 111).
%%      Loads IDB_RAINBOW via CDib.Load.   <-- TransitionPlan step 2 evidence
%%      Builds m_arrColormap from raw bitmap bits, RGB triplets reversed.
step(S0, S1) --> [on_create],
    { S0.win == pre_create,
      edit_ops:on_create(S0, S1) }.

%% C++: PenBeamEditView.cpp:181-187  OnSize
%%      Repositions m_graph to (cx/2, 0, cx/2, cy) -- right half of view.
step(S0, S1) --> [on_size(Cx, Cy)],
    { browse_ops:is_created_or_bound(S0),
      edit_ops:on_size(S0, Cx, Cy, S1) }.

%% C++: PenBeamEditView.cpp:189-218  OnUpdate
%%      Sets histogram volume + region from CPlan, fetches cumulative
%%      bins, replaces graph data series with one new CDataSeries
%%      populated by 256 (intensity, percentage) data points.
step(S0, S1) --> [on_update(_Hint)],
    { browse_ops:is_created_or_bound(S0),
      edit_ops:on_update(S0, S1) }.

%% C++: PenBeamEditView.cpp:56-108  OnDraw
%%      Iterates over volume voxels; for each draws a rectangle whose
%%      color is colormap[dose_index] modulated by density intensity.
%%      State-pure (renders to DC).
step(S0, S0) --> [on_draw],
    { browse_ops:is_created_or_bound(S0) }.

%% C++: PenBeamEditView.cpp:113-117  OnPreparePrinting
step(S0, S0) --> [on_prepare_printing],
    { browse_ops:is_created_or_bound(S0) }.

%% C++: forwarded to CView base class (cpp:27-29)
step(S0, S0) --> [on_file_print],
    { browse_ops:is_created_or_bound(S0) }.
step(S0, S0) --> [on_file_print_direct],
    { browse_ops:is_created_or_bound(S0) }.
step(S0, S0) --> [on_file_print_preview],
    { browse_ops:is_created_or_bound(S0) }.

%% C++: PenBeamEditView.h:68  afx_msg void OnLButtonDown(UINT, CPoint);
%% ORPHAN: declared in the .h but NOT defined in the .cpp AND NOT in the
%% message map. The framework default OnLButtonDown handles WM_LBUTTONDOWN.
%% Preserved verbatim as an event terminal that produces no state change.
step(S0, S0) --> [on_lbutton_down(_Pt)],
    { browse_ops:is_created_or_bound(S0) }.

step(S0, S1) --> [destruct],
    { browse_ops:is_created_or_bound(S0),
      S1 = S0.put(win, closed) }.

%% ============================================================================
%% Sanity helpers
%% ============================================================================
valid_next(State, Event) :-
    member(Event, [
        pre_create_window, on_create, on_size(_, _),
        on_update(_), on_draw, on_prepare_printing,
        on_file_print, on_file_print_direct, on_file_print_preview,
        on_lbutton_down(_), destruct
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
%%   1. PenBeamEditView.h:68  afx_msg void OnLButtonDown declared but
%%      NEVER defined in the .cpp AND not in the BEGIN_MESSAGE_MAP block
%%      (cpp:21-30). Orphan declaration. Likely a planned interactive
%%      feature (per TransitionPlan.txt aspirations) that was never
%%      implemented.
%%
%%   2. PenBeamEditView.cpp:80-98  #ifdef SHOW_DOSE / #else / #endif.
%%      SHOW_DOSE is defined immediately above (cpp:80) so the dose
%%      rendering path is always taken. The #else branch (pure density
%%      grayscale) is dead code preserved verbatim.
%%
%%   3. PenBeamEditView.cpp:174  RGB(pRaw[nAtRaw+2], pRaw[nAtRaw+1], pRaw[nAtRaw])
%%      The colormap raw bits are read in BGR order, swizzled to RGB.
%%      Standard for Win32 DIB, but a reviewer might mistake it for
%%      a bug.
%%
%%   4. PenBeamEditView.cpp:90-92  the dose-color modulation by density
%%      intensity reads `intensity * GetRValue(color)` etc. -- this is
%%      a physically-questionable blend (proper would be CT window/level
%%      then alpha-blend). The blend formula is preserved verbatim
%%      because it's load-bearing for visual matching.
%%
%%   5. PenBeamEditView.cpp:201-202  GetStructureAt(0) is hard-coded;
%%      the histogram always uses structure 0. Preserved verbatim --
%%      the testbed assumes a single structure (the strip-region
%%      synthesized by OnFileImport).
