// Graph.h : header file
//

#if !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>
#include <MatrixNxM.h>

#include <ModelObject.h>

// #include <Histogram.h>

#include <Dib.h>

#include <DataSeries.h>
#include "atltypes.h"

/////////////////////////////////////////////////////////////////////////////
// CGraph window

class CGraph : public CWnd
{
// Construction
public:
	CGraph();
	virtual ~CGraph();

// Attributes
public:

	// accessors for data series
	int GetDataSeriesCount();
	CDataSeries *GetDataSeriesAt(int nAt);
	void AddDataSeries(CDataSeries *pSeries);
	void RemoveDataSeries(int nAt, bool bDelete = false);
	void RemoveAllDataSeries(bool bDelete = false);

// Operations
public:

	// sets up graph margins
	void SetMargins(int nLeft, int nTop, int nRight, int nBottom);

	// sets the axes ranges and tick marks
	DECLARE_ATTRIBUTE(AxesMin, CVectorD<2>);
	DECLARE_ATTRIBUTE(AxesMax, CVectorD<2>);
//	const CVectorD<2>& GetAxesMin();
//	void SetAxesMin(const CVectorD<2>& vMin);	
//	const CVectorD<2>& GetAxesMax();
//	void SetAxesMax(const CVectorD<2>& vMin);	

	DECLARE_ATTRIBUTE(AxesMajor, CVectorD<2>);
	DECLARE_ATTRIBUTE(AxesMinor, CVectorD<2>);
//	const CVectorD<2>& GetAxesMinor();
//	void SetAxesMinor(const CVectorD<2>& vMinor);

	// computes the min and max values for the graph
	void AutoScale();
	DECLARE_ATTRIBUTE(TruncateZeroTail, bool);

	// sets up the legend (legend only displayed after this is called)
	void SetLegendLUT(CArray<COLORREF, COLORREF>&  arrLUT, REAL window, REAL level);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraph)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraph)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Implementation
public:

	// draws the minor ticks and grids
	void DrawMinorAxes(CDC * pDC, const CRect& rect);

	// draw major ticks and grids
	void DrawMajorAxes(CDC * pDC, const CRect& rect);

	void DrawSeries(CDC * pDC, const CRect& rect);

	void DrawLegend(CDC * pDC, const CRect& rect);

	// converts to coordinates on the plot
	CPoint ToPlotCoord(const CVectorD<2>& vCoord);
	CVectorD<2> FromPlotCoord(const CPoint& vCoord);

	// called when one of my data series changes
	void OnDataSeriesChanged(CObservableEvent * pOE, void * pParam);

private:
	// the array of data series
	CTypedPtrArray<CObArray, CDataSeries*> m_arrDataSeries;

	// graph plot area
	int m_arrMargins[4];

	// min, max, and major 
//	CVectorD<2> m_vMax;
//	CVectorD<2> m_vMin;
//	CVectorD<2> m_vMajor;
//	CVectorD<2> m_vMinor;

	// dragging logic
	CDataSeries *m_pDragSeries;
	int m_nDragPoint;
	CSize m_ptDragOffset;
	BOOL m_bDragging;

	// legend variables
	CArray<COLORREF, COLORREF> m_arrLegendLUT;
	REAL m_window;
	REAL m_level;
	bool m_bShowLegend;

	// graph buffer
	CDib m_dib;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
