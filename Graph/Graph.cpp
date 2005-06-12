// Graph.cpp : implementation file
//

#include "stdafx.h"

#include <float.h>

#include "Graph.h"

#include <CastVectorD.h>
#include ".\include\graph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int DEFAULT_MARGIN = 50;


/////////////////////////////////////////////////////////////////////////////
// CGraph::CGraph
/////////////////////////////////////////////////////////////////////////////
CGraph::CGraph()
	: m_pDragSeries(NULL)
		, m_bDragging(FALSE)
		, m_bShowLegend(false)
{
	SetMargins(DEFAULT_MARGIN, DEFAULT_MARGIN, DEFAULT_MARGIN * 4, 
		DEFAULT_MARGIN);

}	// CGraph::CGraph

/////////////////////////////////////////////////////////////////////////////
// CGraph::~CGraph
//
// accessors for data series
/////////////////////////////////////////////////////////////////////////////
CGraph::~CGraph()
{
	RemoveAllDataSeries();

}	// CGraph::~CGraph

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetDataSeriesCount
//
// accessors for data series
/////////////////////////////////////////////////////////////////////////////
int CGraph::GetDataSeriesCount()
{
	return (int) m_arrDataSeries.GetSize();

}	// CGraph::GetDataSeriesCount

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetDataSeriesAt
/////////////////////////////////////////////////////////////////////////////
CDataSeries *CGraph::GetDataSeriesAt(int nAt)
{
	return (CDataSeries *) m_arrDataSeries.GetAt(nAt);
}

/////////////////////////////////////////////////////////////////////////////
// CGraph::AddDataSeries
/////////////////////////////////////////////////////////////////////////////
void CGraph::AddDataSeries(CDataSeries *pSeries)
{
	pSeries->m_pGraph = this;
	m_arrDataSeries.Add(pSeries);

}	// CGraph::AddDataSeries

/////////////////////////////////////////////////////////////////////////////
// CGraph::RemoveAllDataSeries
/////////////////////////////////////////////////////////////////////////////
void CGraph::RemoveAllDataSeries(bool bDelete)
{
	if (bDelete)
	{
		for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
		{
			delete m_arrDataSeries[nAt];
		}
	}

	m_arrDataSeries.RemoveAll();

}	// CGraph::RemoveAllDataSeries


/////////////////////////////////////////////////////////////////////////////
// CGraph::GetAxesMin
//
// returns the axes ranges
/////////////////////////////////////////////////////////////////////////////
const CVectorD<2>& CGraph::GetAxesMin()
{
	return m_vMin;

}	// CGraph::GetAxesMin

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetAxesMin
//
// sets the axes ranges
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetAxesMin(const CVectorD<2>& vMin)
{
	m_vMin = vMin;

}	// CGraph::SetAxesMin

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetAxesMax
//
// returns the axes ranges
/////////////////////////////////////////////////////////////////////////////
const CVectorD<2>& CGraph::GetAxesMax()
{
	return m_vMax;

}	// CGraph::GetAxesMin

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetAxesMax
//
// sets the axes ranges
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetAxesMax(const CVectorD<2>& vMax)
{
	m_vMax = vMax;

}	// CGraph::SetAxesMin

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetAxesMinor
//
// gets the axes ranges
/////////////////////////////////////////////////////////////////////////////
const CVectorD<2>& CGraph::GetAxesMinor()
{
	return m_vMinor;

}	// CGraph::GetAxesMinor

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetAxesMinor
//
// sets the axes ranges
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetAxesMinor(const CVectorD<2>& vMinor)
{
	m_vMinor = vMinor;

}	// CGraph::SetAxesMinor

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetMargins
//
// sets the graph margins
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_arrMargins[0] = nLeft;
	m_arrMargins[1] = nTop;
	m_arrMargins[2] = nRight;
	m_arrMargins[3] = nBottom;

}	// CGraph::SetMargins

