// RotateTracker.cpp: implementation of the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"
#include "RotateTracker.h"

#include "OpenGLView.h"

#include "Matrix.h"

#include "gl/gl.h"
#include "gl/glu.h"

#include "glMatrixVector.h"

#include "MatrixFunction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMatrix<4> ComputeRotMatrix(const CVector<3>& vInitPt,
	const CVector<3>& vFinalPt)
{
	// compute the length of the two legs of the triangle (a & b)
	double a = vInitPt.GetLength(); 
	double b = vFinalPt.GetLength();
	
	// compute the length of the segment connecting the initial and final
	//		point (c)
	double c = (vFinalPt - vInitPt).GetLength();

	// compute the angle theta using the law of cosines
	double theta = 0.0;
	if (a * b != 0.0)
	{
		theta = 3.0 * acos((a * a + b * b - c * c) / (2 * a * b));
	}

	// compute the axis of rotation = normalized cross product of 
	//		initial and current drag points
	CVector<3> u = Cross(vInitPt, vFinalPt);
	u.Normalize();

	// now compute the rotation matrix
	return CreateRotate(theta, u);
}

FUNCTION_FACTORY2(ComputeRotMatrix, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRotateTracker::CRotateTracker(COpenGLView *pView)
	: COpenGLTracker(pView),
		privCurrProjMatrix(privInitProjMatrix 
			* ComputeRotMatrix(privInitPoint, privCurrPoint))
{
}

CRotateTracker::~CRotateTracker()
{

}

void CRotateTracker::OnLButtonDown(UINT nFlags, CPoint point)
{
	privInitProjMatrix.Set(m_pView->myProjectionMatrix.Get());

	privInitPoint.Set(m_pView->ModelPtFromWndPt(point, &privInitProjMatrix.Get()));
	privCurrPoint.Set(privInitPoint.Get());

	m_pView->myProjectionMatrix.SyncTo(&privCurrProjMatrix);
}

void CRotateTracker::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pView->myProjectionMatrix.SyncTo(NULL);
}

void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	privCurrPoint.Set(m_pView->ModelPtFromWndPt(point, &privInitProjMatrix.Get()));

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
