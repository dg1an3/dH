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
			targetBins[nAtBin] *= fraction * totalVolume / sum;
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
REAL CKLDivTerm::Eval(CVectorN<> *pvGrad)
{
	REAL sum = 0.0;

	BEGIN_LOG_SECTION(CKLDivTerm::Eval);

	// get the calculated histogram bins
	CVectorN<>& calcGPDF = GetHistogram()->GetGBins();
	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	CVectorN<>& targetGPDF = const_cast< CVectorN<>& >(GetTargetGBins());
	LOG_EXPR_EXT(targetGPDF);

	const long n_dVolCount = GetHistogram()->Get_dVolumeCount();
	// std::vector< CVectorN<> > arrCalc_dGPDF = &GetHistogram()->Get_dGBins(0);
	
	// if a gradient is needed
	if (pvGrad)
	{
		pvGrad->SetDim(n_dVolCount);
		pvGrad->SetZero();

		for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
		{
			GetHistogram()->Get_dGBins(nAt_dVol);
			// arrCalc_dGPDF.push_back(GetHistogram()->Get_dGBins(nAt_dVol));
			// LOG_EXPR_EXT_DESC(arrCalc_dGPDF.back(), FMT("arrCalc_dGPDF %i", nAt_dVol));
		}
	} 
	CVectorBase<> *arrCalc_dGPDF = &const_cast<CVectorBase<>&>(GetHistogram()->Get_dGBins(0));

	// must normalize both distributions
	REAL calcSum = 0.0; 
	REAL targetSum = 0.0;
	for (int nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
	{
		// form sum of bins values
		calcSum += calcGPDF[nAtBin];
		targetSum += targetGPDF[nAtBin];
	}
	LOG_EXPR(calcSum);
	LOG_EXPR(targetSum);

	// form the sum of the sq. difference for target - calc
	const REAL EPS = (REAL) 1e-8;
	for (nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
	{
		// normalize samples
		calcGPDF[nAtBin] /= calcSum;
		targetGPDF[nAtBin] /= targetSum;

#define CROSS_ENTROPY
#ifdef CROSS_ENTROPY
		sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] + EPS);	
		sum -= calcGPDF[nAtBin] * log(targetGPDF[nAtBin] + EPS);
#else
		// form sum of relative entropy termA
		sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin]
			/ (targetGPDF[nAtBin] + EPS) + EPS);
#endif


#ifdef SWAP
		sum += targetGPDF[nAtBin] * log(targetGPDF[nAtBin]
			/ (calcGPDF[nAtBin] + EPS) + EPS);
#endif
		// if a gradient is needed
		if (pvGrad)
		{
			// iterate over the dVolumes
			for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
			{
				// normalize this bin
				arrCalc_dGPDF[nAt_dVol][nAtBin] /= calcSum;

#ifdef CROSS_ENTROPY
				(*pvGrad)[nAt_dVol] += m_weight 
					* (log(calcGPDF[nAtBin] + EPS) 
							+ calcGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS))
						* arrCalc_dGPDF[nAt_dVol][nAtBin]; 
				(*pvGrad)[nAt_dVol] -= m_weight 
					* log(targetGPDF[nAtBin] + EPS)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]; 
#else
				// add to the proper gradient element
				(*pvGrad)[nAt_dVol] += m_weight
					* (log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS) + 1.0)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]; 			
						// * d_vInput[nAt_dVol];
						// * dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);
#endif

#ifdef SWAP		
				(*pvGrad)[nAt_dVol] += 
					- targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]
						* dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);
#endif
#ifdef _FULL_DERIV
					- (targetGPDF[nAtBin] * targetGPDF[nAtBin]) 
						/ ((calcGPDF[nAtBin] + EPS) * (calcGPDF[nAtBin] + EPS))
					/ (targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS) + EPS)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]
						* dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);
#endif

			}
		}
	}

	sum /= (REAL) calcGPDF.GetDim();
	ASSERT(_finite(sum));

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
