#include "StdAfx.h"
#include ".\targetdvhseries.h"

#include <Graph.h>

CTargetDVHSeries::CTargetDVHSeries(CKLDivTerm *pKLDT)
: m_pKLDivTerm(pKLDT)
{
	m_pKLDivTerm->GetChangeEvent().AddObserver(this, 
		(ListenerFunction) &CTargetDVHSeries::OnKLDTChanged);

	OnKLDTChanged(NULL, NULL);
}

CTargetDVHSeries::~CTargetDVHSeries(void)
{
}

void CTargetDVHSeries::SetDataMatrix(const CMatrixNxM<>& mData)
{
	CDataSeries::SetDataMatrix(mData);

	CMatrixNxM<REAL> mTemp(mData.GetCols(), mData.GetRows());
	mTemp = mData;
	for (int nAt = 0; nAt < mData.GetCols(); nAt++)
	{
		mTemp[nAt][0] /= R(100.0);
		mTemp[nAt][1] /= R(100.0);
	}

	m_pKLDivTerm->SetDVPs(mTemp);
}

void CTargetDVHSeries::OnKLDTChanged(CObservableEvent * pEv, void * pVoid)
{
		// m_mData.Reshape(0, 2);

	// now draw the histogram
	const CMatrixNxM<>& mDVPs = m_pKLDivTerm->GetDVPs();
	m_mData.Reshape(mDVPs.GetCols(), mDVPs.GetRows());
	m_mData = mDVPs;

//	m_mData.Reshape(arrBins.GetDim(), 2);
//	REAL sum = m_pHisto->GetRegion()->GetSum();
	// int nStart = -floor(m_pHisto->GetBinMinValue() / m_pHisto->GetBinWidth());
//	REAL binValue = m_pHisto->GetBinMinValue();
	for (int nAt = 0; /* nStart; */ nAt < m_mData.GetCols(); nAt++)
	{
		m_mData[nAt][0] *= R(100.0);
		m_mData[nAt][1] *= R(100.0);
		// AddDataPoint(CVectorD<2>(100.0 * binValue, 100.0 * arrBins[nAt] / sum));
		// AddDataPoint(CVectorD<2>
		//	(1000 * (nAt - nStart) / 256, arrBins[nAt] / sum * 4100.0));
//		binValue += m_pHisto->GetBinWidth();
	}	

	if (m_pGraph)
	{
		m_pGraph->AutoScale();
		m_pGraph->SetAxesMin(CVectorD<2>(0.0, 0.0));
		m_pGraph->Invalidate(TRUE);
	}
}
