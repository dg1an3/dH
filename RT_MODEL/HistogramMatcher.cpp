// HistogramMatcher.cpp: implementation of the CHistogramMatcher class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <VectorD.h>

#include <PlanIMRT.h>

#include <HistogramMatcher.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::CHistogramMatcher
// 
// constructs a new CHistogramMatcher
//////////////////////////////////////////////////////////////////////
CHistogramMatcher::CHistogramMatcher(CPlanIMRT *pPlan, int nScale)
	: CObjectiveFunction(FALSE),
		m_bUseRelativeEntropy(TRUE),
		m_bUseTotalEntropy(TRUE),
		m_totalEntropyWeight(0.5),
		m_inputScale(0.5),
		m_pPlan(pPlan),
		m_nScale(nScale)
{
}	// CHistogramMatcher::CHistogramMatcher


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::~CHistogramMatcher
// 
// destroys the histogram matcher
//////////////////////////////////////////////////////////////////////
CHistogramMatcher::~CHistogramMatcher()
{
/*	for (int nAt = 0; nAt < m_arrHisto.GetSize(); nAt++)
	{
		delete m_arrTargetBins[nAt];
		delete m_arrTargetGBins[nAt];
	} */

}	// CHistogramMatcher::~CHistogramMatcher


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetHistogramCount
// 
// returns the number of histograms being matched
//////////////////////////////////////////////////////////////////////
long CHistogramMatcher::GetHistogramCount() const
{
	return m_arrHisto.GetSize();

}	// CHistogramMatcher::GetHistogramCount


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetHistogram
// 
// returns a particular histogram
//////////////////////////////////////////////////////////////////////
CHistogram *CHistogramMatcher::GetHistogram(long nAt) const
{
	return (CHistogram *) m_arrHisto[nAt];

}	// CHistogramMatcher::GetHistogram


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetHistogramWeight
// 
// computes and returns the d/dx GHistogram
//////////////////////////////////////////////////////////////////////
double CHistogramMatcher::GetHistogramWeight(long nAt) const
{
	return m_arrHistoWeights[(int) nAt];

}	// CHistogramMatcher::GetHistogramWeight



//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::AddHistogram
// 
// adds another histogram
//////////////////////////////////////////////////////////////////////
long CHistogramMatcher::AddHistogram(CHistogram *pHisto, double weight)
{
	// add to the collection of histograms
	long nIndex = m_arrHisto.Add(pHisto);

	// add the weight to the collection
	m_arrHistoWeights.SetDim(m_arrHisto.GetSize());
	m_arrHistoWeights[m_arrHistoWeights.GetDim()-1] = weight;

	// add a collection of target bins
	m_arrTargetBins.push_back(pHisto->GetGBins());

	// add a new (empty) GBin
	m_arrTargetGBins.push_back(CVectorN<>());

	// return the index of the new histogram
	return nIndex;

}	// CHistogramMatcher::AddHistogram



