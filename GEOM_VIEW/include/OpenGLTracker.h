// OpenGLTracker.h: interface for the COpenGLTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLTRACKER_H__1369AB31_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_OPENGLTRACKER_H__1369AB31_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Observer.h>

class COpenGLView;

class COpenGLTracker : public CObservableObject
{
public:
	COpenGLTracker(COpenGLView *pView);
	virtual ~COpenGLTracker();

	virtual BOOL IsInside(CPoint point) { return TRUE; }

	virtual void OnLButtonDown(UINT nFlags, CPoint point) { }
	virtual void OnLButtonUp(UINT nFlags, CPoint point)   { }
	virtual void OnMouseMove(UINT nFlags, CPoint point)   { }
	virtual void OnMouseDrag(UINT nFlags, CPoint point)   { }

protected:
	COpenGLView *m_pView;
};

#endif // !defined(AFX_OPENGLTRACKER_H__1369AB31_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
