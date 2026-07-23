// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanOptimizer.cpp 647 2009-11-05 21:52:59Z dglane001 $
#include "StdAfx.h"
#include "PlanOptimizer.h"

#include <ConjGradOptimizer.h>


namespace dH
{

///////////////////////////////////////////////////////////////////////////////

// section name for settings access
const char * const REG_KEY		= "Prescription";

///////////////////////////////////////////////////////////////////////////////
/// TODO: move this to utils
REAL
	GetProfileRealAt(const char *pszKey, int nAt, REAL defaultValue)
{
	char szKeyAt[128];
	snprintf(szKeyAt, sizeof(szKeyAt), pszKey, nAt);
	return GetProfileReal(REG_KEY, szKeyAt, defaultValue);
}

///////////////////////////////////////////////////////////////////////////////

// other constants + registry keys

const char * const GBINSIGMA_KEY	= "GBinSigma";
const REAL DEFAULT_GBINSIGMA	= 0.2;

const char * const LEVELSIGMA_KEY	= "LevelSigma%i";
const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25}; // { 8.0, 4.0, 2.0, 1.0, 0.5 };

const char * const CGTOL_KEY		= "CGTolerance%i";
const char * const LINETOL_KEY		= "Tolerance%i";

// Per-level convergence tolerances, indexed by nLevel. IMPORTANT: level 0 is
//	the FINEST resolution (0.5mm) and level MAX_SCALES-1 is the COARSEST (8mm)
//	-- see PlanPyramid, where m_arrPlans[0] is the base plan and each higher
//	index doubles the dose resolution. So index 0 must hold the tightest
//	tolerance: the finest level is optimized last and determines final plan
//	quality, so it must be the most thoroughly converged, while the coarse
//	levels only warm-start and can stop early. (A previous version had these
//	reversed -- {1e-3,...,1e-6} -- which gave the finest level the LOOSEST
//	tolerance and made it converge prematurely, the opposite of the intent.)
const REAL DEFAULT_CG_TOLERANCE[]	= {1e-6, 1e-5, 1e-4, 1e-3, 1e-3};
const REAL DEFAULT_LINE_TOLERANCE[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-3};



///////////////////////////////////////////////////////////////////////////////
PlanOptimizer::PlanOptimizer(CPlan *pPlan)
	: m_pPlan(pPlan)
{
	SetupPrescription();
}

///////////////////////////////////////////////////////////////////////////////
PlanOptimizer::~PlanOptimizer(void)
{
	for (int nAt = 0; nAt < m_arrPrescriptions.size(); nAt++)
	{
		delete m_arrPrescriptions[nAt].first;
		delete m_arrPrescriptions[nAt].second;
	}

	delete m_pPyramid;

	/// TODO: delete the plan???
}

///////////////////////////////////////////////////////////////////////////////
Prescription *
	PlanOptimizer::GetPrescription(int nLevel)
	// returns the given level of the pyramid
{
	return m_arrPrescriptions[nLevel].first;

}	// PlanOptimizer::GetPrescription

///////////////////////////////////////////////////////////////////////////////
DynamicCovarianceOptimizer *
	PlanOptimizer::GetOptimizer(int nLevel)
	// returns the given level of the pyramid
{
	return m_arrPrescriptions[nLevel].second;

}	// PlanOptimizer::GetOptimizer

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::AddStructureTerm(VOITerm *pST)
{
	// these need to be generated first, before the call to AddStructureTerm
	// GetPyramid()->CalcPencilSubBeamlets();

	for (int nLevel = 0; nLevel < PlanPyramid::MAX_SCALES; nLevel++)
	{
		GetPrescription(nLevel)->AddStructureTerm(pST);
		pST = pST->Clone();
	}

}	// PlanOptimizer::AddStructureTerm

