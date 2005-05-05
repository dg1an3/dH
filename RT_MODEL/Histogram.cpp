// Histogram.cpp: implementation of the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Histogram.h"

#include <MathUtil.h>

// #include <MatrixBase.inl>

#include <ipps.h>
#include <ippcv.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const REAL GBINS_BUFFER = 2.0;
const REAL GBINS_KERNEL_WIDTH = 4.0;

///////////////////////////////////////////////////////////////////////////////
// CHistogram::ConvGauss
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogram::ConvGauss(const CVectorN<>& buffer_in, 
						   CVectorN<>& buffer_out) const
{
#ifdef USE_IPP
	static __declspec(thread) REAL *pArrElements = NULL;
	if (!pArrElements)
	{
		pArrElements = new REAL[8192];
	}
	CVectorN<> buffer_out_temp;
	buffer_out_temp.SetElements(buffer_in.GetDim() + m_binKernel.GetDim() + 1, pArrElements, FALSE);
	// buffer_out_temp.SetDim(buffer_in.GetDim() + m_binKernel.GetDim() + 1);
	// buffer_out_temp.SetZero();

	IppStatus stat = ippsConv_32f(
		&buffer_in[0], buffer_in.GetDim(),
		&m_binKernel[0], m_binKernel.GetDim(),
		&buffer_out_temp[0]);
	ASSERT(stat == ippStsNoErr);

	int nSize = __min(buffer_out.GetDim(), buffer_out_temp.GetDim() - m_binKernel.GetDim() / 2);
	memcpy(&buffer_out[0], &buffer_out_temp[m_binKernel.GetDim() / 2], nSize * sizeof(REAL));
#else
    int nNeighborhood = m_binKernel.GetDim() / 2;

	buffer_out.SetZero();

    for (int nX = 0; nX < buffer_out.GetDim(); nX++)
	{
		int nMin = __max(nX - nNeighborhood, 0) - nX;
		int nMax = __min(nX + nNeighborhood, buffer_in.GetDim()-1) - nX;
        for (int nZ = nMin; nZ <= nMax; nZ++)
		{
            buffer_out[nX] += m_binKernel[nZ + nNeighborhood]
				* buffer_in[nX + nZ];
		}
	}
#endif

	LOG_EXPR_EXT(buffer_in);
	LOG_EXPR_EXT(buffer_out);
	LOG_EXPR_EXT(m_binKernel);

}	// CHistogram::ConvGauss


///////////////////////////////////////////////////////////////////////////////
// CHistogram::Conv_dGauss
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogram::Conv_dGauss(const CVectorN<>& buffer_in, 
				 CVectorN<>& buffer_out) const
{
#ifdef USE_IPP
	static __declspec(thread) REAL *pArrElements = NULL;
	if (!pArrElements)
	{
		pArrElements = new REAL[8192];
	}
	CVectorN<> buffer_out_temp;
	buffer_out_temp.SetElements(buffer_in.GetDim() + m_bin_dKernel.GetDim() + 1, pArrElements, FALSE);
	// buffer_out_temp.SetDim(buffer_in.GetDim() + m_bin_dKernel.GetDim() + 1);
	// buffer_out_temp.SetZero();

	IppStatus stat = ippsConv_32f(
		&buffer_in[0], buffer_in.GetDim(),
		&m_bin_dKernel[0], m_bin_dKernel.GetDim(),
		&buffer_out_temp[0]);
	ASSERT(stat == ippStsNoErr);

	int nSize = __min(buffer_out.GetDim(), buffer_out_temp.GetDim() - m_bin_dKernel.GetDim() / 2);
	memcpy(&buffer_out[0], &buffer_out_temp[m_bin_dKernel.GetDim() / 2], nSize * sizeof(REAL));
#else

	REAL dx = GetBinWidth();

    int nNeighborhood = ceil(GBINS_KERNEL_WIDTH * m_binKernelSigma / GetBinWidth()); 
	buffer_out.SetZero();

    for (int nX = 0; nX < buffer_out.GetDim(); nX++)
	{
		int nMin = __max(nX - nNeighborhood, 0) - nX;
		int nMax = __min(nX + nNeighborhood, buffer_in.GetDim()-1) - nX;
        for (int nZ = nMin; nZ <= nMax; nZ++)
		{
            buffer_out[nX] += m_bin_dKernel[nZ + nNeighborhood]
				* buffer_in[nX + nZ];
		}
	}
#endif

}	// CHistogram::Conv_dGauss



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CHistogram::CHistogram
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CHistogram::CHistogram(CVolume<REAL> *pVolume, CVolume<REAL> *pRegion)
	: m_bRecomputeBins(TRUE),
		m_bRecomputeCumBins(TRUE),
		m_pVolume(pVolume),

