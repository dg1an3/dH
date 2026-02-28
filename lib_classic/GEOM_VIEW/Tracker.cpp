// Tracker.cpp: implementation of the CTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTracker::CTracker(CSceneView *pView)
: m_pView(pView)
{

}

CTracker::~CTracker()
{

}

BOOL CTracker::IsInside(CPoint point) 
{ 
	return TRUE; 
}

void CTracker::OnButtonDown(UINT nFlags, CPoint point) 
{ 
}	

void CTracker::OnButtonUp(UINT nFlags, CPoint point)   
{ 
}	

void CTracker::OnMouseMove(UINT nFlags, CPoint point)  
{ 
}

void CTracker::OnMouseDrag(UINT nFlags, CPoint point)  
{ 
}
