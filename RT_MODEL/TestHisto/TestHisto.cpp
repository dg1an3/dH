// TestHisto.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestHisto.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>

#include <MathUtil.h>

#include <VectorD.h>

#include <Histogram.h>
#include <HistogramMatcher.h>

#include <BrentOptimizer.h>
#include <LineFunction.h>

#include <PowellOptimizer.h>
#include <ConjGradOptimizer.h>

// #include "InvFilterEM.h"

#include "PlanIMRT.h"


const double MAX_DOSE = 0.65;

CSeries *g_pSeries = NULL;

CPlanIMRT *g_pPlan = NULL;

CVolume<double> *pRegionTarget = NULL;
CVolume<double> *pRegionAvoid = NULL;

CHistogramMatcher *g_pMatcher[] = { NULL, NULL, NULL };


///////////////////////////////////////////////////////////////////////////////
// InitPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitPlan()
{
	g_pSeries = new CSeries();

	g_pPlan = new CPlanIMRT();
	g_pPlan->SetSeries(g_pSeries);

	g_pPlan->SetBeamCount(7);


/*	g_pPlan->GetBeam(0)->SetGantryAngle( -25.0 * PI / -180.0);
	g_pPlan->GetBeam(1)->SetGantryAngle(  25.0 * PI / -180.0);
	g_pPlan->GetBeam(2)->SetGantryAngle( 155.0 * PI / -180.0);
	g_pPlan->GetBeam(3)->SetGantryAngle( 205.0 * PI / -180.0);
	g_pPlan->GetBeam(4)->SetGantryAngle(  90.0 * PI / -180.0);
	g_pPlan->GetBeam(5)->SetGantryAngle( 245.0 * PI / -180.0);
	g_pPlan->GetBeam(6)->SetGantryAngle( 295.0 * PI / -180.0); */

}	// InitPlan

///////////////////////////////////////////////////////////////////////////////
// InitUTarget
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitUTarget(CVolume<double> *pTarget, CVolume<double> *pAvoid)
{
	BEGIN_LOG_SECTION(InitUTarget);

	int nBeamletCount = 31;
	int nRegionSize = ((double) nBeamletCount / sqrt(2.0)) / 2 + 1;
	int nMargin = pTarget->GetWidth() / 2 - nRegionSize;

//	int nMargin = 4;
//	int nRegionSize = pTarget->GetWidth() / 2 - nMargin;

	double ***pppVoxelsTarget = pTarget->GetVoxels();
	double ***pppVoxelsAvoid = pAvoid->GetVoxels();
	for (int nAtRow = nMargin; nAtRow < pTarget->GetHeight()-nMargin; nAtRow++)
	{
		for (int nAtCol = nMargin; nAtCol < pTarget->GetWidth()-nMargin; nAtCol++)
		{
			if ((nAtCol - nMargin < nRegionSize - nRegionSize / 2) 
				|| (nAtCol - nMargin > nRegionSize + nRegionSize / 2)
				|| (nAtRow - nMargin > nRegionSize + nRegionSize / 2))
			{
				pppVoxelsTarget[0][nAtRow][nAtCol] = 1.0;
			}

			if ((nAtCol - nMargin > nRegionSize - nRegionSize / 2) 
				&& (nAtCol - nMargin < nRegionSize + nRegionSize / 2)
				&& (nAtRow - nMargin < nRegionSize + nRegionSize / 2))
			{
				pppVoxelsAvoid[0][nAtRow][nAtCol] = 1.0;
			}
		}
	}

	LOG_OBJECT((*pTarget));
	LOG_OBJECT((*pAvoid));

	END_LOG_SECTION();	// InitUTarget

}	// InitUTarget