#if defined(USE_IPP)
		m_pVolume_BinScaled(NULL),
		m_bRecomputeBinScaledVolume(TRUE),
		m_pVolume_BinLowInt(NULL),
		m_pVolume_Frac(NULL),
		m_pVolume_FracLow(NULL),
#endif
		m_pRegion(pRegion),
		m_binKernelSigma(0.0)
{
	SetGBinSigma(0.25);
	SetBinning((REAL) 0.0, (REAL) 0.1, GBINS_BUFFER);

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

}	// CHistogram::CHistogram


//////////////////////////////////////////////////////////////////////
// CHistogram::~CHistogram
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CHistogram::~CHistogram()
{
#if defined(USE_IPP)
	delete m_pVolume_BinScaled;
	delete m_pVolume_BinLowInt;
	delete m_pVolume_Frac;
	delete m_pVolume_FracLow;

	for (int nAt = 0; nAt < m_arr_dVolumes_x_Region.GetSize(); nAt++)
	{
		delete m_arr_dVolumes_x_Region[nAt];
	}
#endif
}	// CHistogram::~CHistogram


//////////////////////////////////////////////////////////////////////
// CHistogram::GetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
// association to the volume over which the histogram is formed
CVolume<REAL> *CHistogram::GetVolume()
{
	return m_pVolume;

}	// CHistogram::GetVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::SetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::SetVolume(CVolume<REAL> *pVolume)
{
	if (m_pVolume)
	{
		RemoveObserver<CHistogram>(&m_pVolume->GetChangeEvent(), this, 
			OnVolumeChange);
	}

	// set the pointer
	m_pVolume = pVolume;

	// update for new volume
	OnVolumeChange(NULL, NULL);

	if (m_pVolume)
	{
		AddObserver<CHistogram>(&m_pVolume->GetChangeEvent(), this, 
			OnVolumeChange);
	}

	// fire a change event
	GetChangeEvent().Fire();

}	// CHistogram::SetVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::GetRegion
// 
// association to a congruent volume describing the region over
//		which the histogram is formed -- contains a 1.0 for voxels
//		within the region, 0.0 elsewhere
//////////////////////////////////////////////////////////////////////
CVolume<REAL> *CHistogram::GetRegion() const
{
	return m_pRegion;

}	// CHistogram::GetRegion


//////////////////////////////////////////////////////////////////////
// CHistogram::SetRegion
// 
// sets the computation region for the histogram
//////////////////////////////////////////////////////////////////////
void CHistogram::SetRegion(CVolume<REAL> *pRegion)
{
	// set the pointer
	m_pRegion = pRegion;

	// check region for negatives
	for (int nAtY = 0; nAtY < m_pRegion->GetHeight(); nAtY++)
	{
		for (int nAtX = 0; nAtX < m_pRegion->GetWidth(); nAtX++)
		{
			ASSERT(m_pRegion->GetVoxels()[0][nAtY][nAtX] >= 0.0);
		}
	}

	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

	// fire a change event
	GetChangeEvent().Fire();

}	// CHistogram::SetRegion



//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinMinValue
// 
// returns the minimum bin value
//////////////////////////////////////////////////////////////////////
REAL CHistogram::GetBinMinValue() const
{
	return m_minValue;

}	// CHistogram::GetBinMinValue


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinWidth
// 
// returns the bin step value
//////////////////////////////////////////////////////////////////////
REAL CHistogram::GetBinWidth() const
{
	return m_binWidth;

}	// CHistogram::GetBinWidth