//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetTargetBins
// 
// returns the target bins for the histogram
//////////////////////////////////////////////////////////////////////
CVectorN<>& CHistogramMatcher::GetTargetBins(long nAt)
{
	return m_arrTargetBins[nAt]; 

}	// CHistogramMatcher::GetTargetBins


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetTargetBins
// 
// returns the target bins for the histogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogramMatcher::GetTargetBins(long nAt) const
{
	return m_arrTargetBins[nAt]; 

}	// CHistogramMatcher::GetTargetBins


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::GetTargetGBins
// 
// returns the target GBins for the histogram
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CHistogramMatcher::GetTargetGBins(long nAt) const
{
	CVectorN<>& targetGBins = m_arrTargetGBins[nAt];

	if (TRUE)
	{
		// get source
		targetGBins.SetDim(GetHistogram(nAt)->GetGBins().GetDim());
		GetHistogram(nAt)->ConvGauss(GetTargetBins(nAt), targetGBins);
	}

	return targetGBins;

}	// CHistogramMatcher::GetTargetGBins


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::SetInterval
// 
// sets the target histgram to an interval
//////////////////////////////////////////////////////////////////////
void CHistogramMatcher::SetInterval(long nAt, 
									double low, double high, 
									double fraction)
{
	BEGIN_LOG_SECTION(CHistogramMatcher::SetInterval);
	LOG("Setting interval for histogram %i", nAt);
	LOG_EXPR(low);
	LOG_EXPR(high);
	LOG_EXPR(fraction);

	// get the main volume
	CVolume<double> *pVolume = GetHistogram(nAt)->GetVolume();
	CVectorN<>& targetBins = GetTargetBins(nAt);
	targetBins.SetDim(GetHistogram(nAt)->GetBinForValue(high)+1);
	targetBins.SetZero();

	double sum = 0.0;
	for (int nAtBin = GetHistogram(nAt)->GetBinForValue(low); 
		nAtBin <= GetHistogram(nAt)->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = 1.0;
		sum += 1.0;
	}

	double totalVolume = pVolume->GetVoxelCount();
	if (NULL != GetHistogram(nAt)->GetRegion())
	{
		totalVolume = GetHistogram(nAt)->GetRegion()->GetSum();
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
	LOG_EXPR_EXT_DESC(GetHistogram(nAt)->GetBinMeans(), "Bin Means");

	END_LOG_SECTION();	// CHistogramMatcher::SetInterval

}	// CHistogramMatcher::SetInterval


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::SetRamp
// 
// sets the target histgram to a ramp
//////////////////////////////////////////////////////////////////////
void CHistogramMatcher::SetRamp(long nAt, 
									double low, double low_frac,
									double high, double high_frac)
{
	BEGIN_LOG_SECTION(CHistogramMatcher::SetRamp);
	LOG("Setting ramp for histogram %i", nAt);
	LOG_EXPR(low);
	LOG_EXPR(low_frac);
	LOG_EXPR(high);
	LOG_EXPR(high_frac);

	// get the main volume
	CVolume<double> *pVolume = GetHistogram(nAt)->GetVolume();
	CVectorN<>& targetBins = GetTargetBins(nAt);
	targetBins.SetZero();

	double sum = 0.0;
	double step = (high_frac - low_frac) 
		/ (GetHistogram(nAt)->GetBinForValue(high)
			- GetHistogram(nAt)->GetBinForValue(low));
	double bin_value = low_frac;
	for (int nAtBin = GetHistogram(nAt)->GetBinForValue(low); 
		nAtBin <= GetHistogram(nAt)->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = bin_value;
		bin_value += step;
	}

	LOG_EXPR_EXT(targetBins);
	LOG_EXPR_EXT_DESC(GetHistogram(nAt)->GetBinMeans(), "Bin Means");

	END_LOG_SECTION();	// CHistogramMatcher::SetRamp

}	// CHistogramMatcher::SetRamp


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::Eval_DiffSq
// 
// objective function = sum of the squared difference between 
//		corresponding histogram bins
//////////////////////////////////////////////////////////////////////
double CHistogramMatcher::Eval_DiffSq(long nAt, const CVectorN<>& vInput, CVectorN<> *pGrad) const
{
	double sum = 0.0;

	BEGIN_LOG_SECTION(CHistogramMatcher::Eval_DiffSq);

	// get the calculated histogram bins
	const CVectorN<>& calcGBins = GetHistogram(nAt)->GetGBins();

	// get the target bins
	const CVectorN<>& targetGBins = GetTargetGBins(nAt);

	if (pGrad)
	{
		pGrad->SetDim(GetHistogram(nAt)->Get_dVolumeCount());
		pGrad->SetZero();
	}

	// form the sum of the sq. difference for target - calc
	for (int nAtBin = 0; nAtBin < calcGBins.GetDim(); nAtBin++)
	{
		ASSERT(targetGBins.GetDim() == calcGBins.GetDim());
		sum += (calcGBins[nAtBin] - targetGBins[nAtBin]) 
			* (calcGBins[nAtBin] - targetGBins[nAtBin]);

		if (pGrad)
		{
			for (int nAt_dVol = 0; nAt_dVol < GetHistogram(nAt)->Get_dVolumeCount();
					nAt_dVol++)
			{
				(*pGrad)[nAt_dVol] += GetHistogramWeight(nAt)
					* 2.0 * (calcGBins[nAtBin] - targetGBins[nAtBin])
						* GetHistogram(nAt)->Get_dGBins(nAt_dVol)[nAtBin]
						* dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);;
			}
		}
	}

	LOG_EXPR(sum);

	END_LOG_SECTION();	// CHistogramMatcher::Eval_DiffSq

	return GetHistogramWeight(nAt) * sum;	

}	// CHistogramMatcher::Eval_DiffSq


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::Eval_RelativeEntropy
// 
// evaluates the relative entropy between the two distributions
//////////////////////////////////////////////////////////////////////
double CHistogramMatcher::Eval_RelativeEntropy(long nAt, const CVectorN<>& vInput, 
											   CVectorN<> *pGrad) const
{
	double sum = 0.0;

	BEGIN_LOG_SECTION(CHistogramMatcher::Eval_RelativeEntropy);

	// get the calculated histogram bins
	CVectorN<> calcGPDF = GetHistogram(nAt)->GetGBins();
	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	CVectorN<> targetGPDF = GetTargetGBins(nAt);
	LOG_EXPR_EXT(targetGPDF);

	const long n_dVolCount = GetHistogram(nAt)->Get_dVolumeCount();
	std::vector< CVectorN<> > arrCalc_dGPDF;

	// if a gradient is needed
	if (pGrad)
	{
		pGrad->SetDim(n_dVolCount);
		pGrad->SetZero();

		for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
		{
			arrCalc_dGPDF.push_back(GetHistogram(nAt)->Get_dGBins(nAt_dVol));
			LOG_EXPR_EXT_DESC(arrCalc_dGPDF.back(), FMT("arrCalc_dGPDF %i", nAt_dVol));
		}
	} 

	// must normalize both distributions
	double calcSum = 0.0; 
	double targetSum = 0.0;
	for (int nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
	{
		// form sum of bins values
		calcSum += calcGPDF[nAtBin];
		targetSum += targetGPDF[nAtBin];
	}
	LOG_EXPR(calcSum);
	LOG_EXPR(targetSum);

	// form the sum of the sq. difference for target - calc
	const double EPS = 1e-12;
	for (nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
	{
		// normalize samples
		calcGPDF[nAtBin] /= calcSum;
		targetGPDF[nAtBin] /= targetSum;

		// form sum of relative entropy termA
		sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin]
			/ (targetGPDF[nAtBin] + EPS) + EPS);

#ifdef SWAP
		sum += targetGPDF[nAtBin] * log(targetGPDF[nAtBin]
			/ (calcGPDF[nAtBin] + EPS) + EPS);
#endif
		// if a gradient is needed
		if (pGrad)
		{
			// iterate over the dVolumes
			for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
			{
				// normalize this bin
				arrCalc_dGPDF[nAt_dVol][nAtBin] /= calcSum;

				// add to the proper gradient element
				(*pGrad)[nAt_dVol] += GetHistogramWeight(nAt) 
					* (log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS) + 1.0)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]
						* dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);
#ifdef SWAP		
				(*pGrad)[nAt_dVol] += 
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

	LOG_EXPR(sum);
	END_LOG_SECTION();	// CHistogramMatcher::Eval_RelativeEntropy

	return GetHistogramWeight(nAt) * sum;

}	// CHistogramMatcher::Eval_RelativeEntropy


