// StructureCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "vsim_ogl.h"
#include "StructureCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStructureCtrl dialog


CStructureCtrl::CStructureCtrl() // CWnd* pParent /*=NULL*/)
	// : CDialogBar(CStructureCtrl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStructureCtrl)
	m_bStructEnabled1 = FALSE;
	m_bStructEnabled2 = FALSE;
	m_bStructEnabled3 = FALSE;
	m_bStructEnabled4 = FALSE;
	m_bStructEnabled5 = FALSE;
	m_bStructEnabled6 = FALSE;
	m_bStructEnabled7 = FALSE;
	m_bStructEnabled8 = FALSE;
	//}}AFX_DATA_INIT
}


void CStructureCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate)
	{
		m_bStructEnabled1 = m_arrEnabled[0];
		m_bStructEnabled2 = m_arrEnabled[1];
		m_bStructEnabled3 = m_arrEnabled[2];
		m_bStructEnabled4 = m_arrEnabled[3];
		m_bStructEnabled5 = m_arrEnabled[4];
		m_bStructEnabled6 = m_arrEnabled[5];
		m_bStructEnabled7 = m_arrEnabled[6];
		m_bStructEnabled8 = m_arrEnabled[7];
	}

	//{{AFX_DATA_MAP(CStructureCtrl)
	DDX_Check(pDX, IDC_CHECK1, m_bStructEnabled1);
	DDX_Check(pDX, IDC_CHECK2, m_bStructEnabled2);
	DDX_Check(pDX, IDC_CHECK3, m_bStructEnabled3);
	DDX_Check(pDX, IDC_CHECK4, m_bStructEnabled4);
	DDX_Check(pDX, IDC_CHECK5, m_bStructEnabled5);
	DDX_Check(pDX, IDC_CHECK6, m_bStructEnabled6);
	DDX_Check(pDX, IDC_CHECK7, m_bStructEnabled7);
	DDX_Check(pDX, IDC_CHECK8, m_bStructEnabled8);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		m_arrEnabled[0] = m_bStructEnabled1;
		m_arrEnabled[1] = m_bStructEnabled2;
		m_arrEnabled[2] = m_bStructEnabled3;
		m_arrEnabled[3] = m_bStructEnabled4;
		m_arrEnabled[4] = m_bStructEnabled5;
		m_arrEnabled[5] = m_bStructEnabled6;
		m_arrEnabled[6] = m_bStructEnabled7;
		m_arrEnabled[7] = m_bStructEnabled8;
	}
}


BEGIN_MESSAGE_MAP(CStructureCtrl, CDialogBar)
	//{{AFX_MSG_MAP(CStructureCtrl)
	ON_BN_CLICKED(IDC_CHECK1, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK2, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK3, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK4, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK5, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK6, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK7, OnApplyChanges)
	ON_BN_CLICKED(IDC_CHECK8, OnApplyChanges)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStructureCtrl message handlers

void CStructureCtrl::OnApplyChanges() 
{
	BOOL bResult = UpdateData(TRUE);
	ASSERT(bResult);
}
