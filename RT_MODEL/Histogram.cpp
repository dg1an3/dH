// Histogram.cpp: implementation of the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Histogram.h"

#include <MathUtil.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const double GBINS_BUFFER = 2.0;

///////////////////////////////////////////////////////////////////////////////
// CHistogram::ConvGauss
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogram::ConvGauss(const CVectorN<>& buffer_in, 
						   CVectorN<>& buffer_out) const
{
    int nNeighborhood = m_binKernel.GetDim() / 2;

	buffer_out.SetZero();

    for (int nX = 0; nX < buffer_out.GetDim(); nX++)
	{
        for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
		{
            if ((nX + nZ) >= 0 && (nX + nZ) < buffer_in.GetDim())
			{
                buffer_out[nX] += m_binKernel[nZ + nNeighborhood]
					* buffer_in[nX + nZ];
            }
		}
	} 

}	// CHistogram::ConvGauss


///////////////////////////////////////////////////////////////////////////////
// CHistogram::Conv_dGauss
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogram::Conv_dGauss(const CVectorN<>& buffer_in, 
				 CVectorN<>& buffer_out) const
{
	REAL dx = GetBinWidth();

    int nNeighborhood = ceil(4.0 * m_binKernelSigma / GetBinWidth()); 
	buffer_out.SetZero();

    for (int nX = 0; nX < buffer_out.GetDim(); nX++)
	{
        for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
		{
            if ((nX + nZ) >= 0 && (nX + nZ) < buffer_in.GetDim())
			{
                buffer_out[nX] += m_bin_dKernel[nZ + nNeighborhood]
					* buffer_in[nX + nZ];
            }
		}
	}

}	// CHistogram::Conv_dGauss



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CHistogram::CHistogram
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CHistogram::CHistogram(CVolume<double> *pVolume, CVolume<double> *pRegion)
	: m_bRecomputeBins(TRUE),
		m_bRecomputeCumBins(TRUE),
		m_pVolume(pVolume),
		m_pRegion(pRegion),
		m_binKernelSigma(0.0),
		m_pArr_dVolumes(NULL),
		m_arr_dBins(NULL),
		m_arr_dGBins(NULL)
{
	SetGBinSigma(0.25);
	SetBinning(0.0, 0.1, GBINS_BUFFER);

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
	delete m_pArr_dVolumes;

	delete [] m_arr_dBins;
	delete [] m_arr_dGBins;

}	// CHistogram::~CHistogram


