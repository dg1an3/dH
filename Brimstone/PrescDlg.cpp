// PrescDlg.cpp : implementation file
//

#include "stdafx.h"
#include "brimstone.h"
#include "PrescDlg.h"

#include "Graph.h"

#include <Series.h>
#include <Prescription.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrescDlg dialog


CPrescDlg::CPrescDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrescDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrescDlg)
	m_Dose1 = 0.0;
	m_Dose2 = 0.0;
	m_Volume1 = 0.0;
	//}}AFX_DATA_INIT
}


void CPrescDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrescDlg)
	DDX_Control(pDX, IDC_STRUCTCOMBO, m_StructCombo);
	DDX_Text(pDX, IDC_DOSE2_EDIT, m_Dose2);
	DDV_MinMaxDouble(pDX, m_Dose2, 0., 10.);
	DDX_Text(pDX, IDC_VOLUME1_EDIT, m_Volume1);
	DDV_MinMaxDouble(pDX, m_Volume1, 0., 1.);
	DDX_Text(pDX, IDC_DOSE1_EDIT, m_Dose1);
	DDV_MinMaxDouble(pDX, m_Dose1, 0., 10.);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrescDlg, CDialog)
	//{{AFX_MSG_MAP(CPrescDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescDlg message handlers

BOOL CPrescDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for (int nStruct = 0; nStruct < m_pSeries->GetStructureCount(); nStruct++)
	{
		CStructure *pStruct = m_pSeries->GetStructureAt(nStruct);
		int nIndex = m_StructCombo.AddString(pStruct->GetName());
		m_StructCombo.SetItemDataPtr(nIndex, (void *) pStruct);
	} 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrescDlg::OnOK() 
{
	CDialog::OnOK();

	m_pStruct = (CStructure *) m_StructCombo.GetItemDataPtr(m_StructCombo.GetCurSel());

	CKLDivTerm *pKLDT1 = new CKLDivTerm(m_pStruct, 2.5);
	m_pPrescParam->AddStructureTerm(pKLDT1);

	CKLDivTerm *pKLDT2 = new CKLDivTerm(m_pStruct, 2.5);
	m_pPresc->AddStructureTerm(pKLDT2);
	
	pKLDT1->SetInterval(m_Dose1, m_Dose2, 1.0, TRUE);
	pKLDT2->SetInterval(m_Dose1, m_Dose2, 1.0, TRUE);

	m_pPresc->SetElementInclude();

	m_pPresc->UpdateTerms(m_pPrescParam);
	m_pPrescParam->SetElementInclude();
}
