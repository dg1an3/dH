// OpenGLView.cpp : implementation file
//

#include "stdafx.h"

#include "OpenGLView.h"

#include "math.h"

#include "gl\gl.h"
#include "gl\glu.h"

#include "Matrix.h"
#include "glMatrixVector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenGLView

IMPLEMENT_DYNCREATE(COpenGLView, CWnd)

COpenGLView::COpenGLView()
	: m_pDC(NULL),
		m_hrc(NULL),
		m_bDragging(FALSE)
{
	projectionMatrix.AddObserver(this);
}

COpenGLView::~COpenGLView()
{
	int nAt;
	for (nAt = 0; nAt < m_arrRenderers.GetSize(); nAt++)
	{
		delete m_arrRenderers[nAt];
	}
	m_arrRenderers.RemoveAll();

	for (nAt = 0; nAt < m_arrTrackers.GetSize(); nAt++)
	{
		delete m_arrTrackers[nAt];
	}
	m_arrTrackers.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// COpenGLView diagnostics

#ifdef _DEBUG
void COpenGLView::AssertValid() const
{
	CWnd::AssertValid();
}

void COpenGLView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}
#endif //_DEBUG

void COpenGLView::AddRenderer(COpenGLRenderer *pRenderer)
{
	m_arrRenderers.Add(pRenderer);

	RedrawWindow();
}

void COpenGLView::AddTracker(COpenGLTracker *pTracker)
{
	m_arrTrackers.Add(pTracker);

	RedrawWindow();
}

void COpenGLView::SetClippingPlanes(float nearPlane, float farPlane)
{
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;

	RecalcProjectionMatrix();
}

float COpenGLView::GetNearPlane()
{
	return m_nearPlane;
}

float COpenGLView::GetFarPlane()
{
	return m_farPlane;
}

void COpenGLView::SetMaxObjSize(float maxObjSize)
{
	m_farPlane = m_nearPlane + maxObjSize * 2.5f;
	m_radius = m_nearPlane + maxObjSize / 1.2f;

	RecalcProjectionMatrix();
}

BOOL COpenGLView::SetupPixelFormat()
{
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

	if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}

	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}

	return TRUE;
}

void COpenGLView::RecalcProjectionMatrix()
{
	MakeCurrentGLRC();

	// operate on the projection matrix
	glMatrixMode(GL_PROJECTION);

	// load an identity matrix
	glLoadIdentity();

	// get the window's client rectangle
	CRect rect;
	GetClientRect(&rect);

	// only change OpenGL rendering parameters if we have a valid window height
	if (rect.Height() != 0)
	{
		// set the OpenGL viewport parameter
		glViewport(0, 0, rect.Width(), rect.Height());

		// calculate the aspect ratio
		float aspect = (float) rect.Width() / (float) rect.Height();

		// set a perspective projection with the given aspect ratio and clipping planes
		gluPerspective(45.0f, aspect, m_nearPlane, m_farPlane);

		// move the camera back by the desired viewing distance
		glTranslatef(0.0f, 0.0f, -m_radius);

		projectionMatrix.Set(glGetMatrix(GL_PROJECTION_MATRIX));
	}

	// set back to operate on the model matrix
	glMatrixMode(GL_MODELVIEW);
}


CVector<3> COpenGLView::ModelPtFromWndPt(CPoint wndPt, const CMatrix<4> *mProj, float z)
{
	if (mProj == NULL)
		mProj = &projectionMatrix.Get();

//	if (z == -999.0)
//		z = GetNearPlane();

	// retrieve the model and projection matrices
	CMatrix<4> mModel;
	GLdouble modelMatrix[16];
	mModel.ToArray(modelMatrix);

	GLdouble projMatrix[16];
	mProj->ToArray(projMatrix);

	// retrieve the viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	CVector<3> vModelPt;

	// un-project the window coordinates into the model coordinate system
	gluUnProject((GLdouble)viewport[2] - wndPt.x, (GLdouble)wndPt.y, GetNearPlane(),
		modelMatrix, projMatrix, viewport, 
		&vModelPt[0], &vModelPt[1], &vModelPt[2]);

	return vModelPt;
}


