// BrimstoneDoc.cpp : implementation of the CBrimstoneDoc class
//

#include "stdafx.h"
#include ".\brimstonedoc.h"

#include "Brimstone.h"

#include <Prescription.h>

#include <BeamDoseCalc.h>

#include <SeriesDicomImporter.h>

#include "PlanSetupDlg.h"
#include "PrescDlg.h"

#include "OptimizerDashboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	ON_COMMAND(ID_OPT_DASHBOARD, OnOptDashboard)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneDoc construction/destruction

CBrimstoneDoc::CBrimstoneDoc()
#ifdef THREAD_OPT
	, m_pOptThread(NULL)
#endif
{
	// triggers creation of subordinates
	DeleteContents();

#ifdef THREAD_OPT
	m_pOptThread = (COptThread *) ::AfxBeginThread(RUNTIME_CLASS(COptThread),
		THREAD_PRIORITY_HIGHEST, // THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
#endif
}

CBrimstoneDoc::~CBrimstoneDoc()
{
#ifdef THREAD_OPT
	m_pOptThread->SuspendThread();
	delete m_pOptThread;
#endif
}

void CBrimstoneDoc::DeleteContents() 
{
	// create series * plan
	m_pSeries.Free();
	m_pSeries.Attach(new CSeries());

	m_pPlan.Free();
	m_pPlan.Attach(new CPlan());
	m_pPlan->SetSeries(m_pSeries);

	m_pPresc.Free();
	m_pPresc.Attach(new CPrescription(m_pPlan, 0));

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
		arrWorkspace.Add(m_pPresc);
	}

	arrWorkspace.Serialize(ar);

	if (ar.IsLoading())
	{
		m_pSeries.Free();
		m_pPlan.Free();
		m_pPresc.Free();
		for (int nAt = 0; nAt < arrWorkspace.GetSize(); nAt++)
		{
			if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CSeries)))
			{
				m_pSeries.Attach((CSeries *) arrWorkspace[nAt]);
			}
			else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPlan)))
			{
				m_pPlan.Attach((CPlan *) arrWorkspace[nAt]);
			}
			else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPrescription)))
			{
				m_pPresc.Attach((CPrescription *) arrWorkspace[nAt]);
			}
		}

		// check everything loads OK
		ASSERT(m_pSeries != NULL);
		ASSERT(m_pPlan != NULL);

		// link plan to series
		m_pPlan->SetSeries(m_pSeries);

		// create an empty prescription, if none available
		if (m_pPresc == NULL)
		{
			m_pPresc.Attach(new CPrescription(m_pPlan, 0));
		}

		// set up the structure regions for histograms

		m_pPlan->GetDoseMatrix()->GetChangeEvent().AddObserver(this,
			(ListenerFunction) &CBrimstoneDoc::OnDoseChange);

		// set up element includes
		// TODO: wire this up properly
		m_pPresc->SetElementInclude();
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

#ifdef THREAD_OPT
void CBrimstoneDoc::UpdateFromOptimizer()
{
	m_pOptThread->UpdatePlan();
}
#endif

#ifdef THREAD_OPT
void CBrimstoneDoc::OnParamChange(CObservableEvent *, void *)
{
	m_pOptThread->m_evtParamChanged = TRUE;

	// make sure the thread is still running
	// m_pOptThread->ResumeThread();
}
#endif


///////////////////////////////////////////////////////////////////////////////
// InitUTarget
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void InitUTarget(CPlan *pPlan, CStructure *pTarget, CStructure *pAvoid)
{
	BEGIN_LOG_SECTION(InitUTarget);

	CVolume<VOXEL_REAL> *pRegionTarget = pTarget->GetRegion(0);
	pRegionTarget->ClearVoxels();

	CVolume<VOXEL_REAL> *pRegionAvoid = pAvoid->GetRegion(0);
	pRegionAvoid->ClearVoxels();

	int nBeamletCount = pPlan->GetBeamAt(0)->GetBeamletCount(0); // 31;
	int nRegionSize = Round<int>(((REAL) nBeamletCount / sqrt(2.0)) / 2.0) + 1;
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

	for (int nAt = 0; nAt < 5; nAt++)
	{
		arrVertAvoid[nAt] += CVectorD<2>(0.5, 0.5);
		pAvoidContour->AddVertex(arrVertAvoid[nAt]);
	}
	
	pAvoid->AddContour(pAvoidContour, 0.0);

	VOXEL_REAL ***pppVoxelsTarget = pRegionTarget->GetVoxels();
	VOXEL_REAL ***pppVoxelsAvoid = pRegionAvoid->GetVoxels();
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
	CWaitCursor cursor;

	// call optimize
	CVectorN<> vInit;
	m_pPresc->GetInitStateVector(vInit);
	if (m_pPresc->Optimize(vInit, NULL, NULL))
	{
		m_pPresc->SetStateVectorToPlan(vInit);
	}


	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = GetNextView(pos);
		pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return;

#ifdef _USE_THREAD_
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
#endif
}

BOOL CBrimstoneDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// create prescription if its their
	ASSERT(m_pPresc != NULL);

	return TRUE;
}

