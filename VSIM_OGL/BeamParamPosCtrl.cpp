// BeamParamPosCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "vsim_ogl.h"
#include "BeamParamPosCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBeamParamPosCtrl dialog


CBeamParamPosCtrl::CBeamParamPosCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CBeamParamPosCtrl::IDD, pParent),
		m_pBeam(NULL)
{
	//{{AFX_DATA_INIT(CBeamParamPosCtrl)
	m_nCouchAngle = 0;
	m_nGantryAngle = 0;
	m_nTableX = 0;
	m_nTableY = 0;
	m_nTableZ = 0;
	//}}AFX_DATA_INIT
}


void CBeamParamPosCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// if loading the data into the dialog box
	if (!pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		m_nGantryAngle = (int)(m_pBeam->GetGantryAngle() * 180.0 / PI);
		m_nCouchAngle  = (int)(m_pBeam->GetCouchAngle()  * 180.0 / PI);

		const CVector<3>& vOffset = m_pBeam->GetTableOffset();
		m_nTableX = (int) vOffset[0];
		m_nTableY = (int) vOffset[1];
		m_nTableZ = (int) vOffset[2];
	}
	
	//{{AFX_DATA_MAP(CBeamParamPosCtrl)
	DDX_Text(pDX, IDC_EDIT_GANTRY, m_nGantryAngle);
	DDV_MinMaxInt(pDX, m_nGantryAngle, 0, 359);
	DDX_Text(pDX, IDC_EDIT_COUCH, m_nCouchAngle);
	DDV_MinMaxInt(pDX, m_nCouchAngle, 0, 359);
	DDX_Text(pDX, IDC_EDIT_TABLEX, m_nTableX);
	DDV_MinMaxInt(pDX, m_nTableX, -200, 200);
	DDX_Text(pDX, IDC_EDIT_TABLEY, m_nTableY);
	DDV_MinMaxInt(pDX, m_nTableY, -200, 200);
	DDX_Text(pDX, IDC_EDIT_TABLEZ, m_nTableZ);
	DDV_MinMaxInt(pDX, m_nTableZ, -200, 200);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		// m_pBeam->gantryAngle.RemoveObserver(this, (ChangeFunction) OnChange);
		m_pBeam->SetGantryAngle(((double)m_nGantryAngle) * PI / 180.0);
		//m_pBeam->gantryAngle.AddObserver(this, (ChangeFunction) OnChange);

		// m_pBeam->couchAngle.RemoveObserver(this, (ChangeFunction) OnChange);
		m_pBeam->SetCouchAngle(((double)m_nCouchAngle) * PI / 180.0);
		// m_pBeam->couchAngle.AddObserver(this, (ChangeFunction) OnChange);

		CVector<3> vOffset(m_nTableX, m_nTableY, m_nTableZ);
		m_pBeam->SetTableOffset(vOffset);
	}
}

void CBeamParamPosCtrl::SetBeam(CBeam *pBeam)
{
	m_pBeam = pBeam;

	if (::IsWindow(m_hWnd))
	{
		UpdateData(FALSE);
	}
}

BEGIN_MESSAGE_MAP(CBeamParamPosCtrl, CDialog)
	//{{AFX_MSG_MAP(CBeamParamPosCtrl)
	ON_EN_CHANGE(IDC_EDIT_GANTRY, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_GANTRY, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_COUCH, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COUCH, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_TABLEX, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TABLEX, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_TABLEY, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TABLEY, OnApplySpinChanges)
	ON_EN_CHANGE(IDC_EDIT_TABLEZ, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TABLEZ, OnApplySpinChanges)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBeamParamPosCtrl message handlers

BOOL CBeamParamPosCtrl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// set the range for the spin buttons
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_GANTRY))->SetRange(0, 359);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_COUCH))->SetRange(0, 359);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_TABLEX))->SetRange(-200, 200);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_TABLEY))->SetRange(-200, 200);
	((CSpinButtonCtrl *) GetDlgItem(IDC_SPIN_TABLEZ))->SetRange(-200, 200);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBeamParamPosCtrl::OnApplyChanges() 
{
	if (::IsWindow(m_hWnd) && m_pBeam != NULL)
	{
		BOOL bResult = UpdateData(TRUE);
		ASSERT(bResult);

		// AfxGetMainWnd()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}
}

void CBeamParamPosCtrl::OnApplySpinChanges(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	OnApplyChanges();
	
	*pResult = 0;
}