///////////////////////////////////////////////////////////////////////////////
// InitVolumes
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitVolumes()
{
	BEGIN_LOG_SECTION(InitVolumes);

	CVolume<double> *pDose = g_pPlan->GetDoseMatrix(0); // 1);

	CStructure *pTargetStruct = new CStructure;
	g_pSeries->m_arrStructures.Add(pTargetStruct);

	pRegionTarget = pTargetStruct->GetRegion(0); // new CVolume<double>();
	pRegionTarget->SetDimensions(pDose->GetWidth(), pDose->GetHeight(), 1);
	pRegionTarget->ClearVoxels();

	CStructure *pAvoidStruct = new CStructure;
	g_pSeries->m_arrStructures.Add(pAvoidStruct);

	pRegionAvoid = pAvoidStruct->GetRegion(0); // new CVolume<double>();
	pRegionAvoid->SetDimensions(pDose->GetWidth(), pDose->GetHeight(), 1);
	pRegionAvoid->ClearVoxels();

	InitUTarget(pRegionTarget, pRegionAvoid);

	g_pPlan->AddRegion(pRegionTarget);
	g_pPlan->AddRegion(pRegionAvoid);

	REAL binWidth = 0.0125;
	for (int nScale = 0; nScale < MAX_SCALES; nScale++)
	{
		CHistogram *pHistoTarget = g_pPlan->GetRegionHisto(0, nScale);
		pHistoTarget->SetBinning(0.0, binWidth, 2.0);

		CHistogram *pHistoAvoid = g_pPlan->GetRegionHisto(1, nScale);
		pHistoAvoid->SetBinning(0.0, binWidth, 2.0);

		g_pMatcher[nScale] = new CHistogramMatcher(g_pPlan, nScale);

		long nTargetIndex = g_pMatcher[nScale]->AddHistogram(pHistoTarget, 10.0);
		g_pMatcher[nScale]->SetInterval(nTargetIndex, MAX_DOSE, 
			MAX_DOSE * 1.05, 1.0);

		long nAvoidIndex = g_pMatcher[nScale]->AddHistogram(pHistoAvoid, 1.0);
		g_pMatcher[nScale]->SetInterval(nAvoidIndex, 0.0, 0.30,
			// MAX_DOSE * 0.50, 
			1.0);

		binWidth *= 2.0;
	}

	END_LOG_SECTION();	// InitVolumes

}	// InitVolumes

///////////////////////////////////////////////////////////////////////////////
// FreeAll
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void FreeAll()
{
	delete g_pMatcher[0];
	delete g_pMatcher[1];
	delete g_pMatcher[2];
	delete g_pPlan;
	delete pRegionTarget;
	delete pRegionAvoid;

}	// FreeAll