//////////////////////////////////////////////////////////////////////
// CHistogram::SetBinning
// 
// sets up the binning parameters
//////////////////////////////////////////////////////////////////////
void CHistogram::SetBinning(REAL min_value, REAL width, 
							REAL sigma_mult)
{
	m_minValue = min_value - sigma_mult * m_binKernelSigma;
	m_binWidth = width;

	// recalculate kernels
	SetGBinSigma(m_binKernelSigma);

	// GetChangeEvent().Fire();
	// no need to fire change here, because SetGBinSigma does it

}	// CHistogram::SetBinning



//////////////////////////////////////////////////////////////////////
// CHistogram::SetGBinSigma
// 
// sets up the binning parameters
//////////////////////////////////////////////////////////////////////
void CHistogram::SetGBinSigma(REAL sigma)
{
	m_binKernelSigma = sigma;

	REAL dx = GetBinWidth();

    int nNeighborhood = ceil(GBINS_KERNEL_WIDTH * m_binKernelSigma / GetBinWidth());

	m_binKernel.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_binKernel[nZ + nNeighborhood] = 
			dx * Gauss<REAL>(nZ * dx, m_binKernelSigma);
	}

	m_bin_dKernel.SetDim(nNeighborhood * 2 + 1);
    for (nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_bin_dKernel[nZ + nNeighborhood] = 
			dx * dGauss<REAL>(-nZ * dx, m_binKernelSigma);
	}

	GetChangeEvent().Fire();

}	// CHistogram::SetGBinSigma


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinMeans
// 
// returns a vector with mean bin values
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::GetBinMeans() const
{
	m_arrBinMeans.SetDim(m_arrBins.GetDim());
	for (int nAt = 0; nAt < m_arrBinMeans.GetDim(); nAt++)
	{
		m_arrBinMeans[nAt] = m_minValue + (REAL) nAt * m_binWidth;
	}

	return m_arrBinMeans;

}	// CHistogram::GetBinMeans


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBins
// 
// retrieves the bins for this histogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::GetBins() const
{
	return ((CHistogram&)(*this)).GetBins();

}	// CHistogram::GetBins	


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBins
// 
// retrieves the bins for this histogram
//////////////////////////////////////////////////////////////////////
CVectorN<>& CHistogram::GetBins()
{
	if (m_bRecomputeBins)
	{
		CRect rectBounds = m_pRegion->GetThresholdBounds();

		IppiSize roiSize;
		roiSize.width = rectBounds.Width()+1;
		roiSize.height = rectBounds.Height()+1;

		////////////////////////////////////////////////////////////////////
		// create low bin volume

		if (!m_pVolume_BinLowInt)
		{
			m_pVolume_BinLowInt = new CVolume<short>();
		}

		// IMPORTANT: this triggers update for subsequent m_pVolume_BinScaled's
		m_pVolume_BinLowInt->ConformTo(GetBinScaledVolume());

		IppStatus stat = ippiConvert_32f16s_C1R(
			&m_pVolume_BinScaled->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_BinScaled->GetWidth() * sizeof(REAL),
			&m_pVolume_BinLowInt->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_BinLowInt->GetWidth() * sizeof(short),
			roiSize, ippRndZero);

		// flag voxels change
		m_pVolume_BinLowInt->VoxelsChanged();

		// get voxel iliffe
		const short * const * const *pppVoxelsBinLow = 
			m_pVolume_BinLowInt->GetVoxels();

		////////////////////////////////////////////////////////////////////
		// get bin frac volume

		if (!m_pVolume_Frac)
		{
			m_pVolume_Frac = new CVolume<REAL>();
		}
		m_pVolume_Frac->ConformTo(m_pVolume_BinLowInt);

		stat = ippiConvert_16s32f_C1R(
			&m_pVolume_BinLowInt->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_BinLowInt->GetWidth() * sizeof(short),
			&m_pVolume_Frac->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_Frac->GetWidth() * sizeof(REAL),
			roiSize);

		// leaves Frac = -High Fraction
		stat = ippiSub_32f_C1IR(
			&m_pVolume_BinScaled->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_BinScaled->GetWidth() * sizeof(REAL),
			&m_pVolume_Frac->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_Frac->GetWidth() * sizeof(REAL),
			roiSize);

		m_pVolume_Frac->VoxelsChanged();

		////////////////////////////////////////////////////////////////////
		// get bin frac low volume

		if (!m_pVolume_FracLow)
		{
			m_pVolume_FracLow = new CVolume<REAL>();
		}
		m_pVolume_FracLow->ConformTo(m_pVolume_Frac);

		stat = ippiAddC_32f_C1R(
			&m_pVolume_Frac->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_Frac->GetWidth() * sizeof(REAL),
			1.0,
			&m_pVolume_FracLow->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_FracLow->GetWidth() * sizeof(REAL),
			roiSize);

		// scale by region
		stat = ippiMul_32f_C1IR(
			&m_pRegion->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pRegion->GetWidth() * sizeof(REAL),
			&m_pVolume_FracLow->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_FracLow->GetWidth() * sizeof(REAL),
			roiSize);		

		m_pVolume_FracLow->VoxelsChanged();

		const REAL * const * const *pppVoxelsFracLow = 
			m_pVolume_FracLow->GetVoxels();

		////////////////////////////////////////////////////////////////////
		// now get final bin frac volume

		stat = ippiMul_32f_C1IR(
			&m_pRegion->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pRegion->GetWidth() * sizeof(REAL),
			&m_pVolume_Frac->GetVoxels()[0][rectBounds.top][rectBounds.left], 
				m_pVolume_Frac->GetWidth() * sizeof(REAL),
			roiSize);		

		m_pVolume_Frac->VoxelsChanged();

		const REAL * const * const *pppVoxelsFrac = 
			m_pVolume_Frac->GetVoxels();


		// now set up the bins
		REAL maxValue = m_pVolume->GetMax();
		m_arrBins.SetDim(GetBinForValue(maxValue)+2);
		m_arrBins.SetZero();

		// and do the binning
		for (int nAtZ = 0; nAtZ < 1; /* m_pRegion->GetDepth(); */ nAtZ++)
		{
			for (int nAtY = rectBounds.top; nAtY <= rectBounds.bottom; nAtY++)
			{
				for (int nAtX = rectBounds.left; nAtX <= rectBounds.right; nAtX++)
				{
					const int nLowBin = pppVoxelsBinLow[nAtZ][nAtY][nAtX];

					// check that Frac and FracLow are summing to 1.0
					ASSERT(IsApproxEqual<REAL>(pppVoxelsFracLow[nAtZ][nAtY][nAtX]
						- pppVoxelsFrac[nAtZ][nAtY][nAtX],
						m_pRegion->GetVoxels()[nAtZ][nAtY][nAtX], 
						1e-3));

					ASSERT(m_pRegion->GetVoxels()[nAtZ][nAtY][nAtX] >= 0.0);

					ASSERT(pppVoxelsFracLow[nAtZ][nAtY][nAtX] >= 0.0);
					m_arrBins[nLowBin] += pppVoxelsFracLow[nAtZ][nAtY][nAtX];

					// TODO: ippi
					ASSERT(pppVoxelsFrac[nAtZ][nAtY][nAtX] <= 0.0);
					m_arrBins[nLowBin+1] -= pppVoxelsFrac[nAtZ][nAtY][nAtX];
				}
			}
		}

		if (m_binKernelSigma > 0.0)
		{			
			m_arrGBins.SetDim(GetBinForValue(maxValue + GBINS_BUFFER * m_binKernelSigma) + 1);
			ConvGauss(m_arrBins, m_arrGBins);
		}

		// now normalize
		REAL calcSum = GetRegion()->GetSum();
		for (int nAtBin = 0; nAtBin < m_arrGBins.GetDim(); nAtBin++)
		{
			// normalize each bin
			m_arrGBins[nAtBin] /= calcSum;
		}

		m_bRecomputeBins = FALSE;
	}

	return m_arrBins;

}	// CHistogram::GetBins


