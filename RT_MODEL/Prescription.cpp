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
CPrescription::CPrescription(CPlan *pPlan, int nLevel)
	: CObjectiveFunction(FALSE),
		m_pPlan(pPlan),
		m_nLevel(nLevel),
		m_pOptimizer(NULL),
		m_pNextLevel(NULL),
		m_totalEntropyWeight((REAL) 0.05),
		m_inputScale((REAL) 0.5)
{
	m_pOptimizer = new CConjGradOptimizer(this);

	if (m_nLevel == 0)
	{
		REAL GBinSigma = (REAL) 0.20;
		int nAtSigma[] = {4, 2, 1};
		REAL tol[] = // {(REAL) 1e-3, (REAL) 1e-4, (REAL) 1e-4};
			{(REAL) 1e-3, (REAL) 1e-3, (REAL) 1e-3};
			// {(REAL) 1e-5, (REAL) 1e-5, (REAL) 1e-5};
		CPrescription *pPresc = this;
		pPresc->SetGBinSigma(GBinSigma / nAtSigma[0]);
		pPresc->m_tolerance = tol[0];

		for (int nAtLevel = 1; nAtLevel < MAX_SCALES; nAtLevel++)
		{
			pPresc->m_pNextLevel = new CPrescription(pPlan, nAtLevel);

			pPresc = pPresc->m_pNextLevel;
			pPresc->SetGBinSigma(GBinSigma / nAtSigma[nAtLevel]);

			pPresc->m_tolerance = tol[nAtLevel];
		}
	}

}	// CPrescription::CPrescription

///////////////////////////////////////////////////////////////////////////////
// CPrescription::~CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPrescription::~CPrescription()
{
	CStructure *pStruct = NULL;
	CVOITerm *pVOIT = NULL;
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
		delete pVOIT;
	}

	delete m_pOptimizer; 

}	// CPrescription::~CPrescription


///////////////////////////////////////////////////////////////////////////////
// CPrescription::CalcSumSigmoid
// 
// computes the sum of weights from an input vector
///////////////////////////////////////////////////////////////////////////////
void CPrescription::CalcSumSigmoid(CHistogram *pHisto, const CVectorN<>& vInput) const
{
	// get the main volume
	CVolume<REAL> *pVolume = pHisto->GetVolume();
	pVolume->ClearVoxels();

	// iterate over the component volumes, accumulating the weighted volumes
	ASSERT(vInput.GetDim() == pHisto->Get_dVolumeCount());

	int nMaxGroup = pHisto->GetGroupCount();
	for (int nAtGroup = 0; nAtGroup < nMaxGroup; nAtGroup++)
	{
		static CVolume<REAL> volGroup;
		BOOL bInitVolGroup = TRUE;

		for (int nAt_dVolume = 0; nAt_dVolume < pHisto->Get_dVolumeCount();
			nAt_dVolume++)
		{
			int nGroup = 0;
			CVolume<REAL> *p_dVolume = pHisto->Get_dVolume(nAt_dVolume, &nGroup);

			if (nGroup == nAtGroup)
			{
				
				if (bInitVolGroup)
				{
					volGroup.ConformTo(p_dVolume);
					volGroup.ClearVoxels();

					bInitVolGroup = FALSE;
				}

				// add to weighted sum
				volGroup.Accumulate(p_dVolume, 
					Sigmoid(vInput[nAt_dVolume], m_inputScale), TRUE);
			}
		}

		// now rotate the groups sum to the main sumVolume basis
		static CVolume<REAL> volGroupMain;
		volGroupMain.ConformTo(pVolume);
		volGroupMain.ClearVoxels();
		Resample(&volGroup, &volGroupMain, // FALSE); 
			TRUE);

		// and accumulate
		pVolume->Accumulate(&volGroupMain, 1.0, FALSE);
	}

}	// CPrescription::CalcSumSigmoid


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Eval_TotalEntropy
// 
// Evaluates the total entropy and gradient for the input vector
///////////////////////////////////////////////////////////////////////////////
REAL CPrescription::Eval_TotalEntropy(const CVectorN<>& vInput, 
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
		GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

		vBeamWeights[nBeam] += Sigmoid(vInput[nAt], m_inputScale);
		sum += Sigmoid(vInput[nAt], m_inputScale);
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
			GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

			// p is probability of intensity of this beamlet
			double p = vBeamWeights[nBeam] / sum;

			// derivative of prob is found by applying ratio rule
			double d_p = dSigmoid(vInput[nAt], m_inputScale) 
				* (sum - vBeamWeights[nBeam]) 
					/ (sum * sum);

			// apply chain rule to compute the derivative
			(*pGrad)[nAt] = -d_p * (1 + log(p));
		}
	}

	// return total entropy scaled by number of beamlets
	return totalEntropy;	

}	// CPrescription::Eval_TotalEntropy


