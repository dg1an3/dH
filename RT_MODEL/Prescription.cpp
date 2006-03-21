// Prescription.cpp: implementation of the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Prescription.h"

#include <ConjGradOptimizer.h>

#include <iostream>
#include ".\include\prescription.h"

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
	: CObjectiveFunction(FALSE)
		, m_pPlan(pPlan)
		, m_nLevel(nLevel)
		, m_pOptimizer(NULL)
		, m_pNextLevel(NULL)
		, m_totalEntropyWeight(GetProfileReal("Prescription", "EntropyWeight", 0.25))
		, m_intensityMapSumWeight((REAL) 0.0) //1.10)
		, m_inputScale(GetProfileReal("Prescription", "InputScale", 0.5))
{
	m_pOptimizer = new CConjGradOptimizer(this);

	if (m_nLevel == 0)
	{
		// TODO: put these in registry
		REAL GBinSigma = GetProfileReal("Prescription", "GBinSigma", 0.2);

		REAL atLevelSigma[3];
		atLevelSigma[0] = GetProfileReal("Prescription", "LevelSigma0", 4.0);
		atLevelSigma[1] = GetProfileReal("Prescription", "LevelSigma1", 2.0);
		atLevelSigma[2] = GetProfileReal("Prescription", "LevelSigma2", 1.0);

		REAL tol[3];
		tol[0] = GetProfileReal("Prescription", "Tolerance0", 1e-3);
		tol[1] = GetProfileReal("Prescription", "Tolerance1", 1e-3);
		tol[2] = GetProfileReal("Prescription", "Tolerance2", 1e-3);

		CPrescription *pPresc = this;
		pPresc->SetGBinSigma(GBinSigma / atLevelSigma[0]);
		pPresc->m_tolerance = tol[0];

		for (int nAtLevel = 1; nAtLevel < MAX_SCALES; nAtLevel++)
		{
			pPresc->m_pNextLevel = new CPrescription(pPlan, nAtLevel);

			pPresc = pPresc->m_pNextLevel;
			pPresc->SetGBinSigma(GBinSigma / atLevelSigma[nAtLevel]);

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
	REAL binWidth = R(0.0030625);  
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

/*
	// set up tolerances
	if (m_nLevel == 0)
	{
		REAL sumWeight = 0.0;
		POSITION pos = m_mapVOITs.GetStartPosition();
		while (pos != NULL)
		{
			CStructure *pStruct = NULL;
			CVOITerm *pVOIT = NULL;
			m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
			sumWeight += pVOIT->GetWeight();
		}

		REAL tol[3];
		tol[0] = GetProfileReal("Prescription", "Tolerance0", 1e-3) * sumWeight;
		tol[1] = GetProfileReal("Prescription", "Tolerance1", 1e-3) * sumWeight;
		tol[2] = GetProfileReal("Prescription", "Tolerance2", 1e-3) * sumWeight;

		CPrescription *pPresc = this;
		pPresc->m_tolerance = tol[0];
		for (int nAtLevel = 1; nAtLevel < MAX_SCALES; nAtLevel++)
		{
			pPresc = pPresc->m_pNextLevel;
			pPresc->m_tolerance = tol[nAtLevel];
		}
	}
*/
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

	// TODO: use CObjectiveFunction::Transform to do this (or should optimizer do transform?)
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		ASSERT(_finite(vInit[nAt]));
		vInit[nAt] = InvSigmoid(vInit[nAt], m_inputScale);
		ASSERT(_finite(vInit[nAt]));
	}

	// calculate tolerance for this level

	// begin by summing weights of terms
	REAL totalWeight = 0.0;
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);
		totalWeight += pVOIT->GetWeight();
	}

	m_pOptimizer->SetTolerance(m_tolerance * totalWeight);
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

	// compute final state vector (add a small amount if zero)
	// TODO: use Transform to do this
	for (int nAt = 0; nAt < vRes.GetDim(); nAt++)
	{
		vInit[nAt] = R(__max(Sigmoid<double>(vRes[nAt], m_inputScale), 1e-4));
	}

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
								   const CVectorN<>& vInput, 
								   const CArray<BOOL, BOOL>& arrInclude) const
{
	// get the main volume
	CVolume<VOXEL_REAL> *pVolume = pHisto->GetVolume();
	ASSERT(pVolume == &m_sumVolume);
	pVolume->ClearVoxels();

	// iterate over the component volumes, accumulating the weighted volumes
	ASSERT(vInput.GetDim() == pHisto->Get_dVolumeCount());

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
					// TODO: use Transform instead to calc sigmoid
					const REAL weight = Sigmoid(vInput[nAt_dVolume], m_inputScale);
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
			CalcSumSigmoid(pVOIT->GetHistogram(), vInput, m_arrIncludeElement);
			bCalcSum = false;
		}

		if (pGrad)
		{
			m_vPartGrad.SetDim(vInput.GetDim());
			m_vPartGrad.SetZero();

			totalSum += pVOIT->Eval(&m_vPartGrad, m_arrIncludeElement);

			// apply the chain rule for the sigmoid
			for (int nAt = 0; nAt < m_vPartGrad.GetDim(); nAt++)
			{
				// TODO: use dTransform to do this
				m_vPartGrad[nAt] *= dSigmoid(vInput[nAt], m_inputScale);
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

	static bool bCalcEntropy = true;
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
// #define RAW_ENTROPY_XXX
#ifdef RAW_ENTROPY_XXX

	double sum = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		// TODO: use Transform instead of Sigmoid
		sum += Sigmoid(vInput[nAt], m_inputScale);
	}

	if (sum == 0.0)
	{
		if (pGrad)
		{
			pGrad->SetZero();
		}

		return 0.0;
	}

	const double EPS = 1e-3;

	// to accumulate total entropy
	double totalEntropy = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		// p is probability of intensity of this beamlet
		// TODO: use Transform instead of Sigmoid
		REAL p_beamlet = R(Sigmoid<double>(vInput[nAt], m_inputScale) / sum);

		// contribution to total is entropy of the probability
		totalEntropy += - p_beamlet * log(p_beamlet + EPS);
	}
	totalEntropy /= ((double) vInput.GetDim() 
		* -log(1.0 / (double) vInput.GetDim()));


	if (pGrad)
	{
		// do we evaluate gradient?
		pGrad->SetDim(vInput.GetDim());
		pGrad->SetZero();

		for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
		{
			// don't need this (zeroed above)
			for (int nAt2 = 0; nAt2 < vInput.GetDim(); nAt2++)
			{
				double p_beamlet2 = Sigmoid(vInput[nAt2], m_inputScale) / sum;

				(*pGrad)[nAt] += R(m_totalEntropyWeight * 
					-(p_beamlet2 / (p_beamlet2 + EPS) 
								+ log(p_beamlet2 + EPS)) 
						// TODO: use Transform instead of Sigmoid
						* (-Sigmoid<double>(vInput[nAt2], m_inputScale) + ((nAt2 == nAt) ? sum : 0.0))
							/ (sum * sum)
						* dSigmoid<double>(vInput[nAt], m_inputScale));
			}

			(*pGrad)[nAt] /= R((double) vInput.GetDim() 
				* -log(1.0 / (double) vInput.GetDim()));
		}
	}

	return m_totalEntropyWeight * (REAL) totalEntropy 
		- m_intensityMapSumWeight * (REAL) sum / (REAL) m_pPlan->GetBeamCount();