//////////////////////////////////////////////////////////////////////
// CHistogram::GetCumBins
// 
// computes and returns the cumulative bins
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::GetCumBins() const
{
	if (m_bRecomputeCumBins)
	{
		GetBins();

		m_arrCumBins.SetDim(m_arrBins.GetDim());
		m_arrCumBins.SetZero();

		REAL sum = 0.0;
		for (int nAt = m_arrBins.GetDim()-1; nAt >= 0; nAt--)
		{
			sum += m_arrBins[nAt];
			m_arrCumBins[nAt] = sum;
		} 
		m_bRecomputeCumBins = FALSE;
	}

	return m_arrCumBins;

}	// CHistogram::GetCumBins


//////////////////////////////////////////////////////////////////////
// CHistogram::GetGBins
// 
// computes and returns the GHistogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::GetGBins() const
{
	return ((CHistogram&)(*this)).GetGBins();

}	// CHistogram::GetGBins


//////////////////////////////////////////////////////////////////////
// CHistogram::GetGBins
// 
// computes and returns the GHistogram
//////////////////////////////////////////////////////////////////////
CVectorN<>& CHistogram::GetGBins()
{
	GetBins();
	if (m_binKernelSigma == 0.0)
	{
		ASSERT(FALSE);
		return m_arrBins;
	}

	return m_arrGBins;

}	// CHistogram::GetGBins


