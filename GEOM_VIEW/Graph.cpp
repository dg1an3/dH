// Graph.cpp : implementation file
//

#include "stdafx.h"

#include <float.h>

#include "Graph.h"

#include <MatrixBase.inl>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDataSeries::CDataSeries() 
	: m_color(RGB(255, 0, 0)), 
		m_bHandles(FALSE),
		m_pObject(NULL)
{ 
}

COLORREF CDataSeries::GetColor() const
{
	return m_color;
}

void CDataSeries::SetColor(COLORREF color)
{
	m_color = color;
	GetChangeEvent().Fire();
}

// accessors for the data series data
const CMatrixNxM<>& CDataSeries::GetDataMatrix() const
{
	return m_mData;
}

void CDataSeries::SetDataMatrix(const CMatrixNxM<>& mData)
{
	m_mData = mData;
	GetChangeEvent().Fire();
}

void CDataSeries::AddDataPoint(const CVectorD<2>& vDataPt)
{
	// TODO: fix Reshape to correct this problem
	// ALSO TODO: CMatrixNxM serialization
	CMatrixNxM<> mTemp(m_mData.GetCols()+1, 2);
	for (int nAt = 0; nAt < m_mData.GetCols(); nAt++)
	{
		mTemp[nAt][0] = m_mData[nAt][0];
		mTemp[nAt][1] = m_mData[nAt][1];
	}

	mTemp[mTemp.GetCols()-1][0] = vDataPt[0];
	mTemp[mTemp.GetCols()-1][1] = vDataPt[1];

	m_mData = mTemp;

	GetChangeEvent().Fire();
}

// flag to indicate whether the data series should have handles
//		for interaction
BOOL CDataSeries::HasHandles() const
{
	return m_bHandles;
}

void CDataSeries::SetHasHandles(BOOL bHandles)
{
	m_bHandles = TRUE;
}

// accessors to indicate the data series monotonic direction: 
//		-1 for decreasing, 
//		1 for increasing,
//		0 for not monotonic
int CDataSeries::GetMonotonicDirection() const
{
	return m_nMonotonicDirection;
}

void CDataSeries::SetMonotonicDirection(int nDirection)
{
	m_nMonotonicDirection = nDirection;
}

/////////////////////////////////////////////////////////////////////////////
// CGraph

CGraph::CGraph()
: m_pDragSeries(NULL), m_bDragging(FALSE)
{
}

CGraph::~CGraph()
{
	RemoveAllDataSeries();
}


BEGIN_MESSAGE_MAP(CGraph, CWnd)
	//{{AFX_MSG_MAP(CGraph)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGraph message handlers

#define MARGIN 20

void CGraph::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (m_arrDataSeries.GetSize() == 0)
		return;

	ComputeMinMax();

	// draw the axes
	CRect rect;
	GetClientRect(&rect);

	dc.Rectangle(rect);
	
	CVectorD<2> vAt(m_vMin);
	dc.MoveTo(ToPlotCoord(vAt)); // MARGIN, rect.Height() - MARGIN);
	int nCount = 0;
	for (vAt[1] = m_vMin[1]; vAt[1] <= m_vMax[1]; vAt[1] += m_vMajor[1] / 10)
	{
		dc.LineTo(ToPlotCoord(vAt));
		dc.LineTo(ToPlotCoord(vAt) 
			+ (nCount % 10 == 0 ? CPoint(-10, 0) : CPoint(-5, 0)));
		dc.LineTo(ToPlotCoord(vAt));
		nCount++;
	}

	vAt = m_vMin;
	dc.MoveTo(ToPlotCoord(vAt)); // MARGIN, rect.Height() - MARGIN);
	nCount = 0;
	for (vAt[0] = m_vMin[0]; vAt[0] <= m_vMax[0]; vAt[0] += m_vMajor[0] / 10)
	{
		dc.LineTo(ToPlotCoord(vAt));
		dc.LineTo(ToPlotCoord(vAt) 
			+ (nCount % 10 == 0 ? CPoint(0, 10) : CPoint(0, 5)));
		dc.LineTo(ToPlotCoord(vAt));
		nCount++;
	}

	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];

		CPen pen(PS_DOT, 1, pSeries->GetColor());
		CPen *pOldPen = dc.SelectObject(&pen);

		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		dc.MoveTo(ToPlotCoord(CVectorD<2>(mData[0])));
		for (int nAtPoint = 1; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			dc.LineTo(ToPlotCoord(CVectorD<2>(mData[nAtPoint])));
		}

		CPen penHandle(PS_SOLID, 1, RGB(0, 0, 0));
		dc.SelectObject(&penHandle);
		if (pSeries->HasHandles())
		{
			CRect rectHandle(0, 0, 5, 5);
			for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
			{
				rectHandle.OffsetRect(ToPlotCoord(CVectorD<2>(mData[nAtPoint]))
					- rectHandle.CenterPoint());
				dc.Rectangle(rectHandle);
			}
		}

		dc.SelectObject(pOldPen);
	}

	// Do not call CWnd::OnPaint() for painting messages
}

