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
		m_bStructEnabled1 = arrEnabled[0].Get();
		m_bStructEnabled2 = arrEnabled[1].Get();
		m_bStructEnabled3 = arrEnabled[2].Get();
		m_bStructEnabled4 = arrEnabled[3].Get();
		m_bStructEnabled5 = arrEnabled[4].Get();
		m_bStructEnabled6 = arrEnabled[5].Get();
		m_bStructEnabled7 = arrEnabled[6].Get();
		m_bStructEnabled8 = arrEnabled[7].Get();
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
		arrEnabled[0].Set(m_bStructEnabled1);
		arrEnabled[1].Set(m_bStructEnabled2);
		arrEnabled[2].Set(m_bStructEnabled3);
		arrEnabled[3].Set(m_bStructEnabled4);
		arrEnabled[4].Set(m_bStructEnabled5);
		arrEnabled[5].Set(m_bStructEnabled6);
		arrEnabled[6].Set(m_bStructEnabled7);
		arrEnabled[7].Set(m_bStructEnabled8);
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
