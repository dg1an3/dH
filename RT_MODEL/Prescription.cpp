// Prescription.cpp: implementation of the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Prescription.h"

#include <ConjGradOptimizer.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CPrescription::CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPrescription::CPrescription(CPlan *pPlan)
	: CObjectiveFunction(FALSE),
		m_pPlan(pPlan),
		m_pOptimizer(NULL),
		m_pNextLevel(NULL)
{
	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		m_arrHistoMatchers.Add(new CHistogramMatcher(this, nAtScale));
	}

	m_pOptimizer = new CConjGradOptimizer(this);

}	// CPrescription::CPrescription

///////////////////////////////////////////////////////////////////////////////
// CPrescription::~CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPrescription::~CPrescription()
{
	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		delete m_arrHistoMatchers[nAtScale];
	}

}	// CPrescription::~CPrescription


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Optimize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CPrescription::Optimize(CCallbackFunc func)
{
	return TRUE;

}	// CPrescription::Optimize


///////////////////////////////////////////////////////////////////////////////
// CPrescription::operator
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
REAL CPrescription::operator()(const CVectorN<>& vInput, 
	CVectorN<> *pGrad ) const
{
	return 0.0;

}	// CPrescription::operator


int CPrescription::AddStructure(CStructure *pStruct, REAL weight)
{
	int nIndex = 0;

	BEGIN_LOG_SECTION(CPrescription::AddStructure);

/*	CVolume<double> *pDose = m_pPlan->GetDoseMatrix(0);
	CVolume<double> *pRegion = pStruct->GetRegion(0);
	pRegion->SetDimensions(pDose->GetWidth(), pDose->GetHeight(), pDose->GetDepth());
	pRegion->ClearVoxels();
*/
	ASSERT(pStruct->GetRegion(0)->GetWidth() == m_pPlan->GetDoseMatrix(0)->GetWidth());

	REAL binWidth = 0.0125;
	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		LOG_OBJECT((*pStruct->GetRegion(nAtScale))); 

		CHistogram *pHisto = new CHistogram();
		pHisto->SetVolume(m_pPlan->GetDoseMatrix(nAtScale));
		pHisto->SetRegion(pStruct->GetRegion(nAtScale));

		pHisto->SetBinning(0.0, binWidth, 2.0);
		binWidth *= 2.0;

		for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(nAtScale); nAtElem++)
		{
			int nBeam;
			int nBeamlet;
			GetBeamletFromSVElem(nAtElem, nAtScale, &nBeam, &nBeamlet);

			pHisto->Add_dVolume(m_pPlan->GetBeamAt(nBeam)->GetBeamlet(nBeamlet, nAtScale));
		}

		m_mapHistograms[nAtScale].SetAt(pStruct, pHisto);

		// add to the histogram matchers
		nIndex = m_arrHistoMatchers[nAtScale]->AddHistogram(pHisto, weight);
	}

	END_LOG_SECTION();

	return nIndex; // m_mapHistograms[0].GetCount();

}	// CPlanIMRT::AddStructure


///////////////////////////////////////////////////////////////////////////////
// CPrescription::AddStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::AddStructureTerm(CVOITerm *pVOIT)
{
	pVOIT->GetHistogram()->SetVolume(m_pSumVolume);
	m_mapVOITerms[pVOIT->GetVOI()] = pVOIT;

	// set up structure terms for sub-levels
	if (m_pNextLevel)
	{
		m_pNextLevel->AddStructureTerm(pVOIT->Subcopy());
	}

}	// CPrescription::AddStructureTerm


///////////////////////////////////////////////////////////////////////////////
// CPrescription::RemoveStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::RemoveStructureTerm(CStructure *pStruct)
{
	CVOITerm *pVOIT = NULL;
	if (m_mapVOITerms.Lookup(pStruct, pVOIT))
	{
		m_mapVOITerms.RemoveKey(pStruct);
		delete pVOIT;

		// set up structure terms for sub-levels
		if (m_pNextLevel)
		{
			m_pNextLevel->RemoveStructureTerm(pStruct);
		}
	}

}	// CPrescription::RemoveStructureTerm


