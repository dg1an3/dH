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
#include <Volumep.h>


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
	CHistogram(CVolume<REAL> *pImage = NULL, 
		CVolume<REAL> *pRegion = NULL);

	// destructor
	virtual ~CHistogram();

	// association to the volume over which the histogram is formed
	CVolume<REAL> *GetVolume();
	void SetVolume(CVolume<REAL> *pVolume);

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<REAL> *GetRegion();
	void SetRegion(CVolume<REAL> *pVolume);

	// histogram parameters
	REAL GetBinMinValue() const;
	REAL GetBinWidth() const;
	void SetBinning(REAL min_value, REAL width, 
		REAL sigma_mult = 0.0);
	int GetBinForValue(REAL value) const;
	const CVectorN<>& GetBinMeans() const;

	// accessors for bin data
	CVectorN<>& GetBins();
	const CVectorN<>& GetBins() const;
	const CVectorN<>& GetCumBins() const;

	// Gbinning parameter
	void SetGBinSigma(REAL sigma);

	// Gbin accessor
	CVectorN<>& GetGBins();
	const CVectorN<>& GetGBins() const;
	const CVectorN<>& GetGBinMeans() const;

	// partial derivative volumes
	int Get_dVolumeCount() const;
	CVolume<REAL> *Get_dVolume(int nAt, int *pnGroup = NULL) const;
	int Add_dVolume(CVolume<REAL> *p_dVolume, int nGroup);

	// partial derivatives
	const CVectorBase<>& Get_dBins(int nAt) const;
	const CVectorBase<>& Get_dGBins(int nAt) const;

	// evaluation for testing
	REAL Eval_GBin(REAL x) const;
	REAL Eval_dGBin(int nAt_dVolume, REAL x) const;

	void ConvGauss(const CVectorBase<>& buffer_in, 
							CVectorBase<>& buffer_out) const;
	void Conv_dGauss(const CVectorBase<>& buffer_in, 
							CVectorBase<>& buffer_out) const;

protected:
	// change handler for when the volume or region changes
	void OnVolumeChange(CObservableEvent *pSource, void *);

private:
	// association to the volume over which the histogram is formed
	CVolume<REAL> *m_pVolume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<REAL> *m_pRegion;

	// 
	REAL m_minValue;
	REAL m_binWidth;

	// bin volume
	mutable CVolume<short> m_binVolume;
	mutable BOOL m_bRecomputeBinVolume;

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
	REAL m_binKernelSigma;
	CVectorN<> m_binKernel;
	CVectorN<> m_bin_dKernel;

	mutable CVectorN<> m_arrGBins;
	mutable CVectorN<> m_arrGBinMeans;
	// BOOL m_bRecomputeGBins;

	// array of partial derivative volumes
	CTypedPtrArray<CPtrArray, CVolume<REAL>* > m_arr_dVolumes;
	CArray<int, int> m_arrVolumeGroups;

	// array of partial derivative X region
	CTypedPtrArray<CPtrArray, CVolume<REAL>* > m_arr_dVolumes_x_Region;

	// flags for recalc
	mutable CArray<BOOL, BOOL> m_arr_bRecompute_dVolumes_x_Region;

	// partial derivative histogram bins
	mutable CMatrixNxM<> m_arr_dBins;

	// partial derivative histogram bins
	mutable CMatrixNxM<> m_arr_dGBins;

	// flags for recalc
	mutable CArray<BOOL, BOOL> m_arr_bRecompute_dBins;

};	// class CHistogram


#endif // !defined(HISTOGRAM_H)