void CBrimstoneDoc::OnGenbeamlets() 
{
	CPlanSetupDlg dlgSetup;
	if (dlgSetup.DoModal() == IDOK)
	{
		int nBeams = dlgSetup.m_nBeamCount; // m_pPlan->GetBeamCount();

		// TODO: set as parameter
		const REAL dosePixSpacing = 2.0; // mm
		CMatrixD<4> mBasis;
		mBasis[0][0] = dosePixSpacing; // mm
		mBasis[1][1] = dosePixSpacing;	// mm
		mBasis[2][2] = dosePixSpacing; // mm

		CVolume<VOXEL_REAL> *pOrigDensity = m_pSeries->m_pDens;
		pOrigDensity->VoxelsChanged();
		TRACE("pOrigDensity->GetMax() = %f\n", pOrigDensity->GetMax());
		TRACE("pOrigDensity->GetMin() = %f\n", pOrigDensity->GetMin());

		int nHeight = 
			Round<int>(pOrigDensity->GetWidth() * pOrigDensity->GetBasis()[0][0] / mBasis[0][0]);
		int nWidth = 
			Round<int>(pOrigDensity->GetWidth() * pOrigDensity->GetBasis()[1][1] / mBasis[1][1]);
		int nDepth = 10;
		m_pPlan->m_dose.SetDimensions(nWidth, nHeight, nDepth);

		mBasis[3] = pOrigDensity->GetBasis()[3];
		m_pPlan->m_dose.SetBasis(mBasis);

		CVolume<VOXEL_REAL> massDensity;
		massDensity.ConformTo(pOrigDensity);
		massDensity.ClearVoxels();

		VOXEL_REAL ***pppCTVoxels = pOrigDensity->GetVoxels();
		VOXEL_REAL ***pppMDVoxels = massDensity.GetVoxels();
		for (int nX = 0; nX < massDensity.GetWidth(); nX++)
			for (int nY = 0; nY < massDensity.GetHeight() * 3 / 4; nY++)
				for (int nZ = 0; nZ < massDensity.GetDepth(); nZ++)
				{
					if (pppCTVoxels[nZ][nY][nX] < 0.0)
					{
						pppMDVoxels[nZ][nY][nX] = (VOXEL_REAL)
							(0.0 + 1.0 * (pppCTVoxels[nZ][nY][nX] - -1024.0) / 1024.0);
					}
					else if (pppCTVoxels[nZ][nY][nX] < 1024.0)
					{
						pppMDVoxels[nZ][nY][nX] = (VOXEL_REAL)
							(1.0 + 0.5 * pppCTVoxels[nZ][nY][nX] / 1024.0);
					}
					else
					{
						pppMDVoxels[nZ][nY][nX] = 1.5;
					}
				}

		massDensity.VoxelsChanged();
		TRACE("massDensity.GetMax() = %f\n", massDensity.GetMax());
		TRACE("massDensity.GetMin() = %f\n", massDensity.GetMin());

		for (int nAt = nBeams-1; nAt >= 0; nAt--)
		{
			TRACE("Generating Beamlets for Beam %i\n", nAt);

			double gantry;
			gantry = 90.0 + (double) nAt * 360.0 / (double) nBeams;

			CBeam *pBeam = new CBeam();
			pBeam->SetGantryAngle(gantry * PI / 180.0);
			pBeam->m_pDoseCalc = new CBeamDoseCalc(pBeam, m_pPlan->m_pKernel);
			pBeam->m_dose.ConformTo(&m_pPlan->m_dose);

			// TODO: move this to the beam -- it should create it's own basis, rotated about center of plan's dose volume
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
			mDoseBasis[3][0] -= m_pPlan->m_dose.GetWidth() * dosePixSpacing / 2.0;
			mDoseBasis[3][1] -= m_pPlan->m_dose.GetHeight() * dosePixSpacing / 2.0;

			CMatrixD<4> mBeamBasis = mRotateBasisHG * mDoseBasis; ;
			mBeamBasis[3][0] += m_pPlan->m_dose.GetWidth() * dosePixSpacing / 2.0;
			mBeamBasis[3][1] += m_pPlan->m_dose.GetHeight() * dosePixSpacing / 2.0;
			
			pBeam->m_dose.SetBasis(mBeamBasis);

			// set up isocenter

			CMatrixD<4> mBeamBasisInv = mBeamBasis;
			mBeamBasisInv.Invert();

			CVectorD<3> vIso(dlgSetup.m_isoX, dlgSetup.m_isoY, 0.0);
			pBeam->m_pDoseCalc->m_vIsocenter_vxl = FromHG<3, REAL>(mBeamBasisInv * ToHG<3, REAL>(vIso));
			pBeam->m_pDoseCalc->m_vIsocenter_vxl[2] = pBeam->m_dose.GetDepth() / 2.0;

			pBeam->m_pDoseCalc->m_vSource_vxl = pBeam->m_pDoseCalc->m_vIsocenter_vxl;
			pBeam->m_pDoseCalc->m_vSource_vxl[0] -= 1000.0 / dosePixSpacing;

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
/*	CPrescDlg dlg;
	dlg.m_pSeries = m_pSeries;
	dlg.m_pPresc = m_pPresc;

	if (dlg.DoModal())
	{
		UpdateAllViews(NULL, IDD_ADDHISTO, dlg.m_pStruct);
	} */
}

void CBrimstoneDoc::OnOptDashboard()
{
	COptimizerDashboard optDash;
	optDash.m_pPresc = m_pPresc;
	optDash.DoModal();
}

// generates a histogram for the specified structure
void CBrimstoneDoc::AddHistogram(CStructure * pStruct)
{
	// generate the histogram
	m_pPlan->GetHistogram(pStruct, true);

	// trigger updates
	UpdateAllViews(NULL, IDD_ADDHISTO, pStruct);
}

// removes histogram for designated structure
void CBrimstoneDoc::RemoveHistogram(CStructure * pStruct)
{
	UpdateAllViews(NULL, IDD_REMOVEHISTO, pStruct);
	m_pPlan->RemoveHistogram(pStruct);
}

// add new structure term
void CBrimstoneDoc::AddStructTerm(CVOITerm * pVOIT)
{
	m_pPresc->AddStructureTerm(pVOIT);
	UpdateAllViews(NULL, IDD_ADDPRESC, pVOIT->GetVOI());
}
