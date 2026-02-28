//////////////////////////////////////////////////////////////////////
// ZoomTracker.cpp: implementation of the CZoomTracker class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// class declaration
#include "ZoomTracker.h"

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
// CZoomTracker::CZoomTracker
// 
// creates a zoom tracker for the sceneview
//////////////////////////////////////////////////////////////////////
CZoomTracker::CZoomTracker(CSceneView *pView)
	: CTracker(pView)
{
}


//////////////////////////////////////////////////////////////////////
// CZoomTracker::~CZoomTracker
// 
// destroys the zoom tracker
//////////////////////////////////////////////////////////////////////
CZoomTracker::~CZoomTracker()
{
}


//////////////////////////////////////////////////////////////////////
// CZoomTracker::OnButtonDown
// 
// begins processing of the ZOOM
//////////////////////////////////////////////////////////////////////
void CZoomTracker::OnButtonDown(UINT nFlags, CPoint point)
{
#ifdef _DEBUG
	// check the camera's state
	m_pView->GetCamera().AssertValid();
#endif

	// store the initial zoom factor
	m_initZoom = m_pView->GetCamera().GetDistance();

	// get the client rectangle
	CRect rect;
	m_pView->GetClientRect(&rect);

	// compute the relative y position
	m_initY = (double) point.y / (double) rect.Height();
}


//////////////////////////////////////////////////////////////////////
// CZoomTracker::OnMouseDrag
// 
// processes the mouse-drag ZOOM
//////////////////////////////////////////////////////////////////////
void CZoomTracker::OnMouseDrag(UINT nFlags, CPoint point)
{
#ifdef _DEBUG
	// check the camera's state
	m_pView->GetCamera().AssertValid();
#endif

	// get the client rectangle
	CRect rect;
	m_pView->GetClientRect(&rect);

	// compute the relative y position
	double currY = (double) point.y / (double) rect.Height();

	// compute the zoom factor
	double zoom = // m_initZoom * 
		exp(2.0 * (m_initY - currY));

	// set the new model xform
	m_pView->GetCamera().SetDistance(m_initZoom / zoom);
	// m_pView->GetCamera().SetZoom(zoom);

	// redraw the window
	m_pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
