// Histogram.h: interface for the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Association.h>
#include <Collection.h>
#include "Volumep.h"

class CHistogram : public CObject  
{
public:
	CHistogram(CVolume<double> *pImage = NULL, CVolume<int> *pRegion = NULL);
	virtual ~CHistogram();

	// association to the volume over which the histogram is formed
	CAssociation< CVolume<double> > volume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CAssociation< CVolume<int> > region;

	// accessor for the binning kernel -- set to zero for a traditional
	//		histogram
	CValue< double > binKernelSigma;

	// accessors for bin data
	CArray<double, double>& GetBins();
	CArray<double, double>& GetCumBins();

	// collection of the differential volumes
	CCollection< CVolume<double> > diffVolumes;

	// accessor for the differential bins
	CArray<double, double>& GetDiffBins(CVolume<double> *pWrt);
	CArray<double, double>& GetCumDiffBins(CVolume<double> *pWrt);

	// change handler for when the volume or region changes
	virtual void OnChange(CObservableObject *pSource, void *);

private:
	// flag to indicate bins should be recomputed
	BOOL m_bRecomputeBins;

	// histogram bins
	CArray<double, double> m_arrBins;

	// flag to indicate cumulative bins should be recomputed
	BOOL m_bRecomputeCumBins;

	// cumulative bins
	CArray<double, double> m_arrCumBins;

	// map of differential histograms
	CMapPtrToPtr m_mapDiffHistograms;
};

#endif // !defined(AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
