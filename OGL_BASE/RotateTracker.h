// RotateTracker.h: interface for the CRotateTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>

#include <Vector.h>
#include <Matrix.h>

#include "OpenGLTracker.h"

class CRotateTracker : public COpenGLTracker  
{
public:
	CRotateTracker(COpenGLView *pView);
	virtual ~CRotateTracker();

	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

private:
	// stores the initial point (in 3-d)
	CVector<3> m_vInitPoint;

	// stores the initial projection matrix
	CMatrix<4> m_initProjMatrix;
	// CMatrix<4> m_initModelXform;
	CVector<3> m_vInitDirection;
};

#endif // !defined(AFX_ROTATETRACKER_H__1369AB32_F2ED_11D4_9E3C_00B0D0609AB0__INCLUDED_)
