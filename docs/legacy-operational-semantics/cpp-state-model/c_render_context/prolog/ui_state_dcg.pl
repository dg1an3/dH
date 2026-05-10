%% Source: GEOM_VIEW/include/RenderContext.h, GEOM_VIEW/RenderContext.cpp
%%
%% LTS for CRenderContext -- the "rendering API" object that classes
%% like CRenderable use to issue primitives. Glue-medium with a
%% notable preserved quirk: **half-migrated**. The class holds a
%% LPDIRECT3DDEVICE8 pointer but all rendering methods still call
%% OpenGL functions (glBegin, glVertex, glNormal, glMatrixMode, etc.)
%% Only CreateVertexBuffer actually uses the D3D8 device. So D3D
%% device + OpenGL rendering coexist incongruously.
%%
%% Used as an immediate-mode API by the dead Draw* helpers in
%% CBeamRenderable (RT_VIEW). Pattern: BeginX -> Vertex/Normal* -> End.

:- module(ui_state_dcg, [
    initial_state/1,
    step//2,
    valid_next/2,
    valid_sequence/1
]).

%% ============================================================================
%% State alphabet
%% ============================================================================
%%   active   -- ctor ran; ready to emit primitives
%%   primitive_open(Kind) -- inside a BeginX/End pair

%% ============================================================================
%% Event alphabet (categorical buckets; ~25 individual methods)
%% ============================================================================
%%   construct(Pd3dDevice)            -- RenderContext.cpp:23
%%   create_vertex_buffer(N, FVF)     -- RenderContext.cpp:36   uses D3D8
%%   begin(Kind)                       -- RenderContext.cpp:46-74  glBegin
%%   end                               -- RenderContext.cpp:76      glEnd
%%   vertex(V)                         -- RenderContext.cpp:81-94   glVertex*
%%   normal(N)                         -- RenderContext.cpp:96-104  glNormal*
%%   triangles_from_surface(Mesh)     -- RenderContext.cpp:120    glDraw*Elements
%%   set_color(C)                     -- RenderContext.cpp:168    glColor4f
%%   set_alpha(A)                     -- RenderContext.cpp:176
%%   set_smooth_shading(B)            -- RenderContext.cpp:185    glShadeModel
%%   set_lighting(B)                  -- RenderContext.cpp:191    glEnable/Disable
%%   setup_lines / opaque_surface / transp_front / transp_back
%%   load_matrix(M) / mult_matrix(M)
%%   translate(V) / rotate(Angle, Axis)
%%   push_matrix / pop_matrix
%%   bind(Tex) / unbind
%%   destruct                         -- RenderContext.cpp:31  empty

initial_state(state{
    win:                     pre_construct,
    pd3d_device:             none,
    color:                   rgb(0, 0, 0),
    alpha:                   1.0,
    bound_texture:           none,
    %% primitive mode tracking
    primitive_open:          none,
    %% commented-out field: m_pSceneView at RenderContext.h:90 is dead
    psceneview:              none
}).

step(S0, S1) --> [construct(Pd3dDev)],
    { S0.win == pre_construct,
      S1 = S0.put([win - active, pd3d_device - Pd3dDev]) }.

%% C++: CreateVertexBuffer is the ONLY method that uses m_pd3dDevice;
%% everything else uses OpenGL.
step(S0, S0) --> [create_vertex_buffer(_N, _FVF)],
    { S0.win == active }.

%% Categorical begin: one event for the 6 BeginX methods.
step(S0, S1) --> [begin(Kind)],
    { S0.win == active,
      S0.primitive_open == none,
      member(Kind, [lines, line_loop, triangles, quads, quad_strip, polygon]),
      S1 = S0.put(primitive_open, Kind) }.

step(S0, S1) --> [end],
    { S0.win == active,
      S0.primitive_open \= none,
      S1 = S0.put(primitive_open, none) }.

step(S0, S0) --> [vertex(_V)],
    { S0.win == active, S0.primitive_open \= none }.

step(S0, S0) --> [normal(_N)],
    { S0.win == active, S0.primitive_open \= none }.