///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::Eval_TotalEntropy
// 
// Evaluates the total entropy and gradient for the input vector
///////////////////////////////////////////////////////////////////////////////
REAL CHistogramMatcher::Eval_TotalEntropy(const CVectorN<>& vInput, 
											   CVectorN<> *pGrad) const
{
	static CVectorN<> vBeamWeights;
	vBeamWeights.SetDim(m_pPlan->GetBeamCount());
	vBeamWeights.SetZero();

	double sum = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		int nBeam;
		int nBeamlet;
		m_pPlan->GetBeamletFromSVElem(nAt, m_nScale, &nBeam, &nBeamlet);

		vBeamWeights[nBeam] += Sigmoid(vInput[nAt]);
		sum += Sigmoid(vInput[nAt]);
	}

	// to accumulate total entropy
	double totalEntropy = 0.0;

	// for each beamlet
	for (int nAtBeam = 0; nAtBeam < vBeamWeights.GetDim(); nAtBeam++)
	{
		// p is probability of intensity of this beamlet
		double p = vBeamWeights[nAtBeam] / sum;

		// contribution to total is entropy of the probability
		totalEntropy += - p * log(p);
	}

	// do we evaluate gradient?
	if (pGrad)
	{
		for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
		{
			int nBeam;
			int nBeamlet;
			m_pPlan->GetBeamletFromSVElem(nAt, m_nScale, &nBeam, &nBeamlet);

			// p is probability of intensity of this beamlet
			double p = vBeamWeights[nBeam] / sum;

			// derivative of prob is found by applying ratio rule
			double d_p = dSigmoid(vInput[nAt]) 
				* (sum - vBeamWeights[nBeam]) 
					/ (sum * sum);

			// apply chain rule to compute the derivative
			(*pGrad)[nAt] = -d_p * (1 + log(p));
		}
	}

	// return total entropy scaled by number of beamlets
	return totalEntropy;	

}	// CHistogramMatcher::Eval_TotalEntropy


