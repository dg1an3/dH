// ZoomTracker.cpp: implementation of the CZoomTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"
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

CMatrix<4> ComputeScaleMatrix(const double& initY, const double& finalY)
{
	// compute the difference
	double scale = exp(4.0 * (initY - finalY));
	TRACE1("Scale = %lf\n", scale);

	// now compute the rotation matrix
	return CreateScale(CVector<3>(scale, scale, scale));
}

FUNCTION_FACTORY2(ComputeScaleMatrix, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZoomTracker::CZoomTracker(COpenGLView *pView)
	: COpenGLTracker(pView),
		privCurrProjMatrix(privInitProjMatrix 
			* ComputeScaleMatrix(privInitY, privCurrY))
{
}

CZoomTracker::~CZoomTracker()
{

}

void CZoomTracker::OnLButtonDown(UINT nFlags, CPoint point)
{
	privInitProjMatrix.Set(m_pView->projectionMatrix.Get());

	CRect rect;
	m_pView->GetClientRect(&rect);
	privInitY.Set((double) point.y / (double) rect.Height());

	privCurrY.Set(privInitY.Get());

	m_pView->projectionMatrix.SyncTo(&privCurrProjMatrix);
}

void CZoomTracker::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pView->projectionMatrix.SyncTo(NULL);
	m_pView->projectionMatrix.Set(privCurrProjMatrix.Get());
}

void CZoomTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	CRect rect;
	m_pView->GetClientRect(&rect);
	privCurrY.Set((double) point.y / (double) rect.Height());

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
