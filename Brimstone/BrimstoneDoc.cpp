// BrimstoneDoc.cpp : implementation of the CBrimstoneDoc class
//

#include "stdafx.h"
#include "Brimstone.h"

#include "BrimstoneDoc.h"

#include <Prescription.h>

#include <BeamDoseCalc.h>

#include <SeriesDicomImporter.h>

#include "PlanSetupDlg.h"
#include "PrescDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


COptThread::~COptThread() 
{ 
	delete m_pPresc;
	delete m_pPrescParam;
}


IMPLEMENT_DYNCREATE(COptThread, CWinThread);


BOOL ExtCallback(REAL lambda, const CVectorN<>& vDir, 
							 void *pParam)
{
	COptThread *pOptThread = (COptThread *) pParam;
	return pOptThread->Callback(lambda, vDir);
}

void COptThread::Immediate()
{
	m_evtParamChanged = FALSE;

	// call optimize
	CVectorN<> vInit;
	m_pPresc->GetInitStateVector(vInit);
	if (m_pPresc->Optimize(vInit, ExtCallback, (void *) this))
	{
		m_evtNewResult = TRUE;

		// set updated results
		// ...
		m_pPrescParam->SetStateVectorToPlan(m_vResult);
	}
}

int COptThread::Run()
{
	while (m_pPresc)
	{
		nIteration = 0;

		m_bDone = FALSE;

		// call optimize
		CVectorN<> vInit;
		m_pPresc->GetInitStateVector(vInit);
		if (m_pPresc->Optimize(vInit, ExtCallback, (void *) this))
		{
			// set updated results
			// ...
			m_evtNewResult = TRUE;
			m_bDone = TRUE;

			// wait for new params
			SuspendThread();
			while (!m_evtParamChanged) 
			{
			}
		}

		m_csPrescParam.Lock();

		// transfer parameters
		// ...
		m_pPresc->UpdateTerms(m_pPrescParam);

		m_evtParamChanged = FALSE;

		m_csPrescParam.Unlock();
	}

	return 0;
}

BOOL COptThread::Callback(REAL value, const CVectorN<>& vRes)
{
	if (m_evtParamChanged)
	{
		return FALSE;
	}

	nIteration++;

	if (vRes.GetDim() == m_pPresc->m_pPlan->GetTotalBeamletCount(0))
	{
		// m_csResult.Lock();

		// set results
		m_bestValue = value;
		m_vResult.SetDim(vRes.GetDim());
		m_vResult = vRes;
		for (int nAt = 0; nAt < m_vResult.GetDim(); nAt++)
		{
//			ASSERT(_finite(vInit[nAt]));
			m_vResult[nAt] = Sigmoid(vRes[nAt], m_pPresc->m_inputScale);
//			ASSERT(_finite(vInit[nAt]));
		}

		// flag new result
//		m_evtNewResult = TRUE;

		// m_csResult.Unlock();
	}

	return TRUE;
}

