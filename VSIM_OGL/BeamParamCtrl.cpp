// BeamParamCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "VSIM_OGL.h"
#include "BeamParamCtrl.h"

#include <Plan.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCtrl dialog


CBeamParamCtrl::CBeamParamCtrl() // CWnd* pParent /*=NULL*/)
	: m_pBeam(NULL) // CDialogBar(CBeamParamCtrl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBeamParamCtrl)
	m_nCollimAngle = 0;
	m_nGantryAngle = 0;
	m_nCouchAngle = 0;
	m_jawx1 = 0.0;
	m_jawx2 = 0.0;
	//}}AFX_DATA_INIT
}


void CBeamParamCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	// if loading the data into the dialog box
	if (!pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		m_nCollimAngle = (int)(m_pBeam->GetCollimAngle() * 180.0 / PI);
		m_nGantryAngle = (int)(m_pBeam->GetGantryAngle() * 180.0 / PI);
		m_nCouchAngle  = (int)(m_pBeam->GetCouchAngle()  * 180.0 / PI);

		m_jawx1 = m_pBeam->GetCollimMin()[0];
		m_jawx2 = m_pBeam->GetCollimMax()[0];
	}
	
	//{{AFX_DATA_MAP(CBeamParamCtrl)
	DDX_Control(pDX, IDC_SPIN_COUCH, m_spinCouch);
	DDX_Control(pDX, IDC_SPIN_GANTRY, m_spinGantry);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
	DDX_Text(pDX, IDC_EDIT_GANTRY, m_nGantryAngle);
	DDV_MinMaxInt(pDX, m_nGantryAngle, 0, 359);
	DDX_Text(pDX, IDC_EDIT_COUCH, m_nCouchAngle);
	DDV_MinMaxInt(pDX, m_nCouchAngle, 0, 359);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pBeam != NULL)
	{
		m_pBeam->SetAngles(((double)m_nCollimAngle) * PI / 180.0,
			((double)m_nGantryAngle)  * PI / 180.0,
			((double)m_nCouchAngle)  * PI / 180.0);

		CVector<2> vMin = m_pBeam->GetCollimMin();
		vMin[0] = m_jawx1;
		m_pBeam->SetCollimMin(vMin);

		CVector<2> vMax = m_pBeam->GetCollimMax();
		vMax[0] = m_jawx2;
		m_pBeam->SetCollimMax(vMax);
	}
	
}


BEGIN_MESSAGE_MAP(CBeamParamCtrl, CDialogBar)
	//{{AFX_MSG_MAP(CBeamParamCtrl)
    ON_MESSAGE( WM_INITDIALOG, OnInitDialog )
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_EN_KILLFOCUS(IDC_EDIT_GANTRY, OnApplyChanges)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_GANTRY, OnApplyChanges)
	ON_EN_KILLFOCUS(IDC_EDIT_COUCH, OnApplyChanges)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

	// ON_EN_CHANGE(IDC_EDIT_GANTRY, OnApplyChanges)
	// ON_EN_KILLFOCUS(IDC_EDIT_COLLIM, OnApplyChanges)
	// ON_EN_KILLFOCUS(IDC_EDIT_JAWX1, OnApplyChanges)
	// ON_EN_KILLFOCUS(IDC_EDIT_JAWX2, OnApplyChanges)
	// ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLLIM, OnDeltaposSpinCollim)
	// ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COUCH, OnDeltaposSpinCouch)
	// ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_GANTRY, OnDeltaposSpinGantry)

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCtrl message handlers

LONG CBeamParamCtrl::OnInitDialog( UINT wParam, LONG lParam )
{
    if ( !HandleInitDialog( wParam, lParam ) || !UpdateData( FALSE ) )
    {
        TRACE0( "Warning:  UpdateData failed during CWinLevCtrl dialog "
                "initialization\n" );
        return FALSE;
    }

	// initialize the tab control
	m_tabCtrl.InsertItem(0, "Position");
	m_tabCtrl.InsertItem(1, "Collim");
	m_tabCtrl.InsertItem(2, "Blocks");

	m_spinGantry.SetRange(0, 359);
	m_spinCouch.SetRange(0, 359);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBeamParamCtrl::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CBeamParamCtrl::OnApplyChanges() 
{
	BOOL bResult = UpdateData(TRUE);
	ASSERT(bResult);
}

void CBeamParamCtrl::SetBeam(CBeam *pBeam)
{
	m_pBeam = pBeam;

	// load the beam's data into the dialog box
	UpdateData(FALSE);
}

#define ANGLE_DELTA (5.0)

void CBeamParamCtrl::OnDeltaposSpinCollim(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (m_pBeam)
	{
		double delta = - (double) (pNMUpDown->iDelta) * ANGLE_DELTA;
		m_pBeam->SetAngles(((double) m_nCollimAngle + delta) * PI / 180.0, 
			(double) m_nGantryAngle * PI / 180.0, 
			(double) m_nCouchAngle * PI / 180.0);

		// load the data into the controls
		UpdateData(FALSE);

		CFrameWnd *pFrame = (CFrameWnd *)AfxGetMainWnd();
		pFrame->GetActiveView()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}

	*pResult = 0;
}

void CBeamParamCtrl::OnDeltaposSpinCouch(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (m_pBeam)
	{
		double delta = - (double) (pNMUpDown->iDelta) * ANGLE_DELTA;
		m_pBeam->SetAngles((double) m_nCollimAngle * PI / 180.0, 
			(double) m_nGantryAngle * PI / 180.0, 
			((double) m_nCouchAngle + delta) * PI / 180.0);

		// load the data into the controls
		UpdateData(FALSE);

		CFrameWnd *pFrame = (CFrameWnd *)AfxGetMainWnd();
		pFrame->GetActiveView()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}

	*pResult = 0;
}

void CBeamParamCtrl::OnDeltaposSpinGantry(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if (m_pBeam)
	{
		double delta = - (double) (pNMUpDown->iDelta) * ANGLE_DELTA;
		m_pBeam->SetAngles((double) m_nCollimAngle * PI / 180.0, 
			( (double) m_nGantryAngle + delta ) * PI / 180.0, 
			(double) m_nCouchAngle * PI / 180.0);

		// load the data into the controls
		UpdateData(FALSE);

		CFrameWnd *pFrame = (CFrameWnd *)AfxGetMainWnd();
		pFrame->GetActiveView()->RedrawWindow(NULL, NULL, RDW_UPDATENOW);
	}

	*pResult = 0;
}
 