///////////////////////////////////////////////////////////////////////////////
// CPrescription::operator
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
REAL CPrescription::operator()(const CVectorN<>& vInput, 
	CVectorN<> *pGrad ) const
{
	// initialize total sum of objective function
	REAL totalSum = 0.0;

	// initialize gradient vector
	if (pGrad)
	{
		pGrad->SetDim(vInput.GetDim());
		pGrad->SetZero();
	}

	BEGIN_LOG_SECTION(CPrescription::operator());

	//
	// Compute match of target bins to histo bins
	//

	bool bCalcSum = true;

	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		// calculate the summed volume, if this is the first VOIT
		if (bCalcSum)
		{
			CalcSumSigmoid(pVOIT->GetHistogram(), vInput);
			bCalcSum = false;
		}

		if (pGrad)
		{
			static CVectorN<> vPartGrad;
			vPartGrad.SetDim(vInput.GetDim());
			vPartGrad.SetZero();

			totalSum += pVOIT->Eval(&vPartGrad);

			// apply the chain rule for the sigmoid
			for (int nAt = 0; nAt < vPartGrad.GetDim(); nAt++)
			{
				vPartGrad[nAt] *= dSigmoid(vInput[nAt], m_inputScale);
			}

			(*pGrad) += vPartGrad;
		}
		else
		{
			totalSum += pVOIT->Eval();
		}
	}

	//
	// now evalute total entropy
	//

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

	END_LOG_SECTION();	// CPrescription::operator()

	return totalSum;

}	// CPrescription::operator()


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Optimize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL CPrescription::Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam)
{
	if (m_pNextLevel)
	{
		// FilterStateVector(vInit, m_nLevel, vInit);
		if (!m_pNextLevel->Optimize(vInit, pFunc, pParam))
		{
			// problem with prev level optimization
			return FALSE;
		}

		// continue with our optimization
		InvFilterStateVector(vInit, m_nLevel+1, vInit, TRUE);
	}

	BEGIN_LOG_SECTION(FMT("Optimizing scale %n", m_nLevel));

	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		ASSERT(_finite(vInit[nAt]));
		vInit[nAt] = InvSigmoid(vInit[nAt], m_inputScale);
		ASSERT(_finite(vInit[nAt]));
	}

	m_pOptimizer->SetTolerance(m_tolerance);
	m_pOptimizer->SetCallback(pFunc, pParam);
	CVectorN<> vRes = m_pOptimizer->Optimize(vInit);

	// check for problem with optimization
	if (m_pOptimizer->GetIterations() == -1)
	{
		return FALSE;
	}

	LOG_EXPR(m_pOptimizer->GetIterations());
	cout << "Iterations for S" << m_nLevel << " = " 
		<< m_pOptimizer->GetIterations() << endl;

	// compute final state vector (add a small amount if zero)
	for (nAt = 0; nAt < vRes.GetDim(); nAt++)
	{
		vInit[nAt] = __max(Sigmoid(vRes[nAt], m_inputScale), 1e-4);
	}

	BEGIN_LOG_ONLY("Optimization level results");

	CHistogram *pHisto = NULL;
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
		pHisto = pVOIT->GetHistogram();

		LOG_EXPR_EXT_DESC(pHisto->GetBins(), pStruct->GetName() + " Bins");
	}
	LOG_EXPR_EXT_DESC(pHisto->GetBinMeans(), "Bin Mean")

	LOG_OBJECT(m_sumVolume);

	END_LOG_SECTION();	// "Optimization level results"

	END_LOG_SECTION(); // FMT("Optimizing scale %n", nAtLevel)

	FLUSH_LOG();

	return TRUE;

}	// CPrescription::Optimize


///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVOITerm *CPrescription::GetStructureTerm(CStructure *pStruct)
{
	CVOITerm *pVOIT = NULL;
	m_mapVOITs.Lookup(pStruct, pVOIT);

	return pVOIT;
}

