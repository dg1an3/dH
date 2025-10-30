// PlanSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "brimstone.h"
#include "PlanSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg dialog


CPlanSetupDlg::CPlanSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlanSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlanSetupDlg)
	m_nBeamCount = 1;
	m_isoX = 0.0;
	m_isoY = 0.0;
	//}}AFX_DATA_INIT
}


void CPlanSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlanSetupDlg)
	DDX_Text(pDX, IDC_EDIT_BEAMCOUNT, m_nBeamCount);
	DDV_MinMaxUInt(pDX, m_nBeamCount, 1, 99);
	DDX_Text(pDX, IDC_EDIT_ISO_OFS_X, m_isoX);
	DDX_Text(pDX, IDC_EDIT_ISO_OFS_Y, m_isoY);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlanSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CPlanSetupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg message handlers