//////////////////////////////////////////////////////////////////////
// CHistogramMatcher::operator()
// 
// objective function = sum of the squared difference between 
//		corresponding histogram bins
//////////////////////////////////////////////////////////////////////
REAL CHistogramMatcher::operator()(const CVectorN<>& vInput, 
	CVectorN<> *pGrad) const
{
	double totalSum = 0.0;

	BEGIN_LOG_SECTION(CHistogramMatcher::operator);

	// get the main volume
	CVolume<double> *pVolume = GetHistogram(0)->GetVolume();

	// clear main
	pVolume->ClearVoxels();

	// iterate over the component volumes, accumulating the weighted volumes
	ASSERT(vInput.GetDim() == GetHistogram(0)->Get_dVolumeCount());
	for (int nAt_dVolume = 0; nAt_dVolume < GetHistogram(0)->Get_dVolumeCount();
		nAt_dVolume++)
	{
		CVolume<double> *p_dVolume = GetHistogram(0)->Get_dVolume(nAt_dVolume);
		ASSERT(p_dVolume->GetWidth() == pVolume->GetWidth());

		pVolume->Accumulate(p_dVolume, Sigmoid(vInput[nAt_dVolume]));
	}

	if (pGrad)
	{
		pGrad->SetDim(vInput.GetDim());
		pGrad->SetZero();
	}

	// now compute match of target bins to histo bins
	for (int nAtHisto = 0; nAtHisto < GetHistogramCount(); nAtHisto++)
	{
		if (pGrad)
		{
			static CVectorN<> vPartGrad;
			vPartGrad.SetDim(vInput.GetDim());
			vPartGrad.SetZero();

			totalSum += m_bUseRelativeEntropy 
				? Eval_RelativeEntropy(nAtHisto, vInput, &vPartGrad)
				: Eval_DiffSq(nAtHisto, vInput, &vPartGrad);

			(*pGrad) += vPartGrad;
		}
		else
		{
			totalSum += m_bUseRelativeEntropy 
				? Eval_RelativeEntropy(nAtHisto, vInput)
				: Eval_DiffSq(nAtHisto, vInput);
		}
	}

	if (m_bUseTotalEntropy)
	{
		if (pGrad)
		{
			static CVectorN<> vPartGrad;
			vPartGrad.SetDim(vInput.GetDim());
			vPartGrad.SetZero();

			totalSum -=  m_totalEntropyWeight * Eval_TotalEntropy(vInput, &vPartGrad);

			(*pGrad) -= vPartGrad;
		}
		else
		{
			totalSum -=  m_totalEntropyWeight * Eval_TotalEntropy(vInput);
		}
	}

	END_LOG_SECTION();	// CHistogramMatcher::operator

	return totalSum;

}	// CHistogramMatcher::operator()


///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::StateVectorToParam
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogramMatcher::StateVectorToParam(const CVectorN<>& vState, CVectorN<>& vParam)
{
	vParam.SetDim(vState.GetDim());
	for (int nAt = 0; nAt < vState.GetDim(); nAt++)
	{
		vParam[nAt] = InvSigmoid(vState[nAt]);
	}

}	// CHistogramMatcher::StateVectorToParam

///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::ParamToStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CHistogramMatcher::ParamToStateVector(const CVectorN<>& vParam, CVectorN<>& vState)
{
	vState.SetDim(vParam.GetDim());
	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
	{
		vState[nAt] = Sigmoid(vParam[nAt]);
	}

}	// CHistogramMatcher::ParamToStateVector


///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::Sigmoid
// 
// TODO: move to MathUtil
///////////////////////////////////////////////////////////////////////////////
REAL CHistogramMatcher::Sigmoid(REAL x) const
{
	REAL res = 1.0 / (1.0 + exp(-m_inputScale * x));
		// exp(m_inputScale * x) / (1.0 + exp(m_inputScale * x));

	if (!_finite(res))
	{
		if (x > 0.0)
			res = 1.0;
		else
			res = 0.0;
	}

	return res;

}	// CHistogramMatcher::Sigmoid


///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::dSigmoid
// 
// TODO: move to MathUtil
///////////////////////////////////////////////////////////////////////////////
REAL CHistogramMatcher::dSigmoid(REAL x) const
{
	REAL u = exp(m_inputScale * x);
	REAL du = m_inputScale * u;

	REAL v = 1.0 + exp(m_inputScale * x);
	REAL dv = 0.0 + du;

	if (_finite(u) && _finite(v))
	{
		return (du * v - u * dv) / (v * v);
	}
	else
	{
		return 0.0;
	}

}	// CHistogramMatcher::dSigmoid


///////////////////////////////////////////////////////////////////////////////
// CHistogramMatcher::InvSigmoid
// 
// TODO: move to MathUtil
///////////////////////////////////////////////////////////////////////////////
REAL CHistogramMatcher::InvSigmoid(REAL y) const
{
	if (y >= 1.0) y = 1.0 - 1e-6;
	REAL value = -log(1.0 / y - 1.0) 
		/ m_inputScale;
	ASSERT(IsApproxEqual(Sigmoid(value), y));
	return value;

}	// CHistogramMatcher::InvSigmoid

