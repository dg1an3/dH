// Prescription.cpp: implementation of the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Prescription.h"

#include <ConjGradOptimizer.h>
#include <DFPOptimizer.h>

#include <iostream>
#include ".\include\prescription.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const REAL SIGMOID_SCALE = 1.0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CPrescription::CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPrescription::CPrescription(CPlan *pPlan, int nLevel)
	: CObjectiveFunction(FALSE)
		, m_pPlan(pPlan)
		, m_nLevel(nLevel)
		, m_pOptimizer(NULL)
		, m_pNextLevel(NULL)
		, m_totalEntropyWeight(GetProfileReal("Prescription", "EntropyWeight", 0.25))
		, m_intensityMapSumWeight((REAL) 0.0) //1.10)
		, m_inputScale(GetProfileReal("Prescription", "InputScale", 0.5))
{
	CConjGradOptimizer *pOptimizer = new // CDFPOptimizer(this); // 
		CConjGradOptimizer(this);
	pOptimizer->m_bSetLineToleranceEqual = false;
	m_pOptimizer = pOptimizer;

	if (m_nLevel == 0)
	{
		// TODO: put these in registry
		REAL GBinSigma = GetProfileReal("Prescription", "GBinSigma", 0.2);

		REAL atLevelSigma[MAX_SCALES];
		atLevelSigma[0] = GetProfileReal("Prescription", "LevelSigma0", 4.0);
		atLevelSigma[1] = GetProfileReal("Prescription", "LevelSigma1", 2.0);
		atLevelSigma[2] = GetProfileReal("Prescription", "LevelSigma2", 1.0);
		atLevelSigma[3] = GetProfileReal("Prescription", "LevelSigma3", 0.5);

		REAL tol[MAX_SCALES];
		tol[0] = GetProfileReal("Prescription", "Tolerance0", 1e-3);
		tol[1] = GetProfileReal("Prescription", "Tolerance1", 1e-3);
		tol[2] = GetProfileReal("Prescription", "Tolerance2", 1e-3);
		tol[3] = GetProfileReal("Prescription", "Tolerance3", 1e-3);

		REAL cgtol[MAX_SCALES];
		cgtol[0] = GetProfileReal("Prescription", "CGTolerance0", 1e-3);
		cgtol[1] = GetProfileReal("Prescription", "CGTolerance1", 1e-3);
		cgtol[2] = GetProfileReal("Prescription", "CGTolerance2", 1e-3);
		cgtol[3] = GetProfileReal("Prescription", "CGTolerance3", 1e-3);

		CPrescription *pPresc = this;
		pPresc->SetGBinSigma(GBinSigma / atLevelSigma[0]);
		pPresc->m_tolerance = tol[0];
		pPresc->m_cgtolerance = cgtol[0];
		pPresc->m_nLevel = 0;

		for (int nAtLevel = 1; nAtLevel < MAX_SCALES; nAtLevel++)
		{
			pPresc->m_pNextLevel = new CPrescription(pPlan, nAtLevel);

			pPresc = pPresc->m_pNextLevel;
			pPresc->m_nLevel = nAtLevel;
			pPresc->SetGBinSigma(GBinSigma / atLevelSigma[nAtLevel]);

			pPresc->m_tolerance = tol[nAtLevel];
			pPresc->m_cgtolerance = cgtol[nAtLevel];
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
	delete m_pNextLevel;

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
// CPrescription::GetPlan
// 
// accessor for associated plan
///////////////////////////////////////////////////////////////////////////////
CPlan * CPrescription::GetPlan()
{
	return m_pPlan;

}	// CPrescription::GetPlan

///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetLevel
// 
// returns the given level of the pyramid
///////////////////////////////////////////////////////////////////////////////
CPrescription *CPrescription::GetLevel(int nLevel)
{
	if (nLevel == 0) 
	{
		return this; 
	}

	return m_pNextLevel->GetLevel(nLevel-1);

}	// CPrescription::GetLevel


///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetLevel
// 
// returns the given level of the pyramid
///////////////////////////////////////////////////////////////////////////////
const CPrescription *CPrescription::GetLevel(int nLevel) const
{
	if (nLevel == 0) 
	{
		return this; 
	}

	return m_pNextLevel->GetLevel(nLevel-1);

}	// CPrescription::GetLevel


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

}	// CPrescription::GetStructureTerm

///////////////////////////////////////////////////////////////////////////////
// CPrescription::AddStructureTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::AddStructureTerm(CVOITerm *pVOIT)
{
	BEGIN_LOG_SECTION(CPrescription::AddStructure);

	// initialize the sum volume, so as to coincide with the beamlets
	CVolume<VOXEL_REAL> *pBeamlet = m_pPlan->GetBeamAt(m_pPlan->GetBeamCount()-1)->GetBeamletSub(0, m_nLevel);
	m_sumVolume.ConformTo(pBeamlet);

	// initialize the histogram region
	// TODO: fix this memory leak
	CVolume<VOXEL_REAL> *pResampRegion = pVOIT->GetVOI()->GetConformRegion(&m_sumVolume);

	// set histogram options
	CHistogram *pHisto = pVOIT->GetHistogram();
	pHisto->SetVolume(&m_sumVolume);
	pHisto->SetRegion(pResampRegion);

	// need to do this before initialize target bins
	//	 original = 0.0125	0.006125  // * pow(2.0, (double) m_nLevel));
	REAL binWidth = R(0.02); // R(0.0030625);  
	pHisto->SetBinning(0.0, binWidth, GBINS_BUFFER);
	pHisto->SetGBinSigma(m_GBinSigma);

	// set up dVolumes
	for (int nAtElem = 0; nAtElem < m_pPlan->GetTotalBeamletCount(m_nLevel); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, m_nLevel, &nBeam, &nBeamlet);

#ifdef DVOLUME_ADAPT
		CVolume<VOXEL_REAL> *pBeamletAdapt = m_pPlan->GetBeamAt(nBeam)->GetBeamletAdapt(nBeamlet, m_nLevel);

		// set default threshold based on config items
		pBeamletAdapt->SetThreshold(
			(VOXEL_REAL) GetProfileReal("Prescription", "BeamletThreshold", VOXEL_REAL(0.005)));

		pHisto->Add_dVolume(pBeamletAdapt, nBeam, m_nLevel, 
			m_pPlan->GetBeamAt(nBeam)->GetBeamletSub(nBeamlet, m_nLevel));
#else
		CVolume<VOXEL_REAL> *pBeamlet = m_pPlan->GetBeamAt(nBeam)->GetBeamletSub(nBeamlet, m_nLevel);
		// ASSERT(pBeamlet->GetBasis().IsApproxEqual(pResampRegion->GetBasis()));

		// set default threshold based on config items
		pBeamlet->SetThreshold(
			(VOXEL_REAL) GetProfileReal("Prescription", "BeamletThreshold", VOXEL_REAL(0.005)));

		pHisto->Add_dVolume(pBeamlet, nBeam);
#endif
	}

	// add to the current prescription
	m_mapVOITs[pVOIT->GetVOI()] = pVOIT;

	// set up element include flags
	SetElementInclude();

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
// CPrescription::UpdateTerms
// 
// updates terms from another prescription object
///////////////////////////////////////////////////////////////////////////////
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

	if (m_pNextLevel)
	{
		m_pNextLevel->UpdateTerms(pPresc->m_pNextLevel);
	}
}	// CPrescription::UpdateTerms

///////////////////////////////////////////////////////////////////////////////
// CPrescription::SetGBinSigma
// 
// sets GBinSigma for all histograms
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
// CPrescription::SetEntropyWeight
// 
// sets entropy weight parameter
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetEntropyWeight(REAL weight)
{
	m_totalEntropyWeight = weight;

	if (m_pNextLevel)
	{
		m_pNextLevel->SetEntropyWeight(weight);
	}

}	// CPrescription::SetEntropyWeight

///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetOptimizer
// 
// returns embedded optimizer
///////////////////////////////////////////////////////////////////////////////
COptimizer * CPrescription::GetOptimizer(void)
{
	return m_pOptimizer;

}	// CPrescription::GetOptimizer

///////////////////////////////////////////////////////////////////////////////
// CPrescription::Optimize
// 
// performs multi-level optimization
///////////////////////////////////////////////////////////////////////////////
BOOL CPrescription::Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam)
{
	BEGIN_LOG_SECTION(CPrescription::Optimize);
	LOG(FMT("Optimizing Scale %i", m_nLevel));

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

	// inverse transform the initial vector to form initial optimizer parameter
	InvTransform(&vInit);

	m_pOptimizer->SetTolerance(m_cgtolerance); 
	// DGL: not for DFP 
	((CConjGradOptimizer *)m_pOptimizer)->GetBrentOptimizer().SetTolerance(m_tolerance);
	m_pOptimizer->SetCallback(pFunc, pParam);
	CVectorN<> vRes = m_pOptimizer->Optimize(vInit);

	// check for problem with optimization
	if (m_pOptimizer->GetIterations() == -1)
	{
		EXIT_LOG_SECTION();
		return FALSE;
	}

	LOG_EXPR(m_pOptimizer->GetIterations());
	cout << "Iterations for S" << m_nLevel << " = " 
		<< m_pOptimizer->GetIterations() << endl;

	int nIter = ((CConjGradOptimizer*)m_pOptimizer)->GetBrentOptimizer().GetIterations();
	TRACE("\tBrent Iterations = %i\n", nIter);

	// compute final state vector
	vInit = vRes;
	Transform(&vInit);

	//////////////////////////////////////////////////////////////////////////
	//  L O G   O N L Y   S E C T I O N
		BEGIN_LOG_ONLY(CPrescription::Optimize!Level_Results);

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

		END_LOG_SECTION();	// CPrescription::Optimize!Level_Results
	//
	//////////////////////////////////////////////////////////////////////////

	END_LOG_SECTION(); // FMT("Optimizing scale %n", nAtLevel)

	FLUSH_LOG();

	return TRUE;

}	// CPrescription::Optimize




