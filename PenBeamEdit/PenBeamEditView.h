// PenBeamEditView.h : interface of the CPenBeamEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PENBEAMEDITVIEW_H__34348F48_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_PENBEAMEDITVIEW_H__34348F48_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#include <Dib.h>	// Added by ClassView
#include "Graph.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPenBeamEditView : public CView
{
protected: // create from serialization only
	CPenBeamEditView();
	DECLARE_DYNCREATE(CPenBeamEditView)

// Attributes
public:
	CPenBeamEditDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPenBeamEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	CGraph m_graph;
	CArray<COLORREF, COLORREF> m_arrColormap;
	virtual ~CPenBeamEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPenBeamEditView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in PenBeamEditView.cpp
inline CPenBeamEditDoc* CPenBeamEditView::GetDocument()
   { return (CPenBeamEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PENBEAMEDITVIEW_H__34348F48_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
