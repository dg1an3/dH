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

void CRotateTracker::OnButtonDown(UINT nFlags, CPoint point)
{
	// store the initial model transform matrix; subsequent rotations will be applied
	//		to this matrix directly
	m_initXform = m_pView->GetCamera().GetModelXform();

	// store the current projection matrix from the view, for transforming
	//		subsequent coordinates
	m_initProjMatrix = m_pView->GetCamera().GetProjection();

	// store the current mouse position in 3-d
	m_vInitPoint = m_pView->ModelPtFromWndPt(point, m_initProjMatrix);
}

void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
	// compute the final point
	CVector<3> vFinalPoint = 
		m_pView->ModelPtFromWndPt(point, m_initProjMatrix);

	// compute the rotation matrix
	CMatrix<4> mRotate = 
		CMatrix<4>(CreateRotate(m_vInitPoint, vFinalPoint, 3.0));

	// set the new model xform to the initial composed with the rotation
	m_pView->GetCamera().SetModelXform(m_initXform * mRotate);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
