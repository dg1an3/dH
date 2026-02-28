// KLDivTerm.cpp: implementation of the CKLDivTerm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\include\kldivterm.h"


const REAL EPS = (REAL) 1e-5;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::CKLDivTerm
// 
// constructor
///////////////////////////////////////////////////////////////////////////////
CKLDivTerm::CKLDivTerm(CStructure *pStructure, REAL weight)
	: CVOITerm(pStructure, weight)
		, m_bRecompTarget(true)
		, m_bReconvolve(true)
		, m_bTargetCrossEntropy(false)
{
	// default interval
	SetInterval(0.0, 0.01, 1.0, FALSE);

	// initialize flag
	int nTargetCrossEntropy = 
		::AfxGetApp()->GetProfileInt("KLDivTerm", "TargetCrossEntropy", 0);
	m_bTargetCrossEntropy = (nTargetCrossEntropy != 0);

	// store value back to registry
	::AfxGetApp()->WriteProfileInt("KLDivTerm", "TargetCrossEntropy", nTargetCrossEntropy);
		
	// set up to receive binning change events
	m_histogram.GetBinningChangeEvent().AddObserver(this, 
		(ListenerFunction) &CKLDivTerm::OnHistogramBinningChange);

}	// CKLDivTerm::CKLDivTerm


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::~CKLDivTerm
// 
// destructor
///////////////////////////////////////////////////////////////////////////////
CKLDivTerm::~CKLDivTerm()
{
}	// CKLDivTerm::~CKLDivTerm



///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
#define KLDIVTERM_SCHEMA 2
	// 1 - initial
	// 2 - DVP matrix

IMPLEMENT_SERIAL(CKLDivTerm, CVOITerm, VERSIONABLE_SCHEMA | KLDIVTERM_SCHEMA);

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CKLDivTerm::Serialize(CArchive& ar)
{
	// schema for the voiterm object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : KLDIVTERM_SCHEMA;

	CVOITerm::Serialize(ar);

	SERIALIZE_VALUE(ar, m_vTargetBins);

	if (nSchema >= 2)
	{
		SERIALIZE_VALUE(ar, m_mDVPs);

		if (ar.IsLoading())
		{
			// trigger recomp of target bins
			m_bRecompTarget = true;
			m_bReconvolve = true;
		}
	}

}	// CKLDivTerm::Serialize



//////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetDVPs
// 
// gets dose-volume points for the target histogram;
// mDVPs   =	|x x x x ... |
//				|y y y y     |
//////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& CKLDivTerm::GetDVPs() const
{
	return m_mDVPs;
}

//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetDVPs
// 
// sets dose-volume points for the target histogram;
// mDVPs   =	|x x x x ... |
//				|y y y y     |
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetDVPs(const CMatrixNxM<>& mDVPs)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetDVPs);
	LOG_EXPR_EXT(mDVPs);

	// store the points
	// NOTE: don't use true parameter, because of assign-to-self from Serialize
	m_mDVPs.Reshape(mDVPs.GetCols(), mDVPs.GetRows());
	m_mDVPs = mDVPs;

	// check that cumulative DVPs start at 100%
	ASSERT(IsApproxEqual(m_mDVPs[0][1], R(1.0)));
	ASSERT(IsApproxEqual(m_mDVPs[mDVPs.GetCols()-1][1], R(0.0)));

	// trigger recomp of target bins
	m_bRecompTarget = true;

	// triggers recalc of GBins
	m_bReconvolve = true;

	if (m_pNextScale)
	{
		static_cast<CKLDivTerm*>(m_pNextScale)->SetDVPs(mDVPs);
	}

	// notify of change
	GetChangeEvent().Fire();

	END_LOG_SECTION();	// CKLDivTerm::SetInterval

}	// CKLDivTerm::SetInterval

