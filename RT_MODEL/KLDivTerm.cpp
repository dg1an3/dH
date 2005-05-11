// KLDivTerm.cpp: implementation of the CKLDivTerm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <KLDivTerm.h>

#include <Structure.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::CKLDivTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CKLDivTerm::CKLDivTerm(CStructure *pStructure, REAL weight)
	: CVOITerm(pStructure, weight),
		m_bReconvolve(true)
{
	m_histogram.GetChangeEvent().AddObserver(this, 
		(ListenerFunction) OnHistogramChange);

}	// CKLDivTerm::CKLDivTerm


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::~CKLDivTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CKLDivTerm::~CKLDivTerm()
{
}	// CKLDivTerm::~CKLDivTerm


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::Subcopy
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVOITerm *CKLDivTerm::Subcopy(CVOITerm *) 
{
	CKLDivTerm *pSubcopy = 
		static_cast<CKLDivTerm*>(CVOITerm::Subcopy(new CKLDivTerm(GetVOI(), m_weight)));

	pSubcopy->m_vTargetBins = m_vTargetBins;
	pSubcopy->m_vTargetGBins = m_vTargetGBins;
	pSubcopy->m_bReconvolve = true;

	return pSubcopy;

}	// CKLDivTerm::Subcopy

//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetInterval
// 
// sets the target histgram to an interval
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetInterval(REAL low, REAL high, REAL fraction)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetInterval);
	LOG("Setting interval for histogram");
	LOG_EXPR(low);
	LOG_EXPR(high);
	LOG_EXPR(fraction);

	// get the main volume
	CVolume<REAL> *pVolume = GetHistogram()->GetVolume();
	CVectorN<>& targetBins = GetTargetBins();
	targetBins.SetDim(GetHistogram()->GetBinForValue(high)+1);
	targetBins.SetZero();

	REAL sum = 0.0;
	for (int nAtBin = GetHistogram()->GetBinForValue(low); 
		nAtBin <= GetHistogram()->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = 1.0;
		sum += 1.0;
	}

	REAL totalVolume = pVolume->GetVoxelCount();
	if (NULL != GetHistogram()->GetRegion())
	{
		totalVolume = GetHistogram()->GetRegion()->GetSum();
	}

	if (sum > 0.0)
	{
		for (nAtBin = 0; nAtBin < targetBins.GetDim(); nAtBin++)
		{
			// re-normalize bins
			targetBins[nAtBin] *= 1.0 /* fraction * totalVolume */ / sum;
		}
	}

	LOG_EXPR_EXT(targetBins);
	LOG_EXPR_EXT_DESC(GetHistogram()->GetBinMeans(), "Bin Means");

	// triggers recalc of GBins
	m_bReconvolve = true;

	END_LOG_SECTION();	// CKLDivTerm::SetInterval

	if (m_pNextScale)
	{
		static_cast<CKLDivTerm*>(m_pNextScale)->SetInterval(low, high, fraction);
	}

}	// CKLDivTerm::SetInterval