#else

	m_vBeamWeights.SetDim(m_pPlan->GetBeamCount());
	m_vBeamWeights.SetZero();

	double sum = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

		// TODO: use Transform instead of Sigmoid
		double beamWeight = Sigmoid(vInput[nAt], m_inputScale);
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
	// totalEntropy *= 100.0;


	// do we evaluate gradient?
	if (pGrad)
	{
		pGrad->SetZero();

		for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
		{
			int nBeam;
			int nBeamlet;
			GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

			// p is probability of intensity of this beamlet
			// double p_beam = m_vBeamWeights[nBeam] / sum;

#if !defined(_OLD_ENTROPY)

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
						// TODO: use Transform instead of Sigmoid
						* dSigmoid(vInput[nAt], m_inputScale));
			}

			// now normalize entropy
			(*pGrad)[nAt]
				/= R(((double) m_vBeamWeights.GetDim() 
					* -log(1.0 / (double) m_vBeamWeights.GetDim())));

#ifdef BEAMLET_BEAM_ENTROPY
			double p_beamlet = wgts[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet] / beamSum[nAtBeam];
			entrBeamlets += ENTR_BEAMLET_WEIGHT * -p_beamlet * log(p_beamlet + ZETA);

			d_entrBeamlets_Beamlet[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet] = 0.0;
			for (int nAtBeamlet2 = 0; nAtBeamlet2 < BEAMLETS_PER_BEAM; nAtBeamlet2++)
			{
				double p_beamlet2 = wgts[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet2] / beamSum[nAtBeam];

				d_entrBeamlets_Beamlet[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet] += 
					ENTR_BEAMLET_WEIGHT 
						* -((p_beamlet2 / (p_beamlet2 + ZETA)) 
							+ log(p_beamlet2 + ZETA)) 
						* (-wgts[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet2] + ((nAtBeamlet2 == nAtBeamlet) ? beamSum[nAtBeam] : 0.0))
							/ (beamSum[nAtBeam] * beamSum[nAtBeam]);
			}
			d_entrBeamlets_Beamlet[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet] /= 
				(double) (STATE_DIM); 
			// entrBeamlets /= (double) (STATE_DIM);
