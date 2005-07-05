// PrescriptionToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "Brimstone.h"
#include "PrescriptionToolbar.h"

#include <Prescription.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar

CPrescriptionToolbar::CPrescriptionToolbar()
{
}

CPrescriptionToolbar::~CPrescriptionToolbar()
{
}


BEGIN_MESSAGE_MAP(CPrescriptionToolbar, CDialogBar)
	//{{AFX_MSG_MAP(CDialogBar)
	ON_CBN_SELCHANGE(IDC_STRUCTSELECT, OnSelchangeStructselectcombo)
	ON_CBN_DROPDOWN(IDC_STRUCTSELECT, OnDropdownStructselectcombo)
	ON_BN_CLICKED(IDC_STRUCT_VISIBLE, OnVisibleCheck)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar message handlers

void CPrescriptionToolbar::OnSelchangeStructselectcombo() 
{
	CComboBox *pSSelectCB = (CComboBox *) GetDlgItem(IDC_STRUCTSELECT);

	CString strName;
	pSSelectCB->GetWindowText(strName);

	m_pDoc->SelectStructure(strName);

	CStructure *pStruct = (CStructure *) pSSelectCB->GetItemDataPtr(
		pSSelectCB->GetCurSel());

	if (m_pDoc->m_pOptThread->m_pPrescParam)
	{
		CVOITerm *pVOIT = m_pDoc->m_pOptThread->m_pPrescParam->GetStructureTerm(pStruct);

		if (pVOIT != NULL)
		{
			CSliderCtrl *pSWSlider = (CSliderCtrl *) GetDlgItem(IDC_STRUCTWEIGHT);
			pSWSlider->SetPos(pVOIT->GetWeight() * 20.0);
		}
	}

	// set visibility check
	CButton *pVizCheck = (CButton*) GetDlgItem(IDC_STRUCT_VISIBLE);
	pVizCheck->SetCheck(pStruct->m_bVisible ? 1 : 0);
}

void CPrescriptionToolbar::OnDropdownStructselectcombo() 
{
	if (!m_pDoc)
		return;

	CComboBox *pSSelectCB = (CComboBox *) GetDlgItem(IDC_STRUCTSELECT);
	pSSelectCB->ResetContent();

	CSeries *pSeries = m_pDoc->m_pSeries;
	for (int nStruct = 0; nStruct < pSeries->GetStructureCount(); nStruct++)
	{
		CStructure *pStruct = pSeries->GetStructureAt(nStruct);
		int nIndex = pSSelectCB->AddString(pStruct->GetName());
		pSSelectCB->SetItemDataPtr(nIndex, (void *) pStruct);
	} 
}

void CPrescriptionToolbar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CComboBox *pSSelectCB = (CComboBox *) GetDlgItem(IDC_STRUCTSELECT);
	int nIndex = pSSelectCB->GetCurSel();
	if (nIndex != CB_ERR)
	{
		CStructure *pStruct = (CStructure *) pSSelectCB->GetItemDataPtr(nIndex);

		// m_pDoc->m_pOptThread->m_csPrescParam.Lock();
		// get term
		CVOITerm *pVOIT = m_pDoc->m_pOptThread->m_pPrescParam->GetStructureTerm(pStruct);

		CSliderCtrl *pSWSlider = (CSliderCtrl *) GetDlgItem(IDC_STRUCTWEIGHT);
		nPos = pSWSlider->GetPos();
		pVOIT->SetWeight((REAL) nPos / 20.0);
		TRACE("SW Slider %i\n", nPos);

		m_pDoc->m_pOptThread->m_pPresc->UpdateTerms(
			m_pDoc->m_pOptThread->m_pPrescParam);

		// call optimize
		CVectorN<> vInit;
		m_pDoc->m_pOptThread->m_pPresc->GetInitStateVector(vInit);
		if (FALSE) // m_pDoc->m_pOptThread->m_pPresc->Optimize(vInit, NULL, NULL))
		{
			// m_pDoc->m_pOptThread->m_vResult.SetDim(vInit.GetDim());
			// m_pDoc->m_pOptThread->m_vResult = vInit;
			// for (int nAt = 0; nAt < vInit.GetDim(); nAt++)
			{
				// vInit[nAt] = 
				//	Sigmoid(vInit[nAt], m_pDoc->m_pOptThread->m_pPresc->m_inputScale);
			}

			m_pDoc->m_pOptThread->m_pPresc->SetStateVectorToPlan(vInit);
//			m_pDoc->m_pOptThread->m_pPresc->UpdateTerms(
//				m_pDoc->m_pOptThread->m_pPrescParam);
		}

		// m_pDoc->m_pOptThread->m_evtParamChanged = TRUE;
		// m_pDoc->m_pOptThread->m_csPrescParam.Unlock();

		// m_pDoc->m_pOptThread->Immediate();
		// m_pDoc->m_pOptThread->ResumeThread();

/*		do
		{
			Sleep(0);	

		} while (!m_pDoc->m_pOptThread->m_evtNewResult); */

//		m_pDoc->UpdateFromOptimizer();
		POSITION pos = m_pDoc->GetFirstViewPosition();
		while (pos != NULL)
		{
			CView *pView = m_pDoc->GetNextView(pos);
			pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		
		// m_graph.Invalidate(TRUE);

		// m_pDoc->UpdateAllViews(NULL);
	}
}


