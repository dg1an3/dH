// Graph.cpp : implementation file
//

#include "stdafx.h"

#include <float.h>

#include "Graph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraph

CGraph::CGraph()
{
}

CGraph::~CGraph()
{
}


BEGIN_MESSAGE_MAP(CGraph, CWnd)
	//{{AFX_MSG_MAP(CGraph)
	ON_WM_PAINT()
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

		CPen pen(PS_SOLID, 2, pSeries->m_color);
		CPen *pOldPen = dc.SelectObject(&pen);

		dc.MoveTo(ToPlotCoord(pSeries->m_arrData[0]));
		for (int nAtPoint = 1; nAtPoint < pSeries->m_arrData.size(); nAtPoint++)
		{
			dc.LineTo(ToPlotCoord(pSeries->m_arrData[nAtPoint]));
		}

		dc.SelectObject(pOldPen);
	}

	// Do not call CWnd::OnPaint() for painting messages
}

CPoint CGraph::ToPlotCoord(CVectorD<2> vCoord)
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

void CGraph::ComputeMinMax()
{
	m_vMin[0] = 0;
	m_vMin[1] = 0;
	m_vMax[0] = 0;
	m_vMax[1] = 0;
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];
		for (int nAtPoint = 0; nAtPoint < pSeries->m_arrData.size(); nAtPoint++)
		{
			m_vMin[0] = min(m_vMin[0], pSeries->m_arrData[nAtPoint][0]);
			m_vMin[1] = min(m_vMin[1], pSeries->m_arrData[nAtPoint][1]);
			m_vMax[0] = max(m_vMax[0], pSeries->m_arrData[nAtPoint][0]);
			m_vMax[1] = max(m_vMax[1], pSeries->m_arrData[nAtPoint][1]);
		}
	}

	m_vMajor[0] = pow(10, floor(log10(m_vMax[0])));
	m_vMax[0] = m_vMajor[0] * (floor(m_vMax[0] / m_vMajor[0]) + 1);

	m_vMajor[1] = pow(10, floor(log10(m_vMax[1])));
	m_vMax[1] = m_vMajor[1] * (floor(m_vMax[1] / m_vMajor[1]) + 1);
}
