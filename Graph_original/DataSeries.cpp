#include "StdAfx.h"
#include ".\dataseries.h"

////////////////////////////////////////////////////////////////////////////
	CDataSeries::CDataSeries(void)
		: m_pGraph(NULL)
		, m_pObject(NULL)
		, m_Color(RGB(255, 0, 0))
		, m_HasHandles(false)
		, m_PenStyle(PS_SOLID)
{ 
}	// CDataSeries::CDataSeries


////////////////////////////////////////////////////////////////////////////
	CDataSeries::~CDataSeries(void)
{
}	// CDataSeries::~CDataSeries


////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::SetColor(const COLORREF& color)
{
	m_Color = color;
	GetChangeEvent().Fire();

}	// CDataSeries::SetColor

////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& 
	CDataSeries::GetDataMatrix() const
		// accessor for the data series data
{
	return m_mData;

}	// CDataSeries::GetDataMatrix()

////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::SetDataMatrix(const CMatrixNxM<>& mData)
{
	// copy the data
	m_mData.Reshape(mData.GetCols(), mData.GetRows());
	m_mData = mData;

	// notify
	GetChangeEvent().Fire();

}	// CDataSeries::SetDataMatrix

////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::AddDataPoint(const CVectorD<2>& vDataPt)
{
	// TODO: fix Reshape to correct this problem
	// ALSO TODO: CMatrixNxM serialization
	CMatrixNxM<> mTemp(m_mData.GetCols()+1, 2);
	for (int nAt = 0; nAt < m_mData.GetCols(); nAt++)
	{
		mTemp[nAt][0] = m_mData[nAt][0];
		mTemp[nAt][1] = m_mData[nAt][1];
	}

	// set the data point
	mTemp[mTemp.GetCols()-1][0] = vDataPt[0];
	mTemp[mTemp.GetCols()-1][1] = vDataPt[1];

	// call this, so that virtual over-rides can do their job
	SetDataMatrix(mTemp);

	// DON'T call change event, because SetDataMatrix calls it
	// GetChangeEvent().Fire();

}	// CDataSeries::AddDataPoint
