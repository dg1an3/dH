//////////////////////////////////////////////////////////////////////
// OpenGLView.h: declaration of the COpenGLView class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLVIEW_H__0C017CB0_E5B8_11D4_9E2E_00B0D0609AB0__INCLUDED_)
#define AFX_OPENGLVIEW_H__0C017CB0_E5B8_11D4_9E2E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Collection.h>

#include <Matrix.h>

#include "OpenGLRenderer.h"
#include "OpenGLCamera.h"
#include "OpenGLLight.h"
#include "OpenGLTracker.h"

//////////////////////////////////////////////////////////////////////
// class COpenGLView
//
// represents a window with an OpenGL rendering context.
//////////////////////////////////////////////////////////////////////
class COpenGLView : public CWnd
{
public:
	COpenGLView();           // protected constructor used by dynamic creation
	virtual ~COpenGLView();

	// make sure we can create this window dynamically (for splitter window)
	DECLARE_DYNCREATE(COpenGLView)

// Attributes
public:
	
	// the background color for the view
	CValue< COLORREF > backgroundColor;

	// collection of renderers
	CCollection< COpenGLRenderer > renderers;

	// the view's camera
	COpenGLCamera camera;

	// stores the lights for the view
	CCollection< COpenGLLight > lights;

	// collection of trackers
	CCollection< COpenGLTracker > leftTrackers;
	CCollection< COpenGLTracker > middleTrackers;

// Operations
public:
	// compute the model point from a window coordinate
	CVector<3> ModelPtFromWndPt(CPoint wndPt);

	// compute the model point from a window coordinate, using a different
	//		projection matrix
	CVector<3> ModelPtFromWndPt(CPoint wndPt, const CMatrix<4>& mProj);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenGLView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// helper function to set up the OpenGL pixel format
	BOOL SetupPixelFormat(void);

	// call to make this view's HGLRC the current one (so that future OpenGL calls
	//		will affect its rendering state
	void MakeCurrentGLRC();

	// friend classes can access the OpenGL rendering context
	friend class COpenGLRenderer;
	friend class COpenGLTexture;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(COpenGLView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// holds the device context for OpenGL rendering
	CClientDC *m_pDC;

	// holds the OpenGL rendering context
	HGLRC m_hrc;

	// flag to indicate that the dragging is occurring
	BOOL m_bDragging;
	BOOL m_bMiddleDragging;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENGLVIEW_H__0C017CB0_E5B8_11D4_9E2E_00B0D0609AB0__INCLUDED_)