//////////////////////////////////////////////////////////////////////
// CHistogram::GetGBinMeans
// 
// returns a vector with mean GBin values
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::GetGBinMeans() const
{
	m_arrGBinMeans.SetDim(m_arrGBins.GetDim());
	for (int nAt = 0; nAt < m_arrGBinMeans.GetDim(); nAt++)
	{
		m_arrGBinMeans[nAt] = m_minValue + (REAL) nAt * m_binWidth;
	}

	return m_arrGBinMeans;

}	// CHistogram::GetGBinMeans


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dVolumeCount
// 
// returns the count of dVolumes
//////////////////////////////////////////////////////////////////////
int CHistogram::Get_dVolumeCount() const
{
	return m_arr_dVolumes.GetSize();

}	// CHistogram::Get_dVolumeCount

int CHistogram::GetGroupCount() const
{
	int nMaxGroup = -1;
	for (int nAt = 0; nAt < m_arrVolumeGroups.GetSize(); nAt++)
	{
		nMaxGroup = __max(nMaxGroup, m_arrVolumeGroups[nAt]);
	}

	return nMaxGroup+1;
}


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dVolume
// 
// returns the dVolume
//////////////////////////////////////////////////////////////////////
CVolume<REAL> *CHistogram::Get_dVolume(int nAt, int *pnGroup) const
{
	if (pnGroup)
	{
		(*pnGroup) = m_arrVolumeGroups[nAt];
	}

	return m_arr_dVolumes[nAt];

}	// CHistogram::Get_dVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::Add_dVolume
// 
// adds another dVolume
//////////////////////////////////////////////////////////////////////
int CHistogram::Add_dVolume(CVolume<REAL> *p_dVolume, int nGroup)
{
	int nNewVolumeIndex = m_arr_dVolumes.Add(p_dVolume);
	m_arrVolumeGroups.Add(nGroup);
	while (m_arrBinVolume.GetSize() <= nGroup)
	{
		m_arrBinVolume.Add(NULL);
	}
	if (m_arrBinVolume[nGroup] == NULL)
	{
		m_arrBinVolume[nGroup] = new CVolume<short>();
	}

	while (m_arr_bRecomputeBinVolume.GetSize() <= nGroup)
	{
		m_arr_bRecomputeBinVolume.Add(TRUE);
	}

	while (m_arrRegionRotate.GetSize() <= nGroup)
	{
		m_arrRegionRotate.Add(NULL);
	}
	if (m_arrRegionRotate[nGroup] == NULL)
	{
		// rotate region
		m_arrRegionRotate[nGroup] = new CVolume<REAL>();
		m_arrRegionRotate[nGroup]->ConformTo(p_dVolume);
		m_arrRegionRotate[nGroup]->ClearVoxels();
		Resample(m_pRegion, m_arrRegionRotate[nGroup], TRUE);
		m_arrRegionRotate[nGroup]->SetThreshold(m_pRegion->GetThreshold()); //  / 2.0);
	}

	// set flag for computing bins for new dVolume
	m_arr_bRecompute_dBins.Add(TRUE);

	// add new product volume
	CVolume<REAL> *p_dVolume_x_Region = new CVolume<REAL>();
	p_dVolume_x_Region->ConformTo(p_dVolume);
	m_arr_dVolumes_x_Region.Add(p_dVolume_x_Region);

	// flag to recompute 
	m_arr_bRecompute_dVolumes_x_Region.Add(TRUE);

	return nNewVolumeIndex;

}	// CHistogram::Add_dVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dBins
// 
// computes and returns the d/dx bins
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::Get_dBins(int nAt) const
{
	// recompute dBins if needed
	if (m_arr_bRecompute_dBins[nAt])
	{
		// set size of dBins & dGBins
		REAL maxValue = m_pVolume->GetMax();
		int nRows = GetBinForValue(maxValue)+1;
		if (m_arr_dBins.GetRows() < nRows
			|| m_arr_dBins.GetCols() < Get_dVolumeCount())
		{
			m_arr_dBins.Reshape(Get_dVolumeCount(), nRows);
		}

		// initialize reference to proper dBins and zero
		CVectorN<>& arr_dBins = m_arr_dBins[nAt];
		arr_dBins.SetZero();

		// now compute bins
		if (m_pRegion)
		{
			// get dVoxels * Region
			const REAL * const * const *ppp_dVoxels_x_Region = 
				Get_dVolume_x_Region(nAt)->GetVoxels();

			// get the bin voxels, recompute if needed
			const short * const * const *pppBinVolumeVoxels = 
				GetBinVolume(nAt)->GetVoxels(); 

			CRect rectBounds = m_arr_dVolumes_x_Region[nAt]->GetThresholdBounds();
			LOG_EXPR_EXT(rectBounds);
			for (int nAtZ = 0; nAtZ < 1; /* m_arr_dVolumes_x_Region[nAt]->GetDepth(); */ nAtZ++)
			{
				const REAL * const *pp_dVoxels_x_Region = ppp_dVoxels_x_Region[nAtZ];
				const short * const *ppBinVolumeVoxels = pppBinVolumeVoxels[nAtZ];
				for (int nAtY = rectBounds.top; nAtY <= rectBounds.bottom; nAtY++)
				{
					const REAL *p_dVoxels_x_Region = pp_dVoxels_x_Region[nAtY];
					const short *pBinVolumeVoxels = ppBinVolumeVoxels[nAtY];
					for (int nAtX = rectBounds.left; nAtX <= rectBounds.right; nAtX++)
					{
						int nBin = pBinVolumeVoxels[nAtX];
						arr_dBins[nBin] -= p_dVoxels_x_Region[nAtX];
					}
				}
			}
		}
		else
		{
			// get the dVoxels
			const REAL *p_dVoxels = &Get_dVolume(nAt)->GetVoxels()[0][0][0];

			const short *pBinVolumeVoxels = &GetBinVolume(nAt)->GetVoxels()[0][0][0];

			for (int nAtVoxel = 0; nAtVoxel < m_pVolume->GetVoxelCount(); nAtVoxel++)
			{
				int nBin = pBinVolumeVoxels[nAtVoxel];
				arr_dBins[nBin] += -p_dVoxels[nAtVoxel];
			}
		}

		// convolve for dGBins, if needed
		if (m_binKernelSigma > 0.0)
		{	
			// set shape for dGBins
			int nRows = GetBinForValue(maxValue 
						+ GBINS_BUFFER * m_binKernelSigma) + 1;
			if (m_arr_dGBins.GetRows() < nRows
				|| m_arr_dGBins.GetCols() < Get_dVolumeCount())
			{
				m_arr_dGBins.Reshape(Get_dVolumeCount(), nRows);
			}

			Conv_dGauss(arr_dBins, m_arr_dGBins[nAt]);

			// now normalize?
			REAL calcSum = GetRegion()->GetSum();

			for (int nAtBin = 0; nAtBin < m_arr_dGBins.GetRows(); nAtBin++)
			{
				// normalize this bin
				m_arr_dGBins[nAt][nAtBin] /= calcSum;
			}
		}

		m_arr_bRecompute_dBins[nAt] = FALSE;
	}

	return m_arr_dBins[nAt];

}	// CHistogram::Get_dBins