///////////////////////////////////////////////////////////////////////////////
// CPrescription::CalcSumSigmoid
// 
// computes the sum of weights from an input vector
///////////////////////////////////////////////////////////////////////////////
void CPrescription::CalcSumSigmoid(CHistogram *pHisto, 
								   const CVectorN<>& vInputTrans, 
								   const CArray<BOOL, BOOL>& arrInclude) const
{
	// get the main volume
	CVolume<VOXEL_REAL> *pVolume = pHisto->GetVolume();
	ASSERT(pVolume == &m_sumVolume);
	pVolume->ClearVoxels();

	// iterate over the component volumes, accumulating the weighted volumes
	ASSERT(vInputTrans.GetDim() == pHisto->Get_dVolumeCount());

	int nMaxGroup = pHisto->GetGroupCount();
	for (int nAtGroup = 0; nAtGroup < nMaxGroup; nAtGroup++)
	{
		BOOL bInitVolGroup = TRUE;

		for (int nAt_dVolume = 0; nAt_dVolume < pHisto->Get_dVolumeCount();
			nAt_dVolume++)
		{
			int nGroup = 0;
			CVolume<VOXEL_REAL> *p_dVolume = pHisto->Get_dVolume(nAt_dVolume, &nGroup);

			if (nGroup == nAtGroup)
			{
				if (bInitVolGroup)
				{
					m_volGroup.ConformTo(p_dVolume);
					m_volGroup.ClearVoxels();

					bInitVolGroup = FALSE;
				}

				// add to weighted sum
				if (arrInclude[nAt_dVolume])
				{
					// use Transform'd input to calc sigmoid
					const REAL weight = vInputTrans[nAt_dVolume]; 
					m_volGroup.Accumulate(p_dVolume, weight, TRUE);
				}
			}
		}

		// now rotate the groups sum to the main sumVolume basis
		m_volGroupMain.ConformTo(pVolume);
		m_volGroupMain.ClearVoxels();
		Resample(&m_volGroup, &m_volGroupMain, TRUE);

		// and accumulate
		pVolume->Accumulate(&m_volGroupMain, 1.0, FALSE);
	}

}	// CPrescription::CalcSumSigmoid


