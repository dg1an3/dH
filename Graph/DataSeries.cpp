#include "StdAfx.h"
#include ".\dataseries.h"

CDataSeries::CDataSeries(void)
	: m_pGraph(NULL)
		, m_pObject(NULL)
		, m_color(RGB(255, 0, 0))
		, m_bHandles(FALSE)
		, m_nPenStyle(PS_SOLID)
{ 
}


CDataSeries::~CDataSeries(void)
{
}


CObject * CDataSeries::GetObject()
{
	return m_pObject;
}

void CDataSeries::SetObject(CObject *pObject)
{
	m_pObject = pObject;
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


