// ZoomTracker.h: interface for the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ZOOMTRACKER_H)
#define ZOOMTRACKER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>
#include <Matrix.h>

#include "OpenGLTracker.h"

class CZoomTracker : public COpenGLTracker  
{
public:
	CZoomTracker(COpenGLView *pView);
	virtual ~CZoomTracker();

	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	double privInitY;

	CMatrix<4> privInitProjMatrix;

	double privCurrY;

	CMatrix<4> privCurrProjMatrix;
};

#endif // !defined(ZOOMTRACKER_H)
