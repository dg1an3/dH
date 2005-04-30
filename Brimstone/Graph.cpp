// Graph.cpp : implementation file
//

#include "stdafx.h"

#include <float.h>

#include "Graph.h"

#include <CastVectorD.h>

// #include <MatrixBase.inl>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDataSeries::CDataSeries() 
	: m_color(RGB(255, 0, 0)), 
		m_bHandles(FALSE),
			m_pGraph(NULL)

//		m_pObject(NULL)
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

	m_mData.Reshape(mTemp.GetCols(), mTemp.GetRows());
	m_mData = (const CMatrixNxM<>&)(mTemp);

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


CHistogramDataSeries::CHistogramDataSeries(CHistogram *pHisto)
: m_pHisto(pHisto)
{
	m_pHisto->GetChangeEvent().AddObserver(this, (ListenerFunction) OnHistogramChanged);

	OnHistogramChanged(NULL, NULL);
}

void CHistogramDataSeries::OnHistogramChanged(CObservableEvent *, void *)
{
		// m_mData.Reshape(0, 2);

	// now draw the histogram
	const CVectorN<>& arrBins = m_pHisto->GetCumBins();

	m_mData.Reshape(arrBins.GetDim(), 2);
	REAL sum = m_pHisto->GetRegion()->GetSum();
	// int nStart = -floor(m_pHisto->GetBinMinValue() / m_pHisto->GetBinWidth());
	REAL binValue = m_pHisto->GetBinMinValue();
	for (int nAt = 0; /* nStart; */ nAt < arrBins.GetDim(); nAt++)
	{
		m_mData[nAt][0] = 100.0 * binValue;
		m_mData[nAt][1] = 100.0 * arrBins[nAt] / sum;
		// AddDataPoint(CVectorD<2>(100.0 * binValue, 100.0 * arrBins[nAt] / sum));
		// AddDataPoint(CVectorD<2>
		//	(1000 * (nAt - nStart) / 256, arrBins[nAt] / sum * 4100.0));
		binValue += m_pHisto->GetBinWidth();
	}	

	if (m_pGraph)
		m_pGraph->Invalidate(TRUE);
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

#define MARGIN 50

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

	if (m_arrDataSeries.GetSize() == 0)
		return;

	rect.DeflateRect(MARGIN, MARGIN, MARGIN * 8, MARGIN);

	ComputeMinMax();

/*	CVectorD<2> vAt(m_vMin);
	dcMem.MoveTo(ToPlotCoord(vAt)); // MARGIN, rect.Height() - MARGIN);
	int nCount = 0;
	for (vAt[1] = m_vMin[1]; vAt[1] <= m_vMax[1]; vAt[1] += m_vMajor[1] / 10)
	{
		dcMem.LineTo(ToPlotCoord(vAt));
		dcMem.LineTo(ToPlotCoord(vAt) 
			+ (nCount % 10 == 0 ? CPoint(-10, 0) : CPoint(-5, 0)));
		dcMem.LineTo(ToPlotCoord(vAt));
		nCount++;
	}

	vAt = m_vMin;
	dcMem.MoveTo(ToPlotCoord(vAt)); // MARGIN, rect.Height() - MARGIN);
	nCount = 0;
	for (vAt[0] = m_vMin[0]; vAt[0] <= m_vMax[0]; vAt[0] += m_vMajor[0] / 10)
	{
		dcMem.LineTo(ToPlotCoord(vAt));
		dcMem.LineTo(ToPlotCoord(vAt) 
			+ (nCount % 10 == 0 ? CPoint(0, 10) : CPoint(0, 5)));
		dcMem.LineTo(ToPlotCoord(vAt));
		nCount++;
	} */

	// stores size of text (for vert centering)

	CSize sz = dcMem.GetTextExtent("Test");

	///////////////////////////////////////////////////////////////////

	// draw minor ticks and grids

	CPen penMinorTicks(PS_SOLID, 1, RGB(0, 0, 0));
	CPen penMinorGrids(PS_DOT, 1, RGB(176, 176, 176));

	for (double x = m_vMinor[0]; x < m_vMax[0]; x += m_vMinor[0])
	{
		int nAtX = rect.left 
			+ (int) ((double) rect.Width() * x / m_vMax[0]);

		dcMem.SelectObject(&penMinorTicks);
		dcMem.MoveTo(nAtX, rect.bottom);
		dcMem.LineTo(nAtX, rect.bottom + 5);

		dcMem.SelectObject(&penMinorGrids);	
		dcMem.MoveTo(nAtX, rect.bottom);
		dcMem.LineTo(nAtX, rect.top);
	}

	for (double y = m_vMinor[1]; y < m_vMax[1]; y += m_vMinor[1])
	{
		int nAtY = rect.bottom
			- (int) ((double) rect.Height() * y / m_vMax[1]);

		dcMem.SelectObject(&penMinorTicks);
		dcMem.MoveTo(rect.left, nAtY);
		dcMem.LineTo(rect.left - 5, nAtY);

		dcMem.SelectObject(&penMinorGrids);
		dcMem.MoveTo(rect.left, nAtY);
		dcMem.LineTo(rect.right, nAtY);
	}

	///////////////////////////////////////////////////////////////////

	// draw major ticks, grids, and labels

	CPen penMajorTicks(PS_SOLID, 1, RGB(0, 0, 0));
	CPen penMajorGrids(PS_DOT, 1, RGB(160, 160, 160));

	dcMem.SetTextAlign(TA_CENTER);

	for (x = m_vMajor[0]; x < m_vMax[0]; x += m_vMajor[0])
	{
		int nAtX = rect.left 
			+ (int) ((double) rect.Width() * x / m_vMax[0]);

		dcMem.SelectObject(&penMajorTicks);
		dcMem.MoveTo(nAtX, rect.bottom);
		dcMem.LineTo(nAtX, rect.bottom + 10);

		CString strLabel;
		strLabel.Format("%0.0lf", x);
		dcMem.TextOut(nAtX, rect.bottom + 15, strLabel);

		dcMem.SelectObject(&penMajorGrids);
		dcMem.MoveTo(nAtX, rect.bottom);
		dcMem.LineTo(nAtX, rect.top);
	}

	dcMem.SetTextAlign(TA_RIGHT);

	for (y = m_vMajor[1]; y < m_vMax[1]; y += m_vMajor[1])
	{
		int nAtY = rect.bottom
			- (int) ((double) rect.Height() * y / m_vMax[1]);

		dcMem.SelectObject(&penMajorTicks);
		dcMem.MoveTo(rect.left, nAtY);
		dcMem.LineTo(rect.left - 10, nAtY);

		CString strLabel;
		strLabel.Format("%0.0lf", y);
		dcMem.TextOut(rect.left - 15, nAtY - sz.cy / 2, strLabel);

		dcMem.SelectObject(&penMajorGrids);
		dcMem.MoveTo(rect.left, nAtY);
		dcMem.LineTo(rect.right, nAtY);
	}

	///////////////////////////////////////////////////////////////////

	// draw curves

	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];

		CPen pen(PS_SOLID, 2, pSeries->GetColor());
		// CPen *pOldPen = 
		dcMem.SelectObject(&pen);

		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		dcMem.MoveTo(ToPlotCoord(CCastVectorD<2>(mData[0])));
		for (int nAtPoint = 1; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			if (mData[nAtPoint][0] >= 0.0)
			{
				dcMem.LineTo(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint])));
			}
			else
			{
				dcMem.MoveTo(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint])));
			}
		}

		CPen penHandle(PS_SOLID, 1, RGB(0, 0, 0));
		dcMem.SelectObject(&penHandle);
		if (pSeries->HasHandles())
		{
			CRect rectHandle(0, 0, 5, 5);
			for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
			{
				rectHandle.OffsetRect(ToPlotCoord(CCastVectorD<2>(mData[nAtPoint]))
					- rectHandle.CenterPoint());
				dcMem.Rectangle(rectHandle);
			}
		}

		// dcMem.SelectObject(pOldPen);
	}

	///////////////////////////////////////////////////////////////////

	// draw rectangle

	CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
	dcMem.SelectObject(&pen);
	dcMem.SelectStockObject(HOLLOW_BRUSH);
	dcMem.Rectangle(rect);


	///////////////////////////////////////////////////////////////////

	// draw legend

	CRect rectLegend(rect);
	rectLegend.bottom = rectLegend.top - 2;
	rectLegend.top -= 12;

	REAL max_legend_value = m_arrLegendLUT.GetSize();

	for (int nAtX = rectLegend.left; nAtX < rectLegend.right; nAtX++)
	{
		// compute x coord
		x = (double) (nAtX - rectLegend.left) / (double) rectLegend.Width() 
			* m_vMax[0];
		x /= 100.0;

		REAL pix_value = max_legend_value * m_window * (x - m_level) + max_legend_value / 2.0;

		// scale to 0..255
		pix_value = __min(pix_value, max_legend_value - 1.0);
		pix_value = __max(pix_value, 0.0);

		//  colorIndex = (int) pix_value1 // * (REAL) (max_value - 1.0));
		int colorIndex = __min(pix_value, max_legend_value - 1.0);
		COLORREF color = m_arrLegendLUT[colorIndex];

		// convert to color using LUT
		// COLORREF color = RGB(255.0 * x / m_vMax[0], 0, 0);

		CPen penLegend(PS_SOLID, 1, color);
		dcMem.SelectObject(&penLegend);

		dcMem.MoveTo(nAtX, rectLegend.bottom - 1);
		dcMem.LineTo(nAtX, rectLegend.top);
	}

	dcMem.SelectObject(&pen);
	dcMem.SelectStockObject(HOLLOW_BRUSH);
	dcMem.Rectangle(rectLegend);

	GetClientRect(&rect);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