/////////////////////////////////////////////////////////////////////////////
// CGraph::AutoScale
//
// determines graph range and tick marks
/////////////////////////////////////////////////////////////////////////////
void CGraph::AutoScale()
{
	// accumulate mins and maxes
	m_vMin[0] = FLT_MAX;
	m_vMin[1] = FLT_MAX;
	m_vMax[0] = -FLT_MAX; 
	m_vMax[1] = -FLT_MAX; 

	// find the min / max for mantissa and abcsisca
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		const CMatrixNxM<>& mData = m_arrDataSeries[nAt]->GetDataMatrix();
		for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			m_vMin[0] = __min(m_vMin[0], mData[nAtPoint][0]);
			m_vMin[1] = __min(m_vMin[1], mData[nAtPoint][1]);
			m_vMax[0] = __max(m_vMax[0], mData[nAtPoint][0]);
			m_vMax[1] = __max(m_vMax[1], mData[nAtPoint][1]);
		}
	}

	// compute tick marks
	m_vMajor[0] = pow(R(10.0), floor(log10(m_vMax[0]) + R(0.5)) - R(1.0));
	m_vMinor[0] = m_vMajor[0] / 4.0f;
	m_vMax[0] = m_vMajor[0] * (ceil(m_vMax[0] / m_vMajor[0]));
	m_vMin[0] = m_vMajor[0] * (floor(m_vMin[0] / m_vMajor[0]));

	// compute tick marks
	m_vMajor[1] = pow(R(10.0), floor(log10(m_vMax[1]) + R(0.5)) - R(1.0));
	m_vMinor[1] = m_vMajor[1] / 2.0f;
	m_vMax[1] = m_vMajor[1] * (ceil(m_vMax[1] / m_vMajor[1]));
	m_vMin[1] = m_vMajor[1] * (floor(m_vMin[1] / m_vMajor[1]));

}	// CGraph::AutoScale

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetLegendLUT
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetLegendLUT(CArray<COLORREF, COLORREF>&  arrLUT, REAL win, REAL level)
{
	m_arrLegendLUT.Copy(arrLUT);
	m_window = win;
	m_level = level;

	m_bShowLegend = true;

}	// CGraph::SetLegendLUT



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

void CGraph::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(&rect);

	if (m_dib.GetSize().cx != rect.Width())
	{
		m_dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
		m_dib.Attach(bm);
	}
	dcMem.SelectObject(m_dib);

	// draw the axes
	CBrush brushBack(RGB(192, 192, 192));
	dcMem.SelectObject(&brushBack);
	dcMem.Rectangle(rect);

	dcMem.SetBkColor(RGB(192, 192, 192));

	if (m_arrDataSeries.GetSize() != 0)
	{
		rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
			m_arrMargins[2], m_arrMargins[3]);

		// draw minor ticks and grids
		DrawMinorAxes(&dcMem, rect);

		// draw major ticks, grids, and labels
		DrawMajorAxes(&dcMem, rect);

		// draw curves
		DrawSeries(&dcMem, rect);

		// draw legend
		if (m_bShowLegend)
		{
			DrawLegend(&dcMem, rect);
		}

		// draw rectangle
		CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
		dcMem.SelectObject(&pen);
		dcMem.SelectStockObject(HOLLOW_BRUSH);
		dcMem.Rectangle(rect);
	}

	GetClientRect(&rect);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
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
						- ToPlotCoord(CCastVectorD<2>(mData[m_nDragPoint]));
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
		mData[m_nDragPoint][0] = FromPlotCoord(point - m_ptDragOffset)[0];
		mData[m_nDragPoint][1] = FromPlotCoord(point - m_ptDragOffset)[1];

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


/////////////////////////////////////////////////////////////////////////////
// drawing helpers


// draws the minor ticks and grids
void CGraph::DrawMinorAxes(CDC * pDC, const CRect& rect)
{
	CPen penMinorTicks(PS_SOLID, 1, RGB(0, 0, 0));
	CPen penMinorGrids(PS_DOT, 1, RGB(176, 176, 176));

	CVectorD<2> vAtTick;
	vAtTick[1] = m_vMin[1];
	for (vAtTick[0] = m_vMin[0]; vAtTick[0] < m_vMax[0]; vAtTick[0] += m_vMinor[0])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMinorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.bottom + 5);

		pDC->SelectObject(&penMinorGrids);	
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.top);
	}

	vAtTick[0] = m_vMin[0];
	for (vAtTick[1] = m_vMin[1]; vAtTick[1] < m_vMax[1]; vAtTick[1] += m_vMinor[1])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMinorTicks);
		pDC->MoveTo(ptTick); 
		pDC->LineTo(rect.left - 5, ptTick.y);

		pDC->SelectObject(&penMinorGrids);
		pDC->MoveTo(ptTick); 
		pDC->LineTo(rect.right, ptTick.y);
	}
}

