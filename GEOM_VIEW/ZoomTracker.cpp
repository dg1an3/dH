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

/*
//////////////////////////////////////////////////////////////////////
// declare function factories for matrix arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(operator+, CMatrix<4>)
FUNCTION_FACTORY2_ARG(operator*, CMatrix<4>, CMatrix<4>, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix creation from parameters
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(CreateRotate, CMatrix<4>)
FUNCTION_FACTORY1(CreateTranslate, CMatrix<4>)
FUNCTION_FACTORY2(CreateTranslate, CMatrix<4>)
FUNCTION_FACTORY1(CreateScale, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix inversion
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY1(Invert, CMatrix<4>)
*/
CMatrix<4> ComputeScaleMatrix(const double& initY, const double& finalY)
{
	// compute the difference
	double scale = exp(4.0 * (initY - finalY));
	TRACE1("Scale = %lf\n", scale);

	// now compute the rotation matrix
	return CreateScale(CVector<3>(scale, scale, scale));
}

// FUNCTION_FACTORY2(ComputeScaleMatrix, CMatrix<4>)

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

void CZoomTracker::OnLButtonDown(UINT nFlags, CPoint point)
{
	privInitProjMatrix = m_pView->GetCamera().GetModelXform();

	CRect rect;
	m_pView->GetClientRect(&rect);
	privInitY = (double) point.y / (double) rect.Height();

	privCurrY = privInitY;

}

void CZoomTracker::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pView->GetCamera().SetModelXform(privCurrProjMatrix);
}

void CZoomTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	CRect rect;
	m_pView->GetClientRect(&rect);
	privCurrY = (double) point.y / (double) rect.Height();

	privCurrProjMatrix = privInitProjMatrix 
		* ComputeScaleMatrix(privInitY, privCurrY);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
