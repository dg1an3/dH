#if !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include <vector>
// using namespace std;
#include <ModelObject.h>

#include <VectorD.h>
#include <MatrixNxM.h>

#include <Histogram.h>

// Graph.h : header file
//

class CGraph;

/////////////////////////////////////////////////////////////////////////////
// CGraph window

class CDataSeries : public CModelObject 
{
public:
	CDataSeries();

	COLORREF GetColor() const;
	void SetColor(COLORREF color);

//	CObject * m_pObject;
//	void SetObject(CObject *pObject);
//	CObject * GetObject();

	// accessors for the data series data
	const CMatrixNxM<>& GetDataMatrix() const;
	void SetDataMatrix(const CMatrixNxM<>& mData);
	void AddDataPoint(const CVectorD<2>& vDataPt);

	// flag to indicate whether the data series should have handles
	//		for interaction
	BOOL HasHandles() const;
	void SetHasHandles(BOOL bHandles = TRUE);

	// accessors to indicate the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	int GetMonotonicDirection() const;
	void SetMonotonicDirection(int nDirection);

	CGraph *m_pGraph;

protected:
	// use a std vector instead of CArray, because CArray is bad
	CMatrixNxM<> m_mData;	

	COLORREF m_color;

	// flag to indicate display of handles for the series
	BOOL m_bHandles;

	// indicates the data series monotonic direction: 
	//		-1 for decreasing, 
	//		1 for increasing,
	//		0 for not monotonic
	int m_nMonotonicDirection;
};

class CHistogramDataSeries : public CDataSeries
{
public:
	CHistogramDataSeries(CHistogram *pHisto);
	
	void OnHistogramChanged(CObservableEvent *, void *);

private:
	CHistogram *m_pHisto;
};


class CGraph : public CWnd
{
// Construction
public:
	CGraph();
	virtual ~CGraph();

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

	// computes the min and max values for the graph
	void ComputeMinMax();

	// converts to coordinates on the plot
	CPoint ToPlotCoord(const CVectorD<2>& vCoord);
	CVectorD<2> FromPlotCoord(const CPoint& vCoord);

	// accessors for data series
	int GetDataSeriesCount();
	CDataSeries *GetDataSeriesAt(int nAt);
	void AddDataSeries(CDataSeries *pSeries);
	void RemoveAllDataSeries();

private:
	// the array of data series
	CObArray m_arrDataSeries;

	// min, max, and major 
	CVectorD<2> m_vMax;
	CVectorD<2> m_vMin;
	CVectorD<2> m_vMajor;

	CDataSeries *m_pDragSeries;
	int m_nDragPoint;
	CSize m_ptDragOffset;
	BOOL m_bDragging;

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraph)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPH_H__37CA0912_5524_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
