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
#include <Prescription.h>

#include <Plan.h>


const REAL MAX_DOSE = (REAL) 0.65;

CSeries *g_pSeries = NULL;

char g_pszPlanFile[128];
CPlan *g_pPlan = NULL;

CStructure *g_pStructTarget = NULL;
CStructure *g_pStructAvoid = NULL;

CPrescription *g_pPresc = NULL;


///////////////////////////////////////////////////////////////////////////////
// InitPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitPlan()
{
	// create archive and store
	CFile planFile(g_pszPlanFile, CFile::modeRead
		| CFile::shareExclusive
		| CFile::typeBinary);
	CArchive ar(&planFile, CArchive::load);

	CObArray arrWorkspace;
	arrWorkspace.Serialize(ar);
	ar.Close();
	planFile.Close();

	for (int nAt = 0; nAt < arrWorkspace.GetSize(); nAt++)
	{
		if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPlan)))
		{
			g_pPlan = (CPlan *) arrWorkspace[nAt];
		}
		else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CSeries)))
		{
			g_pSeries = (CSeries *) arrWorkspace[nAt];
		}
	}
	if (!g_pSeries)
	{
		g_pSeries = new CSeries();
	}

	g_pPlan->SetSeries(g_pSeries);

	g_pPresc = new CPrescription(g_pPlan, 0);
//	g_pPresc->SetPlan(g_pPlan);

}	// InitPlan

///////////////////////////////////////////////////////////////////////////////
// InitUTarget
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitUTarget(CVolume<REAL> *pTarget, CVolume<REAL> *pAvoid)
{
	BEGIN_LOG_SECTION(InitUTarget);

	int nBeamletCount = 31;
	int nRegionSize = ((REAL) nBeamletCount / sqrt(2.0)) / 2 + 1;
	int nMargin = pTarget->GetWidth() / 2 - nRegionSize;

//	int nMargin = 4;
//	int nRegionSize = pTarget->GetWidth() / 2 - nMargin;

	REAL ***pppVoxelsTarget = pTarget->GetVoxels();
	REAL ***pppVoxelsAvoid = pAvoid->GetVoxels();
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

	CVolume<REAL> *pDose = g_pPlan->GetDoseMatrix(); // 0); // 1);

	g_pStructTarget = new CStructure("Target");
	g_pSeries->m_arrStructures.Add(g_pStructTarget);

	CVolume<REAL> *pRegionTarget = g_pStructTarget->GetRegion(0);
	pRegionTarget->SetDimensions(pDose->GetWidth(), pDose->GetHeight(), 1);
	pRegionTarget->ClearVoxels();

	g_pStructAvoid = new CStructure("Avoid");
	g_pSeries->m_arrStructures.Add(g_pStructAvoid);

	CVolume<REAL> *pRegionAvoid = g_pStructAvoid->GetRegion(0);
	pRegionAvoid->SetDimensions(pDose->GetWidth(), pDose->GetHeight(), 1);
	pRegionAvoid->ClearVoxels();

	InitUTarget(pRegionTarget, pRegionAvoid);

	CKLDivTerm *pKLDT_Target = new CKLDivTerm(g_pStructTarget, 10.0);
	g_pPresc->AddStructureTerm(pKLDT_Target);

	// need to call after adding to prescription (so that binning has been set)
	pKLDT_Target->SetInterval(MAX_DOSE, MAX_DOSE * 1.05, 1.0);

	CKLDivTerm *pKLDT_Avoid = new CKLDivTerm(g_pStructAvoid, 1.0);
	g_pPresc->AddStructureTerm(pKLDT_Avoid);
	pKLDT_Avoid->SetInterval((REAL) 0.0, (REAL) 0.30,
		/* MAX_DOSE * 0.50 */ (REAL) 1.0);

	END_LOG_SECTION();	// InitVolumes

}	// InitVolumes

///////////////////////////////////////////////////////////////////////////////
// FreeAll
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void FreeAll()
{
	delete g_pPresc;

	delete g_pPlan;
	delete g_pSeries;

}	// FreeAll



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

		strcpy(g_pszPlanFile, argv[1]);
		cout << "Reading file " << g_pszPlanFile << endl;

		InitPlan();

		FLUSH_LOG();

		cout << "*************************************" << endl;
		cout << "Initializing volumes..." << endl;

		InitVolumes();

		FLUSH_LOG();

		cout << "*************************************" << endl;
		cout << "Testing optimizer..." << endl;

		const int LOOP_COUNT = 10;
		for (int nAt = 0; nAt < LOOP_COUNT; nAt++)
		{
			CVectorN<> vInit;
			g_pPresc->GetInitStateVector(vInit);
			g_pPresc->Optimize(vInit, NULL);
		}

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


