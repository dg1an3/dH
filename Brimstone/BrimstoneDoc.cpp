// BrimstoneDoc.cpp : implementation of the CBrimstoneDoc class
//

#include "stdafx.h"
#include "Brimstone.h"

#include "BrimstoneDoc.h"

#include <Prescription.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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
	ON_COMMAND(ID_SCANBEAMLETS, OnScanbeamlets)
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
}

BOOL CBrimstoneDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	delete m_pPlan;
	m_pPlan = NULL;
	delete m_pSeries;
	m_pSeries = NULL;
	m_pSelectedStruct = NULL;

	return TRUE;
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
				m_pSeries = (CSeries *) arrWorkspace[nAt];
			}
			else if (arrWorkspace[nAt]->IsKindOf(RUNTIME_CLASS(CPlan)))
			{
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
	m_pOptThread->ResumeThread();
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
	
	pTarget->AddContour(pTargetContour);

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
	
	pAvoid->AddContour(pAvoidContour);

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
	CVectorN<> vParam;
	m_pOptThread->m_pPrescParam->GetInitStateVector(vParam);
	BOOL bRes = m_pOptThread->m_pPrescParam->Optimize(vParam, NULL, NULL);

	m_pOptThread->m_pPrescParam->SetStateVectorToPlan(vParam);
}

void CBrimstoneDoc::OnScanbeamlets() 
{
	CPlan *pPlan = m_pPlan;
	for (int nAt = 0; nAt < pPlan->GetBeamAt(0)->GetBeamletCount(); nAt++)
	{
		CVectorN<> vWeights;
		vWeights.SetDim(pPlan->GetBeamAt(0)->GetBeamletCount());
		vWeights.SetZero();
		vWeights[nAt] = 0.8;

		for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
		{
			CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
			pBeam->SetIntensityMap(vWeights);
		}
		::AfxGetMainWnd()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}
}

BOOL CBrimstoneDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	m_pOptThread->m_pPrescParam = new CPrescription(m_pPlan, 0);
	InitVolumes(m_pSeries, m_pPlan, m_pOptThread->m_pPrescParam);

	m_pOptThread->m_pPresc = new CPrescription(m_pPlan, 0);
	InitVolumes(m_pSeries, m_pPlan, m_pOptThread->m_pPresc);
	m_pOptThread->m_pPresc->UpdateTerms(m_pOptThread->m_pPrescParam);

	m_pOptThread->ResumeThread();

	return TRUE;
}
