//////////////////////////////////////////////////////////////////////
// RotateTracker.cpp: implementation of the CRotateTracker class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// OpenGL includes
#include "glMatrixVector.h"

// class declaration
#include "RotateTracker.h"

// parent class
#include "SceneView.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CRotateTracker::CRotateTracker
// 
// creates a rotate tracker for the sceneview
//////////////////////////////////////////////////////////////////////
CRotateTracker::CRotateTracker(CSceneView *pView)
	: CTracker(pView)
{
}


//////////////////////////////////////////////////////////////////////
// CRotateTracker::~CRotateTracker
// 
// destroys the rotate tracker
//////////////////////////////////////////////////////////////////////
CRotateTracker::~CRotateTracker()
{
}


//////////////////////////////////////////////////////////////////////
// CRotateTracker::OnButtonDown
// 
// begins processing of the rotation
//////////////////////////////////////////////////////////////////////
void CRotateTracker::OnButtonDown(UINT nFlags, CPoint point)
{
#ifdef _DEBUG
	// check the camera's state
	m_pView->GetCamera().AssertValid();
#endif

	// store the initial model transform matrix; subsequent rotations will be applied
	//		to this matrix directly
	m_initXform = m_pView->GetCamera().GetXform();

	// store the current projection matrix from the view, for transforming
	//		subsequent coordinates
	m_initProjMatrix = m_pView->GetCamera().GetProjection();

	// store the current mouse position in 3-d
	m_vInitPoint = m_pView->ModelPtFromWndPt(point, m_initProjMatrix);
}


//////////////////////////////////////////////////////////////////////
// CRotateTracker::OnMouseDrag
// 
// processes the mouse-drag rotation
//////////////////////////////////////////////////////////////////////
void CRotateTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
#ifdef _DEBUG
	// check the camera's state
	m_pView->GetCamera().AssertValid();
#endif

	// compute the final point
	CVector<3> vFinalPoint = 
		m_pView->ModelPtFromWndPt(point, m_initProjMatrix);

	// compute the rotation matrix
	CMatrix<4> mRotate = 
		CMatrix<4>(CreateRotate(m_vInitPoint, vFinalPoint, 3.0));

	// set the new model xform to the initial composed with the rotation
	m_pView->GetCamera().SetXform(m_initXform * mRotate);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
