//////////////////////////////////////////////////////////////////////
// RotateTracker.cpp: implementation of the CRotateTracker class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

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

	// local reference to the view's camera
	CCamera& camera = m_pView->GetCamera();

	// store the current mouse position transformed to model space
	m_vPrevPoint = m_pView->ModelPtFromWndPt(point);
	m_vPrevPoint -= camera.GetTarget();
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

	// local reference to the view's camera
	CCamera& camera = m_pView->GetCamera();

	// compute the final point
	CVectorD<3> vCurrPoint = m_pView->ModelPtFromWndPt(point);
	vCurrPoint -= camera.GetTarget();

	// compute the rotation matrix
	CMatrixD<3> mRotate = CreateRotate(m_vPrevPoint, vCurrPoint, 4.0);

	// rotate the camera's direction
	CVectorD<3> vNewDir = mRotate * camera.GetDirection();

	// set the new direction
	camera.SetDirection(vNewDir);

	// compute the new previous point
	m_vPrevPoint = m_pView->ModelPtFromWndPt(point);
	m_vPrevPoint -= camera.GetTarget();

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
