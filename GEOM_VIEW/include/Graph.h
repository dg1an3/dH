#if !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>

// Graph.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGraph window

class CDataSeries : public CObject 
{
public:
	CDataSeries() : m_color(RGB(255, 0, 0)) { }
//	virtual ~CDataSeries();

	COLORREF m_color;
	CArray< CVector<2>, CVector<2> > m_arrData;
};

class CGraph : public CWnd
{
// Construction
public:
	CGraph();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraph)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ComputeMinMax();
	CPoint ToPlotCoord(CVector<2> vCoord);
	CObArray m_arrDataSeries;

	CVector<2> m_vMax;
	CVector<2> m_vMin;
	CVector<2> m_vMajor;

	virtual ~CGraph();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraph)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