//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dGBins
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::Get_dGBins(int nAt) const
{
	if (m_binKernelSigma == 0.0)
	{
		ASSERT(FALSE);	// not OK, because we need to normalize
		return Get_dBins(nAt);
	}

	Get_dBins(nAt);

	return m_arr_dGBins[(int) nAt];

}	// CHistogram::Get_dGBins


//////////////////////////////////////////////////////////////////////
// CHistogram::Eval_GBin
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
REAL CHistogram::Eval_GBin(REAL x) const
{
	REAL *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
	REAL *pRegionVoxel = NULL;
	if (m_pRegion)
	{
		pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
	}

	REAL sum = 0.0;
	for (int nAtVoxel = 0; nAtVoxel < m_pVolume->GetVoxelCount(); nAtVoxel++)
	{
		if (!m_pRegion || pRegionVoxel[nAtVoxel])
		{
			sum += Gauss(pVoxels[nAtVoxel] - x, m_binKernelSigma);
		}
	}
	
	return m_binWidth * sum;

}	// CHistogram::Eval_GBin


//////////////////////////////////////////////////////////////////////
// CHistogram::Eval_dGBin
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
REAL CHistogram::Eval_dGBin(int nAt_dVolume, REAL x) const
{
	REAL *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
	REAL *pRegionVoxel = NULL;
	if (m_pRegion)
	{
		pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
	}

	REAL *p_dVoxels = NULL;
	if (nAt_dVolume >= 0)
	{	
		p_dVoxels = &Get_dVolume(nAt_dVolume)->GetVoxels()[0][0][0];
	}

	REAL sum = 0.0;
	for (int nAtVoxel = 0; nAtVoxel < m_pVolume->GetVoxelCount(); nAtVoxel++)
	{
		if (!m_pRegion || pRegionVoxel[nAtVoxel])
		{
			if (p_dVoxels)
			{
				sum += -p_dVoxels[nAtVoxel] 
					* -dGauss(pVoxels[nAtVoxel] - x, m_binKernelSigma);
			}
			else
			{
				sum += -dGauss(pVoxels[nAtVoxel] - x, m_binKernelSigma);
			}
		}
	}
	
	return m_binWidth * sum;

}	// CHistogram::Eval_dGBin


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
	m_bRecomputeBinScaledVolume = TRUE;

	int nGroups = GetGroupCount();
	for (int nAt = 0; nAt < nGroups; nAt++)
	{
		m_arr_bRecomputeBinVolume[nAt] = TRUE;
	}

	// set recalc flag for dBins
	for (nAt = 0; nAt < Get_dVolumeCount(); nAt++)
	{
		m_arr_bRecompute_dBins[nAt] = TRUE;
	}

	// TODO: flag recompute dVolume_x_Region

	// fire a change event
	GetChangeEvent().Fire();

}	// CHistogram::OnVolumeChange