void COptThread::UpdatePlan()
{
	if (m_evtNewResult)
	{
//		m_csResult.Lock();

		m_pPrescParam->SetStateVectorToPlan(m_vResult);
				
		m_evtNewResult = FALSE;

//		m_csResult.Unlock();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc

IMPLEMENT_DYNCREATE(CBrimstoneDoc, CDocument)

BEGIN_MESSAGE_MAP(CBrimstoneDoc, CDocument)
	//{{AFX_MSG_MAP(CBrimstoneDoc)
	ON_COMMAND(ID_OPTIMIZE, OnOptimize)
	ON_COMMAND(ID_GENBEAMLETS, OnGenbeamlets)
	ON_COMMAND(ID_FILE_IMPORT_DCM, OnFileImportDcm)
	ON_COMMAND(ID_PLAN_ADD_PRESC, OnPlanAddPresc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc construction/destruction

CBrimstoneDoc::CBrimstoneDoc()
: m_pSeries(NULL),
	m_pPlan(NULL),
	m_pOptThread(NULL)
{
	m_pOptThread = (COptThread *) ::AfxBeginThread(RUNTIME_CLASS(COptThread),
		THREAD_PRIORITY_HIGHEST, // THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
}

CBrimstoneDoc::~CBrimstoneDoc()
{
	m_pOptThread->SuspendThread();

	delete m_pOptThread;

	delete m_pPlan;
	delete m_pSeries;
}

void CBrimstoneDoc::DeleteContents() 
{
	delete m_pPlan;
	m_pPlan = NULL;
	delete m_pSeries;
	m_pSeries = NULL;

	// create series * plan
	m_pSeries = new CSeries();
	m_pPlan = new CPlan();
	m_pPlan->SetSeries(m_pSeries);

	m_pSelectedStruct = NULL;
	
	CDocument::DeleteContents();
}



/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc serialization

void CBrimstoneDoc::Serialize(CArchive& ar)
{
	CObArray arrWorkspace;
	if (ar.IsStoring())
	{
		arrWorkspace.Add(m_pSeries);
		arrWorkspace.Add(m_pPlan);
	}

	arrWorkspace.Serialize(ar);

	if (ar.IsLoading())
	{
		for (int nAt = 0; nAt < arrWorkspace.GetSize(); nAt++)
		{
			if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CSeries)))
			{
				delete m_pSeries;
				m_pSeries = (CSeries *) arrWorkspace[nAt];
			}
			else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPlan)))
			{
				delete m_pPlan;
				m_pPlan = (CPlan *) arrWorkspace[nAt];
			}
		}

		m_pPlan->GetDoseMatrix()->GetChangeEvent().AddObserver(this,
			(ListenerFunction) OnDoseChange);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc diagnostics

#ifdef _DEBUG
void CBrimstoneDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBrimstoneDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc commands

BOOL CBrimstoneDoc::SelectStructure(const CString &strName)
{
	CStructure *pStruct = m_pSeries->GetStructureFromName(strName);
	if (pStruct != NULL)
	{
		m_pSelectedStruct = pStruct;
		UpdateAllViews(NULL, IDC_STRUCTSELECT, pStruct);
		return TRUE;
	}

	return FALSE;
}

CStructure * CBrimstoneDoc::GetSelectedStructure()
{
	return m_pSelectedStruct;
}

void CBrimstoneDoc::OnDoseChange(CObservableEvent *, void *)
{
	UpdateAllViews(NULL, NULL, NULL);
}

void CBrimstoneDoc::UpdateFromOptimizer()
{
	m_pOptThread->UpdatePlan();
}

void CBrimstoneDoc::OnParamChange(CObservableEvent *, void *)
{
	m_pOptThread->m_evtParamChanged = TRUE;

	// make sure the thread is still running
	// m_pOptThread->ResumeThread();
}


