%% Source: VSIM_OGL/simview.h, VSIM_OGL/simview.cpp
%%
%% Mirrors CObArray m_arrSurfaceRenderables (simview.h:84) -- the array of
%% CSurfaceRenderable* held by CSimView, populated by OnInitialUpdate and
%% iterated by OnBeamChanged.
%%
%% Records are SWI-Prolog dicts with tag `surface_renderable`. The list is
%% loaded once from CSeries::GetStructureAt(i) in OnInitialUpdate; element 0
%% is also pinned to state.patient_surface_renderable.

:- module(surface_renderable_list, [
    load_surface_renderable_list/2,     % +Series, -List
    apply_modelview/3,                  % +ModelviewMatrix, +ListIn, -ListOut
    patient_surface/2                   % +List, -PatientRenderable
]).

%% C++ struct-equivalent fields per renderable (dict keys):
%%
%%   id       : opaque atom (identity of the source CMesh structure)
%%   color    : COLORREF (arrColors[i % 13] from simview.cpp:30)
%%   icon_id  : UINT     (arrIconResourceIDs[i % 13] from simview.cpp:46)
%%   modelview: 4x4 matrix term (initially identity; set by OnBeamChanged)

%% C++: simview.cpp:339-372  for (nAtSurf = 0; ... pSeries->GetStructureCount(); ...)
%%      The CSurfaceRenderable for each structure is created via
%%      pNewItem->CreateRenderableForObject(&m_wndREV) and pinned with
%%      arrColors[nAt] / arrIconResourceIDs[nAt%13].
load_surface_renderable_list(Series, List) :-
    findall(R, source_surface(Series, R), List).

source_surface(Series, surface_renderable{
        id: Id,
        color: Color,
        icon_id: IconId,
        modelview: identity
    }) :-
    member(Idx-Id, Series.structures),
    nth_color(Idx, Color),
    nth_icon(Idx, IconId).

%% C++: simview.cpp:30-44  COLORREF arrColors[]
nth_color(I, C) :- I0 is I mod 13, nth0(I0, [
    rgb(192,192,192), rgb(240,0,0), rgb(0,240,0), rgb(0,0,255),
    rgb(255,0,255),   rgb(0,255,255), rgb(240,240,0),
    rgb(240,0,0),     rgb(0,240,0),  rgb(0,0,255),
    rgb(255,0,255),   rgb(0,255,255), rgb(240,240,0)
], C).

%% C++: simview.cpp:46-61  UINT arrIconResourceIDs[]
nth_icon(I, Icon) :- I0 is I mod 13, nth0(I0, [
    idb_contour_grey, idb_contour_red, idb_contour_green, idb_contour_blue,
    idb_contour_magenta, idb_contour_cyan, idb_contour_yellow,
    idb_contour_red, idb_contour_green, idb_contour_blue,
    idb_contour_magenta, idb_contour_cyan, idb_contour_yellow
], Icon).

%% C++: simview.cpp:301-308  for (nAt = 0; nAt < m_arrSurfaceRenderables.GetSize(); ...)
%%      pSR->SetModelviewMatrix(mMV)
%%      Pure-functional analogue: rebuild the list with the new matrix
%%      stamped into each record's modelview field.
apply_modelview(_Mv, [], []).
apply_modelview(Mv, [R | Rest], [R2 | Rest2]) :-
    R2 = R.put(modelview, Mv),
    apply_modelview(Mv, Rest, Rest2).

%% C++: simview.cpp:363-371  if (nAtSurf == 0) m_pSurfaceRenderable = pSurfaceRenderable
patient_surface(List, Patient) :-
    nth0(0, List, Patient).
