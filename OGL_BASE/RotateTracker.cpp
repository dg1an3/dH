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

FUNCTION_FACTORY2(ComputeRotMatrix, CMatrix<4>)

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
	// store the current projection matrix from the view
	m_initProjMatrix = m_pView->camera.projection.Get();
	m_initModelXform = m_pView->camera.modelXform.Get();
	// m_vInitDirection = m_pView->camera.direction.Get();

	// store the current mouse position in 3-d
	m_vInitPoint = m_pView->ModelPtFromWndPt(point, m_initProjMatrix);
}

void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	// compute the final point
	CVector<3> vFinalPoint = 
		m_pView->ModelPtFromWndPt(point, m_initProjMatrix);

	CMatrix<4> mRotate = ComputeRotMatrix(vFinalPoint, m_vInitPoint);

#ifndef NONE
	m_pView->camera.modelXform.Set(m_initModelXform * mRotate);

#else
	// mRotate.Transpose();
	mRotate = m_initModelXform * mRotate; // * Transpose(m_initModelXform);

	// now solve for the three angles
	double new_phi = acos(mRotate[2][2]);
	double sin_phi = sin(new_phi);
	double new_theta = 0.0;
	double new_kappa = 0.0;
	if (sin_phi > EPS)
	{
		new_theta = AngleFromSinCos(mRotate[0][2] / sin_phi,
			-mRotate[1][2] / sin_phi);
		new_kappa = AngleFromSinCos(mRotate[2][0] / sin_phi,
			mRotate[2][1] / sin_phi);
	}

	// TRACE2("Angles in CRotateTracker: phi = %lf\t theta = %lf\n",
	//	phi, theta);

	// CMatrix<4> mDirRotate = CreateRotate(phi, CVector<3>(1.0, 0.0, 0.0))
	//	* CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0));
	// TRACE_MATRIX4("mDirRotate in RotateTracker = ", mDirRotate);

	// mDirRotate.Invert();

	// CVector<4> vNewDirection = mDirRotate * CVector<4>(0.0, 0.0, -1.0, 1.0);
			// * ToHomogeneous(m_vInitDirection);
	// m_pView->camera.direction.Set(
	//	FromHomogeneous<3, double>(vNewDirection));
	m_pView->camera.phi.Set(new_phi);
	m_pView->camera.theta.Set(new_theta);
	m_pView->camera.rollAngle.Set(-new_kappa);

	// mDirRotate.Invert();

	// TRACE_MATRIX4("mDirRotate in RotateTracker = ", mDirRotate);
	// TRACE_MATRIX4("modelXform in RotateTracker = ", m_pView->camera.modelXform.Get());
	// ASSERT(m_pView->camera.modelXform.Get().IsApproxEqual(mDirRotate));

	// set the camera roll angle
	// m_pView->camera.rollAngle.Set(kappa);
#endif

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
