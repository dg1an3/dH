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

//////////////////////////////////////////////////////////////////////
// CHistogram::CHistogram
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CHistogram::CHistogram(CVolume<double> *pVolume, CVolume<int> *pRegion)
	: m_bRecomputeBins(TRUE),
		m_bRecomputeCumBins(TRUE),
		m_pVolume(pVolume),
		m_pRegion(pRegion)
{
	if (m_pVolume)
	{
		AddObserver<CHistogram>(&m_pVolume->GetChangeEvent(), this, 
			OnVolumeChange);
	}

	if (m_pRegion)
	{
		AddObserver<CHistogram>(&m_pRegion->GetChangeEvent(), this, 
			OnVolumeChange);
	}
}

//////////////////////////////////////////////////////////////////////
// CHistogram::~CHistogram
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CHistogram::~CHistogram()
{
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
// association to the volume over which the histogram is formed
CVolume<double> *CHistogram::GetVolume()
{
	return m_pVolume;
}

//////////////////////////////////////////////////////////////////////
// CHistogram::SetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::SetVolume(CVolume<double> *pVolume)
{
	// set the pointer
	m_pVolume = pVolume;

	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

	// fire a change event
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetRegion
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
// association to a congruent volume describing the region over
//		which the histogram is formed -- contains a 1.0 for voxels
//		within the region, 0.0 elsewhere
CVolume<int> *CHistogram::GetRegion()
{
	return m_pRegion;
}

//////////////////////////////////////////////////////////////////////
// CHistogram::SetRegion
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::SetRegion(CVolume<int> *pRegion)
{
	// set the pointer
	m_pRegion = pRegion;

	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

	// fire a change event
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinKernelSigma
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
// accessor for the binning kernel -- set to zero for a traditional
//		histogram
double CHistogram::GetBinKernelSigma() const
{
	return 0.0;
}

//////////////////////////////////////////////////////////////////////
// CHistogram::SetBinKernelSigma
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::SetBinKernelSigma(double sigma)
{
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetBins
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CArray<double, double>& CHistogram::GetBins()
{
	if (m_bRecomputeBins)
	{
		m_arrBins.RemoveAll();
		m_arrBins.SetSize(256);

		for (int nAtZ = 0; nAtZ < m_pVolume->GetDepth(); nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_pVolume->GetHeight(); nAtY++)
			{
				for (int nAtX = 0; nAtX < m_pVolume->GetWidth(); nAtX++)
				{
					double intensity = m_pVolume->GetVoxels()[nAtZ][nAtY][nAtX];

					// compute the bin
					m_arrBins[(int)(intensity * 255.0)] += 
						(double) m_pRegion->GetVoxels()[nAtZ][nAtY][nAtX];
				}
			}
		}

		// normalize by the volume
		double volume = m_pRegion->GetSum();
		for (int nAt = 0; nAt < m_arrBins.GetSize(); nAt++)
		{
			m_arrBins[nAt] /= volume;
		}

		m_bRecomputeBins = FALSE;
	}

	return m_arrBins;
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetCumBins
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// CHistogram::OnVolumeChange
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::OnVolumeChange(CObservableEvent *pSource, void *)
{
	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

	// fire a change event
	GetChangeEvent().Fire();
}

