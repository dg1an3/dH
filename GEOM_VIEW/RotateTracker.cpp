// RotateTracker.cpp: implementation of the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"
#include "RotateTracker.h"

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

// computes a rotation matrix about two points
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
		theta = 3.0 * (2 * PI - acos((a * a + b * b - c * c) / (2 * a * b)));
	}

	// compute the axis of rotation = normalized cross product of 
	//		initial and current drag points
	CVector<3> u = Cross(vInitPt, vFinalPt);
	u.Normalize();

	// now compute the rotation matrix
	return CreateRotate(theta, u);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRotateTracker::CRotateTracker(COpenGLView *pView)
	: COpenGLTracker(pView)
{
}

CRotateTracker::~CRotateTracker()
{

}

void CRotateTracker::OnLButtonDown(UINT nFlags, CPoint point)
{
	TRACE2("OnLButtonDown %i, %i\n", point.x, point.y);

	// store the initial model transform matrix; subsequent rotations will be applied
	//		to this matrix directly
	m_initModelXform = m_pView->camera.modelXform.Get();

	// store the current projection matrix from the view, for transforming
	//		subsequent coordinates
	m_initProjMatrix = m_pView->camera.projection.Get();

	// store the current mouse position in 3-d
	m_vInitPoint = m_pView->ModelPtFromWndPt(point, m_initProjMatrix);
}

void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	TRACE2("OnMouseDrag %i, %i\n", point.x, point.y);

	// compute the final point
	CVector<3> vFinalPoint = 
		m_pView->ModelPtFromWndPt(point, m_initProjMatrix);

	// compute the rotation matrix
	CMatrix<4> mRotate = ComputeRotMatrix(vFinalPoint, m_vInitPoint);

	// set the new model xform to the initial composed with the rotation
	m_pView->camera.modelXform.Set(m_initModelXform * mRotate);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