//////////////////////////////////////////////////////////////////////
// CHistogram::OnVolumeChange
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
BOOL CHistogram::IsContributing(int nElement)
{
	const CVolume<REAL>* pVolume = Get_dVolume_x_Region(nElement);
	return (pVolume->GetThresholdBounds().left < pVolume->GetThresholdBounds().right
		&& pVolume->GetThresholdBounds().top < pVolume->GetThresholdBounds().bottom);
}


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dVolume_x_Region
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
const CVolume<REAL> * CHistogram::Get_dVolume_x_Region(int nAt) const
{
	if (m_arr_bRecompute_dVolumes_x_Region[nAt])
	{
		int nGroup = m_arrVolumeGroups[nAt];

		REAL *p_dVoxels = &Get_dVolume(nAt)->GetVoxels()[0][0][0];

		// get the region voxels
		REAL *pRegionVoxel = m_pRegion ? 
			&m_arrRegionRotate[nGroup]->GetVoxels()[0][0][0] : NULL;

		REAL *p_dVoxels_x_Region = &m_arr_dVolumes_x_Region[nAt]->GetVoxels()[0][0][0];
		for (int nAtVoxel = 0; nAtVoxel < m_pRegion->GetVoxelCount(); nAtVoxel++)
		{
			p_dVoxels_x_Region[nAtVoxel] = pRegionVoxel[nAtVoxel] * p_dVoxels[nAtVoxel];
		}
		m_arr_dVolumes_x_Region[nAt]->VoxelsChanged();

		m_arr_dVolumes_x_Region[nAt]->SetThreshold(
			Get_dVolume(nAt)->GetThreshold());

		m_arr_bRecompute_dVolumes_x_Region[nAt] = FALSE;
	}

	return m_arr_dVolumes_x_Region[nAt];
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinVolume
// 
// 
//////////////////////////////////////////////////////////////////////
const CVolume<short> * CHistogram::GetBinVolume(int nAt) const
{
	int nGroup = m_arrVolumeGroups[nAt];

	if (m_arr_bRecomputeBinVolume[nGroup])
	{
		// rotate to proper orientation
		static CVolume<REAL> volRotate;
		volRotate.ConformTo(Get_dVolume(nAt));
		volRotate.ClearVoxels();

		Resample(GetBinScaledVolume(), &volRotate, TRUE);
		m_arrBinVolume[nGroup]->ConformTo(&volRotate);
		m_arrBinVolume[nGroup]->ClearVoxels();

		CRect rect = m_arrRegionRotate[nGroup]->GetThresholdBounds();

		IppiSize roiSize;
		roiSize.width = rect.Width()+1;
		roiSize.height = rect.Height()+1;

		// now convert to integer bin values

		// get the main volume voxels
		IppStatus stat = ippiConvert_32f16s_C1R(
			&volRotate.GetVoxels()[0][rect.top][rect.left], 
				volRotate.GetWidth() * sizeof(REAL),
			&m_arrBinVolume[nGroup]->GetVoxels()[0][rect.top][rect.left], 
				m_arrBinVolume[nGroup]->GetWidth() * sizeof(short),
			roiSize, ippRndNear);

		// flag change
		m_arrBinVolume[nGroup]->VoxelsChanged();
		m_arr_bRecomputeBinVolume[nGroup] = FALSE;
	}

	return m_arrBinVolume[nGroup];
}

//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinScaledVolume
// 
// 
//////////////////////////////////////////////////////////////////////
CVolume<REAL> * CHistogram::GetBinScaledVolume() const
{
	if (m_bRecomputeBinScaledVolume)
	{
		if (!m_pVolume_BinScaled)
		{
			m_pVolume_BinScaled = new CVolume<REAL>();
		}
		m_pVolume_BinScaled->ConformTo(m_pVolume);
		m_pVolume_BinScaled->ClearVoxels();

		// restrict to the region threshold bounds
		CRect rect = m_pRegion->GetThresholdBounds();

		IppiSize roiSize;
		roiSize.width = rect.Width()+1;
		roiSize.height = rect.Height()+1;

		// now convert
		IppStatus stat = ippiSubC_32f_C1R(
			&m_pVolume->GetVoxels()[0][rect.top][rect.left], 
				m_pVolume->GetWidth() * sizeof(REAL),
			m_minValue,
			&m_pVolume_BinScaled->GetVoxels()[0][rect.top][rect.left],
				m_pVolume_BinScaled->GetWidth() * sizeof(REAL),
			roiSize);
		
		// and scale
		stat = ippiMulC_32f_C1IR(1.0 / m_binWidth,
			&m_pVolume_BinScaled->GetVoxels()[0][rect.top][rect.left], 
				m_pVolume_BinScaled->GetWidth() * sizeof(REAL),
			roiSize);

		// flag voxels changed
		m_pVolume_BinScaled->VoxelsChanged();

		m_bRecomputeBinScaledVolume = FALSE;
	}

	return m_pVolume_BinScaled;
}
