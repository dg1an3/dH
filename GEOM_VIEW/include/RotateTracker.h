// RotateTracker.h: interface for the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ROTATETRACKER_H)
#define ROTATETRACKER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// matrix/vector include files
#include <Vector.h>
#include <Matrix.h>

// base class
#include "OpenGLTracker.h"

//////////////////////////////////////////////////////////////////////
// class CRotateTracker
//
// tracker for rotating the view about a central point
//////////////////////////////////////////////////////////////////////
class CRotateTracker : public COpenGLTracker  
{
public:
	// constructor/destructor
	CRotateTracker(COpenGLView *pView);
	virtual ~CRotateTracker();

	// over-rides for handling button down and drag
	virtual void OnButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	// stores the initial point (in 3-d)
	CVector<3> m_vInitPoint;

	// stores the initial projection matrix
	CMatrix<4> m_initProjMatrix;

	// stores the initial view xform
	CMatrix<4> m_initXform;
};

#endif // !defined(ROTATETRACKER_H)
