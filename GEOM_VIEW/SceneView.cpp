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

// class declaration
#include "SceneView.h"

// render context object
#include "RenderContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// constant for setting alpha to opaque
const double MAX_ALPHA = 0.99;

// constants for reflectance properties
GLfloat DEFAULT_SPECULAR[] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat DEFAULT_SHININESS[] = { 20.0 };

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
		delete m_arrRenderablesNearest[nAt];
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
	return m_arrRenderablesNearest.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetRenderableAt
// 
// returns the renderable at the given index
/////////////////////////////////////////////////////////////////////////////
CRenderable *CSceneView::GetRenderableAt(int nAt)
{
	return (CRenderable *) m_arrRenderablesNearest[nAt];
}

/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddRenderable
// 
// adds a new renderable to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddRenderable(CRenderable *pRenderable) 
{ 
	// set this as the scene view
	pRenderable->m_pView = this;

	// add to both renderable arrays
	m_arrRenderablesFurthest.Add(pRenderable);
	return m_arrRenderablesNearest.Add(pRenderable);
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
void CSceneView::SortRenderables(CObArray *pArray, 
		double (CRenderable::*DistFunc)(const CVector<3>& vPoint))
{
	// current camera position
	CVector<3> vCameraPos = CVector<3>(GetCamera().GetXform()
		* ToHomogeneous(CVector<3>(0.0, 0.0, 0.0)));

	// stores the distances for the renderable centroids
	CArray<double, double> arrDistances;

	// compute the nearest distances
	for (int nAt = 0; nAt < pArray->GetSize(); nAt++)
	{
		// get the renderable
		CRenderable *pRenderable = (CRenderable *) pArray->GetAt(nAt);

		// add to the array
		double distance = (pRenderable->*DistFunc)(vCameraPos);
		arrDistances.Add(distance);
	}

	// flag to indicate a rearrangement has occurred
	BOOL bRearrange;
	do 
	{
		// initially, no rearrangement has occurred
		bRearrange = FALSE;

		// for each link,
		for (nAt = 0; nAt < pArray->GetSize()-1; nAt++)
		{
			// compare their weights
			if (arrDistances[nAt] < arrDistances[nAt+1])
			{
				// if first is less than second, swap the pointers
				CObject *pTemp = pArray->GetAt(nAt);
				pArray->SetAt(nAt, pArray->GetAt(nAt+1));
				pArray->SetAt(nAt+1, pTemp);

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
	CRenderContext rc(this);

	//////////////////////////////////////////////////////////////////////////
	// set up rendering

	// clear the buffers
	glClearColor(
		(float) GetRValue(GetBackgroundColor()) / 255.0f, 
		(float) GetGValue(GetBackgroundColor()) / 255.0f, 
		(float) GetBValue(GetBackgroundColor()) / 255.0f, 
		1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// load the projection matrix
	glMatrixMode(GL_PROJECTION);
	rc.LoadMatrix(GetCamera().GetProjection());

	// set up the lights
	for (int nAtLight = 0; nAtLight < GetLightCount(); nAtLight++)
	{
		GetLightAt(nAtLight)->TurnOn(nAtLight);
	}

	// sort the renderables from furthest to nearest
	SortRenderables(&m_arrRenderablesNearest, 
		CRenderable::GetNearestDistance);
	SortRenderables(&m_arrRenderablesFurthest, 
		CRenderable::GetFurthestDistance);

	// Set the polygon mode to fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Define material properties of specular color and degree of 
	// shininess.  
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, DEFAULT_SPECULAR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, DEFAULT_SHININESS);

	// Set the GL_AMBIENT_AND_DIFFUSE color state variable to be the
	// one referred to by all following calls to glColor
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	//////////////////////////////////////////////////////////////////////////
	// render opaque objects

	// for each renderer, 
	for (int nAt = 0; nAt < GetRenderableCount(); nAt++)
	{
		// enable depth buffer writing
		glDepthMask(GL_TRUE);

		// disable blending
		glDisable(GL_BLEND);

		// enable culling
		glDisable(GL_CULL_FACE);

		// enable lighting effects
		glEnable(GL_LIGHTING);

		// save the current model matrix state
		rc.PushMatrix();

		// draw its scene
		GetRenderableAt(nAt)->DrawOpaqueList(&rc);

		// restore the model matrix state
		rc.PopMatrix();

		// also draw the alpha part of the renderable, 
		//		if alpha is greater than the max
		if (GetRenderableAt(nAt)->GetAlpha() > MAX_ALPHA)
		{
			// save the current model matrix state
			rc.PushMatrix();

			// draw its scene
			GetRenderableAt(nAt)->DrawTransparentList(&rc);

			// restore the model matrix state
			rc.PopMatrix();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// render alpha-blended objects

	// arrays holding parameters for the four passes
	GLenum arrFaces[] =		   { GL_FRONT, GL_FRONT, GL_BACK, GL_BACK };
	GLboolean arrDepthMask[] = { GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE };

	for (int nAtPass = 0; nAtPass < 4; nAtPass++)
	{
		// use the furthest array for back-faced rendering
		CObArray *pArray = (arrFaces[nAtPass] == GL_FRONT)
			? &m_arrRenderablesFurthest : &m_arrRenderablesNearest;

		// for each renderer, 
		for (nAt = 0; nAt < pArray->GetSize(); nAt++)
		{
			// get the renderable
			CRenderable *pRenderable = (CRenderable *) pArray->GetAt(nAt);

			if (pRenderable->GetAlpha() <= MAX_ALPHA)
			{
				// enable culling
				glEnable(GL_CULL_FACE);

				// set face culling
				glCullFace(arrFaces[nAtPass]);

				// disable lighting if back faces are displayed
				if (GL_FRONT == arrFaces[nAtPass])
				{
					glDisable(GL_LIGHTING);
				}
				else
				{
					glEnable(GL_LIGHTING);
				}

				// set depth buffer writing
				glDepthMask(arrDepthMask[nAtPass]);

				// enable blending
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				// save the current model matrix state
				rc.PushMatrix();

				// draw its scene
				pRenderable->DrawTransparentList(&rc);

				// restore the model matrix state
				rc.PopMatrix();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// finish up rendering

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