///////////////////////////////////////////////////////////////////////////////
// CPrescription::operator
// 
// objective function evaluator
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

	// transform input for calc purposes
	CVectorN<> vInputTrans = vInput;
	Transform(&vInputTrans);

	CVectorN<> vdInputTrans;
	bool bCalcdInputTrans = true;

	BEGIN_LOG_SECTION(CPrescription::operator());

	//
	// Compute match of target bins to histo bins
	//

	bool bCalcSum = true;

	static bool bCalcKLDiv = true;
	POSITION pos = bCalcKLDiv ? m_mapVOITs.GetStartPosition() : NULL;
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		if (pVOIT->GetWeight() < DEFAULT_EPSILON)
		{
			continue;
		}

		// calculate the summed volume, if this is the first VOIT
		if (bCalcSum)
		{
			CalcSumSigmoid(pVOIT->GetHistogram(), vInputTrans, m_arrIncludeElement);
			bCalcSum = false;
		}

		if (pGrad)
		{
			m_vPartGrad.SetDim(vInput.GetDim());
			m_vPartGrad.SetZero();

			totalSum += pVOIT->Eval(&m_vPartGrad, m_arrIncludeElement);

			if (bCalcdInputTrans)
			{
				vdInputTrans.SetDim(vInput.GetDim());
				vdInputTrans = vInput;
				dTransform(&vdInputTrans);
				bCalcdInputTrans = false;
			}

			// apply the chain rule for the sigmoid
			for (int nAt = 0; nAt < m_vPartGrad.GetDim(); nAt++)
			{
				// use dTransform'd vInput
				m_vPartGrad[nAt] *= vdInputTrans[nAt]; 
			}

			(*pGrad) += m_vPartGrad;
		}
		else
		{
			totalSum += pVOIT->Eval(NULL, m_arrIncludeElement);
		}
	}

	//
	// now evalute total entropy
	//

	static bool bCalcEntropy = false; // true;
	if (bCalcEntropy)
	{
		if (pGrad)
		{
			m_vPartGrad.SetDim(vInput.GetDim());
			m_vPartGrad.SetZero();

			totalSum -=  Eval_TotalEntropy(vInput, &m_vPartGrad);

			(*pGrad) -= m_vPartGrad;
		}
		else
		{
			totalSum -=  Eval_TotalEntropy(vInput);
		}
	}

	ASSERT(_finite(totalSum));

