//////////////////////////////////////////////////////////////////////
// Histogram.h: declaration of the CHistogram class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Volumep.h"

//////////////////////////////////////////////////////////////////////
// class CHistogram
//
// forms a histogram of a volume, possibly within a "region" which
//		is a binary volume
//////////////////////////////////////////////////////////////////////
class CHistogram : public CModelObject  
{
public:
	// constructor from a volume and a "region"
	CHistogram(CVolume<double> *pImage = NULL, 
		CVolume<int> *pRegion = NULL);

	// destructor
	virtual ~CHistogram();

	// association to the volume over which the histogram is formed
	CVolume<double> *GetVolume();
	void SetVolume(CVolume<double> *pVolume);

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<int> *GetRegion();
	void SetRegion(CVolume<int> *pVolume);

	// accessor for the binning kernel -- set to zero for a traditional
	//		histogram
	double GetBinKernelSigma() const;
	void SetBinKernelSigma(double sigma);

	// accessors for bin data
	CArray<double, double>& GetBins();
	CArray<double, double>& GetCumBins();

protected:
	// change handler for when the volume or region changes
	void OnVolumeChange(CObservableEvent *pSource, void *);

private:
	// association to the volume over which the histogram is formed
	CVolume<double> *m_pVolume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<int> *m_pRegion;

	// the binning kernel width -- set to zero for a traditional
	//		histogram
	double m_binKernelSigma;

	// flag to indicate bins should be recomputed
	BOOL m_bRecomputeBins;

	// histogram bins
	CArray<double, double> m_arrBins;

	// flag to indicate cumulative bins should be recomputed
	BOOL m_bRecomputeCumBins;

	// cumulative bins
	CArray<double, double> m_arrCumBins;
};

#endif // !defined(AFX_HISTOGRAM_H__B8400D13_5462_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