CPoint CGraph::ToPlotCoord(const CVectorD<2>& vCoord)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(MARGIN, MARGIN, MARGIN, MARGIN);

	CPoint pt;
	pt.x = MARGIN + (int)(rect.Width() * (vCoord[0] - m_vMin[0]) 
		/ (m_vMax[0] - m_vMin[0]));
	pt.y = rect.Height() + MARGIN - (int)(rect.Height() * (vCoord[1] - m_vMin[1]) 
		/ (m_vMax[1] - m_vMin[1]));

	return pt;
}

CVectorD<2> CGraph::FromPlotCoord(const CPoint& pt)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(MARGIN, MARGIN, MARGIN, MARGIN);

	CVectorD<2> vCoord;
	vCoord[0] = (pt.x - MARGIN) * (m_vMax[0] - m_vMin[0]) 
		/ rect.Width() + m_vMin[0];
	vCoord[1] = -(pt.y - rect.Height() - MARGIN) * (m_vMax[1] - m_vMin[1]) 
		/ rect.Height() + m_vMin[1];  

	return vCoord;
}

void CGraph::ComputeMinMax()
{
	m_vMin[0] = 0;
	m_vMin[1] = 0;
	m_vMax[0] = 0;
	m_vMax[1] = 0;
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];
		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			m_vMin[0] = __min(m_vMin[0], mData[nAtPoint][0]);
			m_vMin[1] = __min(m_vMin[1], mData[nAtPoint][1]);
			m_vMax[0] = __max(m_vMax[0], mData[nAtPoint][0]);
			m_vMax[1] = __max(m_vMax[1], mData[nAtPoint][1]);
		}
	}

	m_vMajor[0] = pow(10, ceil(log10(m_vMax[0])) - 1);
	m_vMax[0] = m_vMajor[0] * (ceil(m_vMax[0] / m_vMajor[0]));

	m_vMajor[1] = pow(10, ceil(log10(m_vMax[1])) - 1);
	m_vMax[1] = m_vMajor[1] * (ceil(m_vMax[1] / m_vMajor[1]));
}

void CGraph::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bDragging)
	{
		m_pDragSeries = NULL;
		for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
		{
			m_pDragSeries = (CDataSeries *)m_arrDataSeries[nAt];
			const CMatrixNxM<>& mData = m_pDragSeries->GetDataMatrix();
			if (m_pDragSeries->HasHandles())
			{
				for (m_nDragPoint = 0; m_nDragPoint < mData.GetCols(); 
						m_nDragPoint++)
				{
					m_ptDragOffset = point 
						- ToPlotCoord(CVectorD<2>(mData[m_nDragPoint]));
					if (abs(m_ptDragOffset.cx) < 10 && abs(m_ptDragOffset.cy) < 10)
					{
						::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));

						CWnd::OnMouseMove(nFlags, point);
						return;
					}
				}
			}
		}
	}
	else
	{
		CMatrixNxM<> mData = m_pDragSeries->GetDataMatrix();
		mData[m_nDragPoint] = FromPlotCoord(point - m_ptDragOffset);

		if (m_nDragPoint > 0)
		{
			mData[m_nDragPoint][0] = __max(mData[m_nDragPoint][0], 
				mData[m_nDragPoint-1][0]);
			mData[m_nDragPoint][1] = __min(mData[m_nDragPoint][1], 
				mData[m_nDragPoint-1][1]);
		}

		if (m_nDragPoint < mData.GetCols()-1)
		{
			mData[m_nDragPoint][0] = __min(mData[m_nDragPoint][0], 
				mData[m_nDragPoint+1][0]);
			mData[m_nDragPoint][1] =__max(mData[m_nDragPoint][1], 
					mData[m_nDragPoint+1][1]);
		}


		if (m_nDragPoint == 0)
		{
			mData[m_nDragPoint][0] = 0.0;
		}

		if (m_nDragPoint == mData.GetCols()-1)
		{
			mData[m_nDragPoint][1] = 0.0;
		}
		m_pDragSeries->SetDataMatrix(mData);

		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));

		Invalidate();
		CWnd::OnMouseMove(nFlags, point);
		return;
	}

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	
	CWnd::OnMouseMove(nFlags, point);
}

void CGraph::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bDragging = TRUE;

	if (NULL != m_pDragSeries)
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CGraph::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDragging = FALSE;
	
	CWnd::OnLButtonUp(nFlags, point);
}

// accessors for data series
int CGraph::GetDataSeriesCount()
{
	return m_arrDataSeries.GetSize();
}

CDataSeries *CGraph::GetDataSeriesAt(int nAt)
{
	return (CDataSeries *) m_arrDataSeries.GetAt(nAt);
}

void CGraph::AddDataSeries(CDataSeries *pSeries)
{
	m_arrDataSeries.Add(pSeries);
}


void CGraph::RemoveAllDataSeries()
{
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		delete m_arrDataSeries[nAt];
	}

	m_arrDataSeries.RemoveAll();
}



CObject * CDataSeries::GetObject()
{
	return m_pObject;
}

void CDataSeries::SetObject(CObject *pObject)
{
	m_pObject = pObject;
}
