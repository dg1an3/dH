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

	// store the current mouse position in 3-d
	m_vPrevPoint = m_pView->ModelPtFromWndPt(point);
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
	CVectorD<3> vCurrPoint = m_pView->ModelPtFromWndPt(point);

	// compute the rotation matrix
	CMatrixD<4> mRotate = 
		CMatrixD<4>(CreateRotate(m_vPrevPoint, vCurrPoint, 3.0));

	// set the new model xform to the initial composed with the rotation
	CMatrixD<4> mXform = m_pView->GetCamera().GetXform();
	m_pView->GetCamera().SetXform(mXform * mRotate);

	// set the new initial point
	m_vPrevPoint = m_pView->ModelPtFromWndPt(point);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
