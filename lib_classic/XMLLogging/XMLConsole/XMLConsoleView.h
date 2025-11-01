// XMLConsoleView.h : interface of the CXMLConsoleView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLCONSOLEVIEW_H__15580994_FAE7_4A11_808E_4D17DE60A919__INCLUDED_)
#define AFX_XMLCONSOLEVIEW_H__15580994_FAE7_4A11_808E_4D17DE60A919__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CXMLConsoleView : public CListView
{
protected: // create from serialization only
	CXMLConsoleView();
	DECLARE_DYNCREATE(CXMLConsoleView)

// Attributes
public:
	CXMLConsoleDoc* GetDocument();

	CTreeCtrl * m_pTreeCtrl;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLConsoleView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXMLConsoleView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CXMLConsoleView)
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in XMLConsoleView.cpp
inline CXMLConsoleDoc* CXMLConsoleView::GetDocument()
   { return (CXMLConsoleDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XMLCONSOLEVIEW_H__15580994_FAE7_4A11_808E_4D17DE60A919__INCLUDED_)