#if defined(_DEBUG) && defined(TEST_GRADIENT)

	// TODO: remove this when OptDB is ready
	static bool bTest = false;
	if (pGrad && !bTest)
	{
		bTest = true;
		bCalcKLDiv = true;
		bCalcEntropy = false;

		double tempTEW = m_totalEntropyWeight;
		// const_cast<REAL&>(m_totalEntropyWeight) = 0.0;
		double tempIMSW = m_intensityMapSumWeight;
		// const_cast<REAL&>(m_intensityMapSumWeight) = 0.0;

		LOG_EXPR_EXT(vInput);

		CVectorN<> vEvalGrad;
		(*this)(vInput, &vEvalGrad);

		CVectorN<> vDiffGrad;
		Gradient(vInput, R(1e-3), vDiffGrad);

		LOG_EXPR_EXT(vEvalGrad);
		LOG_EXPR_EXT(vDiffGrad);

		// const_cast<REAL&>(m_totalEntropyWeight) = tempTEW;
		// const_cast<REAL&>(m_intensityMapSumWeight) = tempIMSW;

		bCalcKLDiv = true;
		bCalcEntropy = true;

		bTest = false;
	}
#endif

	END_LOG_SECTION();	// CPrescription::operator()

	// TRACE("objFunc = %f\n", totalSum);

	// DGL: adding 0.1 to hold it up off 0.0 
	totalSum += 0.1;

	return totalSum;

}	// CPrescription::operator()


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Eval_TotalEntropy
// 
// Evaluates the total entropy and gradient for the input vector
///////////////////////////////////////////////////////////////////////////////
REAL CPrescription::Eval_TotalEntropy(const CVectorN<>& vInput, 
											   CVectorN<> *pGrad) const
{
	m_vBeamWeights.SetDim(m_pPlan->GetBeamCount());
	m_vBeamWeights.SetZero();

	CVectorN<> vInputTrans = vInput;
	Transform(&vInputTrans);

	double sum = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

		double beamWeight = vInputTrans[nAt];
		m_vBeamWeights[nBeam] += beamWeight;
		sum += beamWeight;
	}

	if (sum == 0.0)
	{
		if (pGrad)
		{
			pGrad->SetZero();
		}

		return 0.0;
	}

	const double EPS = 1e-5;

	// to accumulate total entropy
	double totalEntropy = 0.0;

	// for each beamlet
	for (int nAtBeam = 0; nAtBeam < m_vBeamWeights.GetDim(); nAtBeam++)
	{
		// p is probability of intensity of this beamlet
		double p_beam = m_vBeamWeights[nAtBeam] / sum;

		// contribution to total is entropy of the probability
		totalEntropy += - p_beam * log(p_beam + EPS);
	}
	totalEntropy /= ((double) m_vBeamWeights.GetDim() 
		* -log(1.0 / (double) m_vBeamWeights.GetDim()));


	// do we evaluate gradient?
	if (pGrad)
	{
		pGrad->SetZero();

		CVectorN<> vdInputTrans = vInput;
		dTransform(&vdInputTrans);

		for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
		{
			int nBeam;
			int nBeamlet;
			GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

			// don't need this (zeroed above)
			for (int nAtBeam2 = 0; nAtBeam2 < m_vBeamWeights.GetDim(); nAtBeam2++)
			{
				double p_beam2 = m_vBeamWeights[nAtBeam2] / sum;

				(*pGrad)[nAt] += 
					R(m_totalEntropyWeight 
						* -((p_beam2 / (p_beam2 + EPS)) 
								+ log(p_beam2 + EPS)) 
						* (-m_vBeamWeights[nAtBeam2] + ((nAtBeam2 == nBeam) ? sum : R(0.0)))
							/ (sum * sum)

						* vdInputTrans[nAt]);
			}

			// now normalize entropy
			(*pGrad)[nAt]
				/= R(((double) m_vBeamWeights.GetDim() 
					* -log(1.0 / (double) m_vBeamWeights.GetDim())));


			(*pGrad)[nAt] -= m_intensityMapSumWeight * vdInputTrans[nAt]
				/ ((REAL) vInput.GetDim());
		}
	}

	// return total entropy scaled by number of beamlets
	return R(m_totalEntropyWeight * totalEntropy 
		- m_intensityMapSumWeight * sum / ((REAL) vInput.GetDim()));

}	// CPrescription::Eval_TotalEntropy


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Transform
// 
// transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::Transform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = SIGMOID_SCALE * Sigmoid((*pvInOut)[nAt], m_inputScale));

}	// CPrescription::Transform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::dTransform
// 
// derivative transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::dTransform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = SIGMOID_SCALE * dSigmoid((*pvInOut)[nAt], m_inputScale));

}	// CPrescription::dTransform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::InvTransform
// 
// inverse transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::InvTransform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = InvSigmoid((*pvInOut)[nAt] / SIGMOID_SCALE, m_inputScale));

}	// CPrescription::InvTransform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetInitStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetInitStateVector(CVectorN<>&vInit)
{
	CPrescription *pLevelMax = this;
	while (pLevelMax->m_nLevel < MAX_SCALES-1)
		pLevelMax = pLevelMax->m_pNextLevel;

	vInit.SetDim(m_pPlan->GetTotalBeamletCount(MAX_SCALES-1));
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		vInit[nAt] = (pLevelMax->m_arrIncludeElement[nAt]) ? R(0.001) // R(0.001) 
			: R(0.001) 
			/ R(m_pPlan->GetBeamCount()); // (REAL) vInit.GetDim();
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
	m_pPlan->GetDoseMatrix();

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






///////////////////////////////////////////////////////////////////////////////
// CPrescription::InvFilterStateVector
// 
// transfers beamlet weights from level n-1 to level n
///////////////////////////////////////////////////////////////////////////////
void CPrescription::InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg)
{
	BEGIN_LOG_SECTION(CPrescription::InvFilterStateVector);

	// TODO: make a member so don't have to re-initialize
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
// CPrescription::SetElementInclude
// 
// determines array of flags of beamlets (elements) to include in optimization
///////////////////////////////////////////////////////////////////////////////
void CPrescription::SetElementInclude()
{
	for (int nAt = 0; nAt < m_arrIncludeElement.GetSize(); nAt++)
	{
		m_arrIncludeElement[nAt] = false; // true;
	}

	// iterate over terms to find target term
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		// check type
		// CKLDivTerm *pKLDivTerm = (CKLDivTerm *) pVOIT;

		CHistogram *pHisto = pVOIT->GetHistogram();

		if (m_arrIncludeElement.GetSize() < pHisto->Get_dVolumeCount())
		{
			m_arrIncludeElement.SetSize(pHisto->Get_dVolumeCount());

			for (int nAt = 0; nAt < m_arrIncludeElement.GetSize(); nAt++)
			{
				m_arrIncludeElement[nAt] = false; // true;
			}
		}

		// check if its a target
		if (CStructure::eTARGET == pVOIT->GetVOI()->GetType()) 
		{
			// iterate over beamlets, seeing which are included in target term
			for (int nAt = 0; nAt < pHisto->Get_dVolumeCount(); nAt++)
			{
				// TODO: fix this for multiple targets
				m_arrIncludeElement[nAt] = 
					m_arrIncludeElement[nAt]
						|| pHisto->IsContributing(nAt);
			}
		}
	}

	if (m_pNextLevel)
	{
		m_pNextLevel->SetElementInclude();
	}

}	// CPrescription::SetElementInclude

