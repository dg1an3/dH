// Histogram.cpp: implementation of the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Histogram.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHistogram::CHistogram(CVolume<double> *pVolume, CVolume<int> *pRegion)
: m_bRecomputeBins(TRUE),
	m_bRecomputeCumBins(TRUE)
{
	volume.AddObserver(this, (ChangeFunction) CHistogram::OnChange);
	region.AddObserver(this, (ChangeFunction) CHistogram::OnChange);

	volume.Set(pVolume);
	region.Set(pRegion);
}

CHistogram::~CHistogram()
{

}

CArray<double, double>& CHistogram::GetBins()
{
	if (m_bRecomputeBins)
	{
		m_arrBins.RemoveAll();
		m_arrBins.SetSize(256);

		for (int nAtZ = 0; nAtZ < volume->depth.Get(); nAtZ++)
			for (int nAtY = 0; nAtY < volume->height.Get(); nAtY++)
				for (int nAtX = 0; nAtX < volume->width.Get(); nAtX++)
				{
					double intensity = volume->GetVoxels()[nAtZ][nAtY][nAtX];

					// compute the bin
					m_arrBins[(int)(intensity * 255.0)] += 
						(double) region->GetVoxels()[nAtZ][nAtY][nAtX];
				}

		// normalize by the volume
		double volume = region->GetSum();
		for (int nAt = 0; nAt < m_arrBins.GetSize(); nAt++)
			m_arrBins[nAt] /= volume;

		m_bRecomputeBins = FALSE;
	}

	return m_arrBins;
}

CArray<double, double>& CHistogram::GetCumBins()
{
	if (m_bRecomputeCumBins)
	{
		GetBins();

		m_arrCumBins.RemoveAll();
		m_arrCumBins.SetSize(256);

		double sum = 0.0;
		for (int nAt = m_arrBins.GetSize()-1; nAt >= 0; nAt--)
		{
			sum += m_arrBins[nAt];
			m_arrCumBins[nAt] = sum;
		}
		m_bRecomputeCumBins = FALSE;
	}

	return m_arrCumBins;

}

void CHistogram::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if (pSource == &volume)
	{
		if (pOldValue)
			((CVolume<double> *)pOldValue)->RemoveObserver(this, (ChangeFunction) CHistogram::OnChange);
		if (volume.Get())
			volume->AddObserver(this, (ChangeFunction) CHistogram::OnChange);
	}
	else if (pSource == &region)
	{
		if (pOldValue)
			((CVolume<double> *)pOldValue)->RemoveObserver(this, (ChangeFunction) CHistogram::OnChange);
		if (region.Get())
			region->AddObserver(this, (ChangeFunction) CHistogram::OnChange);
	}

	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;
}