//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetInterval
// 
// sets the target histgram to an interval
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetInterval(REAL low, REAL high, REAL fraction, BOOL bMid)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetInterval);
	LOG_EXPR(low);
	LOG_EXPR(high);
	LOG_EXPR(fraction);

	CMatrixNxM<> mDVPs;
	mDVPs.Reshape(bMid ? 3 : 2, 2);
	mDVPs[0][0] = low;
	mDVPs[0][1] = 1.0;
	if (bMid)
	{
		mDVPs[1][0] = (low + high) / 2.0;
		mDVPs[1][1] = 0.5;
	}
	mDVPs[bMid ? 2 : 1][0] = high;
	mDVPs[bMid ? 2 : 1][1] = 0.0;

	// set up the DVPs
	SetDVPs(mDVPs);

	END_LOG_SECTION();	// CKLDivTerm::SetInterval

}	// CKLDivTerm::SetInterval


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetMinDose
// 
// returns minimum target dose to the structure
///////////////////////////////////////////////////////////////////////////////
REAL CKLDivTerm::GetMinDose(void) const
{
	REAL doseValue = m_mDVPs[0][0];
	return doseValue; 

}	// CKLDivTerm::GetMinDose

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetMaxDose
// 
// returns maximum target dose to the structure
///////////////////////////////////////////////////////////////////////////////
REAL CKLDivTerm::GetMaxDose(void) const
{
	REAL doseValue = m_mDVPs[m_mDVPs.GetCols()-1][0];
	return doseValue;

}	// CKLDivTerm::GetMaxDose

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetBins
// 
// accessor for target bins (after calling SetDVPs)
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CKLDivTerm::GetTargetBins() const
{ 
	if (m_bRecompTarget)
	{
		// get the main volume
		REAL high = m_mDVPs[m_mDVPs.GetCols()-1][0];
		m_vTargetBins.SetDim(GetHistogram()->GetBinForValue(high) + 1);
		m_vTargetBins.SetZero();

		// now for each interval
		for (int nAtInterval = 0; nAtInterval < m_mDVPs.GetCols()-1; nAtInterval++)
		{
			REAL interMin = m_mDVPs[nAtInterval][0];
			REAL interMax = m_mDVPs[nAtInterval+1][0];

			// compute slope (= per-bin value within this interval)
			REAL interSlope = (m_mDVPs[nAtInterval][1] - m_mDVPs[nAtInterval+1][1])
									/ (interMax - interMin); 
			interSlope /= (REAL) GetHistogram()->GetBinWidth();

			// now set bin values in the interval
			for (int nAtBin = GetHistogram()->GetBinForValue(interMin); 
				nAtBin <= GetHistogram()->GetBinForValue(interMax); nAtBin++)
			{
				m_vTargetBins[nAtBin] = interSlope;
			}
		}

		// ok done
		m_bRecompTarget = false;

		LOG_EXPR_EXT(m_vTargetBins);
		LOG_EXPR_EXT_DESC(GetHistogram()->GetBinMeans(), "Bin Means");
	}

	return m_vTargetBins; 

}	// CKLDivTerm::GetTargetBins

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::GetTargetGBins
// 
// returns convolved target bins
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CKLDivTerm::GetTargetGBins() const 
{ 
	if (m_bReconvolve)
	{
		BEGIN_LOG_SECTION(CKLDivTerm::GetTargetGBins!Reconvolve);

		// now convolve GBins
		m_vTargetGBins.SetDim(GetTargetBins().GetDim() 
				+ (int) (GBINS_BUFFER * GetHistogram()->GetGBinSigma() / GetHistogram()->GetBinWidth()));

		// perform convolve
		GetHistogram()->ConvGauss(GetTargetBins(), m_vTargetGBins);

		// normalize
		REAL sum = 0.0;
		ITERATE_VECTOR(m_vTargetGBins, nAt, sum += m_vTargetGBins[nAt]);
		m_vTargetGBins *= R(1.0) / sum;

		m_bReconvolve = false;

		END_LOG_SECTION(); // CKLDivTerm::GetTargetGBins!Reconvolve
	}

	return m_vTargetGBins; 

}	// CKLDivTerm::GetTargetGBins