#endif
#else

			// derivative of prob is found by applying ratio rule
			// TODO: use Transform instead of Sigmoid
			double d_p = dSigmoid(vInput[nAt], m_inputScale) 
				* (sum - m_vBeamWeights[nBeam]) 
					/ (sum * sum);

			// apply chain rule to compute the derivative
			(*pGrad)[nAt] += m_totalEntropyWeight * -d_p * (1.0 + log(p + EPS));

#endif
			(*pGrad)[nAt] -= m_intensityMapSumWeight * dSigmoid(vInput[nAt], m_inputScale) 
				/ ((REAL) vInput.GetDim());
		}
	}

	// return total entropy scaled by number of beamlets
	return R(m_totalEntropyWeight * totalEntropy 
		- m_intensityMapSumWeight * sum / ((REAL) vInput.GetDim()));

#endif // !RAW_ENTROPY

}	// CPrescription::Eval_TotalEntropy


///////////////////////////////////////////////////////////////////////////////
// CPrescription::Transform
// 
// transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::Transform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = Sigmoid((*pvInOut)[nAt], m_inputScale));

}	// CPrescription::Transform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::dTransform
// 
// derivative transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::dTransform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = dSigmoid((*pvInOut)[nAt], m_inputScale));

}	// CPrescription::dTransform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::InvTransform
// 
// inverse transform function from linear to sigmoid parameter space
///////////////////////////////////////////////////////////////////////////////
void CPrescription::InvTransform(CVectorN<> *pvInOut) const
{
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = InvSigmoid((*pvInOut)[nAt], m_inputScale));

}	// CPrescription::InvTransform

///////////////////////////////////////////////////////////////////////////////
// CPrescription::GetInitStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPrescription::GetInitStateVector(CVectorN<>&vInit)
{
	CPrescription *pLevel2 = this;
	while (pLevel2->m_nLevel < 2)
		pLevel2 = pLevel2->m_pNextLevel;

	vInit.SetDim(m_pPlan->GetTotalBeamletCount(2));
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		vInit[nAt] = (pLevel2->m_arrIncludeElement[nAt]) ? R(0.001) : R(0.001) 
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