// draw major ticks and grids
void CGraph::DrawMajorAxes(CDC * pDC, const CRect& rect)
{
	CPen penMajorTicks(PS_SOLID, 1, RGB(0, 0, 0));
	CPen penMajorGrids(PS_DOT, 1, RGB(160, 160, 160));

	// stores size of text (for vert centering)
	CSize sz = pDC->GetTextExtent("Test");
	pDC->SetTextAlign(TA_CENTER);

	CVectorD<2> vAtTick;
	vAtTick[1] = m_vMin[1];
	for (vAtTick[0] = m_vMin[0]; vAtTick[0] < m_vMax[0]; vAtTick[0] += m_vMajor[0])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMajorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.bottom + 10);

		CString strLabel;
		strLabel.Format((m_vMajor[0] > 1.0) ? "%0.0lf" : "%6.3lf", vAtTick[0]);
		pDC->TextOut(ptTick.x, rect.bottom + 15, strLabel);

		pDC->SelectObject(&penMajorGrids);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.top);
	}

	pDC->SetTextAlign(TA_RIGHT);

	int nPrevTop = INT_MAX;
	vAtTick[0] = m_vMin[0];
	for (vAtTick[1] = m_vMin[1]; vAtTick[1] < m_vMax[1]; vAtTick[1] += m_vMajor[1])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMajorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(rect.left - 10, ptTick.y);

		if (ptTick.y + sz.cy / 2 < nPrevTop)
		{
			CString strLabel;
			strLabel.Format((m_vMajor[1] > 1.0) ? "%0.0lf" : "%6.3lf", vAtTick[1]);
			pDC->TextOut(rect.left - 15, ptTick.y - sz.cy / 2, strLabel);
			nPrevTop = ptTick.y - sz.cy / 2;
		}

		pDC->SelectObject(&penMajorGrids);
		pDC->MoveTo(ptTick);
		pDC->LineTo(rect.right, ptTick.y);
	}
}

void CGraph::DrawSeries(CDC * pDC, const CRect& rect)
{
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];

		CPen pen(PS_SOLID, 2, pSeries->GetColor());
		// CPen *pOldPen = 
		pDC->SelectObject(&pen);

		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		pDC->MoveTo(ToPlotCoord(CCastVectorD<2>(mData[0])));
		for (int nAtPoint = 1; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			if (mData[nAtPoint][0] >= 0.0)
			{
				pDC->LineTo(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint])));
			}
			else
			{
				pDC->MoveTo(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint])));
			}
		}

		CPen penHandle(PS_SOLID, 1, RGB(0, 0, 0));
		pDC->SelectObject(&penHandle);
		if (pSeries->HasHandles())
		{
			CRect rectHandle(0, 0, 5, 5);
			for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
			{
				rectHandle.OffsetRect(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint]))
					- rectHandle.CenterPoint());
				pDC->Rectangle(rectHandle);
			}
		}

		// pDC->SelectObject(pOldPen);
	}
}


void CGraph::DrawLegend(CDC * pDC, const CRect& rect)
{
	CRect rectLegend(rect);
	rectLegend.bottom = rectLegend.top - 2;
	rectLegend.top -= 12;

	REAL max_legend_value = R(m_arrLegendLUT.GetSize());

	for (int nAtX = rectLegend.left; nAtX < rectLegend.right; nAtX++)
	{
		// compute x coord
		double x = (double) (nAtX - rectLegend.left) / (double) rectLegend.Width() 
			* (m_vMax[0] - m_vMin[0]);
		x /= 100.0;

		REAL pix_value = max_legend_value * m_window * (R(x) - m_level) + max_legend_value / R(2.0);

		// scale to 0..255
		pix_value = __min(pix_value, max_legend_value - 1.0f);
		pix_value = __max(pix_value, 0.0f);

		int colorIndex = (int) __min(pix_value, max_legend_value - 1.0f);
		CPen penLegend(PS_SOLID, 1, m_arrLegendLUT[colorIndex]);
		pDC->SelectObject(&penLegend);

		pDC->MoveTo(nAtX, rectLegend.bottom - 1);
		pDC->LineTo(nAtX, rectLegend.top);
	}

	CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
	pDC->SelectObject(&pen);
	pDC->SelectStockObject(HOLLOW_BRUSH);
	pDC->Rectangle(rectLegend);
}

CPoint CGraph::ToPlotCoord(const CVectorD<2>& vCoord)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
		m_arrMargins[2], m_arrMargins[3]);
		
	CPoint pt;
	pt.x = rect.left + (int)((REAL) rect.Width() * (vCoord[0] - m_vMin[0]) 
		/ (m_vMax[0] - m_vMin[0]));
	pt.y = rect.Height() + rect.top - (int)((REAL) rect.Height() * (vCoord[1] - m_vMin[1]) 
		/ (m_vMax[1] - m_vMin[1]));

	return pt;
}

CVectorD<2> CGraph::FromPlotCoord(const CPoint& pt)
{
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
		m_arrMargins[2], m_arrMargins[3]);

	CVectorD<2> vCoord;
	vCoord[0] = (pt.x - rect.left) * (m_vMax[0] - m_vMin[0]) 
		/ rect.Width() + m_vMin[0];
	vCoord[1] = -(pt.y - rect.Height() - rect.top) * (m_vMax[1] - m_vMin[1]) 
		/ rect.Height() + m_vMin[1];  

	return vCoord;
}
