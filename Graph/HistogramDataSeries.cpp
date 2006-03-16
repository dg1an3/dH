#include "StdAfx.h"

#include <HistogramDataSeries.h>
#include <Graph.h>
#include ".\include\histogramdataseries.h"

CHistogramDataSeries::CHistogramDataSeries(CHistogram *pHisto)
: m_pHisto(pHisto)
{
	m_pHisto->GetChangeEvent().AddObserver(this, 
		(ListenerFunction) &CHistogramDataSeries::OnHistogramChanged);

	OnHistogramChanged(NULL, NULL);
}

CHistogramDataSeries::~CHistogramDataSeries(void)
{
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
		m_mData[nAt][0] = R(100.0) * binValue;
		m_mData[nAt][1] = R(100.0) * arrBins[nAt] / sum;
		// AddDataPoint(CVectorD<2>(100.0 * binValue, 100.0 * arrBins[nAt] / sum));
		// AddDataPoint(CVectorD<2>
		//	(1000 * (nAt - nStart) / 256, arrBins[nAt] / sum * 4100.0));
		binValue += m_pHisto->GetBinWidth();
	}	

	if (m_pGraph)
	{
		m_pGraph->AutoScale();
		m_pGraph->SetAxesMin(CVectorD<2>(0.0, 0.0));
		m_pGraph->Invalidate(TRUE);
	}
}


CHistogram * CHistogramDataSeries::GetHistogram(void)
{
	return m_pHisto;
}
