// RotateTracker.h: interface for the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLTracker.h"
#include "Vector.h"
#include "Matrix.h"

#include "Value.h"
#include "Function.h"

class CRotateTracker : public COpenGLTracker  
{
public:
	CRotateTracker(COpenGLView *pView);
	virtual ~CRotateTracker();

	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	CValue< CVector<3> > privInitPoint;

	CValue< CMatrix<4> > privInitProjMatrix;

	CValue< CVector<3> > privCurrPoint;

	CValue< CMatrix<4> >& privCurrProjMatrix;
};

#endif // !defined(AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