///////////////////////////////////////////////////////////////////////////////
// InitUTarget
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitUTarget(CPlan *pPlan, CStructure *pTarget, CStructure *pAvoid)
{
	BEGIN_LOG_SECTION(InitUTarget);

	CVolume<REAL> *pRegionTarget = pTarget->GetRegion(0);
	pRegionTarget->ClearVoxels();

	CVolume<REAL> *pRegionAvoid = pAvoid->GetRegion(0);
	pRegionAvoid->ClearVoxels();

	int nBeamletCount = pPlan->GetBeamAt(0)->GetBeamletCount(0); // 31;
	int nRegionSize = ((REAL) nBeamletCount / sqrt(2.0)) / 2 + 1;
	int nMargin = pRegionTarget->GetWidth() / 2 - nRegionSize;

	CPolygon *pTargetContour = new CPolygon();

	REAL inMin = (REAL) nMargin + (REAL) nRegionSize - 0.5 * (REAL) nRegionSize;
	REAL inMax = (REAL) nMargin + (REAL) nRegionSize + 0.5 * (REAL) nRegionSize;

	CVectorD<2> arrVertices[] = 
	{
		CVectorD<2>(nMargin, nMargin), 
		CVectorD<2>(nMargin, pRegionTarget->GetHeight() - 1 - nMargin),
		CVectorD<2>(pRegionTarget->GetWidth() - 1 - nMargin, 
			pRegionTarget->GetHeight() - 1 - nMargin),
		CVectorD<2>(pRegionTarget->GetWidth() - 1- nMargin, nMargin),

		CVectorD<2>(inMax, nMargin),
		CVectorD<2>(inMax, inMax), 

		CVectorD<2>(inMin, inMax),
		CVectorD<2>(inMin, nMargin),

		CVectorD<2>(nMargin, nMargin), 
	};

	for (int nAt = 0; nAt < 9; nAt++)
	{
		arrVertices[nAt] += CVectorD<2>(0.5, 0.5);
		pTargetContour->AddVertex(arrVertices[nAt]);
	}
	
	pTarget->AddContour(pTargetContour, 0.0);

	CPolygon *pAvoidContour = new CPolygon();

	CVectorD<2> arrVertAvoid[] = 
	{
		CVectorD<2>(inMin + 1.0, nMargin),
		CVectorD<2>(inMax - 1.0, nMargin), 

		CVectorD<2>(inMax - 1.0, inMax - 1.0),
		CVectorD<2>(inMin + 1.0, inMax - 1.0),

		CVectorD<2>(inMin + 1.0, nMargin), 
	};

	for (nAt = 0; nAt < 5; nAt++)
	{
		arrVertAvoid[nAt] += CVectorD<2>(0.5, 0.5);
		pAvoidContour->AddVertex(arrVertAvoid[nAt]);
	}
	
	pAvoid->AddContour(pAvoidContour, 0.0);

	REAL ***pppVoxelsTarget = pRegionTarget->GetVoxels();
	REAL ***pppVoxelsAvoid = pRegionAvoid->GetVoxels();
	for (int nAtRow = nMargin; nAtRow < pRegionTarget->GetHeight()-nMargin; nAtRow++)
	{
		for (int nAtCol = nMargin; nAtCol < pRegionTarget->GetWidth()-nMargin; nAtCol++)
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

			ASSERT(pppVoxelsTarget[0][nAtRow][nAtCol] == 1.0
				|| pppVoxelsAvoid[0][nAtRow][nAtCol] == 1.0
				|| (pppVoxelsTarget[0][nAtRow][nAtCol] == 0.0
					&& pppVoxelsAvoid[0][nAtRow][nAtCol] == 0.0));
		}
	}

	LOG_OBJECT((*pRegionTarget));
	LOG_OBJECT((*pRegionAvoid));

	END_LOG_SECTION();	// InitUTarget

}	// InitUTarget

///////////////////////////////////////////////////////////////////////////////
// InitVolumes
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitVolumes(CSeries *pSeries, CPlan *pPlan, CPrescription *pPresc)
{
	BEGIN_LOG_SECTION(InitVolumes);

	CStructure *pStructTarget = pSeries->GetStructureFromName("Target");	
	CStructure *pStructAvoid = pSeries->GetStructureFromName("Avoid");

	InitUTarget(pPlan, pStructTarget, pStructAvoid);

	CKLDivTerm *pKLDT_Target = new CKLDivTerm(pStructTarget, 5.0);
	pPresc->AddStructureTerm(pKLDT_Target);

	// need to call after adding to prescription (so that binning has been set)
	const REAL MAX_DOSE = (REAL) 0.65;

	pKLDT_Target->SetInterval(MAX_DOSE, MAX_DOSE * /* 1.05 */ 1.01, 1.0);

	CKLDivTerm *pKLDT_Avoid = new CKLDivTerm(pStructAvoid, 0.5);
	pPresc->AddStructureTerm(pKLDT_Avoid);
	pKLDT_Avoid->SetInterval((REAL) 0.0, (REAL) 0.40,
		/* MAX_DOSE * 0.50 */ (REAL) 1.0);

	END_LOG_SECTION();	// InitVolumes

}	// InitVolumes


void CBrimstoneDoc::OnOptimize() 
{
	m_pOptThread->m_csPrescParam.Lock();
	m_pOptThread->m_evtParamChanged = TRUE;
	m_pOptThread->m_csPrescParam.Unlock();

	// m_pDoc->m_pOptThread->Immediate();
	// m_pOptThread->ResumeThread();

	do
	{
		Sleep(0);	

	} while (!m_pOptThread->m_evtNewResult);

	UpdateFromOptimizer();
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = GetNextView(pos);
		pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

/*	CVectorN<> vParam;
	m_pOptThread->m_pPrescParam->GetInitStateVector(vParam);
	BOOL bRes = m_pOptThread->m_pPrescParam->Optimize(vParam, NULL, NULL);

	m_pOptThread->m_pPrescParam->SetStateVectorToPlan(vParam); */
}

