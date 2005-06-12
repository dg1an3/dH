#pragma once

#include <Histogram.h>
#include <DataSeries.h>

class CHistogramDataSeries :
	public CDataSeries
{
public:
	CHistogramDataSeries(CHistogram *pHisto);
	virtual ~CHistogramDataSeries(void);

	void OnHistogramChanged(CObservableEvent *, void *);

private:
	CHistogram *m_pHisto;
};

