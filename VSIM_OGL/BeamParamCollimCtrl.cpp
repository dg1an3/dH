// BeamParamCollimCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "vsim_ogl.h"
#include "BeamParamCollimCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCollimCtrl dialog


CBeamParamCollimCtrl::CBeamParamCollimCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CBeamParamCollimCtrl::IDD, pParent),
		m_pBeam(NULL)
{
	//{{AFX_DATA_INIT(CBeamParamCollimCtrl)
	m_nCollimAngle = 0;
	m_nJawX1 = -20;
	m_nJawX2 = 20;
	m_nJawY1 = -20;
	m_nJawY2 = 20;
	//}}AFX_DATA_INIT
}

BOOL m_bUpdatingData = FALSE;

void CBeamParamCollimCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// if loading the data into the dialog box
	if (!pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		m_nCollimAngle = (int)(m_pBeam->GetCollimAngle()
			* 180.0 / PI);

		m_nJawX1 = (int) m_pBeam->GetCollimMin()[0];
		m_nJawX2 = (int) m_pBeam->GetCollimMax()[0];

		m_nJawY1 = (int) m_pBeam->GetCollimMin()[1];
		m_nJawY2 = (int) m_pBeam->GetCollimMax()[1];
	}
	
	//{{AFX_DATA_MAP(CBeamParamCollimCtrl)
	DDX_Text(pDX, IDC_EDIT_COLLIM, m_nCollimAngle);
	DDV_MinMaxInt(pDX, m_nCollimAngle, 0, 359);
	DDX_Text(pDX, IDC_EDIT_JAWX1, m_nJawX1);
	DDV_MinMaxInt(pDX, m_nJawX1, -20, 20);
	DDX_Text(pDX, IDC_EDIT_JAWX2, m_nJawX2);
	DDV_MinMaxInt(pDX, m_nJawX2, -20, 20);
	DDX_Text(pDX, IDC_EDIT_JAWY1, m_nJawY1);
	DDV_MinMaxInt(pDX, m_nJawY1, -20, 20);
	DDX_Text(pDX, IDC_EDIT_JAWY2, m_nJawY2);
	DDV_MinMaxInt(pDX, m_nJawY2, -20, 20);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		m_bUpdatingData = TRUE;
//		forBeam->SetAngles(((double)m_nCollimAngle) * PI / 180.0,
//			forBeam->myGantryAngle.Get(), forBeam->myCouchAngle.Get());

		m_pBeam->SetCollimAngle(((double)m_nCollimAngle) * PI / 180.0);

		CVector<2> vMin(m_nJawX1, m_nJawY1);
		m_pBeam->SetCollimMin(vMin);

		CVector<2> vMax(m_nJawX2, m_nJawY2);
		m_pBeam->SetCollimMax(vMax);

		m_bUpdatingData = FALSE;
	}
	
}

void CBeamParamCollimCtrl::SetBeam(CBeam *pBeam)
{
	m_pBeam = pBeam;

	if (::IsWindow(m_hWnd) && !m_bUpdatingData)
	{
		UpdateData(FALSE);
	}
}

BEGIN_MESSAGE_MAP(CBeamParamCollimCtrl, CDialog)
	//{{AFX_MSG_MAP(CBeamParamCollimCtrl)
	ON_EN_CHANGE(IDC_EDIT_COLLIM, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLLIM, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_JAWX1, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_JAWX1, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_JAWX2, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_JAWX2, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_JAWY1, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_JAWY1, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_JAWY2, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_JAWY2, OnApplySpinChanges)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCollimCtrl message handlers

BOOL CBeamParamCollimCtrl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// set the range for the spin buttons
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_COLLIM))->SetRange(0, 359);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_JAWX1))->SetRange(-20, 20);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_JAWX2))->SetRange(-20, 20);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_JAWY1))->SetRange(-20, 20);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_JAWY2))->SetRange(-20, 20);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBeamParamCollimCtrl::OnApplyChanges() 
{
	if (::IsWindow(m_hWnd) && m_pBeam != NULL)
	{
		BOOL bResult = UpdateData(TRUE);
		ASSERT(bResult);

		// AfxGetMainWnd()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}
}

void CBeamParamCollimCtrl::OnApplySpinChanges(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	OnApplyChanges();
	
	*pResult = 0;
}