///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::Eval
// 
// evaluates the term (and optionally gradient)
///////////////////////////////////////////////////////////////////////////////
REAL CKLDivTerm::Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude)
{
	REAL sum = 0.0;

	BEGIN_LOG_SECTION(CKLDivTerm::Eval);
	LOG_EXPR(m_weight);

	// get the calculated histogram bins
	const CVectorN<>& calcGPDF = GetHistogram()->GetGBins();

	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	const CVectorN<>& targetGPDF = GetTargetGBins();
	LOG_EXPR_EXT(targetGPDF);

	// form the sum of the sq. difference for target - calc
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
#define IPP_CALC_KLDIV
#ifdef IPP_CALC_KLDIV
		if (m_vCalc_EPS.GetDim() < calcGPDF.GetDim())
		{
			m_vCalc_EPS.SetDim(calcGPDF.GetDim());
		}
		SumValues<REAL>(&m_vCalc_EPS[0], &calcGPDF[0], EPS, calcGPDF.GetDim());

		if (m_vTarget_div_Calc.GetDim() < targetGPDF.GetDim())
		{
			m_vTarget_div_Calc.SetDim(targetGPDF.GetDim());
		}

		// CAREFUL about the order of operands for DivValues
		DivValues<REAL>(&m_vTarget_div_Calc[0], &m_vCalc_EPS[0], 
			&targetGPDF[0], __min(calcGPDF.GetDim(), targetGPDF.GetDim()));
		if (calcGPDF.GetDim() < targetGPDF.GetDim())
		{
			DivValues<REAL>(&m_vTarget_div_Calc[calcGPDF.GetDim()], 
				&targetGPDF[calcGPDF.GetDim()], EPS, targetGPDF.GetDim() - calcGPDF.GetDim());
		}

		if (m_vTarget_div_Calc_EPS.GetDim() < targetGPDF.GetDim())
		{
			m_vTarget_div_Calc_EPS.SetDim(targetGPDF.GetDim());
		}
		SumValues<REAL>(&m_vTarget_div_Calc_EPS[0], &m_vTarget_div_Calc[0], EPS, targetGPDF.GetDim());

		if (m_vLogTarget_div_Calc.GetDim() < targetGPDF.GetDim())
		{
			m_vLogTarget_div_Calc.SetDim(targetGPDF.GetDim());
		}
		::ippsLn_64f(&m_vTarget_div_Calc_EPS[0], &m_vLogTarget_div_Calc[0], targetGPDF.GetDim());
		MultValues<REAL>(&m_vLogTarget_div_Calc[0], &targetGPDF[0], targetGPDF.GetDim());

		// now sum all values
		::ippsSum_64f(&m_vLogTarget_div_Calc[0], targetGPDF.GetDim(), &sum); 
#else
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
#endif
	}
	ASSERT(_finite(sum));

	// if a gradient is needed
	if (pvGrad)
	{
		BEGIN_LOG_SECTION(CKLDivTerm::Eval!CalcGrad);

#ifdef IPP_CALC_KLDIV

		if (m_v_dx_Target_div_Calc.GetDim() < targetGPDF.GetDim())
		{
			m_v_dx_Target_div_Calc.SetDim(targetGPDF.GetDim());
		}

		// divide by derivative of log term (target / calc)
		DivValues<REAL>(&m_v_dx_Target_div_Calc[0], 
			&m_vTarget_div_Calc_EPS[0], &m_vTarget_div_Calc[0], targetGPDF.GetDim());

		// multiply to form target / (calc * calc) (chain rule) term
		DivValues<REAL>(&m_v_dx_Target_div_Calc[0], 
			&m_vCalc_EPS[0], &m_v_dx_Target_div_Calc[0], __min(calcGPDF.GetDim(), targetGPDF.GetDim()));
		if (calcGPDF.GetDim() < targetGPDF.GetDim())
		{
			DivValues<REAL>(&m_v_dx_Target_div_Calc[calcGPDF.GetDim()], 
				&m_v_dx_Target_div_Calc[calcGPDF.GetDim()], EPS, targetGPDF.GetDim() - calcGPDF.GetDim());
		}

		// multiply (scale) by target GPDF
		MultValues<REAL>(&m_v_dx_Target_div_Calc[0], 
			&targetGPDF[0], &m_v_dx_Target_div_Calc[0], targetGPDF.GetDim());

#endif
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

#ifdef IPP_CALC_KLDIV
			
			int nFinalSize = __min(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim());
			if (m_v_dVol_Target.GetDim() < nFinalSize)
			{
				m_v_dVol_Target.SetDim(nFinalSize);
			}
			MultValues<REAL>(&m_v_dVol_Target[0], 
				&arrCalc_dGPDF[0], &m_v_dx_Target_div_Calc[0], nFinalSize);
			::ippsSum_64f(&m_v_dVol_Target[0], nFinalSize, &(*pvGrad)[nAt_dVol]);

#else
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
								* R(1.0) / (calcGPDF[nAtBin] / targetGPDF_EPS[nAtBin] + EPS) 
									* arrCalc_dGPDF[nAtBin] / targetGPDF_EPS[nAtBin];

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(calcGPDF[nAtBin] / targetGPDF_EPS[nAtBin] + EPS);
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
								* log(targetGPDF_EPS[nAtBin]);
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
#endif
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
// CKLDivTerm::Subcopy
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVOITerm *CKLDivTerm::Subcopy(CVOITerm *) 
{
	CKLDivTerm *pSubcopy = 
		static_cast<CKLDivTerm*>(CVOITerm::Subcopy(new CKLDivTerm(GetVOI(), m_weight)));

	// copy the DVPs
	pSubcopy->SetDVPs(GetDVPs());

	return pSubcopy;

}	// CKLDivTerm::Subcopy

///////////////////////////////////////////////////////////////////////////////
// CKLDivTerm::OnHistogramChange
// 
// flags recalc when histogram changes
///////////////////////////////////////////////////////////////////////////////
void CKLDivTerm::OnHistogramBinningChange(CObservableEvent *pSource, void *pValue)
{
	// recompute target
	m_bRecompTarget = true;

	// and reconvolve as well
	m_bReconvolve = true;

}	// CKLDivTerm::OnHistogramChange