//////////////////////////////////////////////////////////////////////
// CHistogram::GetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
// association to the volume over which the histogram is formed
CVolume<double> *CHistogram::GetVolume()
{
	return m_pVolume;

}	// CHistogram::GetVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::SetVolume
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
void CHistogram::SetVolume(CVolume<double> *pVolume)
{
	if (m_pVolume)
	{
		RemoveObserver<CHistogram>(&m_pVolume->GetChangeEvent(), this, 
			OnVolumeChange);
	}

	// set the pointer
	m_pVolume = pVolume;

	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

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
CVolume<double> *CHistogram::GetRegion()
{
	return m_pRegion;

}	// CHistogram::GetRegion


//////////////////////////////////////////////////////////////////////
// CHistogram::SetRegion
// 
// sets the computation region for the histogram
//////////////////////////////////////////////////////////////////////
void CHistogram::SetRegion(CVolume<double> *pRegion)
{
	// set the pointer
	m_pRegion = pRegion;

	// flag recomputation
	m_bRecomputeBins = TRUE;
	m_bRecomputeCumBins = TRUE;

	// fire a change event
	GetChangeEvent().Fire();

}	// CHistogram::SetRegion


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinForValue
// 
// computes the bin for a particular intensity value
//////////////////////////////////////////////////////////////////////
long CHistogram::GetBinForValue(double value) const
{
	return (long) floor((value - m_minValue) / m_binWidth + 0.5);

}	// CHistogram::GetBinForValue


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinMinValue
// 
// returns the minimum bin value
//////////////////////////////////////////////////////////////////////
double CHistogram::GetBinMinValue() const
{
	return m_minValue;

}	// CHistogram::GetBinMinValue


//////////////////////////////////////////////////////////////////////
// CHistogram::GetBinWidth
// 
// returns the bin step value
//////////////////////////////////////////////////////////////////////
double CHistogram::GetBinWidth() const
{
	return m_binWidth;

}	// CHistogram::GetBinWidth


//////////////////////////////////////////////////////////////////////
// CHistogram::SetBinning
// 
// sets up the binning parameters
//////////////////////////////////////////////////////////////////////
void CHistogram::SetBinning(double min_value, double width, 
							double sigma_mult)
{
	m_minValue = min_value - sigma_mult * m_binKernelSigma;
	m_binWidth = width;

	// recalculate kernels
	SetGBinSigma(m_binKernelSigma);

}	// CHistogram::SetBinning



//////////////////////////////////////////////////////////////////////
// CHistogram::SetGBinSigma
// 
// sets up the binning parameters
//////////////////////////////////////////////////////////////////////
void CHistogram::SetGBinSigma(double sigma)
{
	m_binKernelSigma = sigma;

	double dx = GetBinWidth();

    int nNeighborhood = ceil(4.0 * m_binKernelSigma / GetBinWidth()); // 

	m_binKernel.SetDim(nNeighborhood * 2 + 1);
    for (int nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_binKernel[nZ + nNeighborhood] = 
			dx * Gauss<double>(nZ * dx, m_binKernelSigma);
	}

	m_bin_dKernel.SetDim(nNeighborhood * 2 + 1);
    for (nZ = -nNeighborhood; nZ <= nNeighborhood; nZ++)
	{
        m_bin_dKernel[nZ + nNeighborhood] = 
			dx * dGauss<double>(-nZ * dx, m_binKernelSigma);
	}

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
		m_arrBinMeans[nAt] = m_minValue + (double) nAt * m_binWidth;
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
		double *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
		double *pRegionVoxel = NULL;
		if (m_pRegion)
		{
			pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
		}

		double maxValue = m_pVolume->GetMax();
		m_arrBins.SetDim(GetBinForValue(maxValue)+2);
		m_arrBins.SetZero();
		// ZeroMemory(&m_arrBins[0], sizeof(double) * m_arrBins.GetSize());

		for (int nAtVoxel = 0; nAtVoxel < m_pVolume->GetVoxelCount(); nAtVoxel++)
		{
#define _LINEAR_INTERP
#ifdef _LINEAR_INTERP
			double bin = (pVoxels[nAtVoxel] - m_minValue) / m_binWidth;
			long nLowBin = floor(bin);
			long nHighBin = nLowBin+1;

			if (!m_pRegion || pRegionVoxel[nAtVoxel] == 1.0)
			{
				// compute the bin
				double binMinValue = ((double) nLowBin) * m_binWidth + m_minValue;
				m_arrBins[(int) nHighBin] += (pVoxels[nAtVoxel] - binMinValue) / m_binWidth;
				ASSERT(m_arrBins[(int) nHighBin] >= 0.0);

				double binMaxValue = ((double) nHighBin) * m_binWidth + m_minValue;
				m_arrBins[(int) nLowBin] += (binMaxValue - pVoxels[nAtVoxel]) / m_binWidth;
				ASSERT(m_arrBins[(int) nLowBin] >= 0.0);
			}
			else if (m_pRegion)
			{
				// compute the bin
				double binMinValue = ((double) nLowBin) * m_binWidth + m_minValue;
				m_arrBins[(int) nHighBin] += pRegionVoxel[nAtVoxel] * (pVoxels[nAtVoxel] - binMinValue) / m_binWidth;
				ASSERT(m_arrBins[(int) nHighBin] >= 0.0);

				double binMaxValue = ((double) nHighBin) * m_binWidth + m_minValue;
				m_arrBins[(int) nLowBin] += pRegionVoxel[nAtVoxel] * (binMaxValue - pVoxels[nAtVoxel]) / m_binWidth;
				ASSERT(m_arrBins[(int) nLowBin] >= 0.0);
			}
#else
			long nBin = GetBinForValue(pVoxels[nAtVoxel]);
			ASSERT(nBin < m_arrBins.GetSize());

			if (!m_pRegion || pRegionVoxel[nAtVoxel])
			{
				// compute the bin
				m_arrBins[nBin]++;
			}
#endif
		}

		if (m_binKernelSigma > 0.0)
		{			
			m_arrGBins.SetDim(GetBinForValue(maxValue 
				+ GBINS_BUFFER * m_binKernelSigma) + 1);
			ConvGauss(m_arrBins, m_arrGBins);
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

		double sum = 0.0;
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
		m_arrGBinMeans[nAt] = m_minValue + (double) nAt * m_binWidth;
	}

	return m_arrGBinMeans;

}	// CHistogram::GetGBinMeans


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dVolumeCount
// 
// returns the count of dVolumes
//////////////////////////////////////////////////////////////////////
long CHistogram::Get_dVolumeCount() const
{
	if (m_pArr_dVolumes)
	{
		return m_pArr_dVolumes->GetSize();
	}
	else
	{
		return NULL;
	}

}	// CHistogram::Get_dVolumeCount


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dVolume
// 
// returns the dVolume
//////////////////////////////////////////////////////////////////////
CVolume<double> *CHistogram::Get_dVolume(long nAt) const
{
	return ((CVolume<double> *)((*m_pArr_dVolumes)[nAt]));

}	// CHistogram::Get_dVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::Add_dVolume
// 
// adds another dVolume
//////////////////////////////////////////////////////////////////////
long CHistogram::Add_dVolume(CVolume<double> *p_dVolume)
{
	if (NULL == m_pArr_dVolumes)
	{
		m_pArr_dVolumes = new CObArray;
	}
	long nNewVolumeIndex = m_pArr_dVolumes->Add(p_dVolume);
	long nNewVolumeCount = m_pArr_dVolumes->GetSize();

	// resize dBins arrays
	delete [] m_arr_dBins;
	m_arr_dBins = new CVectorN<>[nNewVolumeCount+1];

	delete [] m_arr_dGBins;
	m_arr_dGBins = new CVectorN<>[nNewVolumeCount+1];

	return nNewVolumeIndex;

}	// CHistogram::Add_dVolume


//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dBins
// 
// computes and returns the d/dx bins
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::Get_dBins(long nAt) const
{
	if (!m_arr_dBins)
	{
		m_arr_dBins = new CVectorN<>[1];
	}
	CVectorN<>& arr_dBins = m_arr_dBins[nAt+1];

	if (TRUE) // arr_dBins.GetSize() == 0)
	{
		double *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
		double *pRegionVoxel = NULL;
		if (m_pRegion)
		{
			pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
		}

		double *p_dVoxels = NULL;
		if (nAt >= 0)
		{
			p_dVoxels = &Get_dVolume(nAt)->GetVoxels()[0][0][0];
		}

		double maxValue = m_pVolume->GetMax();
		arr_dBins.SetDim(GetBinForValue(maxValue)+1);
		arr_dBins.SetZero();

		for (int nAtVoxel = 0; nAtVoxel < m_pVolume->GetVoxelCount(); nAtVoxel++)
		{
			int nBin = GetBinForValue(pVoxels[nAtVoxel]);
			ASSERT(nBin < arr_dBins.GetDim());

			if (!m_pRegion || (pRegionVoxel[nAtVoxel] == 1.0))
			{
				if (p_dVoxels)
				{
					// compute the bin
					arr_dBins[nBin] += -p_dVoxels[nAtVoxel];
				}
				else
				{
					arr_dBins[nBin]++;
				}
			}
			else if (m_pRegion)
			{
				if (p_dVoxels)
				{
					// compute the bin
					arr_dBins[nBin] += pRegionVoxel[nAtVoxel] * -p_dVoxels[nAtVoxel];
				}
				else
				{
					arr_dBins[nBin] += pRegionVoxel[nAtVoxel];
				}
			}
			 
		}

		if (m_binKernelSigma > 0.0)
		{	
			if (!m_arr_dGBins)
			{
				m_arr_dGBins = new CVectorN<>[1];
			}
			CVectorN<>& arr_dGBins = m_arr_dGBins[nAt+1];

			arr_dGBins.SetDim(GetBinForValue(maxValue 
				+ GBINS_BUFFER * m_binKernelSigma) + 1);

			Conv_dGauss(arr_dBins, arr_dGBins);
		}
	}

	return arr_dBins;

}	// CHistogram::Get_dBins



//////////////////////////////////////////////////////////////////////
// CHistogram::Get_dGBins
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogram::Get_dGBins(long nAt) const
{
	if (m_binKernelSigma == 0.0)
	{
		return Get_dBins(nAt);
	}

	Get_dBins(nAt);
	return m_arr_dGBins[nAt+1];

}	// CHistogram::Get_dGBins


//////////////////////////////////////////////////////////////////////
// CHistogram::Eval_GBin
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
double CHistogram::Eval_GBin(double x) const
{
	double *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
	double *pRegionVoxel = NULL;
	if (m_pRegion)
	{
		pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
	}

	double sum = 0.0;
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
double CHistogram::Eval_dGBin(long nAt_dVolume, double x) const
{
	double *pVoxels = &m_pVolume->GetVoxels()[0][0][0];
	double *pRegionVoxel = NULL;
	if (m_pRegion)
	{
		pRegionVoxel = &m_pRegion->GetVoxels()[0][0][0];
	}

	double *p_dVoxels = NULL;
	if (nAt_dVolume >= 0)
	{	
		p_dVoxels = &Get_dVolume(nAt_dVolume)->GetVoxels()[0][0][0];
	}

	double sum = 0.0;
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

	if (m_pArr_dVolumes)
	{
		// reset size of all dbins
		for (int nAt = 0; nAt < m_pArr_dVolumes->GetSize()+1; nAt++)
		{
			m_arr_dBins[nAt].SetDim(0);
			m_arr_dGBins[nAt].SetDim(0);
		}
	}

	// fire a change event
	GetChangeEvent().Fire();

}	// CHistogram::OnVolumeChange