///////////////////////////////////////////////////////////////////////////////
// TestOptimizer
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class OPTIMIZER>
void TestOptimizer()
{
	BEGIN_LOG_SECTION(TestOptimizer);

	CVectorN<> vInit;
	vInit.SetDim(g_pPlan->GetTotalBeamletCount(2));
	for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
	{
		vInit[nAt] = 0.01 * MAX_DOSE / (REAL) vInit.GetDim();
	}
	g_pMatcher[2]->StateVectorToParam(vInit, vInit);

//	LOG_EXPR_DESC(g_pMatcher[2]->TestGradient(vInit, 1e-3), "Gradient Test Error");

	OPTIMIZER opt(g_pMatcher[2]);
	opt.SetTolerance((REAL) 1e-4);

	REAL GBinSigma = 0.20;
	for (nAt = 0; nAt < g_pMatcher[2]->GetHistogramCount(); nAt++)
	{
		CHistogram *pHisto = g_pMatcher[2]->GetHistogram(nAt);
		pHisto->SetGBinSigma(GBinSigma);
	}
	LOG_EXPR(GBinSigma);

	LOG("Optimizing...");
	CVectorN<> vRes = opt.Optimize(vInit);
	LOG_EXPR(vRes);

	LOG_EXPR(opt.GetIterations());
	cout << "Iterations 2 = " << opt.GetIterations() << endl;

	LOG_OBJECT((*g_pMatcher[2]->GetHistogram(0)->GetVolume()));

	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 2)->GetBins(), "Target Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(1, 2)->GetBins(), "Avoid Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 2)->GetBinMeans(), "Bin Mean");

	CVectorN<> vState;
	g_pMatcher[2]->ParamToStateVector(vRes, vState);

	// add a small amount to all elements
	for (nAt = 0; nAt < vState.GetDim(); nAt++)
	{
		vState[nAt] = __max(vState[nAt], 1e-4);
	}

	g_pPlan->SetStateVector(2, vState);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(2)));

	CVectorN<> vState1;
	g_pPlan->InvFilterStateVector(vState, 2, vState1, FALSE);
	g_pPlan->SetStateVector(1, vState1);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(1)));

	g_pPlan->InvFilterStateVector(vState, 2, vState1, TRUE);
	g_pPlan->SetStateVector(1, vState1);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(1)));


	CVectorN<> vInit1;
	g_pPlan->GetStateVector(1, vInit1);
	LOG_EXPR_EXT(vInit1);

	FLUSH_LOG();

	g_pMatcher[1]->StateVectorToParam(vInit1, vInit1);

	CVectorN<> vRes1;

	for (int nAtSigma = 2; nAtSigma <= 2; nAtSigma++)
	{

	REAL res = (*g_pMatcher[1])(vInit1);
	OPTIMIZER opt1(g_pMatcher[1]);
	opt1.SetTolerance((REAL) 1e-4);

	for (nAt = 0; nAt < g_pMatcher[1]->GetHistogramCount(); nAt++)
	{
		CHistogram *pHisto = g_pMatcher[1]->GetHistogram(nAt);
		pHisto->SetGBinSigma(GBinSigma / (REAL) nAtSigma);
	}

	vRes1 = opt1.Optimize(vInit1);

	LOG_EXPR(opt1.GetIterations());
	cout << "Iterations 1 = " << opt1.GetIterations() << endl;

	LOG_OBJECT((*g_pMatcher[1]->GetHistogram(0)->GetVolume()));
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 1)->GetBins(), "Target Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(1, 1)->GetBins(), "Avoid Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 1)->GetBinMeans(), "Bin Mean");

	vInit1 = vRes1;

	}

	g_pMatcher[1]->ParamToStateVector(vRes1, vState1);

	// add a small amount to all elements
	for (nAt = 0; nAt < vState1.GetDim(); nAt++)
	{
		vState1[nAt] = __max(vState1[nAt], 1e-4);
	}

	g_pPlan->SetStateVector(1, vState1);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(1)));

	CVectorN<> vState0;
	g_pPlan->InvFilterStateVector(vState1, 1, vState0, FALSE);
	g_pPlan->SetStateVector(0, vState0);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(0)));

	g_pPlan->InvFilterStateVector(vState1, 1, vState0, TRUE);
	g_pPlan->SetStateVector(0, vState0);
	LOG_OBJECT((*g_pPlan->GetDoseMatrix(0)));


	CVectorN<> vInit0;
	g_pPlan->GetStateVector(0, vInit0);
	LOG_EXPR_EXT(vInit0);

	FLUSH_LOG();

	g_pMatcher[0]->StateVectorToParam(vInit0, vInit0);

	for (nAtSigma = 2; nAtSigma <= 2; nAtSigma++)
	{

	REAL res = (*g_pMatcher[0])(vInit0);
	OPTIMIZER opt0(g_pMatcher[0]);
	opt0.SetTolerance((REAL) 1e-3);

	for (nAt = 0; nAt < g_pMatcher[0]->GetHistogramCount(); nAt++)
	{
		CHistogram *pHisto = g_pMatcher[0]->GetHistogram(nAt);
		pHisto->SetGBinSigma(GBinSigma / (REAL) nAtSigma);
	}

	CVectorN<> vRes0 = opt0.Optimize(vInit0);

//	CXMLLogFile::GetLogFile()->ClearLog();

	LOG_EXPR(opt0.GetIterations());
	cout << "Iterations 0 = " << opt0.GetIterations() << endl;

	LOG_OBJECT((*g_pMatcher[0]->GetHistogram(0)->GetVolume()));
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 0)->GetBins(), "Target Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(1, 0)->GetBins(), "Avoid Bins");
	LOG_EXPR_EXT_DESC(g_pPlan->GetRegionHisto(0, 0)->GetBinMeans(), "Bin Mean");

	vInit0 = vRes0;

	}

	END_LOG_SECTION(); // TestOptimizer

	FLUSH_LOG();

}	// TestOptimizer


/////////////////////////////////////////////////////////////////////////////
// The one and only application object

class CTestHistoApp : public CWinApp
{
public:
	virtual BOOL InitApplication()
	{
		SetRegistryKey(_T("Local AppWizard-Generated Applications"));
		return CWinApp::InitApplication();
	}
};

CTestHistoApp theApp;

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// _tmain
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)
		&& AfxGetApp()->InitApplication())
	{
		BEGIN_LOG_SECTION(_tmain);

		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;

		CXMLLogFile::GetLogFile()->SetFormat((float) 0, " % lf ");

		FLUSH_LOG();

		cout << "*************************************" << endl;
		cout << "Initializing plans..." << endl;

		InitPlan();

		FLUSH_LOG();

		cout << "*************************************" << endl;
		cout << "Initializing volumes..." << endl;

		InitVolumes();

		FLUSH_LOG();

		cout << "*************************************" << endl;
		cout << "Testing optimizer..." << endl;

		TestOptimizer<CConjGradOptimizer>();
		// TestOptimizer<CPowellOptimizer>();

		FLUSH_LOG();

		FreeAll();

		END_LOG_SECTION();	// _tmain
	}
	else
	{
		cerr << "Fatal Error: MFC initialization failed";
		nRetCode = 1;
	}

	return nRetCode;

}	// _tmain


