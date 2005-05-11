// Prescription.cpp: implementation of the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Prescription.h"

#include <ConjGradOptimizer.h>

#include <iostream>

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
		m_totalEntropyWeight((REAL) 0.025),
		m_intensityMapSumWeight((REAL) 0.00),
		m_inputScale((REAL) 0.5)
{
	m_pOptimizer = new CConjGradOptimizer(this);

	if (m_nLevel == 0)
	{
		REAL GBinSigma = (REAL) 0.20;
		int nAtSigma[] = {4, 2, 1};
		REAL tol[] = 
			// {(REAL) 1e-4, (REAL) 5e-3, (REAL) 1e-3};
			// {(REAL) 1e-3, (REAL) 1e-3, (REAL) 1e-3};
			//  {(REAL) 1e-3, (REAL) 1e-4, (REAL) 1e-4};
			{(REAL) 1e-7, (REAL) 1e-7, (REAL) 1e-7};
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
// CPrescription::CalcSumSigmoid
// 
// computes the sum of weights from an input vector
///////////////////////////////////////////////////////////////////////////////
void CPrescription::CalcSumSigmoid(CHistogram *pHisto, 
								   const CVectorN<>& vInput, 
								   const CArray<BOOL, BOOL>& arrInclude) const
{
	// get the main volume
	CVolume<REAL> *pVolume = pHisto->GetVolume();
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
			CVolume<REAL> *p_dVolume = pHisto->Get_dVolume(nAt_dVolume, &nGroup);

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
					REAL weight = Sigmoid(vInput[nAt_dVolume], m_inputScale);
					m_volGroup.Accumulate(p_dVolume, 
						weight, TRUE);
				}
			}
		}

		// now rotate the groups sum to the main sumVolume basis
		m_volGroupMain.ConformTo(pVolume);
		m_volGroupMain.ClearVoxels();
		Resample(&m_volGroup, &m_volGroupMain, // FALSE); 
			TRUE);

		// and accumulate
		pVolume->Accumulate(&m_volGroupMain, 1.0, FALSE);
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
	m_vBeamWeights.SetDim(m_pPlan->GetBeamCount());
	m_vBeamWeights.SetZero();

	double sum = 0.0;
	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAt, m_nLevel, &nBeam, &nBeamlet);

		double beamWeight = Sigmoid(vInput[nAt], m_inputScale);
		m_vBeamWeights[nBeam] += beamWeight;
		sum += beamWeight;
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
	totalEntropy /= (double) (m_vBeamWeights.GetDim());


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
			double p_beam = m_vBeamWeights[nBeam] / sum;

#if !defined(_OLD_ENTROPY)

			// don't need this (zeroed above)
			for (int nAtBeam2 = 0; nAtBeam2 < m_vBeamWeights.GetDim(); nAtBeam2++)
			{
				double p_beam2 = m_vBeamWeights[nAtBeam2] / sum;

				(*pGrad)[nAt] += 
					m_totalEntropyWeight 
						* -((p_beam2 / (p_beam2 + EPS)) 
								+ log(p_beam2 + EPS)) 
						* (-m_vBeamWeights[nAtBeam2] + ((nAtBeam2 == nBeam) ? sum : 0.0))
							/ (sum * sum)
						* dSigmoid(vInput[nAt], m_inputScale);
			}
			(*pGrad)[nAt] /= (double) (m_vBeamWeights.GetDim());

/*			double p_beamlet = wgts[nAtBeam * BEAMLETS_PER_BEAM + nAtBeamlet] / beamSum[nAtBeam];
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
				(double) (STATE_DIM); */
			// entrBeamlets /= (double) (STATE_DIM);

#else

			// derivative of prob is found by applying ratio rule
			double d_p = dSigmoid(vInput[nAt], m_inputScale) 
				* (sum - m_vBeamWeights[nBeam]) 
					/ (sum * sum);

			// apply chain rule to compute the derivative
			(*pGrad)[nAt] += m_totalEntropyWeight * -d_p * (1.0 + log(p + EPS));

#endif
			(*pGrad)[nAt] -= m_intensityMapSumWeight * dSigmoid(vInput[nAt], m_inputScale) 
				/ (REAL) m_vBeamWeights.GetDim();
		}
	}


	// TRACE("totalEntropyWeight = %f\n", m_totalEntropyWeight);
	// TRACE("totalEntropy = %f\n", totalEntropy);

	// return total entropy scaled by number of beamlets
	return m_totalEntropyWeight * totalEntropy 
		- m_intensityMapSumWeight * sum / (REAL) m_vBeamWeights.GetDim();;

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

	static bool bCalcOnlyEntropy = false;
	POSITION pos = bCalcOnlyEntropy ? NULL : m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

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

