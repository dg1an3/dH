//////////////////////////////////////////////////////////////////////
// Histogram.h: declaration of the CHistogram class
//
// Copyright (C) 2000-2004
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(HISTOGRAM_H)
#define HISTOGRAM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorN.h>
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
		CVolume<double> *pRegion = NULL);

	// destructor
	virtual ~CHistogram();

	// association to the volume over which the histogram is formed
	CVolume<double> *GetVolume();
	void SetVolume(CVolume<double> *pVolume);

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<double> *GetRegion();
	void SetRegion(CVolume<double> *pVolume);

	// histogram parameters
	double GetBinMinValue() const;
	double GetBinWidth() const;
	void SetBinning(double min_value, double width, 
		double sigma_mult = 0.0);
	long GetBinForValue(double value) const;
	const CVectorN<>& GetBinMeans() const;

	// accessors for bin data
	CVectorN<>& GetBins();
	const CVectorN<>& GetBins() const;
	const CVectorN<>& GetCumBins() const;

	// Gbinning parameter
	void SetGBinSigma(double sigma);

	// Gbin accessor
	CVectorN<>& GetGBins();
	const CVectorN<>& GetGBins() const;
	const CVectorN<>& GetGBinMeans() const;

	// partial derivative volumes
	long Get_dVolumeCount() const;
	CVolume<double> *Get_dVolume(long nAt) const;
	long Add_dVolume(CVolume<double> *p_dVolume);

	// partial derivatives
	const CVectorN<>& Get_dBins(long nAt) const;
	const CVectorN<>& Get_dGBins(long nAt) const;

	// evaluation for testing
	double Eval_GBin(double x) const;
	double Eval_dGBin(long nAt_dVolume, double x) const;

	void ConvGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;
	void Conv_dGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;

protected:
	// change handler for when the volume or region changes
	void OnVolumeChange(CObservableEvent *pSource, void *);

private:
	// association to the volume over which the histogram is formed
	CVolume<double> *m_pVolume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<double> *m_pRegion;

	// 
	double m_minValue;
	double m_binWidth;

	// histogram bins
	mutable CVectorN<> m_arrBins;
	mutable CVectorN<> m_arrBinMeans;

	// flag to indicate bins should be recomputed
	mutable BOOL m_bRecomputeBins;

	// cumulative bins
	mutable CVectorN<> m_arrCumBins;

	// flag to indicate cumulative bins should be recomputed
	mutable BOOL m_bRecomputeCumBins;

	// the binning kernel width -- set to zero for a traditional
	//		histogram
	double m_binKernelSigma;
	CVectorN<> m_binKernel;
	CVectorN<> m_bin_dKernel;

	mutable CVectorN<> m_arrGBins;
	mutable CVectorN<> m_arrGBinMeans;
	// BOOL m_bRecomputeGBins;

	// array of partial derivative volumes
	CObArray *m_pArr_dVolumes;		// CVolume<double>

	// partial derivative histogram bins
	mutable CVectorN<> *m_arr_dBins;

	// partial derivative histogram bins
	mutable CVectorN<> *m_arr_dGBins;

};	// class CHistogram


#endif // !defined(HISTOGRAM_H)
