//////////////////////////////////////////////////////////////////////
// RotateTracker.h: declaration of the CRotateTracker class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(ROTATETRACKER_H)
#define ROTATETRACKER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// matrix/vector include files
#include <VectorD.h>
#include <MatrixD.h>

// base class
#include "Tracker.h"

//////////////////////////////////////////////////////////////////////
// class CRotateTracker
//
// tracker for rotating the view about a central point
//////////////////////////////////////////////////////////////////////
class CRotateTracker : public CTracker  
{
public:
	// constructor/destructor
	CRotateTracker(CSceneView *pView);
	virtual ~CRotateTracker();

	// over-rides for handling button down and drag
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	// stores the initial point (in 3-d)
	CVectorD<3> m_vPrevPoint;
};

#endif // !defined(ROTATETRACKER_H)
