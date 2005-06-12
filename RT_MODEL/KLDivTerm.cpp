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
	: CVOITerm(pStructure, weight)
		, m_bReconvolve(true)
		, m_bTargetCrossEntropy(false)
{
	// initialize flag
	int nTargetCrossEntropy = 
		::AfxGetApp()->GetProfileInt("KLDivTerm", "TargetCrossEntropy", 0);
	m_bTargetCrossEntropy = (nTargetCrossEntropy != 0);

	// store value back to registry
	::AfxGetApp()->WriteProfileInt("KLDivTerm", "TargetCrossEntropy", nTargetCrossEntropy);
		
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
	LOG_EXPR(low);
	LOG_EXPR(high);
	LOG_EXPR(fraction);

	// get the main volume
	CVolume<VOXEL_REAL> *pVolume = GetHistogram()->GetVolume();
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

	REAL totalVolume = R(pVolume->GetVoxelCount());
	if (NULL != GetHistogram()->GetRegion())
	{
		totalVolume = GetHistogram()->GetRegion()->GetSum();
		CVectorD<3> vPixSpacing = GetHistogram()->GetRegion()->GetPixelSpacing();
		totalVolume *= vPixSpacing[0] * vPixSpacing[1]; // * vPixSpacing[2];
	}

	if (totalVolume > 0.0)
	{
		for (nAtBin = 0; nAtBin < targetBins.GetDim(); nAtBin++)
		{
			// re-normalize bins
			targetBins[nAtBin] *= R(1.0 / ((double) totalVolume * (double) GetHistogram()->GetBinWidth()));
				// / sum);
		}
	}

	LOG_EXPR_EXT(targetBins);
	LOG_EXPR_EXT_DESC(GetHistogram()->GetBinMeans(), "Bin Means");

	// triggers recalc of GBins
	m_bReconvolve = true;

	if (m_pNextScale)
	{
		static_cast<CKLDivTerm*>(m_pNextScale)->SetInterval(low, high, fraction);
	}

	END_LOG_SECTION();	// CKLDivTerm::SetInterval

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
	CVolume<VOXEL_REAL> *pVolume = GetHistogram()->GetVolume();
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

	if (m_pNextScale)
	{
		static_cast<CKLDivTerm*>(m_pNextScale)->SetRamp(low, 
			low_frac, high, high_frac);
	}

	END_LOG_SECTION();	// CKLDivTerm::SetRamp

}	// CKLDivTerm::SetRamp

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetBins
// 
// accessor for target bins (after calling SetRamp or SetInterval)
///////////////////////////////////////////////////////////////////////////////
CVectorN<>& CKLDivTerm::GetTargetBins() 
{ 
	return m_vTargetBins; 

}	// CKLDivTerm::GetTargetBins


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetBins
// 
// accessor for target bins (after calling SetRamp or SetInterval)
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CKLDivTerm::GetTargetBins() const 
{ 
	return m_vTargetBins; 

}	// CKLDivTerm::GetTargetBins


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetGBins
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CKLDivTerm::GetTargetGBins() const 
{ 
	if (m_bReconvolve)
	{
		BEGIN_LOG_SECTION(CKLDivTerm::GetTargetGBins!Reconvolve);

		// now convolve GBins
		m_vTargetGBins.SetDim( // GetHistogram()->GetGBins().GetDim())
			GetTargetBins().GetDim() 
				+ (int) (GBINS_BUFFER * GetHistogram()->GetGBinSigma() / GetHistogram()->GetBinWidth()));
		//	(GetBinForValue(maxValue + GBINS_BUFFER * GetHistogram()->GetBinKernelSigma()) + 1);
		GetHistogram()->ConvGauss(GetTargetBins(), m_vTargetGBins);

		REAL sum = 0.0;
		ITERATE_VECTOR(m_vTargetGBins, nAt, sum += m_vTargetGBins[nAt]);
		m_vTargetGBins *= R(1.0) / (sum * GetHistogram()->GetBinWidth());

		m_bReconvolve = false;

		END_LOG_SECTION(); // CKLDivTerm::GetTargetGBins!Reconvolve
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
	LOG_EXPR(m_weight);

#define TEST_REGION_SUM
#ifdef TEST_REGION_SUM
	if (m_pNextScale)
	{
		REAL calcSum = GetHistogram()->GetRegion()->GetSum();
		CVectorD<3> vPixSpacing = GetHistogram()->GetRegion()->GetPixelSpacing();
		calcSum *= vPixSpacing[0] * vPixSpacing[1]; // * vPixSpacing[2];

		REAL calcSumNext = m_pNextScale->GetHistogram()->GetRegion()->GetSum();
		vPixSpacing = m_pNextScale->GetHistogram()->GetRegion()->GetPixelSpacing();
		calcSumNext *= vPixSpacing[0] * vPixSpacing[1]; // * vPixSpacing[2];

		ASSERT(IsApproxEqual(calcSum, calcSumNext, 0.1));
	}
#endif

	// get the calculated histogram bins
	CVectorN<>& calcGPDF = GetHistogram()->GetGBins();

#define TEST_NORM
#ifdef TEST_NORM
	double vec_sum = 0.0;
	ITERATE_VECTOR(calcGPDF, nAt, vec_sum += calcGPDF[nAt]);
	ASSERT(vec_sum < 10.0 * (1.0 / GetHistogram()->GetBinWidth()));
#endif

	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	const CVectorN<>& targetGPDF = GetTargetGBins();
	LOG_EXPR_EXT(targetGPDF);

	// form the sum of the sq. difference for target - calc
	const REAL EPS = (REAL) 1e-5;
	LOG_EXPR(EPS);

	if (!m_bTargetCrossEntropy)
	{
		for (int nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
		{
			if (nAtBin < targetGPDF.GetDim())
			{
				ASSERT(targetGPDF[nAtBin] >= 0.0);
				sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS);	
			}
			else
			{
				sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] / (EPS) + EPS);	
				ASSERT(_finite(sum));
			}
		}
	}
	else // if (m_bTargetCrossEntropy)
	{
		for (int nAtBin = 0; nAtBin < targetGPDF.GetDim(); nAtBin++)
		{
			if (nAtBin < calcGPDF.GetDim())
			{
				sum += targetGPDF[nAtBin] 
					* log(targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS) + EPS);
			}
			else
			{
				sum += targetGPDF[nAtBin] 
					* log(targetGPDF[nAtBin] / (EPS) + EPS);
			}
		}
	}
	ASSERT(_finite(sum));

	// if a gradient is needed
	if (pvGrad)
	{
		BEGIN_LOG_SECTION(CKLDivTerm::Eval!CalcGrad);

		const int n_dVolCount = GetHistogram()->Get_dVolumeCount();
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
			LOG_EXPR_EXT(arrCalc_dGPDF);

			ASSERT(arrCalc_dGPDF.GetDim() >= calcGPDF.GetDim());
			if (!m_bTargetCrossEntropy)
			{
				for (int nAtBin = 0; nAtBin < __max(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim()); nAtBin++)
				{
					if (nAtBin < calcGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						// u * v'
						(*pvGrad)[nAt_dVol] += 
							calcGPDF[nAtBin] 
								* R(1.0) / (calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS) 
									* arrCalc_dGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS);

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS);
					}
					else if (nAtBin < calcGPDF.GetDim())
					{
						// u * v'
						(*pvGrad)[nAt_dVol] += 
							calcGPDF[nAtBin] 
								* R(1.0) / (calcGPDF[nAtBin] / (EPS) + EPS) 
									* arrCalc_dGPDF[nAtBin] / (EPS);

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(calcGPDF[nAtBin] / (EPS) + EPS);

					}
					else if (nAtBin < arrCalc_dGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						// u * v' = 0

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(targetGPDF[nAtBin] + EPS);
					}
					else if (nAtBin < arrCalc_dGPDF.GetDim())
					{
						// u * v' = 0

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(EPS);
					} 
				}
			}
			else // if (m_bTargetCrossEntropy)
			{
				for (int nAtBin = 0; nAtBin < __max(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim()); nAtBin++)
				{
					if (nAtBin < calcGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						(*pvGrad)[nAt_dVol] += 
							targetGPDF[nAtBin] 
							* arrCalc_dGPDF[nAtBin]	// don't forget negate
							* (targetGPDF[nAtBin] / ((calcGPDF[nAtBin] + EPS) * (calcGPDF[nAtBin] + EPS)))
							/ (targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS) + EPS);
					}
					else if (nAtBin < arrCalc_dGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						(*pvGrad)[nAt_dVol] += 
							targetGPDF[nAtBin] 
							* arrCalc_dGPDF[nAtBin]	// don't forget negate
							* (targetGPDF[nAtBin] / ((EPS) * (EPS)))
							/ (targetGPDF[nAtBin] / (EPS) + EPS);
					} 
				}
			}
		}

		// now adjust or integral 
		(*pvGrad) *= GetHistogram()->GetBinWidth();
		LOG_EXPR_EXT((*pvGrad));

		// and weight
		(*pvGrad) *= m_weight;

		END_LOG_SECTION(); // CKLDivTerm::Eval!CalcGrad
	}

	// now adjust or integral 
	sum *= GetHistogram()->GetBinWidth();
	LOG_EXPR(sum);

	// and weight
	sum *= m_weight;

	END_LOG_SECTION();	// CHistogramMatcher::Eval_RelativeEntropy

	return sum;

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