CPoint CGraph::ToPlotCoord(const CVectorD<2>& vCoord)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(MARGIN, MARGIN, MARGIN * 8, MARGIN);

	CPoint pt;
	pt.x = MARGIN + (int)((REAL) rect.Width() * (vCoord[0] - m_vMin[0]) 
		/ (m_vMax[0] - m_vMin[0]));
	pt.y = rect.Height() + MARGIN - (int)((REAL) rect.Height() * (vCoord[1] - m_vMin[1]) 
		/ (m_vMax[1] - m_vMin[1]));

	return pt;
}

CVectorD<2> CGraph::FromPlotCoord(const CPoint& pt)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(MARGIN, MARGIN, MARGIN * 8, MARGIN);

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
	m_vMax[0] = 100;
	m_vMax[1] = 100;
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];
		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
		{
		//	m_vMin[0] = __min(m_vMin[0], mData[nAtPoint][0]);
		//	m_vMin[1] = __min(m_vMin[1], mData[nAtPoint][1]);
			m_vMax[0] = __max(m_vMax[0], mData[nAtPoint][0]);
			m_vMax[1] = __max(m_vMax[1], mData[nAtPoint][1]);
		}
	}

	m_vMajor[0] = 10.0; // pow(10, ceil(log10(m_vMax[0])) - 1);
	m_vMinor[0] = 2.50;
	m_vMax[0] = m_vMajor[0] * (ceil(m_vMax[0] / m_vMajor[0]));

	m_vMajor[1] = 10.0; // pow(10, ceil(log10(m_vMax[1])) - 1);
	m_vMinor[1] = 5.0;
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
	pSeries->m_pGraph = this;
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


/*
CObject * CDataSeries::GetObject()
{
	return m_pObject;
}

void CDataSeries::SetObject(CObject *pObject)
{
	m_pObject = pObject;
}
*/