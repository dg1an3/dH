#if !defined(AFX_SIMVIEW1_H__601FB7E1_EE42_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_SIMVIEW1_H__601FB7E1_EE42_11D4_9E36_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimView1.h : header file
//

#include "Plan.h"
#include "Beam.h"

#include <DRRRenderable.h>

#include "SceneView.h"	// Added by ClassView

#include "BeamRenderable.h"		// Added by ClassView
#include "SurfaceRenderable.h"	// Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CSimView view

class CSimView : public CView //, public CObserver
{
protected:
	CSimView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSimView)

// Attributes
public:
	CPlan* GetDocument();

// Operations
public:
	void SetBEVPerspective(CBeam& beam);
	CMatrix<4> ComputeProjection(CBeam& beam);

	// association to the currently selected beam
	CBeam *m_pCurrentBeam;

	BOOL m_bPatientEnabled;

	BOOL m_bWireFrame;

	BOOL m_bColorWash;

	CSceneView m_wndREV;
	CSceneView m_wndBEV;

	double m_sliderPos;

#ifdef SHOW_ORTHO
	CSceneView m_wndOrtho[3];
#endif

	CSurfaceRenderable *m_pSurfaceRenderable;
	CBeamRenderable *m_pBeamRenderable;
	CDRRRenderable *m_pDRRRenderable;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

// Implementation
protected:
	virtual ~CSimView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSimView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewBeam();
	afx_msg void OnUpdateViewBeam(CCmdUI* pCmdUI);
	afx_msg void OnViewLightfield();
	afx_msg void OnUpdateViewLightfield(CCmdUI* pCmdUI);
	afx_msg void OnViewWireframe();
	afx_msg void OnUpdateViewWireframe(CCmdUI* pCmdUI);
	afx_msg void OnViewColorwash();
	afx_msg void OnUpdateViewColorwash(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#ifndef _DEBUG  // debug version in BeamEditView.cpp
inline CPlan* CSimView::GetDocument()
   { return (CPlan*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMVIEW1_H__601FB7E1_EE42_11D4_9E36_00B0D0609AB0__INCLUDED_)
