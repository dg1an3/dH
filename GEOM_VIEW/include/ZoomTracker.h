//////////////////////////////////////////////////////////////////////
// ZoomTracker.h: declaration of the CZoomTracker class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(ZOOMTRACKER_H)
#define ZOOMTRACKER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// matrix/vector include files
#include <Vector.h>
#include <Matrix.h>

// base class
#include "Tracker.h"

//////////////////////////////////////////////////////////////////////
// class CZoomTracker
//
// tracker for zooming the view
//////////////////////////////////////////////////////////////////////
class CZoomTracker : public CTracker  
{
public:
	// constructor/destructor
	CZoomTracker(CSceneView *pView);
	virtual ~CZoomTracker();

	// over-rides for handling button down and drag
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	// stores the initial projection matrix
	CMatrix<4> m_initXform;

	// stores the initial y position
	double m_initY;
};

#endif // !defined(ZOOMTRACKER_H)
