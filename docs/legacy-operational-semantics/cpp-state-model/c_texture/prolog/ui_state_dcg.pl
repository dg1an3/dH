%% Source: GEOM_VIEW/include/Texture.h, GEOM_VIEW/Texture.cpp
%%
%% LTS for CTexture -- texture-mapping primitive for CRenderable.
%% Base class for CLightfieldTexture (RT_VIEW). Owns a GDI bitmap
%% surface (drawn into via GetDC/DrawBorder/ReleaseDC) plus an OpenGL
%% texture handle.
%%
%% Glue-medium. OBSERVABLE -- m_eventChange fires only on SetProjection.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1,
    fires_change_event/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   pre_construct
%%   ready          -- ctor ran; no DC currently held
%%   dc_held        -- GetDC selected the bitmap into a DC; ReleaseDC pending
%%   bound          -- CRenderContext::Bind selected the texture for rendering
%%   destroyed

%% ============================================================================
%% Event alphabet
%% ============================================================================
%%   construct                              -- Texture.cpp:26
%%   set_width_height(W, H)                 -- Texture.cpp:47
%%   set_wrap(B)                             -- Texture.cpp:65
%%   get_dc                                  -- Texture.cpp:70   selects bitmap into DC
%%   draw_border(W)                          -- Texture.cpp:93   draws GDI rect
%%   release_dc                              -- Texture.cpp:108  deselects bitmap
%%   process_transparency                    -- Texture.cpp:129  protected helper
%%   load_bitmap(ResId)                      -- Texture.cpp:151
%%   set_projection(M)                       -- Texture.cpp:248  fires change!
%%   bind(View)                              -- Texture.cpp:178  friend-only: CRenderContext
%%   unbind                                  -- Texture.cpp:222
%%   destruct                                -- Texture.cpp:38

initial_state(state{
    win:                pre_construct,
    pview:              none,
    width:              0,
    height:             0,
    wrap:               false,
    handle:             -1,                 %% OpenGL texture handle, -1 = unallocated
    %% GDI DC state
    pdc:                none,
    pbitmap:            none,
    pold_bitmap:        none,
    pixels:             [],
    %% projection matrix
    projection:         identity_4x4
}).

step(S0, S1) --> [construct],
    { S0.win == pre_construct,
      S1 = S0.put(win, ready) }.

%% C++: Texture.cpp:47-58  SetWidthHeight
%%      Allocates m_pBitmap as nWidth x nHeight, sizes m_arrPixels.
step(S0, S1) --> [set_width_height(W, H)],
    { S0.win == ready,
      S1 = S0.put([
          width - W,
          height - H,
          pbitmap - allocated,
          pixels - sized_w_times_h
      ]) }.

%% C++: Texture.cpp:65  SetWrap
step(S0, S1) --> [set_wrap(B)],
    { S0.win == ready,
      S1 = S0.put(wrap, B) }.

%% C++: Texture.cpp:70-91  GetDC
%%      Creates a CDC, selects m_pBitmap, stashes the previous bitmap
%%      in m_pOldBitmap. Returns the DC pointer for caller drawing.
step(S0, S1) --> [get_dc],
    { S0.win == ready,
      S0.pbitmap \= none,
      S1 = S0.put([
          win - dc_held,
          pdc - active_dc,
          pold_bitmap - previous_bitmap
      ]) }.

%% C++: Texture.cpp:93-106  DrawBorder
%%      Draws an nWidth-pixel rectangle border on the texture surface
%%      via the held DC.
step(S0, S0) --> [draw_border(_W)],
    { S0.win == dc_held }.

%% C++: Texture.cpp:108-127  ReleaseDC
%%      Re-selects m_pOldBitmap, deletes the DC, calls ProcessTransparency.
step(S0, S1) --> [release_dc],
    { S0.win == dc_held,
      S1 = S0.put([
          win - ready,
          pdc - none,
          pold_bitmap - none
      ]) }.