///////////////////////////////////////////////////////////////////////////////
static void
	RunGradCheck(dH::Prescription *pPresc, const CVectorN<>& x0)
	// finite-difference gradient check of the objective F = KL - w*entropy at
	//	x0. Gated by BRIMSTONE_GRADCHECK (= output file path); writes the
	//	norm-wise relative error between the analytic gradient returned by
	//	operator() and a central-difference numerical gradient. Validates both
	//	the pre-existing KL gradient (run with entropy weight w=0) and the added
	//	softmax-entropy gradient (w>0). Inert unless the env var is set.
{
	const char *pFile = getenv("BRIMSTONE_GRADCHECK");
	if (pFile == NULL)
		return;

	const int n = x0.GetDim();

	// x0 is the real (inv-filtered) operating point for this level, so it has an
	//	active dose distribution: KL and its gradient are non-zero, and the
	//	optimized weights are non-uniform so the softmax-entropy gradient is
	//	non-zero too (dH/dx is exactly 0 only at a perfectly uniform x). Gradient
	//	correctness is point-independent, so this fully exercises the analytic
	//	gradient of both terms.
	CVectorN<> x0v = x0;

	// analytic gradient at the test point (also captures the KL/entropy split)
	CVectorN<> gAna;
	const REAL f0 = (*pPresc)(x0v, &gAna);
	const REAL kl0 = pPresc->GetLastKL();
	const REAL H0 = pPresc->GetLastEntropy();

	// central-difference numerical gradient. xp/xm are copy-constructed from x0
	//	so they carry x0's dimension and values; perturb one component, evaluate,
	//	then restore it (a fresh CVectorN + operator= would NOT resize, leaving a
	//	dim-0 vector and an out-of-bounds write).
	const REAL eps = (REAL) 1e-5;
	double diff2 = 0.0, ana2 = 0.0, num2 = 0.0, maxabs = 0.0;
	CVectorN<> xp(x0v), xm(x0v);
	for (int i = 0; i < n; i++)
	{
		xp[i] = x0v[i] + eps;
		xm[i] = x0v[i] - eps;
		const REAL fp = (*pPresc)(xp, NULL);
		const REAL fm = (*pPresc)(xm, NULL);
		xp[i] = x0v[i];		// restore for the next iteration
		xm[i] = x0v[i];
		const REAL gi = (fp - fm) / (2 * eps);
		const double d = (double) gAna[i] - (double) gi;
		diff2 += d * d;
		ana2 += (double) gAna[i] * (double) gAna[i];
		num2 += (double) gi * (double) gi;
		if (fabs(d) > maxabs) maxabs = fabs(d);
	}
	const double normwise = sqrt(diff2) / (sqrt(ana2) + sqrt(num2) + 1e-30);

	FILE *fpOut = NULL;
	if (fopen_s(&fpOut, pFile, "w") == 0 && fpOut != NULL)
	{
		fprintf(fpOut, "n=%d w=%.6g f0=%.10g KL=%.10g H=%.10g "
			"normwise_rel_err=%.6e max_abs_err=%.6e ana_norm=%.6g num_norm=%.6g\n",
			n, (double) pPresc->GetEntropyWeight(), (double) f0,
			(double) kl0, (double) H0,
			normwise, maxabs, sqrt(ana2), sqrt(num2));
		fclose(fpOut);
	}
}	// RunGradCheck

