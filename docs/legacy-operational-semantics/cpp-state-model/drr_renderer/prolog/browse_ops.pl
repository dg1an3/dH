%% Source: VSIM_OGL/DRRRenderer.cpp, VSIM_OGL/DRRRenderer.h
%%
%% Read-only operations for CDRRRenderer: state classifiers and pure-functional
%% derivations (no UPDATE_COMMAND_UI handlers since this is not a Win32 widget).

:- module(browse_ops, [
    drr_state/1,
    is_unbound/1,
    is_dirty/1,
    is_computed/1,
    is_bg_computing/1,
    needs_recompute/1,
    derive_image_size/2
]).

%% State-tier classifier.
drr_state(unbound).
drr_state(dirty).
drr_state(computed).
drr_state(bg_computing).

%% C++: DRRRenderer.cpp:466  if (forVolume.Get() != NULL)
%% Renderer is "unbound" iff no volume association is set.
is_unbound(State)      :- State.win == unbound.
is_dirty(State)        :- State.win == dirty.
is_computed(State)     :- State.win == computed.
is_bg_computing(State) :- State.win == bg_computing.

%% C++: DRRRenderer.cpp:62  m_bRecomputeDRR(TRUE)   in ctor
%% C++: DRRRenderer.cpp:478  if (m_bRecomputeDRR)   in DrawScene
%% C++: DRRRenderer.cpp:559  m_bRecomputeDRR = TRUE in OnChange
%% Recompute-needed iff the dirty flag is set or no pixel array exists yet.
needs_recompute(State) :- State.recompute == true, !.
needs_recompute(State) :- State.pixels == [].

%% C++: DRRRenderer.cpp:484-485  m_nImageWidth = rect.Width() / m_nResDiv;
%%                               m_nImageHeight = rect.Height() / m_nResDiv;
derive_image_size(State, image_size{w: W, h: H}) :-
    Rect = State.client_rect,
    Div = State.n_res_div,
    W is Rect.width div Div,
    H is Rect.height div Div.