BOOL CBrimstoneDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	m_pOptThread->m_pPrescParam = new CPrescription(m_pPlan, 0);
	m_pOptThread->m_pPrescParam->SetElementInclude();
//	InitVolumes(m_pSeries, m_pPlan, m_pOptThread->m_pPrescParam);

	m_pOptThread->m_pPresc = new CPrescription(m_pPlan, 0);
	m_pOptThread->m_pPresc->SetElementInclude();
//	InitVolumes(m_pSeries, m_pPlan, m_pOptThread->m_pPresc);
//	m_pOptThread->m_pPresc->UpdateTerms(m_pOptThread->m_pPrescParam);

//	m_pOptThread->ResumeThread();


	return TRUE;
}

void CBrimstoneDoc::OnGenbeamlets() 
{
	CPlanSetupDlg dlgSetup;
	if (dlgSetup.DoModal() == IDOK)
	{
		int nBeams = dlgSetup.m_nBeamCount; // m_pPlan->GetBeamCount();

		CMatrixD<4> mBasis;
		mBasis[0][0] = 4.0; // mm
		mBasis[1][1] = 4.0;	// mm
		mBasis[2][2] = 4.0; // mm

		CVolume<REAL> *pOrigDensity = m_pSeries->m_pDens;
		int nHeight = pOrigDensity->GetWidth() * pOrigDensity->GetBasis()[0][0] / mBasis[0][0];
		int nWidth = pOrigDensity->GetWidth() * pOrigDensity->GetBasis()[1][1] / mBasis[1][1];
		int nDepth = 10;
		m_pPlan->m_dose.SetDimensions(nWidth, nHeight, nDepth);

		mBasis[3] = pOrigDensity->GetBasis()[3];
		m_pPlan->m_dose.SetBasis(mBasis);

		CVolume<REAL> massDensity;
		massDensity.ConformTo(pOrigDensity);
		massDensity.ClearVoxels();

		REAL ***pppCTVoxels = pOrigDensity->GetVoxels();
		REAL ***pppMDVoxels = massDensity.GetVoxels();
		for (int nX = 0; nX < massDensity.GetWidth(); nX++)
			for (int nY = 0; nY < massDensity.GetHeight() * 3 / 4; nY++)
				for (int nZ = 0; nZ < massDensity.GetDepth(); nZ++)
				{
					if (pppCTVoxels[nZ][nY][nX] < 0.0)
					{
						pppMDVoxels[nZ][nY][nX] = 
							0.0 + 1.0 * (pppCTVoxels[nZ][nY][nX] - -1024.0) / 1024.0;
					}
					else if (pppCTVoxels[nZ][nY][nX] < 1024.0)
					{
						pppMDVoxels[nZ][nY][nX] = 
							1.0 + 0.5 * pppCTVoxels[nZ][nY][nX] / 1024.0;
					}
					else
					{
						pppMDVoxels[nZ][nY][nX] = 1.5;
					}
				}

		for (int nAt = nBeams-1; nAt >= 0; nAt--)
		{
			TRACE("Generating Beamlets for Beam %i\n", nAt);

			double gantry;
			gantry = 90.0 + (double) nAt * 360.0 / (double) nBeams;

			CBeam *pBeam = new CBeam();
			pBeam->SetGantryAngle(gantry * PI / 180.0);
			pBeam->m_pDoseCalc = new CBeamDoseCalc(pBeam, m_pPlan->m_pKernel);
			pBeam->m_dose.ConformTo(&m_pPlan->m_dose);

			// rotate basis to beam orientation
			CMatrixD<2> mRotateBasis2D;
			mRotateBasis2D[0][0] = mBasis[0][0];
			mRotateBasis2D[1][1] = mBasis[1][1];

			mRotateBasis2D = // mRotateBasis2D * 
				::CreateRotate(gantry * PI / 180.0);
	
			CMatrixD<4> mRotateBasisHG;
			mRotateBasisHG[0][0] = mRotateBasis2D[0][0];
			mRotateBasisHG[0][1] = mRotateBasis2D[0][1];
			mRotateBasisHG[1][0] = mRotateBasis2D[1][0];
			mRotateBasisHG[1][1] = mRotateBasis2D[1][1];


			CMatrixD<4> mDoseBasis = pBeam->m_dose.GetBasis();
//			mDoseBasis.Invert();

			CMatrixD<4> mBeamBasis = mRotateBasisHG * mDoseBasis; ;
			// mBeamBasis[0][0] = mRotateBasis2D[0][0];
			// mBeamBasis[1][1] = mRotateBasis2D[1][1];
			
			/* CVectorD<2> vOrigin(mBeamBasis[3][0], mBeamBasis[3][1]);
			mRotateBasis2D.Invert();
			vOrigin = mRotateBasis2D * vOrigin;
			mBeamBasis[3][0] = vOrigin[0];
			mBeamBasis[3][1] = vOrigin[1]; */

			// CVectorD<3> vPos((REAL) pBeam->m_dose.GetWidth() / 2.0, 
			//	(REAL) pBeam->m_dose.GetHeight() / 2.0, 0.0);
			// TRACE_VECTOR("Middle of image position = ", mBeamBasis * ToHG(vPos));

			pBeam->m_dose.SetBasis(mBeamBasis);

			// set up isocenter

			CMatrixD<4> mBeamBasisInv = mBeamBasis;
			mBeamBasisInv.Invert();

			CVectorD<3> vIso(dlgSetup.m_isoX, dlgSetup.m_isoY, 0.0);
			pBeam->m_pDoseCalc->m_vIsocenter_vxl = FromHG<3, REAL>(mBeamBasisInv * ToHG<3, REAL>(vIso));
			pBeam->m_pDoseCalc->m_vIsocenter_vxl[2] = pBeam->m_dose.GetDepth() / 2.0;
			//	CVectorD<3>((REAL) pBeam->m_dose.GetWidth() / 2.0, 
			//	(REAL) pBeam->m_dose.GetHeight() / 2.0,
			//	(REAL) pBeam->m_dose.GetDepth() / 2.0);
			pBeam->m_pDoseCalc->m_vSource_vxl = pBeam->m_pDoseCalc->m_vIsocenter_vxl;
			pBeam->m_pDoseCalc->m_vSource_vxl[0] -= 1000.0 / 4.0;

			// now trigger convolution
			pBeam->m_pDoseCalc->CalcPencilBeams(&massDensity); // m_pSeries->m_pDens);

			m_pPlan->AddBeam(pBeam);
		}

	}
}