//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetRamp
// 
// sets the target histgram to a ramp
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetRamp(REAL low, REAL low_frac,
							REAL high, REAL high_frac)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetRamp);
	LOG("Setting ramp for histogram");
	LOG_EXPR(low);
	LOG_EXPR(low_frac);
	LOG_EXPR(high);
	LOG_EXPR(high_frac);

	// get the main volume
	CVolume<REAL> *pVolume = GetHistogram()->GetVolume();
	CVectorN<>& targetBins = GetTargetBins();
	targetBins.SetZero();

	REAL sum = 0.0;
	REAL step = (high_frac - low_frac) 
		/ (GetHistogram()->GetBinForValue(high)
			- GetHistogram()->GetBinForValue(low));
	REAL bin_value = low_frac;
	for (int nAtBin = GetHistogram()->GetBinForValue(low); 
		nAtBin <= GetHistogram()->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = bin_value;
		bin_value += step;
	}

	LOG_EXPR_EXT(targetBins);
	LOG_EXPR_EXT_DESC(GetHistogram()->GetBinMeans(), "Bin Means");

	// triggers recalc of GBins
	m_bReconvolve = true;

	END_LOG_SECTION();	// CKLDivTerm::SetRamp

	if (m_pNextScale)
	{
		static_cast<CKLDivTerm*>(m_pNextScale)->SetRamp(low, 
			low_frac, high, high_frac);
	}

}	// CKLDivTerm::SetRamp


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetGBins
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CKLDivTerm::GetTargetGBins() const 
{ 
	if (m_bReconvolve)
	{
		// now convolve GBins
		m_vTargetGBins.SetDim(GetHistogram()->GetGBins().GetDim());
		GetHistogram()->ConvGauss(GetTargetBins(), m_vTargetGBins);
		
		m_bReconvolve = false;
	}

	return m_vTargetGBins; 

}	// CKLDivTerm::GetTargetGBins


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::Eval
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
REAL CKLDivTerm::Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude)
{
	REAL sum = 0.0;

	BEGIN_LOG_SECTION(CKLDivTerm::Eval);

	// get the calculated histogram bins
	CVectorN<>& calcGPDF = GetHistogram()->GetGBins();
	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	const CVectorN<>& targetGPDF = GetTargetGBins();
	LOG_EXPR_EXT(targetGPDF);

	const long n_dVolCount = GetHistogram()->Get_dVolumeCount();

	// form the sum of the sq. difference for target - calc
	const REAL EPS = (REAL) 1e-8;
	ASSERT(calcGPDF.GetDim() <= targetGPDF.GetDim());
	for (int nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
	{

#define CROSS_ENTROPY
#ifdef CROSS_ENTROPY
		ASSERT(calcGPDF[nAtBin] >= 0.0);
		sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] + EPS);	

		ASSERT(targetGPDF[nAtBin] >= 0.0);
		sum -= calcGPDF[nAtBin] * log(targetGPDF[nAtBin] + EPS);
		ASSERT(_finite(sum));
#endif

	}

	// if a gradient is needed
	if (pvGrad)
	{
		pvGrad->SetDim(n_dVolCount);
		pvGrad->SetZero();

		// iterate over the dVolumes
		for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
		{
			if (!arrInclude[nAt_dVol])
			{
				continue;
			}

			// get the dGPDF distribution
			const CVectorN<>& arrCalc_dGPDF = GetHistogram()->Get_dGBins(nAt_dVol);

			for (nAtBin = 0; nAtBin < __max(calcGPDF.GetDim(), targetGPDF.GetDim()); nAtBin++)
			{
				if (nAtBin < calcGPDF.GetDim())
				{
#ifdef CROSS_ENTROPY
					(*pvGrad)[nAt_dVol] -= /* DGL 05072005 += */ m_weight 
						* (log(calcGPDF[nAtBin] + EPS) 
								+ calcGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS))
							* arrCalc_dGPDF[nAtBin]; 

					if (nAtBin < targetGPDF.GetDim())
					{
						ASSERT(nAtBin < arrCalc_dGPDF.GetDim());
						(*pvGrad)[nAt_dVol] += /* DGL 05072005 -= */ m_weight 
							* log(targetGPDF[nAtBin] + EPS)
								* arrCalc_dGPDF[nAtBin]; 
					}
					else
					{
						(*pvGrad)[nAt_dVol]  += /* DGL 05072005 -= */ m_weight 
							* log(EPS) * arrCalc_dGPDF[nAtBin];
					}
#endif
				}
			}

			(*pvGrad)[nAt_dVol] /= (REAL) calcGPDF.GetDim();
		}
	}

	sum /= (REAL) calcGPDF.GetDim();

	LOG_EXPR(sum);
	END_LOG_SECTION();	// CHistogramMatcher::Eval_RelativeEntropy

	return m_weight * sum;

}	// CKLDivTerm::Eval


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::OnHistogramChange
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CKLDivTerm::OnHistogramChange(CObservableEvent *pSource, void *)
{
	m_bReconvolve = true;

}	// CKLDivTerm::OnHistogramChange