BOOL COpenGLView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// An OpenGL window must be created with the following flags and must not
	// include CS_PARENTDC for the class style. Refer to SetPixelFormat
	// documentation in the "Comments" section for further information.
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// COpenGLView drawing

void COpenGLView::OnPaint()
{
	MakeCurrentGLRC();

	CRect rect;
	GetUpdateRect(&rect);

	// clear the buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(projectionMatrix.Get());

	// set the matrix mode to modelview
	glMatrixMode(GL_MODELVIEW);

	// for each renderer, 
	for (int nAt = 0; nAt < m_arrRenderers.GetSize(); nAt++)
	{
		// save the current model matrix state
		glPushMatrix();

		// load the renderer's modelview matrix
		glLoadMatrix(m_arrRenderers[nAt]->myModelviewMatrix.Get());

		// save the current rendering attributes
		// glPushAttrib(GL_ALL_ATTRIB_BITS);

		// draw its scene
		m_arrRenderers[nAt]->DrawScene();

		// restore the rendering attributes
		// glPopAttrib();

		// restore the model matrix state
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	// and finish up the rendering
	glFinish();
	SwapBuffers(wglGetCurrentDC());

	// make sure the rectangle is updated
	ValidateRect(&rect);
}

BEGIN_MESSAGE_MAP(COpenGLView, CWnd)
	//{{AFX_MSG_MAP(COpenGLView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP() 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenGLView message handlers

int COpenGLView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// construct the Window's device context which the OpenGL rendering
	//		context will use
	m_pDC = new CClientDC(this);
	ASSERT(m_pDC != NULL);

	// set up the OpenGL pixel format
	if (!SetupPixelFormat())
		return -1;

	// create the OpenGL rendering context
	m_hrc = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);

	// set the clear color to black
	glClearColor(0.0, 0.0, 0.0, 0.0);

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

	// Create a Directional Light Source
	GLfloat position [] = { 1.0, 1.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	return 0;
}

void COpenGLView::OnDestroy() 
{
	// select no OpenGL rendering context
	wglMakeCurrent(NULL,  NULL);

	// delete the OpenGL rendering context
	if (m_hrc)
		::wglDeleteContext(m_hrc);

	// delete the Window's device context
	if (m_pDC)
		delete m_pDC;

	// call base class destroy
	CWnd::OnDestroy();
}

BOOL COpenGLView::OnEraseBkgnd(CDC* pDC) 
{
	// over-ride default to prevent flicker
	return TRUE;
}

void COpenGLView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// recalculate the projection matrix given the new window size
	RecalcProjectionMatrix();
}

void COpenGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonDown(nFlags, point);

	MakeCurrentGLRC();

	// set the dragging flag to TRUE
	m_bDragging = TRUE;

	for (int nAt = 0; nAt < m_arrTrackers.GetSize(); nAt++)
	{
		if (m_arrTrackers[nAt]->IsInside(point))
		{
			m_arrTrackers[nAt]->OnLButtonDown(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove(nFlags, point);

	MakeCurrentGLRC();

	for (int nAt = 0; nAt < m_arrTrackers.GetSize(); nAt++)
	{
		if (m_arrTrackers[nAt]->IsInside(point))
		{
			if (m_bDragging)
				m_arrTrackers[nAt]->OnMouseDrag(nFlags, point);
			else 
				m_arrTrackers[nAt]->OnMouseMove(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);

	MakeCurrentGLRC();

	// dragging is over -- clear flag
	m_bDragging = FALSE;	
	
	for (int nAt = 0; nAt < m_arrTrackers.GetSize(); nAt++)
	{
		if (m_arrTrackers[nAt]->IsInside(point))
		{
			m_arrTrackers[nAt]->OnLButtonUp(nFlags, point);
			break;
		}
	}
}

void COpenGLView::MakeCurrentGLRC()
{
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);
}

void COpenGLView::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if (pSource == &projectionMatrix)
	{
		MakeCurrentGLRC();
		glMatrixMode(GL_PROJECTION);
		glLoadMatrix(projectionMatrix.Get());
	}
}