///////////////////////////////////////////////////////////////////////////////
bool
	PlanOptimizer::Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam)
	// performs multi-level optimization
{
	// make sure pencil subbeamlets are properly generated
	GetPyramid()->CalcPencilSubBeamlets();

	// make sure prescription terms are synched
	for (int nLevel = 1; nLevel < m_arrPrescriptions.size(); nLevel++)
		GetPrescription(nLevel)->UpdateTerms(GetPrescription(0));

	// compute the starting point
	GetInitStateVector(vInit);

	for (int nLevel = m_arrPrescriptions.size()-1; nLevel >= 0; nLevel--)
	{
		dH::Prescription *pPresc = GetPrescription(nLevel);
		DynamicCovarianceOptimizer *pOpt = GetOptimizer(nLevel);

		// update the histogram regions
		pPresc->UpdateHistogramRegions();

		// set the callback
		pOpt->SetCallback(pFunc, pParam);

		// inverse transform the initial vector to form initial optimizer parameter
		pPresc->InvTransform(&vInit);

		// optional finite-difference gradient check. Inert unless
		//	BRIMSTONE_GRADCHECK is set. Done at the finest level (0), whose
		//	initial vector is the inv-filtered result from the coarser levels --
		//	a real operating point with an active dose distribution, so the
		//	objective and its gradient are non-degenerate (the coarsest level's
		//	standalone evaluation returns 0 here). Initialize the adaptive
		//	variance first so operator() has a valid AV vector.
		if (nLevel == 0)
		{
			pOpt->PrepareAdaptiveVariance(vInit.GetDim());
			RunGradCheck(pPresc, vInit);
		}

		// optional per-level diagnostic: at each level's starting point log the
		//	objective, gradient norm, sigmoid-derivative (chain-rule) norm and the
		//	dose max, to see why the coarse levels are stationary. Inert unless
		//	BRIMSTONE_LEVEL_DIAG is set.
		if (getenv("BRIMSTONE_LEVEL_DIAG") != NULL)
		{
			pOpt->PrepareAdaptiveVariance(vInit.GetDim());
			CVectorN<> gDiag;
			const REAL fDiag = (*pPresc)(vInit, &gDiag);
			CVectorN<> dtDiag = vInit;
			pPresc->dTransform(&dtDiag);
			const REAL doseMax = GetMax<VOXEL_REAL>(pPresc->m_sumVolume);
			RTM_TRACE("LEVELDIAG dim=%d F=%.6g KL=%.6g |gradF|=%.6g |dTransform|=%.6g doseMax=%.6g\n",
				vInit.GetDim(), (double) fDiag, (double) pPresc->GetLastKL(),
				(double) gDiag.GetLength(), (double) dtDiag.GetLength(), (double) doseMax);
		}

		// NOTE: this needs to be in the form of an initializer,
		//	or else SetDim needs to be called for vRes before the call
		// CVectorN<> vRes = pOpt->Optimize(vInit);
		pOpt->minimize(vInit.GetVnlVector());
		CVectorN<> vRes = vInit;

		// check for problem with optimization
		if (pOpt->get_num_iterations() == -1)
		{
			return false;
		}

		// compute final state vector
		pPresc->Transform(&vRes);

		// if we are not at the last level,
		if (nLevel > 0)
		{
			// then inverse filter to the next level
			InvFilterStateVector(nLevel, vRes, vInit);
		}
		else
		{
			// set the final (for output)
			vInit = vRes;
		}
	}

	return true;

}	// PlanOptimizer::Optimize

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::GetInitStateVector(CVectorN<>&vInit)
{
	Prescription *pLevelMax = GetPrescription(PlanPyramid::MAX_SCALES-1);

	vInit.SetDim(GetPyramid()->GetPlan(PlanPyramid::MAX_SCALES-1)->GetTotalBeamletCount());
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		if (pLevelMax->m_arrIncludeElement[nAt])
		{
			vInit[nAt] = R(0.001);
#ifdef ADD_RANDOM_INIT
			vInit[nAt] += R(0.01)*R(rand())/R(RAND_MAX);
#endif
		}
		else
		{
			vInit[nAt] = R(0.001) / R(GetPlan()->GetBeamCount());
			RTM_TRACE("Excluded element %d\n", nAt);
		}
	}

}	// PlanOptimizer::GetInitStateVector

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::GetStateVectorFromPlan(CVectorN<>& vState)
{
	//CMatrixNxM<> mBeamletWeights(GetPlan()->GetBeamCount(), 
	//	GetPlan()->GetBeamAt(0)->GetBeamletCount());
	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		CBeam::IntensityMap *pIM = GetPlan()->GetBeamAt(nAtBeam)->GetIntensityMap();
		IntensityMapToStateVector(0, nAtBeam, pIM, vState);
		//for (int nAt = 0; nAt < pIM->GetBufferedRegion().GetSize()[0]; nAt++)
		//	mBeamletWeights[nAtBeam][nAt] = pIM->GetBufferPointer()[nAt];
	}
	//BeamletWeightsToStateVector(0, mBeamletWeights, vState);

}	// PlanOptimizer::GetStateVectorFromPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::SetStateVectorToPlan(const CVectorN<>& vState)
{
	//CMatrixNxM<> mBeamletWeights;
	//StateVectorToBeamletWeights(0, vState, mBeamletWeights);

	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		StateVectorToIntensityMap(0, nAtBeam, vState, 
			GetPlan()->GetBeamAt(nAtBeam)->GetIntensityMap());
		GetPlan()->GetBeamAt(nAtBeam)->OnIntensityMapChanged();
		//GetPlan()->GetBeamAt(nAtBeam)->SetIntensityMap(mBeamletWeights[nAtBeam]);
	}

	// now iterate through histograms, updating
	GetPlan()->UpdateAllHisto();

}	// PlanOptimizer::SetStateVectorToPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::InvFilterStateVector(int nScale, const CVectorN<>& vIn, CVectorN<>& vOut)
	// transfers beamlet weights from level n-1 to level n
{
	assert(&vIn != &vOut);
	// TODO: make a member so don't have to re-initialize
	//CMatrixNxM<> mBeamletWeights;
	//StateVectorToBeamletWeights(nScale, vIn, mBeamletWeights);

	//CMatrixNxM<> mFiltBeamletWeights(mBeamletWeights.GetCols(),	
	//	GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetBeamletCount());

	CBeam::IntensityMap::Pointer intMapIn = CBeam::IntensityMap::New();
	ConformTo<VOXEL_REAL, 1>(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap(), intMapIn);
	//intMapIn->CopyInformation(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap());
	//intMapIn->SetBufferedRegion( GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap()->GetBufferedRegion() );
	//intMapIn->SetRegions(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetIntensityMap()->GetB);
	//intMapIn->SetRegions(MakeSize(GetPyramid()->GetPlan(nScale)->GetBeamAt(0)->GetBeamletCount()));
	//intMapIn->Allocate();

	CBeam::IntensityMap::Pointer intMapOut = CBeam::IntensityMap::New();
	ConformTo<VOXEL_REAL, 1>(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetIntensityMap(), intMapOut);
	//intMapOut->CopyInformation(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetIntensityMap());
	//intMapOut->SetRegions(MakeSize(GetPyramid()->GetPlan(nScale-1)->GetBeamAt(0)->GetBeamletCount()));
	//intMapOut->Allocate();

	for (int nAtBeam = 0; nAtBeam < GetPlan()->GetBeamCount(); nAtBeam++)
	{
		StateVectorToIntensityMap(nScale, nAtBeam, vIn, intMapIn); 
		//for (int nAt = 0; nAt < intMapIn->GetBufferedRegion().GetSize()[0]; nAt++)
		//	intMapIn->GetBufferPointer()[nAt] = mBeamletWeights[nAtBeam][nAt];

		GetPyramid()->InvFiltIntensityMap(
			nScale,
			intMapIn, // mBeamletWeights[nAtBeam],
			intMapOut); // mFiltBeamletWeights[nAtBeam]);

		IntensityMapToStateVector(nScale-1, nAtBeam, intMapOut, vOut);
		//for (int nAt = 0; nAt < intMapOut->GetBufferedRegion().GetSize()[0]; nAt++)
		//	mFiltBeamletWeights[nAtBeam][nAt] = intMapOut->GetBufferPointer()[nAt];
	} 

	//BeamletWeightsToStateVector(nScale-1, mFiltBeamletWeights, vOut); 

}	// PlanOptimizer::InvFilterStateVector


