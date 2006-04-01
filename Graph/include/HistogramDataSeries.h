#pragma once

#include <Histogram.h>
#include <DataSeries.h>

class CHistogramDataSeries :
	public CDataSeries
{
public:
	CHistogramDataSeries(CHistogram *pHisto);
	virtual ~CHistogramDataSeries(void);

	DECLARE_ATTRIBUTE_PTR(Histogram, CHistogram);

	virtual const CMatrixNxM<>& GetDataMatrix() const;

	void OnHistogramChanged(CObservableEvent *, void *);

private:
//	CHistogram *m_pHisto;
// public:
//	CHistogram * GetHistogram(void);

	mutable bool m_bRecalcCurve;
};