///////////////////////////////////////////////////////////////////////////////
// CPrescription::SetPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetPlan(CPlan *pPlan, int nLevels)
{
	m_pPlan = pPlan;

/*	if (nLevels-1 > 1)
	{
		m_pNextLevel = new CPrescription();
		m_pNextLevel->SetPlan(pPlan, nLevels-1);
	} */

}	// CPrescription::SetPlan


//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetInterval
// 
// sets the target histgram to an interval
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetInterval(long nAt, 
									double low, double high, 
									double fraction)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetInterval);
	LOG("Setting interval for histogram");
	LOG_EXPR(low);
	LOG_EXPR(high);
	LOG_EXPR(fraction);

	// get the main volume
	CVolume<double> *pVolume = GetHistogram()->GetVolume();
	CVectorN<>& targetBins = GetTargetBins();
	targetBins.SetDim(GetHistogram()->GetBinForValue(high)+1);
	targetBins.SetZero();

	double sum = 0.0;
	for (int nAtBin = GetHistogram()->GetBinForValue(low); 
		nAtBin <= GetHistogram()->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = 1.0;
		sum += 1.0;
	}

	double totalVolume = pVolume->GetVoxelCount();
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

	END_LOG_SECTION();	// CKLDivTerm::SetInterval

}	// CKLDivTerm::SetInterval


//////////////////////////////////////////////////////////////////////
// CKLDivTerm::SetRamp
// 
// sets the target histgram to a ramp
//////////////////////////////////////////////////////////////////////
void CKLDivTerm::SetRamp(long nAt, 
									double low, double low_frac,
									double high, double high_frac)
{
	BEGIN_LOG_SECTION(CKLDivTerm::SetRamp);
	LOG("Setting ramp for histogram");
	LOG_EXPR(low);
	LOG_EXPR(low_frac);
	LOG_EXPR(high);
	LOG_EXPR(high_frac);

	// get the main volume
	CVolume<double> *pVolume = GetHistogram()->GetVolume();
	CVectorN<>& targetBins = GetTargetBins();
	targetBins.SetZero();

	double sum = 0.0;
	double step = (high_frac - low_frac) 
		/ (GetHistogram()->GetBinForValue(high)
			- GetHistogram()->GetBinForValue(low));
	double bin_value = low_frac;
	for (int nAtBin = GetHistogram()->GetBinForValue(low); 
		nAtBin <= GetHistogram()->GetBinForValue(high); nAtBin++)
	{
		targetBins[nAtBin] = bin_value;
		bin_value += step;
	}

	LOG_EXPR_EXT(targetBins);
	LOG_EXPR_EXT_DESC(GetHistogram()->GetBinMeans(), "Bin Means");

	END_LOG_SECTION();	// CKLDivTerm::SetRamp

}	// CKLDivTerm::SetRamp




REAL CKLDivTerm::Eval(const CVectorN<>& d_vInput, CVectorN<> *pvGrad)
{
	REAL sum = 0.0;

	BEGIN_LOG_SECTION(CKLDivTerm::Eval);

	// get the calculated histogram bins
	CVectorN<> calcGPDF = GetHistogram()->GetGBins();
	LOG_EXPR_EXT(calcGPDF);

	// get the target bins
	CVectorN<> targetGPDF = GetTargetGBins();
	LOG_EXPR_EXT(targetGPDF);

	const long n_dVolCount = GetHistogram()->Get_dVolumeCount();
	std::vector< CVectorN<> > arrCalc_dGPDF;

	// if a gradient is needed
	if (pvGrad)
	{
		pvGrad->SetDim(n_dVolCount);
		pvGrad->SetZero();

		for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
		{
			arrCalc_dGPDF.push_back(GetHistogram()->Get_dGBins(nAt_dVol));
			LOG_EXPR_EXT_DESC(arrCalc_dGPDF.back(), FMT("arrCalc_dGPDF %i", nAt_dVol));
		}
	} 

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
	const REAL EPS = 1e-12;
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
		if (pvGrad)
		{
			// iterate over the dVolumes
			for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
			{
				// normalize this bin
				arrCalc_dGPDF[nAt_dVol][nAtBin] /= calcSum;

				// add to the proper gradient element
				(*pvGrad)[nAt_dVol] += m_weight
					* (log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS) + 1.0)
						* arrCalc_dGPDF[nAt_dVol][nAtBin]
						* d_vInput[nAt_dVol];
						// * dSigmoid(vInput[nAt_dVol]);
						// * m_inputScale * exp(m_inputScale * vInput[nAt_dVol]);
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

	LOG_EXPR(sum);
	END_LOG_SECTION();	// CHistogramMatcher::Eval_RelativeEntropy

	return m_weight * sum;
}

CHistogram * CPrescription::GetHistogram(CStructure *pStruct, int nScale)
{
	CHistogram *pHisto = NULL;
	m_mapHistograms[nScale].Lookup(pStruct, pHisto);

	return pHisto;
}




///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetStateVector(int nScale, CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights(m_pPlan->GetBeamCount(), 
		m_pPlan->GetBeamAt(0)->GetBeamletCount(nScale));
	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
	{
		mBeamletWeights[nAtBeam] = m_pPlan->GetBeamAt(nAtBeam)->GetIntensityMap(nScale);
	}
	BeamletWeightsToStateVector(nScale, mBeamletWeights, vState);

}	// CPrescription::GetStateVector