///////////////////////////////////////////////////////////////////////////////
void
	PlanOptimizer::SetupPrescription()
	// returns the given level of the pyramid
{

	// create the pyramid
	SetPyramid(new dH::PlanPyramid(GetPlan()));

	// get main sigma parameter from registry
	const REAL GBinSigma = GetProfileReal(REG_KEY, GBINSIGMA_KEY, DEFAULT_GBINSIGMA);

	// form vector with levels of the presc object
	m_arrPrescriptions.clear();
	for (int nLevel = 0; nLevel < PlanPyramid::MAX_SCALES; nLevel++)
	{
		// create a new prescription object
		Prescription * pPresc = new dH::Prescription(GetPlan());
		pPresc->SetPlan(GetPyramid()->GetPlan(nLevel));		// TODO: why is this called here?

		/// TODO: set up slice
		///		or just in Presc::AddStructTerm

		// calculate the variance range for this level
		const REAL sigma = GetProfileRealAt(LEVELSIGMA_KEY, nLevel, DEFAULT_LEVELSIGMA[nLevel]);
		const REAL binVar = pow(GBinSigma / sigma, 2);
		const REAL varMin = binVar * 0.25;
		// widened to cover the true peak of varSlope^2*varWeight^2 (~1.405x at S=2/3,
		//	see Prescription::CalcSumSigmoid), so the actVar clamp isn't discarding
		//	routine excursions above binVar
		const REAL varMax = binVar * 1.5;

		// set the variance range in the objective function
		// NOTE: this has to be done after the Optimizer->SetAdaptiveVariance, because
		//		it will over-ride some of those settings
		// pPresc->SetGBinVar(varMin, varMax);

		// construct the optimizer
		DynamicCovarianceOptimizer *pOptimizer = new DynamicCovarianceOptimizer(pPresc);

		// set the variance range for the optimizer
		pOptimizer->SetAdaptiveVariance(true, varMin, varMax);

		// NOTE: the search-direction-covariance "free energy" diagnostic
		//	(SetComputeFreeEnergy) is intentionally left OFF here. The objective
		//	F = KL - w*softmax-entropy is now formed inside Prescription::operator()
		//	with a true per-parameter entropy gradient, so it actually steers the
		//	optimization; the covariance-entropy diagnostic is a different quantity
		//	and would only waste an eigendecomposition per iteration.

		// set the tolerances
		//pOptimizer->SetLineToleranceEqual(false);

		// set the line tolerance
		const REAL cgTolerance = GetProfileRealAt(CGTOL_KEY, nLevel, DEFAULT_CG_TOLERANCE[nLevel]);
		pOptimizer->set_x_tolerance/*SetTolerance*/(cgTolerance);

		// set the CG tolerance
		const REAL lineTolerance = GetProfileRealAt(LINETOL_KEY, nLevel, DEFAULT_LINE_TOLERANCE[nLevel]);
		// pOptimizer->GetBrentOptimizer().set_x_tolerance(lineTolerance);
		pOptimizer->SetLineOptimizerTolerance(lineTolerance);

		// do not apply transform slope variance for lowest-res level
		if (nLevel == PlanPyramid::MAX_SCALES-1)
			pPresc->SetTransformSlopeVariance(false);

		// set the variance range in the objective function
		// NOTE: this has to be done after the Optimizer->SetAdaptiveVariance, because
		//		it will over-ride some of those settings
		pPresc->SetGBinVar(varMin, varMax);

		m_arrPrescriptions.push_back(std::pair<Prescription*, DynamicCovarianceOptimizer*>(pPresc, pOptimizer));
	}

}	// PlanOptimizer::SetupPrescription


