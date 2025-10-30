// BrimstoneView.h : interface of the CBrimstoneView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_)
#define AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_

#include <Graph.h>
#include "PlanarView.h"	// Added by ClassView
#include "OptThread.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CBrimstoneView : public CView
{
protected: // create from serialization only
	CBrimstoneView();
	DECLARE_DYNCREATE(CBrimstoneView)

// Attributes
public:
	CBrimstoneDoc* GetDocument();

	// planar view of images / contours / dose
	CPlanarView m_wndPlanarView;

	// colormap for the dose display
	CArray<COLORREF, COLORREF> m_arrColormap;

	// graph to display the histogram
	CGraph m_graphDVH;

	// graph to display iterations
	CGraph m_graphIterations;
	CDataSeries m_dsIter;

	// flag to indicate structure colorwash ??? 
	BOOL m_bColorwashStruct;

	// generates a histogram for the specified structure
	void AddHistogram(CStructure * pStruct);

	// removes histogram for designated structure
	void RemoveHistogram(CStructure * pStruct);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	void ScanBeamlets(int nLevel);
	void DrawContours(CDC *pDC, const CRect& rect, CVolume<VOXEL_REAL> *pDens);
	// void OnHistogramChange(CObservableEvent *, void *);
	// void OnDVHChanged();
	virtual ~CBrimstoneView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// my optimizer thread pointer
	COptThread *m_pOptThread;

	// flag when optimizer should continue running
	bool m_bOptimizerRun;

// Generated message map functions
protected:
	//{{AFX_MSG(CBrimstoneView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnScanbeamletsG0();
	afx_msg void OnViewStructColorwash();
	afx_msg void OnUpdateViewStructColorwash(CCmdUI* pCmdUI);
	afx_msg void OnScanbeamletsG1();
	afx_msg void OnScanbeamletsG2();
	afx_msg void OnOptimize();
	afx_msg void OnUpdateOptimize(CCmdUI *pCmdUI);
	// custom handlers for optimizer thread messages
	afx_msg LRESULT OnOptimizerThreadUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOptimizerThreadDone(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// stores data series for iteration graph
	CDataSeries *m_pIterDS;
};

#ifndef _DEBUG  // debug version in BrimstoneView.cpp
inline CBrimstoneDoc* CBrimstoneView::GetDocument()
   { return (CBrimstoneDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_)