#if defined(_DEBUG) // && defined(_TEST_GRADIENT)

	static BOOL bTesting = FALSE;
	if (pGrad && !bTesting)
	{
		bTesting = TRUE;
		bCalcOnlyEntropy = TRUE;

		double tempTEW = m_totalEntropyWeight;
		// const_cast<REAL&>(m_totalEntropyWeight) = 0.0;
		double tempIMSW = m_intensityMapSumWeight;
		const_cast<REAL&>(m_intensityMapSumWeight) = 0.0;

/*		double delta = 1e-4;
		for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
		{
			double start = Sigmoid(vInput[nAt], m_inputScale);
			double end = Sigmoid(vInput[nAt] + delta, m_inputScale);

			double del = (end - start) / delta;

			double eval = dSigmoid(vInput[nAt], m_inputScale);
			TRACE("%6.4lf / %6.4lf, ", eval, del);
		}
		TRACE("\n"); */

		TestGradient(vInput, 1e-5);

		const_cast<REAL&>(m_totalEntropyWeight) = tempTEW;
		const_cast<REAL&>(m_intensityMapSumWeight) = tempIMSW;

		bTesting = FALSE;
		bCalcOnlyEntropy = FALSE;
	}
#endif

	END_LOG_SECTION();	// CPrescription::operator()

	// TRACE("objFunc = %f\n", totalSum);

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

	// initialize the sum volume, so as to coincide with the beamlets
	CVolume<REAL> *pBeamlet = m_pPlan->GetBeamAt(m_pPlan->GetBeamCount()-1)->GetBeamlet(0, m_nLevel);
	m_sumVolume.ConformTo(pBeamlet);

	// initialize the histogram region
	// TODO: fix this memory leak
	CVolume<REAL> *pResampRegion = new CVolume<REAL>(); // pVOIT->GetVOI()->GetRegion(m_nLevel);
	pResampRegion->ConformTo(&m_sumVolume);
	pResampRegion->ClearVoxels();

	int nLevel = -1;
	CVectorD<3> vSumPixelSpacing = m_sumVolume.GetPixelSpacing();
	CVectorD<3> vRegionPixelSpacing;
	do
	{
		nLevel++;
		vRegionPixelSpacing = pVOIT->GetVOI()->GetRegion(nLevel)->GetPixelSpacing();
	} while (
		vRegionPixelSpacing[0] * 2.0 < vSumPixelSpacing[0]
		|| vRegionPixelSpacing[1] * 2.0 < vSumPixelSpacing[1]);


	CVolume<REAL> *pVOITRegion = pVOIT->GetVOI()->GetRegion(nLevel);
	::Resample(pVOIT->GetVOI()->GetRegion(nLevel), pResampRegion, TRUE);

	// set histogram options
	CHistogram *pHisto = pVOIT->GetHistogram();
	pHisto->SetGBinSigma(m_GBinSigma);
	pHisto->SetVolume(&m_sumVolume);
	pHisto->SetRegion(pResampRegion);

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
		// ASSERT(pBeamlet->GetBasis().IsApproxEqual(pResampRegion->GetBasis()));

		pBeamlet->SetThreshold((REAL) 0.005 /* 0.005 */ ); // pow(10, -(m_nLevel+1)));
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
// CPrescription::InvFilterStateVector
// 
// <description>
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

void CPrescription::SetElementInclude()
{
	// iterate over terms to find target term
	POSITION pos = m_mapVOITs.GetStartPosition();
	while (pos != NULL)
	{
		CStructure *pStruct = NULL;
		CVOITerm *pVOIT = NULL;
		m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		// check type
		CKLDivTerm *pKLDivTerm = (CKLDivTerm *) pVOIT;

		CHistogram *pHisto = pKLDivTerm->GetHistogram();

		if (m_arrIncludeElement.GetSize() < pHisto->Get_dVolumeCount())
		{
			m_arrIncludeElement.SetSize(pHisto->Get_dVolumeCount());

			for (int nAt = 0; nAt < m_arrIncludeElement.GetSize(); nAt++)
			{
				m_arrIncludeElement[nAt] = FALSE;
			}
		}

		const CVectorN<>& vBins = pKLDivTerm->GetTargetBins();
		// const CVectorN<>& vBinMeans = pHisto->GetBinMeans();

		REAL binMean = pHisto->GetBinMinValue();
		REAL expect = 0.0;
		REAL sum = 0.0;
		for (int nAt = 0; nAt < vBins.GetDim(); nAt++)
		{
			expect += binMean * vBins[nAt];
			sum += vBins[nAt];
			binMean += pHisto->GetBinWidth();
		}

		// calculate expectation
		REAL targetMean = expect / sum;

		// check if its a target
		if (targetMean > 0.30)
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
}
