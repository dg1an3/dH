// Tracker.h: interface for the CTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(Tracker_H)
#define Tracker_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forward declaration of the view class
class CSceneView;

//////////////////////////////////////////////////////////////////////
// class CTracker
//
// responds to mouse events on the view
//////////////////////////////////////////////////////////////////////
class CTracker : public CObject
{
public:
	// constructor/destructor
	CTracker(CSceneView *pView);
	virtual ~CTracker();

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
	CSceneView *m_pView;
};

#endif // !defined(Tracker_H)