void CPrescriptionToolbar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl *pPRSlider = (CSliderCtrl *) GetDlgItem(IDC_OPTPARAM);
	nPos = pPRSlider->GetPos();

	TRACE("PR Slider %i\n", nPos);
	REAL weight = (REAL) nPos * 10.0;

//	m_pDoc->m_pOptThread->m_csPrescParam.Lock();

	m_pDoc->m_pOptThread->m_pPrescParam->SetEntropyWeight(weight);
//	m_pDoc->m_pOptThread->m_evtParamChanged = TRUE;
//	m_pDoc->m_pOptThread->m_csPrescParam.Unlock();

	// m_pDoc->m_pOptThread->Immediate();
	// m_pDoc->m_pOptThread->ResumeThread();

/*	do
	{
		Sleep(0);	

	} while (!m_pDoc->m_pOptThread->m_evtNewResult);

	m_pDoc->UpdateFromOptimizer(); */

	m_pDoc->m_pOptThread->m_pPresc->UpdateTerms(
		m_pDoc->m_pOptThread->m_pPrescParam);

	// call optimize
	CVectorN<> vInit;
	m_pDoc->m_pOptThread->m_pPresc->GetInitStateVector(vInit);
	if (m_pDoc->m_pOptThread->m_pPresc->Optimize(vInit, NULL, NULL))
	{
			m_pDoc->m_pOptThread->m_pPresc->SetStateVectorToPlan(vInit);
	}


	POSITION pos = m_pDoc->GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = m_pDoc->GetNextView(pos);
		pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	// m_graph.Invalidate(TRUE);

	// m_pDoc->UpdateAllViews(NULL);
}


void CPrescriptionToolbar::SetDocument(CBrimstoneDoc *pDoc)
{
	m_pDoc = pDoc;

/*	// initialize combo box
	CComboBox *pSSelectCB = (CComboBox *) GetDlgItem(IDC_STRUCTSELECT);
	pSSelectCB->ResetContent();

	CSeries *pSeries = pDoc->m_pSeries;
	for (int nStruct = 0; nStruct = pSeries->GetStructureCount(); nStruct++)
	{
		CStructure *pStruct = pSeries->GetStructureAt(nStruct);
		pSSelectCB->AddString(pStruct->GetName());
	}  */
}


void CPrescriptionToolbar::OnVisibleCheck() 
{
	CComboBox *pSSelectCB = (CComboBox *) GetDlgItem(IDC_STRUCTSELECT);
	int nIndex = pSSelectCB->GetCurSel();
	if (nIndex != CB_ERR)
	{
		CStructure *pStruct = (CStructure *) pSSelectCB->GetItemDataPtr(nIndex);

		CButton *pVizCheck = (CButton*) GetDlgItem(IDC_STRUCT_VISIBLE);

		pStruct->m_bVisible = pVizCheck->GetCheck() == 1;
	}
}