///////////////////////////////////////////////////////////////////////////////
// CPrescription::AddStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::AddStructureTerm(CVOITerm *pVOIT)
{
	BEGIN_LOG_SECTION(CPrescription::AddStructure);

	// initialize the sum volume, so as to coincide with the region
	CVolume<REAL> *pRegion = pVOIT->GetVOI()->GetRegion(m_nLevel);
	m_sumVolume.ConformTo(pRegion);

	// set histogram options
	CHistogram *pHisto = pVOIT->GetHistogram();
	pHisto->SetGBinSigma(m_GBinSigma);
	pHisto->SetVolume(&m_sumVolume);

	// need to do this before initialize target bins
	REAL binWidth = 0.025 * pow(2, m_nLevel);
	pHisto->SetBinning(0.0, binWidth, 2.0);

	// set up dVolumes
	for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(m_nLevel); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, m_nLevel, &nBeam, &nBeamlet);

		CVolume<REAL> *pBeamlet = m_pPlan->GetBeamAt(nBeam)->GetBeamlet(nBeamlet, m_nLevel);
		pBeamlet->SetThreshold((REAL) 0.01); // pow(10, -(m_nLevel+1)));
		pHisto->Add_dVolume(pBeamlet, nBeam);
	}

	// add to the current prescription
	m_mapVOITs[pVOIT->GetVOI()] = pVOIT;

	if (m_pNextLevel)
	{
		m_pNextLevel->AddStructureTerm(pVOIT->Subcopy());
	}

	END_LOG_SECTION();

}	// CPrescription::AddStructureTerm


///////////////////////////////////////////////////////////////////////////////
// CPrescription::RemoveStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::RemoveStructureTerm(CStructure *pStruct)
{
	CVOITerm *pVOIT = NULL;
	if (m_mapVOITs.Lookup(pStruct, pVOIT))
	{
		m_mapVOITs.RemoveKey(pStruct);
		delete pVOIT;

		// set up structure terms for sub-levels
		if (m_pNextLevel)
		{
			m_pNextLevel->RemoveStructureTerm(pStruct);
		}
	}

}	// CPrescription::RemoveStructureTerm


///////////////////////////////////////////////////////////////////////////////
// CPrescription::SetGBinSigma
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetGBinSigma(REAL GBinSigma)
{
	m_GBinSigma = GBinSigma;

	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		pVOIT->GetHistogram()->SetGBinSigma(m_GBinSigma);
	}

}	// CPrescription::SetGBinSigma


///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetInitStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetInitStateVector(CVectorN<>&vInit)
{
	vInit.SetDim(m_pPlan->GetTotalBeamletCount(2));
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		vInit[nAt] = 0.01 / (REAL) vInit.GetDim();
	}

}	// CPrescription::GetInitStateVector

///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetStateVectorFromPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetStateVectorFromPlan(CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights(m_pPlan->GetBeamCount(), 
		m_pPlan->GetBeamAt(0)->GetBeamletCount(0));
	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
	{
		mBeamletWeights[nAtBeam] = m_pPlan->GetBeamAt(nAtBeam)->GetIntensityMap();
	}
	BeamletWeightsToStateVector(0, mBeamletWeights, vState);

}	// CPrescription::GetStateVectorFromPlan

///////////////////////////////////////////////////////////////////////////////
// CPrescription::SetStateVectorToPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetStateVectorToPlan(const CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights;
	StateVectorToBeamletWeights(0, vState, mBeamletWeights);

	for (int nAtBeam = 0; nAtBeam < m_pPlan->GetBeamCount(); nAtBeam++)
	{
		m_pPlan->GetBeamAt(nAtBeam)->SetIntensityMap(mBeamletWeights[nAtBeam]);
	}

}	// CPrescription::SetStateVectorToPlan


///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetBeamletFromSVElem
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet) const
{
#ifdef BEAM_INTERLEAVE
	(*pnBeam) = nElem % m_pPlan->GetBeamCount();

	int nShift = (nElem / m_pPlan->GetBeamCount() + 1) / 2;
	int nDir = pow(-1, nElem / m_pPlan->GetBeamCount());
	(*pnBeamlet) = nShift * nDir;
#else
	int nBeamletCount = m_pPlan->GetBeamAt(0)->GetBeamletCount(nScale);
	(*pnBeam) = nElem / nBeamletCount;
	(*pnBeamlet) = nElem % nBeamletCount - nBeamletCount / 2;
#endif

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




void CPrescription::SetEntropyWeight(REAL weight)
{
	m_totalEntropyWeight = weight;

	if (m_pNextLevel)
		m_pNextLevel->SetEntropyWeight(weight);
}

void CPrescription::UpdateTerms(CPrescription *pPresc)
{
	m_GBinSigma = pPresc->m_GBinSigma;
	m_inputScale = pPresc->m_inputScale;
	SetEntropyWeight(pPresc->m_totalEntropyWeight);

	// now for the terms
	POSITION pos = pPresc->m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure * pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		pPresc->m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		CVOITerm *pMyVOIT = NULL;
		if (m_mapVOITs.Lookup(pStruct, pMyVOIT))
		{
			(*pMyVOIT) = (*pVOIT);
		}
	}
}