#define PRESC_SCHEMA 1

IMPLEMENT_SERIAL(CPrescription, CObject, VERSIONABLE_SCHEMA | PRESC_SCHEMA)

void CPrescription::Serialize(CArchive& ar)
{
	// schema for the voiterm object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : PRESC_SCHEMA;

	CObject::Serialize(ar);

	SERIALIZE_VALUE(ar, m_pPlan);

	if (ar.IsStoring())
	{	// storing code
		CTypedPtrArray<CObArray, CVOITerm *> arrTerms;

		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;

		POSITION pos = m_mapVOITs.GetStartPosition();
		while (pos != NULL)
		{
			m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
			arrTerms.Add(pVOIT);
		}

		arrTerms.Serialize(ar);

		// TODO: figure out how GBinSigma is to be stored / retrieved
	}
	else
	{	// loading code
		// set up subordinate presc objects
		CPrescription *pPresc = this->m_pNextLevel;
		while (NULL != pPresc)
		{
			pPresc->m_pPlan = m_pPlan;
			pPresc = pPresc->m_pNextLevel;
		}

		CTypedPtrArray<CObArray, CVOITerm *> arrTerms;
		arrTerms.Serialize(ar);

		for (int nAt = 0; nAt < arrTerms.GetSize(); nAt++)
		{
			AddStructureTerm(arrTerms[nAt]);
		}

		// sets the include flag on prescription (double-check)
		SetElementInclude();
	}
}