///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::StateVectorToIntensityMap(int nScale, int nBeam,
		const CVectorN<>& vState, 
		CBeam::IntensityMap *pIntensityMap) // CMatrixNxM<>& mBeamletWeights)
{
	CPlan *pPlan = GetPyramid()->GetPlan(nScale);
	int nBeamletCount = pPlan->GetBeamAt(nBeam/*0*/)->GetBeamletCount();
	int nBeamletOffset = nBeamletCount / 2;
	ConformTo<VOXEL_REAL>(pPlan->GetBeamAt(nBeam)->GetIntensityMap(), 
		pIntensityMap);		// TODO: make this an ASSERT, as caller should be responsible for this
	assert(nBeamletCount == pIntensityMap->GetBufferedRegion().GetSize()[0]);

	for (int nAtElem = 0; nAtElem < vState.GetDim(); nAtElem++)
	{
		int nCurrBeam;
		int nCurrBeamlet;

		GetPrescription(nScale)->GetBeamletFromSVElem(nAtElem, &nCurrBeam, &nCurrBeamlet);
		if (nCurrBeam == nBeam)
		{
			pIntensityMap->GetBufferPointer()[nCurrBeamlet + nBeamletOffset] = vState[nAtElem];
		}
	}

}	// PlanOptimizer::StateVectorToBeamletWeights

///////////////////////////////////////////////////////////////////////////////
void 
	PlanOptimizer::IntensityMapToStateVector(int nScale, int nBeam,
			const CBeam::IntensityMap *pIntensityMap/*CMatrixNxM<>& mBeamletWeights */, CVectorN<>& vState)
{
	CPlan *pPlan = GetPyramid()->GetPlan(nScale);
	int nBeamletCount = pPlan->GetBeamAt(nBeam/*0*/)->GetBeamletCount();

	int nBeamletOffset = nBeamletCount / 2;

	if (vState.GetDim() != pPlan->GetTotalBeamletCount())
		vState.SetDim(pPlan->GetTotalBeamletCount());

	for (int nAtElem = 0; nAtElem < vState.GetDim(); nAtElem++)
	{
		int nCurrBeam;
		int nCurrBeamlet;

		GetPrescription(nScale)->GetBeamletFromSVElem(nAtElem, &nCurrBeam, &nCurrBeamlet);
		if (nCurrBeam == nBeam)
		{
			vState[nAtElem] = pIntensityMap->GetBufferPointer()[nCurrBeamlet + nBeamletOffset];
		}
	}

}	// PlanOptimizer::BeamletWeightsToStateVector

}	// namespace dH
