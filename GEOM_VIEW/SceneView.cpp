//////////////////////////////////////////////////////////////////////
// SceneView.cpp: implementation of the CSceneView class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <math.h>

// OpenGL include files
#include <gl/glu.h>
#include "glMatrixVector.h"

// class declaration
#include "SceneView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE -- implements the dynamic creation mechanism for
//		CSceneView
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSceneView, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSceneView::CSceneView
// 
// constructs a new CSceneView.
/////////////////////////////////////////////////////////////////////////////
CSceneView::CSceneView()
	: m_pDC(NULL),
		m_hrc(NULL),
		m_backgroundColor(RGB(0, 0, 0)),
		m_bLeftDragging(FALSE),
		m_bMiddleDragging(FALSE)
{
	// add this as a listener on the camera
	::AddObserver<CSceneView>(&GetCamera().GetChangeEvent(), 
		this, OnCameraChanged);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::~CSceneView
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
CSceneView::~CSceneView()
{
	// delete the renderables
	for (int nAt = 0; nAt < GetRenderableCount(); nAt++)
	{
		delete m_arrRenderables[nAt];
	}

	// delete the lights
	for (nAt = 0; nAt < GetLightCount(); nAt++)
	{
		delete m_arrLights[nAt];
	}

	// delete the trackers
	for (nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		delete m_arrLeftTrackers[nAt];
	}

	// delete the middle trackers
	for (nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		delete m_arrMiddleTrackers[nAt];
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetBackgroundColor
// 
// the background color for the view
/////////////////////////////////////////////////////////////////////////////
COLORREF CSceneView::GetBackgroundColor() const
{
	return m_backgroundColor;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::SetBackgroundColor
// 
// the background color for the view
/////////////////////////////////////////////////////////////////////////////
void CSceneView::SetBackgroundColor(COLORREF color)
{
	m_backgroundColor = color;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetCamera
// 
// returns a reference to the view's camera
/////////////////////////////////////////////////////////////////////////////
CCamera& CSceneView::GetCamera()
{
	return m_camera;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetRenderableCount
// 
// collection of renderables
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetRenderableCount() const
{
	return m_arrRenderables.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetRenderableAt
// 
// returns the renderable at the given index
/////////////////////////////////////////////////////////////////////////////
CRenderable *CSceneView::GetRenderableAt(int nAt)
{
	return (CRenderable *) m_arrRenderables[nAt];
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddRenderable
// 
// adds a new renderable to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddRenderable(CRenderable *pRenderer) 
{ 
	// set this as the scene view
	pRenderer->m_pView = this;

	// add to the renderable array
	return m_arrRenderables.Add(pRenderer);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLightCount
// 
// collection of lights for the view
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetLightCount() const 
{ 
	return m_arrLights.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLightAt
// 
// returns the light at the given index
/////////////////////////////////////////////////////////////////////////////
CLight *CSceneView::GetLightAt(int nAt)
{ 
	return (CLight *) m_arrLights[nAt];
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddLight
// 
// adds a light to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddLight(CLight *pLight) 
{ 
	return m_arrLights.Add(pLight);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLeftTrackerCount
// 
// collection of trackers for left button
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetLeftTrackerCount() const 
{ 
	return m_arrLeftTrackers.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLeftTrackerAt
// 
// returns the left tracker at the given index
/////////////////////////////////////////////////////////////////////////////
CTracker *CSceneView::GetLeftTrackerAt(int nAt) 
{ 
	return (CTracker *) m_arrLeftTrackers[nAt];
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddLeftTracker
// 
// adds a new left tracker to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddLeftTracker(CTracker *pRenderer) 
{ 
	return m_arrLeftTrackers.Add(pRenderer);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetMiddleTrackerCount
// 
// collection of trackers for middle button
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetMiddleTrackerCount() const 
{ 
	return m_arrMiddleTrackers.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetMiddleTrackerAt
// 
// returns one of the current middle trackers
/////////////////////////////////////////////////////////////////////////////
CTracker *CSceneView::GetMiddleTrackerAt(int nAt) 
{ 
	return (CTracker *) m_arrMiddleTrackers[nAt];
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddMiddleTracker
// 
// adds a new middle tracker to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddMiddleTracker(CTracker *pRenderer)
{
	return m_arrMiddleTrackers.Add(pRenderer);
}

//////////////////////////////////////////////////////////////////////////////
// CSceneView::OnCameraChanged
// 
// change handler for camera changes
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnCameraChanged(CObservableEvent *, void *)
{
	// re-sort the renderables
	SortRenderables();

	// invalidate the window to redraw
	Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// CSceneView diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////
// CSceneView::AssertValid
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
void CSceneView::AssertValid() const
{
	CWnd::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::Dump
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
void CSceneView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSceneView::SetupPixelFormat
// 
// sets up the pixel format for the view class
/////////////////////////////////////////////////////////////////////////////
BOOL CSceneView::SetupPixelFormat()
{
	// pixel format descriptor
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
		1,                              // version number
		PFD_DRAW_TO_WINDOW |            // support window
		  PFD_SUPPORT_OPENGL |          // support OpenGL
		  PFD_DOUBLEBUFFER,             // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		24,                             // 24-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		32,                             // 32-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};

	int pixelformat;

	// now try to choose the pixel format
	if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}

	// and see if we can successfully set the pixel format
	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::MakeCurrentGLRC
// 
// sets the threads current HGLRC
/////////////////////////////////////////////////////////////////////////////
void CSceneView::MakeCurrentGLRC()
{
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::SortRenderables
// sorts the renderables from the furthest to the nearest based on the
//		current camera position
/////////////////////////////////////////////////////////////////////////////
void CSceneView::SortRenderables()
{
	// current camera xform
	const CMatrix<4>& mXform = GetCamera().GetXform();

	// stores the distances for the renderable centroids
	CArray<double, double> arrDistances;

	// compute the distances
	for (int nAt = 0; nAt < GetRenderableCount(); nAt++)
	{
		CRenderable *pRenderable = GetRenderableAt(nAt);

		// position of renderable centroid in camera coordinates
		CVector<3> vCentroid = CVector<3>(mXform 
			* ToHomogeneous(pRenderable->GetCentroid()));

		// add to the array
		arrDistances.Add(vCentroid.GetLength());
	}

	// flag to indicate a rearrangement has occurred
	BOOL bRearrange;
	do 
	{
		// initially, no rearrangement has occurred
		bRearrange = FALSE;

		// for each link,
		for (nAt = 0; nAt < GetRenderableCount()-1; nAt++)
		{
			// compare their weights
			if (arrDistances[nAt] < arrDistances[nAt+1])
			{
				// if first is less than second, swap the pointers
				CObject *pTemp = m_arrRenderables.GetAt(nAt);
				m_arrRenderables.SetAt(nAt, m_arrRenderables.GetAt(nAt+1));
				m_arrRenderables.SetAt(nAt+1, pTemp);

				// and the distances
				double tempDistance = arrDistances[nAt];
				arrDistances[nAt] = arrDistances[nAt+1];
				arrDistances[nAt+1] = tempDistance;

				// a rearrangement has occurred
				bRearrange = TRUE;
			}
		}

	// continue as long as rearrangements occur
	} while (bRearrange);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::ModelPtFromWndPt
// 
// converts a point in viewport coordinates to a 3-d point using the
//		current projection matrix
/////////////////////////////////////////////////////////////////////////////
CVector<3> CSceneView::ModelPtFromWndPt(CPoint wndPt)
{
	return ModelPtFromWndPt(wndPt, GetCamera().GetProjection());
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::ModelPtFromWndPt
// 
// converts a point in viewport coordinates to a 3-d point given 
//		a projection matrix
/////////////////////////////////////////////////////////////////////////////
CVector<3> CSceneView::ModelPtFromWndPt(CPoint wndPt, const CMatrix<4>& mProj)
{
	// retrieve the model and projection matrices
	CMatrix<4> mModel;
	GLdouble modelMatrix[16];
	mModel.ToArray(modelMatrix);

	GLdouble projMatrix[16];
	mProj.ToArray(projMatrix);

	// retrieve the viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// un-project the window coordinates into the model coordinate system
	CVector<3> vModelPt;
	gluUnProject((GLdouble)viewport[2] - wndPt.x, (GLdouble)wndPt.y, 
		GetCamera().GetNearPlane(),
		modelMatrix, projMatrix, viewport, 
		&vModelPt[0], &vModelPt[1], &vModelPt[2]);

	return vModelPt;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::PreCreateWindow
// 
// ensures that the necessary window styles are applied to this window 
//		during creation
/////////////////////////////////////////////////////////////////////////////
BOOL CSceneView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// An OpenGL window must be created with the following flags and must not
	// include CS_PARENTDC for the class style. Refer to SetPixelFormat
	// documentation in the "Comments" section for further information.
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSceneView, CWnd)
	//{{AFX_MSG_MAP(CSceneView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP() 
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP() 
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneView message handlers

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnCreate
// 
// renders the buffer, and swaps back and front buffer
/////////////////////////////////////////////////////////////////////////////
int CSceneView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	// construct the Window's device context which the OpenGL rendering
	//		context will use
	m_pDC = new CClientDC(this);
	ASSERT(m_pDC != NULL);

	// set up the OpenGL pixel format
	if (!SetupPixelFormat())
	{
		return -1;
	}

	// create the OpenGL rendering context
	m_hrc = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

	// set depth testing for hidden line removal
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	// Set the shading model
	glShadeModel(GL_SMOOTH);

	// Set the polygon mode to fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Define material properties of specular color and degree of 
	// shininess.  Since this is only done once in this particular 
	// example, it applies to all objects.  Material properties can 
	// be set for individual objects, individual faces of the objects,
	// individual vertices of the faces, etc... 
	GLfloat specular [] = { 0.5, 0.5, 0.5, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	GLfloat shininess [] = { 20.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	// Set the GL_AMBIENT_AND_DIFFUSE color state variable to be the
	// one referred to by all following calls to glColor
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// create the default lights for the view

	CLight *pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(255, 255, 255));
	pNewLight->SetPosition(CVector<3>(-5.0, -5.0, 3.0));
	AddLight(pNewLight);

	pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(128, 128, 128));
	pNewLight->SetPosition(CVector<3>(5.0, -5.0, 3.0));
	AddLight(pNewLight);

	pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(128, 128, 128));
	pNewLight->SetPosition(CVector<3>(0.0, 5.0, 3.0));
	AddLight(pNewLight);

	glEnable(GL_LIGHTING);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnDestroy
// 
// removes the rendering context when the window is destroyed
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnDestroy() 
{
	// select no OpenGL rendering context
	wglMakeCurrent(NULL,  NULL);

	// delete the OpenGL rendering context
	if (m_hrc)
	{
		::wglDeleteContext(m_hrc);
	}

	// delete the Window's device context
	if (m_pDC)
	{
		delete m_pDC;
	}

	// call base class destroy
	CWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnEraseBkgnd
// 
// over-ride to do nothing, because double-buffering pre-empts the need
/////////////////////////////////////////////////////////////////////////////
BOOL CSceneView::OnEraseBkgnd(CDC* pDC) 
{
	// over-ride default to prevent flicker
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnSize
// 
// adjusts the camera aspect ratio for the new size
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnSize(UINT nType, int cx, int cy) 
{
	// let the base class have a chance
	CWnd::OnSize(nType, cx, cy);
	
	// make sure we are using the correct rendering context
	MakeCurrentGLRC();

	// set the OpenGL viewport parameter
	glViewport(0, 0, cx, cy);

	// calculate the aspect ratio for the camera
	if (cx > 0 && cy > 0)
	{
		GetCamera().SetAspectRatio((double) cx / (double) cy);
	}
	else
	{
		GetCamera().SetAspectRatio(1.0);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnPaint
// 
// renders the buffer, and swaps back and front buffer
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnPaint()
{
	// make sure we are using the correct rendering context
	MakeCurrentGLRC();

	// clear the buffers
	glClearColor(GetBackgroundColor());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(GetCamera().GetProjection());

	// set up the lights
	for (int nAtLight = 0; nAtLight < GetLightCount(); nAtLight++)
	{
		GetLightAt(nAtLight)->TurnOn(nAtLight);
	}

	// sort the renderables from furthest to nearest
	SortRenderables();

	// for each renderer, 
	for (int nAt = 0; nAt < GetRenderableCount(); nAt++)
	{
		// save the current model matrix state
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// load the renderer's modelview matrix
		glLoadMatrix(GetRenderableAt(nAt)->GetModelviewMatrix());

		// draw its scene
		GetRenderableAt(nAt)->Render();

		// restore the model matrix state
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	// finish up the rendering
	glFinish();

	// and switch the buffers
	SwapBuffers(wglGetCurrentDC());

	// make sure the window is updated
	ValidateRect(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnLButtonDown
// 
// handles a left-button down event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bLeftDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			// found one
			GetLeftTrackerAt(nAt)->OnButtonDown(nFlags, point);
			break;
		}
	}

	// set the capture for this window
	SetCapture();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnLButtonUp
// 
// handles a left-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bLeftDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			GetLeftTrackerAt(nAt)->OnButtonUp(nFlags, point);
			break;
		}
	}

	// release the capture
	::ReleaseCapture();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMButtonDown
// 
// handles a middle-button down event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnMButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bMiddleDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnButtonDown(nFlags, point);
			break;
		}
	}

	// set the capture for this window
	SetCapture();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMButtonUp
// 
// handles a middle-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnMButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bMiddleDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnButtonUp(nFlags, point);
			break;
		}
	}

	// release the capture
	::ReleaseCapture();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMouseMove
// 
// handles a middle-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// process base class move event
	CWnd::OnMouseMove(nFlags, point);

	// see if we are in a left-drag
	if (m_bLeftDragging)
	{
		for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
		{
			if (GetLeftTrackerAt(nAt)->IsInside(point))
			{
				GetLeftTrackerAt(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}

		// return if none found
		return;
	}

	// see if we are in a middle drag
	if (m_bMiddleDragging)
	{
		for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
		{
			if (GetMiddleTrackerAt(nAt)->IsInside(point))
			{
				GetMiddleTrackerAt(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}

		// return if none found
		return;
	}

	// process regular left move
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			GetLeftTrackerAt(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}

	// process regular middle move
	for (nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}
}
