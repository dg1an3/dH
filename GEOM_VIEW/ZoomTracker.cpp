// ZoomTracker.cpp: implementation of the CZoomTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"

// #include <ScalarFunction.h>

#include "ZoomTracker.h"

#include "OpenGLView.h"

#include <Matrix.h>

#include "gl/gl.h"
#include "gl/glu.h"

#include "glMatrixVector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZoomTracker::CZoomTracker(COpenGLView *pView)
	: COpenGLTracker(pView)
{
}

CZoomTracker::~CZoomTracker()
{
}

void CZoomTracker::OnButtonDown(UINT nFlags, CPoint point)
{
	// store the projection matrix
	m_initXform = m_pView->GetCamera().GetModelXform();

	// get the client rectangle
	CRect rect;
	m_pView->GetClientRect(&rect);

	// compute the relative y position
	m_initY = (double) point.y / (double) rect.Height();
}

void CZoomTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	// get the client rectangle
	CRect rect;
	m_pView->GetClientRect(&rect);

	// compute the relative y position
	double currY = (double) point.y / (double) rect.Height();

	// compute the zoom factor
	double zoom = exp(4.0 * (m_initY - currY));

	// compute the projection matrix
	CMatrix<4> currXform = m_initXform
		* CMatrix<4>(CreateScale(CVector<3>(zoom, zoom, zoom)));

	// set the new model xform
	m_pView->GetCamera().SetModelXform(currXform);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
