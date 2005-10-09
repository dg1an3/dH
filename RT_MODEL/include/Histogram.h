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

const REAL GBINS_BUFFER = R(8.0);

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
	CHistogram(CVolume<VOXEL_REAL> *pImage = NULL, 
		CVolume<VOXEL_REAL> *pRegion = NULL);

	// destructor
	virtual ~CHistogram();

	// serialization
	DECLARE_SERIAL(CHistogram);
	virtual void Serialize(CArchive& ar);

	// association to the volume over which the histogram is formed
	CVolume<VOXEL_REAL> *GetVolume();
	void SetVolume(CVolume<VOXEL_REAL> *pVolume);

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<VOXEL_REAL> *GetRegion() const;
	void SetRegion(CVolume<VOXEL_REAL> *pVolume);

	// histogram parameters
	REAL GetBinMinValue() const;
	REAL GetBinWidth() const;
	void SetBinning(REAL min_value, REAL width, 
		REAL sigma_mult = 0.0);
	int GetBinForValue(REAL value) const;
	const CVectorN<>& GetBinMeans() const;

	// Gbinning parameter
	REAL GetGBinSigma(void) const;
	void SetGBinSigma(REAL sigma);

	// returns a reference to change event representing changes in binning parameters
	CObservableEvent& GetBinningChangeEvent() { return m_eventBinningChange; }

	// accessors for bin data
	const CVectorN<>& GetBins() const;
	const CVectorN<>& GetCumBins() const;

	// Gbin accessor
	const CVectorN<>& GetGBins() const;
	const CVectorN<>& GetGBinMeans() const;

	// partial derivative volumes
	int Get_dVolumeCount() const;
	int GetGroupCount() const;
	CVolume<VOXEL_REAL> *Get_dVolume(int nAt, int *pnGroup = NULL) const;
	int Add_dVolume(CVolume<VOXEL_REAL> *p_dVolume, int nGroup, int nLevel = -1, 
		CVolume<VOXEL_REAL> *pIsoVolume = NULL);

	// partial derivatives
	const CVectorN<>& Get_dBins(int nAt) const;
	const CVectorN<>& Get_dGBins(int nAt) const;

	// evaluation for testing
	REAL Eval_GBin(REAL x) const;
	REAL Eval_dGBin(int nAt_dVolume, REAL x) const;

	// convolve helpers
	void ConvGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;
	void Conv_dGauss(const CVectorN<>& buffer_in, 
							CVectorN<>& buffer_out) const;

	// determines if the given dVolume is contributing to the masked region
	bool IsContributing(int nElement);

protected:
	// change handler for when the volume or region changes
	void OnVolumeChange(CObservableEvent *pSource, void *);

	// helpers
	const CVolume<short> * GetBinVolume(int nAt) const;
	const CVolume<VOXEL_REAL> * GetBinScaledVolume() const;
	const CVolume<VOXEL_REAL> * Get_dVolume_x_Region(int nAt) const;

private:
	// association to the volume over which the histogram is formed
	CVolume<VOXEL_REAL> *m_pVolume;

	// association to a congruent volume describing the region over
	//		which the histogram is formed -- contains a 1.0 for voxels
	//		within the region, 0.0 elsewhere
	CVolume<VOXEL_REAL> *m_pRegion;
	CAutoPtrArray<CVolume<VOXEL_REAL> > m_arrRegionRotate;

	// helpers for adaptive beamlets
	int m_nLevel;
	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>* > m_arrRegionIsoRotate;

	// binning parameters
	REAL m_minValue;
	REAL m_binWidth;

	// the change event for this object
	CObservableEvent m_eventBinningChange;

	// bin volume
	mutable CAutoPtrArray< CVolume<short> > m_arrBinVolume;
	mutable CArray<bool, bool> m_arr_bRecomputeBinVolume;

	// histogram bins
	mutable CVectorN<> m_arrBins;
	mutable CVectorN<> m_arrBinMeans;

	// flag to indicate bins should be recomputed
	mutable BOOL m_bRecomputeBins;

#if defined(USE_IPP)
	mutable CVolume<VOXEL_REAL> m_volRotate;
	mutable CVolume<VOXEL_REAL> *m_pVolume_BinScaled;

	mutable bool m_bRecomputeBinScaledVolume;

	mutable CVolume<short> *m_pVolume_BinLowInt;
	mutable CVolume<VOXEL_REAL> *m_pVolume_Frac;
	mutable CVolume<VOXEL_REAL> *m_pVolume_FracLow;
#endif

	// cumulative bins
	mutable CVectorN<> m_arrCumBins;

	// flag to indicate cumulative bins should be recomputed
	mutable bool m_bRecomputeCumBins;

	// the binning kernel width -- set to zero for a traditional
	//		histogram
	REAL m_binKernelSigma;
	CVectorN<> m_binKernel;
	CVectorN<> m_bin_dKernel;

	// array of GBins + means
	mutable CVectorN<> m_arrGBins;
	mutable CVectorN<> m_arrGBinMeans;

	// array of partial derivative volumes
	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>* > m_arr_dVolumes;
	CArray<int, int> m_arrVolumeGroups;

	// array of partial derivative X region
	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>* > m_arr_dVolumes_x_Region;

	// flags for recalc
	mutable CArray<bool, bool> m_arr_bRecompute_dVolumes_x_Region;

	// partial derivative histogram bins
	mutable CMatrixNxM<> m_arr_dBins;

	// partial derivative histogram bins
	mutable CMatrixNxM<> m_arr_dGBins;

	// flags for recalc
	mutable CArray<bool, bool> m_arr_bRecompute_dBins;

};	// class CHistogram


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinForValue
// 
// computes the bin for a particular intensity value
//////////////////////////////////////////////////////////////////////
inline int CHistogram::GetBinForValue(REAL value) const
{
	return Round<int>((value - m_minValue) / m_binWidth);

}	// CHistogram::GetBinForValue


#endif // !defined(HISTOGRAM_H)
