// PrescriptionToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "Brimstone.h"
#include "PrescriptionToolbar.h"

#include ".\prescriptiontoolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar

CPrescriptionToolbar::CPrescriptionToolbar(CWnd *pParent)
{
}

CPrescriptionToolbar::~CPrescriptionToolbar()
{
}

BOOL CPrescriptionToolbar::OnInitDialog()
{
	// CDialogBar::OnInitDialog();
	m_cbSSelect.Attach(::GetDlgItem(m_hWnd, IDC_STRUCTSELECT));

	m_btnVisible.Attach(::GetDlgItem(m_hWnd, IDC_STRUCT_VISIBLE));
	m_btnHistogram.Attach(::GetDlgItem(m_hWnd, IDC_STRUCT_HISTO_VISIBLE));

	m_btnPrescNone.Attach(::GetDlgItem(m_hWnd, IDC_RADIO_NONE));
	m_btnPrescTarget.Attach(::GetDlgItem(m_hWnd, IDC_RADIO_TARGET));
	m_btnPrescOAR.Attach(::GetDlgItem(m_hWnd, IDC_RADIO_OAR));

	m_sliderWeight.Attach(::GetDlgItem(m_hWnd, IDC_STRUCTWEIGHT));

	m_editDose1.Attach(::GetDlgItem(m_hWnd, IDC_DOSE1_EDIT2));
	m_editDose2.Attach(::GetDlgItem(m_hWnd, IDC_DOSE2_EDIT2));

	// set defaults for read-only edit controls
	GetDlgItem(IDC_VOLUME1_EDIT)->SetWindowText("100");
	GetDlgItem(IDC_VOLUME2_EDIT)->SetWindowText("0");

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CPrescriptionToolbar::SetDocument(CBrimstoneDoc *pDoc)
{
	m_pDoc = pDoc;
}

// returns the currently selected structure
CStructure * CPrescriptionToolbar::GetSelectedStruct(void)
{
	int nIndex = m_cbSSelect.GetCurSel();
	if (nIndex != CB_ERR)
	{
		CStructure *pStruct = (CStructure *) m_cbSSelect.GetItemDataPtr(nIndex);
		return pStruct;
	}

	return NULL;
}

CVOITerm * CPrescriptionToolbar::GetSelectedPresc(void)
{
	// set prescription info
	if (NULL != m_pDoc 
		&& NULL != m_pDoc->m_pPresc.m_p)
	{
		CVOITerm *pVOIT = m_pDoc->m_pPresc->GetStructureTerm(GetSelectedStruct());
		return pVOIT;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar::UpdatePresc
// 
// updates the prescription term based on entered dose values
///////////////////////////////////////////////////////////////////////////////
void CPrescriptionToolbar::UpdatePresc(void)
{
	CVOITerm *pVOIT = GetSelectedPresc();
	if (NULL == pVOIT)
	{
		return;
	}

	CString strDose1;
	m_editDose1.GetWindowText(strDose1);

	int nDose1 = 0;
	sscanf(strDose1, "%i", &nDose1);

	CString strDose2;
	m_editDose2.GetWindowText(strDose2);

	int nDose2 = 0;
	sscanf(strDose2, "%i", &nDose2);

	if (nDose1 < nDose2)
	{
		CKLDivTerm *pKLDT = static_cast<CKLDivTerm *>(pVOIT);
		pKLDT->SetInterval((REAL) nDose1 / 100.0, 
			(REAL) nDose2 / 100.0, 1.0);
		ASSERT((int) floor(pKLDT->GetMinDose() * 100.0 + 0.5) == nDose1);
		ASSERT((int) floor(pKLDT->GetMaxDose() * 100.0 + 0.5) == nDose2);
	}
}


BEGIN_MESSAGE_MAP(CPrescriptionToolbar, CDialogBar)
	//{{AFX_MSG_MAP(CDialogBar)
	ON_CBN_SELCHANGE(IDC_STRUCTSELECT, OnSelchangeStructselectcombo)
	ON_CBN_DROPDOWN(IDC_STRUCTSELECT, OnDropdownStructselectcombo)
	ON_BN_CLICKED(IDC_STRUCT_VISIBLE, OnVisibleCheck)
	ON_BN_CLICKED(IDC_STRUCT_HISTO_VISIBLE, OnHistogramCheck)
	ON_BN_CLICKED(IDC_RADIO_NONE, OnPrescriptionChange)
	ON_BN_CLICKED(IDC_RADIO_TARGET, OnPrescriptionChange)
	ON_BN_CLICKED(IDC_RADIO_OAR, OnPrescriptionChange)
	ON_EN_CHANGE(IDC_DOSE1_EDIT2, OnDose1Changed)
	ON_EN_CHANGE(IDC_DOSE2_EDIT2, OnDose2Changed)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar message handlers

void CPrescriptionToolbar::OnSelchangeStructselectcombo() 
{
	CStructure *pStruct = GetSelectedStruct();

	// ensure structure is selected at doc level
	m_pDoc->SelectStructure(pStruct->GetName());

	// set buttons
	m_btnPrescNone.SetCheck(CStructure::eNONE == pStruct->GetType());
	m_btnPrescTarget.SetCheck(CStructure::eTARGET == pStruct->GetType());
	m_btnPrescOAR.SetCheck(CStructure::eOAR == pStruct->GetType());

	// set visibility check
	m_btnVisible.SetCheck(pStruct->IsVisible() ? 1 : 0);

	// set histogram check
	CHistogram *pHisto = m_pDoc->m_pPlan->GetHistogram(pStruct, false);
	m_btnHistogram.SetCheck(NULL != pHisto ? 1 : 0);

	// set prescription info
	CKLDivTerm *pKLDT = static_cast<CKLDivTerm *>(GetSelectedPresc());
	if (NULL != pKLDT)
	{
		m_sliderWeight.SetPos((int) (pKLDT->GetWeight() * 20.0));

		int nDose1 = (int) floor(pKLDT->GetMinDose() * 100.0 + 0.5);
		CString strDose1;
		strDose1.Format("%i", nDose1);

		int nDose2 = (int) floor(pKLDT->GetMaxDose() * 100.0 + 0.5);
		CString strDose2;
		strDose2.Format("%i", nDose2);

		m_editDose1.SetWindowText(strDose1);
		m_editDose2.SetWindowText(strDose2);
	}
	else
	{
		// make sure type is set
		ASSERT(CStructure::eNONE == pStruct->GetType());

		m_editDose1.SetWindowText("");
		m_editDose2.SetWindowText("");
	}

	// updates other controls
	OnPrescriptionChange();
}

void CPrescriptionToolbar::OnDropdownStructselectcombo() 
{
	if (m_pDoc)
	{
		m_cbSSelect.ResetContent();

		CSeries *pSeries = m_pDoc->m_pSeries;
		for (int nStruct = 0; nStruct < pSeries->GetStructureCount(); nStruct++)
		{
			CStructure *pStruct = pSeries->GetStructureAt(nStruct);
			int nIndex = m_cbSSelect.AddString(pStruct->GetName());
			m_cbSSelect.SetItemDataPtr(nIndex, (void *) pStruct);
		}
	}
}

void CPrescriptionToolbar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// get term
	CVOITerm *pVOIT = GetSelectedPresc();
	if (NULL != pVOIT)
	{
		nPos = m_sliderWeight.GetPos();
		pVOIT->SetWeight((REAL) nPos / 20.0);
	}
}


void CPrescriptionToolbar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// no function
}


void CPrescriptionToolbar::OnVisibleCheck() 
{
	CStructure *pStruct = GetSelectedStruct();
	if (NULL != pStruct)
	{
		// set visibility flag
		pStruct->SetVisible(m_btnVisible.GetCheck() == 1);

		// update views
		POSITION pos = m_pDoc->GetFirstViewPosition();
		while (pos != NULL)
		{
			CView *pView = m_pDoc->GetNextView(pos);
			pView->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}

void CPrescriptionToolbar::OnHistogramCheck()
{
	CStructure *pStruct = GetSelectedStruct();
	if (NULL != pStruct)
	{
		if (m_btnHistogram.GetCheck() == 1) 
		{
			// trigger generation of histogram
			m_pDoc->AddHistogram(pStruct);
		}
		else
		{
			// TODO: check that no prescription is present
			// trigger generation of histogram
			m_pDoc->RemoveHistogram(pStruct);
			
			// TODO: fix this with direct call to CBrimstoneView
			// m_pDoc->UpdateAllViews(NULL, 
			//	(m_btnHistogram.GetCheck() == 1) ? IDD_ADDHISTO : IDD_REMOVEHISTO, pStruct);
		}
	}
}

void CPrescriptionToolbar::OnPrescriptionChange()
{
	// create a new prescription object
	CStructure *pStruct = GetSelectedStruct();
	if (NULL == pStruct)
	{
		return;
	}

	// set structure type
	pStruct->SetType(CStructure::eNONE);
	if (1 == m_btnPrescTarget.GetCheck())
	{
		pStruct->SetType(CStructure::eTARGET);
	}
	else if (1 == m_btnPrescOAR.GetCheck())
	{
		pStruct->SetType(CStructure::eOAR);
	}

	// update include flags to reflect changed types
	m_pDoc->m_pPresc->SetElementInclude();

	// set visibility / histogram options
	if (CStructure::eNONE != pStruct->GetType())
	{
		// see if prescription exists
		CVOITerm *pVOIT = GetSelectedPresc();
		if (NULL == pVOIT)
		{
			// need to create new term
			CKLDivTerm *pKLDT = new CKLDivTerm(pStruct, 2.5);

			// NOTE: must AddStructureTerm before SetInterval, because it sets up binning parameters
			m_pDoc->AddStructTerm(pKLDT);
		
			REAL dose1 = (CStructure::eTARGET == pStruct->GetType()) ? 0.60 : 0.0;
			REAL dose2 = (CStructure::eTARGET == pStruct->GetType()) ? 0.70 : 0.30;

			// sets the term prescription interval
			pKLDT->SetInterval(dose1, dose2, 1.0);

			// update tool bar
			OnSelchangeStructselectcombo();

			// update other views
			m_pDoc->AddHistogram(pStruct);
			// TODO: fix this with direct call to CBrimstoneView
			// m_pDoc->UpdateAllViews(NULL, IDD_ADDHISTO, pStruct);
		}

		// make sure visible
		m_btnVisible.SetCheck(1);

		// make sure histogram
		m_btnHistogram.SetCheck(1);
	}

	// make sure visible enabled
	m_btnVisible.EnableWindow(CStructure::eNONE == pStruct->GetType());

	// make sure histogram enabled
	m_btnHistogram.EnableWindow(CStructure::eNONE == pStruct->GetType());

	// set prescription control options
	m_sliderWeight.EnableWindow(CStructure::eNONE != pStruct->GetType());

	// TODO: make the rest as controls
	GetDlgItem(IDC_DOSE1_EDIT2)->EnableWindow(CStructure::eNONE != pStruct->GetType());
	GetDlgItem(IDC_DOSE2_EDIT2)->EnableWindow(CStructure::eNONE != pStruct->GetType());

	GetDlgItem(IDC_VOLUME1_EDIT)->EnableWindow(CStructure::eNONE != pStruct->GetType());
	GetDlgItem(IDC_VOLUME2_EDIT)->EnableWindow(CStructure::eNONE != pStruct->GetType());
}

void CPrescriptionToolbar::OnDose1Changed()
{
	UpdatePresc();
}

void CPrescriptionToolbar::OnDose2Changed()
{
	UpdatePresc();
}



void CPrescriptionToolbar::OnDestroy()
{
	CDialogBar::OnDestroy();

	// CDialogBar::OnInitDialog();
	m_cbSSelect.Detach();

	m_btnVisible.Detach();
	m_btnHistogram.Detach();

	m_btnPrescNone.Detach();
	m_btnPrescTarget.Detach();
	m_btnPrescOAR.Detach();

	m_sliderWeight.Detach();

	m_editDose1.Detach();
	m_editDose2.Detach();

}