///////////////////////////////////////////////////////////////////////////////
// CPrescription::SetStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetStateVector(int nScale, const CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights;
	StateVectorToBeamletWeights(nScale, vState, mBeamletWeights);

	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
	{
		m_pPlan->GetBeamAt(nAtBeam)->SetIntensityMap(nScale, mBeamletWeights[nAtBeam]);
	}

}	// CPrescription::SetStateVector


///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetBeamletFromSVElem
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet)
{
	(*pnBeam) = nElem % m_pPlan->GetBeamCount();

	int nShift = (nElem / m_pPlan->GetBeamCount() + 1) / 2;
	int nDir = pow(-1, nElem / m_pPlan->GetBeamCount());
	(*pnBeamlet) = nShift * nDir;

}	// CPrescription::GetBeamletFromSVElem



///////////////////////////////////////////////////////////////////////////////
// CPrescription::InvFilterStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg)
{
	BEGIN_LOG_SECTION(CPrescription::InvFilterStateVector);

	CMatrixNxM<> mBeamletWeights;
	StateVectorToBeamletWeights(nScale, vIn, mBeamletWeights);
	LOG_EXPR_EXT(mBeamletWeights);

	CMatrixNxM<> mFiltBeamletWeights(mBeamletWeights.GetCols(),
		m_pPlan->GetBeamAt(0)->GetBeamletCount(nScale-1));

	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
	{
		m_pPlan->GetBeamAt(nAtBeam)->InvFiltIntensityMap( 
			nScale,
			mBeamletWeights[nAtBeam],
			mFiltBeamletWeights[nAtBeam]);
	} 
	LOG_EXPR_EXT(mFiltBeamletWeights);

	BeamletWeightsToStateVector(nScale-1, mFiltBeamletWeights, vOut); 


	END_LOG_SECTION();	// CPrescription::InvFilterStateVector

}	// CPrescription::InvFilterStateVector

///////////////////////////////////////////////////////////////////////////////
// CPrescription::StateVectorToBeamletWeights
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights)
{
	int nBeamletCount = m_pPlan->GetBeamAt(0)->GetBeamletCount(nScale);
	int nBeamletOffset = nBeamletCount / 2;
	mBeamletWeights.Reshape(m_pPlan->GetBeamCount(), nBeamletCount);
	for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(nScale); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, nScale, &nBeam, &nBeamlet);

		mBeamletWeights[nBeam][nBeamlet + nBeamletOffset] = vState[nAtElem];
	}

}	// CPrescription::StateVectorToBeamletWeights

///////////////////////////////////////////////////////////////////////////////
// CPrescription::BeamletWeightsToStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState)
{
	int nBeamletCount = m_pPlan->GetBeamAt(0)->GetBeamletCount(nScale);
	vState.SetDim(m_pPlan->GetTotalBeamletCount(nScale));
	int nBeamletOffset = nBeamletCount / 2;
	for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(nScale); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, nScale, &nBeam, &nBeamlet);

		vState[nAtElem] = mBeamletWeights[nBeam][nBeamlet + nBeamletOffset];
	}

}	// CPrescription::BeamletWeightsToStateVector