void CBrimstoneDoc::OnFileImportDcm() 
{
	// remove existing data
	if (!SaveModified())
		return;

	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR);
	CString strFilenames;
	dlg.m_ofn.lpstrFile = strFilenames.GetBuffer(16384);
	dlg.m_ofn.nMaxFile = 16384;
	if (dlg.DoModal() == IDOK)
	{
		// shut it down
		DeleteContents();

		// update views for new document objects
		SendInitialUpdate();

		// construct the importer
		CSeriesDicomImporter dcmImp(m_pSeries, &dlg);

		// process files
		int nCount = 0;
		do
		{
			nCount = dcmImp.ProcessNext();
			TRACE("Processing %i\n", nCount);
		} while (nCount > 0);

		// set path name
		// SetPathName("");

		SetModifiedFlag(TRUE);
	}
}


void CBrimstoneDoc::OnPlanAddPresc() 
{
	CPrescDlg dlg;
	dlg.m_pSeries = m_pSeries;
	dlg.m_pPrescParam = m_pOptThread->m_pPrescParam;
	dlg.m_pPresc = m_pOptThread->m_pPresc;

	if (dlg.DoModal())
	{
		m_pOptThread->m_pPresc->SetElementInclude();
		m_pOptThread->m_pPrescParam->SetElementInclude();

		UpdateAllViews(NULL, IDD_ADDPRESC, dlg.m_pStruct);
	}
}
