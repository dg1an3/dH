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
		upDirection(CVector<3>(0.0, 0.0, 1.0))
{
}

CRotateTracker::~CRotateTracker()
{

}

void CRotateTracker::OnLButtonDown(UINT nFlags, CPoint point)
{
	// store the current projection matrix from the view
	m_initProjMatrix = m_pView->projectionMatrix.Get();

	// store the current mouse position in 3-d
	m_vInitPoint = m_pView->ModelPtFromWndPt(point, &m_initProjMatrix);
}

void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	// compute the final point
	CVector<3> vFinalPoint = 
		m_pView->ModelPtFromWndPt(point, &m_initProjMatrix);

	// compute the new projection matrix
	CMatrix<4> mProj = m_initProjMatrix 
		* ComputeRotMatrix(m_vInitPoint, vFinalPoint);

	// see if we need to adjust for "up"
	if (upDirection.Get().GetLength() > 0.0)
	{
		// need to form the rotation so that the projected "up"
		//		vector is really up
		CVector<3> vProjUp = 
			FromHomogeneous<3, double>(mProj 
				* ToHomogeneous(upDirection.Get()));
				// - mProj * CVector<4>(0.0, 0.0, 0.0, 1.0));
		vProjUp[2] = 0.0;
		vProjUp.Normalize();
		TRACE_VECTOR3("vProjUp", vProjUp);

		// form the rotation of the projected vector
		double angle = -atan2(vProjUp[0], vProjUp[1]);
		TRACE1("angle = %lf\n", angle);

		// form the camera axis
		CVector<3> vCameraAxis = 
			FromHomogeneous<3, double>(Invert(mProj) 
				* CVector<4>(0.0, 0.0, 1.0, 1.0));
		TRACE_VECTOR3("Camera axis", vCameraAxis);

		// now rotate about the central axis by the angle
		mProj = mProj * CreateRotate(angle, vCameraAxis);

		// check to ensure we are straight up
		vProjUp = FromHomogeneous<3, double>(mProj 
			* ToHomogeneous(upDirection.Get()));
		vProjUp[2] = 0.0;
		vProjUp.Normalize();
		TRACE_VECTOR3("vProjUp After Adjustment", vProjUp);
	}

	// set the new projection matrix
	m_pView->projectionMatrix.Set(mProj);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