step(S0, S0) --> [triangles_from_surface(_Mesh)],
    { S0.win == active }.

step(S0, S1) --> [set_color(C)],
    { S0.win == active, S1 = S0.put(color, C) }.

step(S0, S1) --> [set_alpha(A)],
    { S0.win == active, S1 = S0.put(alpha, A) }.

step(S0, S0) --> [set_smooth_shading(_B)],
    { S0.win == active }.

step(S0, S0) --> [set_lighting(_B)],
    { S0.win == active }.

step(S0, S0) --> [setup_lines],            { S0.win == active }.
step(S0, S0) --> [setup_opaque_surface],   { S0.win == active }.
step(S0, S0) --> [setup_transparent_front],{ S0.win == active }.
step(S0, S0) --> [setup_transparent_back], { S0.win == active }.

step(S0, S0) --> [load_matrix(_M)],        { S0.win == active }.
step(S0, S0) --> [mult_matrix(_M)],        { S0.win == active }.
step(S0, S0) --> [translate(_V)],          { S0.win == active }.
step(S0, S0) --> [rotate(_A, _Axis)],      { S0.win == active }.

step(S0, S0) --> [push_matrix],            { S0.win == active }.
step(S0, S0) --> [pop_matrix],             { S0.win == active }.

step(S0, S1) --> [bind(Tex)],
    { S0.win == active, S1 = S0.put(bound_texture, Tex) }.

step(S0, S1) --> [unbind],
    { S0.win == active, S1 = S0.put(bound_texture, none) }.

step(S0, S1) --> [destruct],
    { S0.win == active, S1 = S0.put(win, destroyed) }.

valid_next(State, Event) :-
    member(Event, [
        construct(_), create_vertex_buffer(_, _),
        begin(lines), end, vertex(_), normal(_), triangles_from_surface(_),
        set_color(_), set_alpha(_), set_smooth_shading(_), set_lighting(_),
        setup_lines, setup_opaque_surface, setup_transparent_front, setup_transparent_back,
        load_matrix(_), mult_matrix(_), translate(_), rotate(_, _),
        push_matrix, pop_matrix, bind(_), unbind, destruct
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
%%   1. **Half-migrated.** Holds LPDIRECT3DDEVICE8 m_pd3dDevice but
%%      ONLY CreateVertexBuffer (cpp:36-44) uses it. Every other method
%%      calls OpenGL functions (glBegin, glVertex*, glNormal*,
%%      glMatrixMode, glPushMatrix, glPopMatrix, glColor4f, glEnable
%%      etc.). The D3D and OpenGL contexts coexist incongruously --
%%      you can't actually use this class to render via D3D8, but the
%%      D3D8 handle suggests you could.
%%
%%   2. **Commented-out m_pSceneView.** RenderContext.h:90:
%%        // CSceneView *m_pSceneView;
%%      Originally held a back-pointer to the view. The constructor
%%      comment at .h:26 still says `// CSceneView *pSceneView)` --
%%      the parameter was renamed to pd3dDevice but the comment
%%      preserves the old signature.
%%
%%   3. **Commented-out LineLoopFromPolygon** at cpp:106-119 -- an
%%      earlier helper method that used CPolygon. Replaced by
%%      TrianglesFromSurface which uses CMesh.
%%
%%   4. **Begin/End pairing not enforced**. The state machine here
%%      tracks primitive_open as a sanity check, but the C++ doesn't
%%      validate -- calling Vertex outside a Begin/End pair would
%%      silently call glVertex with no active primitive context,
%%      producing OpenGL state errors at runtime.
%%
%%   5. **TrianglesFromSurface has commented-out array pointers**
%%      (cpp:124, 128): the glVertexPointer/glNormalPointer calls
%%      are commented out, so glEnableClientState(GL_VERTEX_ARRAY)
%%      is called but no pointer is set. This is a real runtime
%%      issue -- the function enables vertex arrays without binding
%%      data to them, then issues glDrawElements with no data.
%%      Preserved verbatim.
