// OpenGLTracker.h: interface for the COpenGLTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(OPENGLTRACKER_H)
#define OPENGLTRACKER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forward declaration of the view class
class COpenGLView;

//////////////////////////////////////////////////////////////////////
// class COpenGLTracker
//
// responds to mouse events on the view
//////////////////////////////////////////////////////////////////////
class COpenGLTracker : public CObject
{
public:
	// constructor/destructor
	COpenGLTracker(COpenGLView *pView);
	virtual ~COpenGLTracker();

	// test for inside
	virtual BOOL IsInside(CPoint point);

	// handlers for button click events
	virtual void OnButtonDown(UINT nFlags, CPoint point);	
	virtual void OnButtonUp(UINT nFlags, CPoint point);

	// handlers for mouse move events
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

protected:
	// pointer to the view
	COpenGLView *m_pView;
};

#endif // !defined(OPENGLTRACKER_H)