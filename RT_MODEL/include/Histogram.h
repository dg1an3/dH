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
	CVolume<REAL> *GetRegion() const;
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
	int GetGroupCount() const;
	CVolume<REAL> *Get_dVolume(int nAt, int *pnGroup = NULL) const;
	int Add_dVolume(CVolume<REAL> *p_dVolume, int nGroup);

	// partial derivatives
	const CVectorN<>& Get_dBins(int nAt) const;
	const CVectorN<>& Get_dGBins(int nAt) const;

	// evaluation for testing
	REAL Eval_GBin(REAL x) const;
	REAL Eval_dGBin(int nAt_dVolume, REAL x) const;

	void ConvGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;
	void Conv_dGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;

	BOOL IsContributing(int nElement);

protected:
	// change handler for when the volume or region changes
	void OnVolumeChange(CObservableEvent *pSource, void *);

	// helpers
	const CVolume<short> * GetBinVolume(int nAt) const;

	// TODO: make const <need to change signature on Resample>
	CVolume<REAL> * GetBinScaledVolume() const;
	const CVolume<REAL> * Get_dVolume_x_Region(int nAt) const;

private:
	// association to the volume over which the histogram is formed
	CVolume<REAL> *m_pVolume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<REAL> *m_pRegion;
	CTypedPtrArray<CObArray, CVolume<REAL> *> m_arrRegionRotate;

	// 
	REAL m_minValue;
	REAL m_binWidth;

	// bin volume
	mutable CTypedPtrArray<CObArray, CVolume<short> *> m_arrBinVolume;
	mutable CArray<BOOL, BOOL> m_arr_bRecomputeBinVolume;

	// histogram bins
	mutable CVectorN<> m_arrBins;
	mutable CVectorN<> m_arrBinMeans;

	// flag to indicate bins should be recomputed
	mutable BOOL m_bRecomputeBins;

#if defined(USE_IPP)
	mutable CVolume<REAL> m_volRotate;
	mutable CVolume<REAL> *m_pVolume_BinScaled;

	mutable BOOL m_bRecomputeBinScaledVolume;

	mutable CVolume<short> *m_pVolume_BinLowInt;
	mutable CVolume<REAL> *m_pVolume_Frac;
	mutable CVolume<REAL> *m_pVolume_FracLow;
#endif

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


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinForValue
// 
// computes the bin for a particular intensity value
//////////////////////////////////////////////////////////////////////
inline int CHistogram::GetBinForValue(REAL value) const
{
	return (int) floor((value - m_minValue) / m_binWidth + 0.5);

}	// CHistogram::GetBinForValue


#endif // !defined(HISTOGRAM_H)
