// OpenGLView.cpp : implementation file
//

#include "stdafx.h"

#include "OpenGLView.h"

#include <math.h>

#include <gl/glu.h>

#include <Matrix.h>

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
		backgroundColor(RGB(0, 0, 0)),
		m_bDragging(FALSE),
		m_bMiddleDragging(FALSE)
{
}

COpenGLView::~COpenGLView()
{
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

CVector<3> COpenGLView::ModelPtFromWndPt(CPoint wndPt, const CMatrix<4>& mProj)
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
		camera.nearPlane.Get(),
		modelMatrix, projMatrix, viewport, 
		&vModelPt[0], &vModelPt[1], &vModelPt[2]);

	return vModelPt;
}

CVector<3> COpenGLView::ModelPtFromWndPt(CPoint wndPt)
{
	return ModelPtFromWndPt(wndPt, camera.projection.Get());
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
	static GLenum arrLightID[] =
	{
		GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
		GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7,
	};

	MakeCurrentGLRC();

	CRect rect;
	GetUpdateRect(&rect);

	// clear the buffers
	glClearColor(backgroundColor.Get());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(camera.projection.Get());

	// set up the lights
	for (int nAtLight = 0; nAtLight < lights.GetSize(); nAtLight++)
	{
		glLightPosition(arrLightID[nAtLight], lights.Get(nAtLight)->position.Get());
		glEnable(arrLightID[nAtLight]);
	}

	// set the matrix mode to modelview
	glMatrixMode(GL_MODELVIEW);

	// for each renderer, 
	for (int nAt = 0; nAt < renderers.GetSize(); nAt++)
	{
		// save the current model matrix state
		glPushMatrix();

		// load the renderer's modelview matrix
		glLoadMatrix(renderers.Get(nAt)->modelviewMatrix.Get());

		// save the current rendering attributes
		// glPushAttrib(GL_ALL_ATTRIB_BITS);

		// draw its scene
		renderers.Get(nAt)->DrawScene();

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
	ON_WM_LBUTTONUP() 
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP() 
	ON_WM_MOUSEMOVE()
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
	// glClearColor(0.0, 0.0, 0.0, 0.0);

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
	GLfloat position [] = { 250.0, -250.0, 300.0, 0.0 };
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
	
	// set the OpenGL viewport parameter
	MakeCurrentGLRC();
	glViewport(0, 0, cx, cy);

	// calculate the aspect ratio for the camera
	camera.aspectRatio.Set((double) cx / (double) cy);
}

void COpenGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < leftTrackers.GetSize(); nAt++)
	{
		if (leftTrackers.Get(nAt)->IsInside(point))
		{
			leftTrackers.Get(nAt)->OnLButtonDown(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < leftTrackers.GetSize(); nAt++)
	{
		if (leftTrackers.Get(nAt)->IsInside(point))
		{
			leftTrackers.Get(nAt)->OnLButtonUp(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnMButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bMiddleDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < middleTrackers.GetSize(); nAt++)
	{
		if (middleTrackers.Get(nAt)->IsInside(point))
		{
			middleTrackers.Get(nAt)->OnLButtonDown(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnMButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bMiddleDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < middleTrackers.GetSize(); nAt++)
	{
		if (middleTrackers.Get(nAt)->IsInside(point))
		{
			middleTrackers.Get(nAt)->OnLButtonUp(nFlags, point);
			break;
		}
	}
}

void COpenGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove(nFlags, point);

	if (m_bDragging)
	{
		for (int nAt = 0; nAt < leftTrackers.GetSize(); nAt++)
		{
			if (leftTrackers.Get(nAt)->IsInside(point))
			{
				leftTrackers.Get(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}
		return;
	}

	if (m_bMiddleDragging)
	{
		for (int nAt = 0; nAt < middleTrackers.GetSize(); nAt++)
		{
			if (middleTrackers.Get(nAt)->IsInside(point))
			{
				middleTrackers.Get(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}
		return;
	}

	for (int nAt = 0; nAt < leftTrackers.GetSize(); nAt++)
	{
		if (leftTrackers.Get(nAt)->IsInside(point))
		{
			leftTrackers.Get(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}

	for (nAt = 0; nAt < middleTrackers.GetSize(); nAt++)
	{
		if (middleTrackers.Get(nAt)->IsInside(point))
		{
			middleTrackers.Get(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}
}

void COpenGLView::MakeCurrentGLRC()
{
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hrc);
}
