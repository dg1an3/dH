%% Source: PenBeamEdit/PenBeamEditView.cpp

:- module(edit_ops, [
    on_create/2,
    on_size/4,
    on_update/2
]).

%% C++: PenBeamEditView.cpp:153-179  CPenBeamEditView::OnCreate
%%      m_graph.Create(NULL, NULL, WS_BORDER|WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
%%                     CRect(0, 0, 200, 200), this, 111);
%%      colormap.Load(GetModuleHandle(NULL), IDB_RAINBOW);   <-- TransitionPlan step 2
%%      m_arrColormap.SetSize(size.cx * size.cy);
%%      Read raw 24-bit pixels and convert BGR -> RGB into m_arrColormap.
on_create(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - created,
        graph_window_created - true,
        colormap_loaded - true,
        colormap_size - 256                       %% IDB_RAINBOW is typically 256-color strip
    ]).

%% C++: PenBeamEditView.cpp:181-187  OnSize
%%      m_graph.MoveWindow(cx/2, 0, cx/2, cy)  -- right half
on_size(StateIn, Cx, Cy, StateOut) :-
    StateOut = StateIn.put(last_size, size(Cx, Cy)).

%% C++: PenBeamEditView.cpp:189-218  OnUpdate
%%      Sets histogram volume = pPlan->GetDoseMatrix()
%%      Sets histogram region = pStructure[0]->GetRegion()
%%      Builds new CDataSeries from cumulative histogram bins
%%      m_graph.RemoveAllDataSeries; m_graph.AddDataSeries(pSeries)
%%      Loop: 256 data points (intensity = 1000*nAt/256, percent = bins[nAt]*100)
on_update(StateIn, StateOut) :-
    StateOut = StateIn.put([
        win - plan_bound,
        histogram_volume_set - dose_matrix,
        histogram_region_set - structure_zero_region,
        graph_data_series_count - 1               %% RemoveAll then Add one
    ]).