%% C++: Texture.cpp:129-149  ProcessTransparency
%%      Walks m_arrPixels; pixels matching a transparency key are
%%      replaced with alpha=0. Protected helper called from ReleaseDC.
step(S0, S0) --> [process_transparency],
    { S0.win == ready ; S0.win == dc_held }.

%% C++: Texture.cpp:151-176  LoadBitmap
%%      Loads a bitmap resource into m_pBitmap, sizes m_arrPixels.
step(S0, S1) --> [load_bitmap(_ResId)],
    { S0.win == ready,
      S1 = S0.put([
          pbitmap - loaded_from_resource,
          pixels - filled_from_bitmap
      ]) }.

%% C++: Texture.cpp:248-253  SetProjection
%%      FIRES GetChangeEvent (the only setter that does).
step(S0, S1) --> [set_projection(M)],
    { (S0.win == ready ; S0.win == bound),
      S1 = S0.put(projection, M) }.

%% C++: Texture.cpp:178-220  Bind(CSceneView*) -- friend-only
%%      Generates OpenGL texture handle if needed, glBindTexture,
%%      uploads pixel data via glTexImage2D, sets wrap mode.
step(S0, S1) --> [bind(View)],
    { S0.win == ready,
      S1 = S0.put([
          win - bound,
          pview - View,
          handle - allocated_gl_handle
      ]) }.

%% C++: Texture.cpp:222-241  Unbind
%%      glBindTexture(0) to unbind.
step(S0, S1) --> [unbind],
    { S0.win == bound,
      S1 = S0.put([
          win - ready,
          pview - none
      ]) }.

step(S0, S1) --> [destruct],
    { member(S0.win, [ready, bound, dc_held]),
      S1 = S0.put(win, destroyed) }.

%% ============================================================================
%% Which mutators fire the observable change event
%% ============================================================================
fires_change_event(set_projection(_)).
%% None of: SetWidthHeight, SetWrap, LoadBitmap, Bind, Unbind, GetDC,
%% ReleaseDC, DrawBorder fire the event. Texture's observable surface
%% is the projection matrix only. Quirk: changing the bitmap content
%% via the GetDC -> draw -> ReleaseDC path is silent.

valid_next(State, Event) :-
    member(Event, [
        construct, set_width_height(_, _), set_wrap(_),
        get_dc, draw_border(_), release_dc, process_transparency,
        load_bitmap(_), set_projection(_), bind(_), unbind, destruct
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
%%   1. **Asymmetric observability.** Only SetProjection fires
%%      GetChangeEvent (cpp:252). Mutating the bitmap content via
%%      GetDC/DrawBorder/ReleaseDC is silent -- a CLightfieldTexture
%%      subclass that redraws the surface inside OnBeamChanged
%%      cannot rely on the base class to notify downstream observers
%%      of the content change; only projection updates are signaled.
%%      Compare to CCamera (8 setters fire) and CBeam (9 setters fire).
%%
%%   2. **GDI + OpenGL coexist.** The class uses GDI primitives (CDC,
%%      CBitmap, Rectangle, SelectObject) for drawing INTO the texture
%%      surface, then uploads the bitmap bits to an OpenGL texture
%%      handle for rendering. The two graphics stacks coexist in one
%%      class -- a Windows-only pattern.
%%
%%   3. **DC state must be balanced.** GetDC must be paired with
%%      ReleaseDC; the state machine here enforces it via the dc_held
%%      sub-state, but the C++ doesn't. Calling GetDC twice in a row
%%      would leak the first DC.
%%
%%   4. **OpenGL despite the surrounding D3D8.** CRenderContext is
%%      "D3D8 device holder + OpenGL renderer"; CTexture is similarly
%%      OpenGL-based (glBindTexture, glTexImage2D). The CLight class
%%      went full D3D8 (D3DLIGHT8 inheritance) but the texture system
%%      didn't migrate.
