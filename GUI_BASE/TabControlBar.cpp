// TabControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "..\VSIM_OGL\resource.h"
#include "TabControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabControlBar dialog


CTabControlBar::CTabControlBar()
{
	//{{AFX_DATA_INIT(CTabControlBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabControlBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabControlBar)
	DDX_Control(pDX, IDC_TABCTRL, m_tabCtrl);
	DDX_Control(pDX, IDC_CHILDRECT, m_btnRect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabControlBar, CDialogBar)
	//{{AFX_MSG_MAP(CTabControlBar)
    ON_MESSAGE( WM_INITDIALOG, OnInitDialog )
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRL, OnSelchangeTabctrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabControlBar message handlers

LONG CTabControlBar::OnInitDialog( UINT wParam, LONG lParam )
{
    if ( !HandleInitDialog( wParam, lParam ) || !UpdateData( FALSE ) )
    {
        TRACE0( "Warning:  UpdateData failed during CWinLevCtrl dialog "
                "initialization\n" );
        return FALSE;
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabControlBar::OnSelchangeTabctrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	for (int nAt = 0; nAt < m_arrTabs.GetSize(); nAt++)
	{
		// get the newly selected child
		CDialog *pTab = (CDialog *) m_arrTabs[nAt];

		// show it
		pTab->ShowWindow(nAt == m_tabCtrl.GetCurSel() ? SW_SHOW : SW_HIDE);
	}
	
	*pResult = 0;
}

int CTabControlBar::AddTab(const CString &strName, CDialog *pNewTab)
						   // CRuntimeClass *pRuntimeClass, UINT nIDTemplate)
{
	// CDialog *pNewTab = (CDialog *)pRuntimeClass->CreateObject();
	// ASSERT(pNewTab->IsKindOf(RUNTIME_CLASS(CDialog)));
	// pNewTab->Create(nIDTemplate, this);

	// position the newly created child
	CRect rect;
	m_btnRect.GetWindowRect(&rect);
	ScreenToClient(&rect);
	pNewTab->MoveWindow(&rect);

	// add it to the map
	int nIndex = m_arrTabs.Add(pNewTab);
	m_tabCtrl.InsertItem(nIndex, strName);

	// now show/hide the created child
	pNewTab->ShowWindow(nIndex == 0 ? SW_SHOW : SW_HIDE);

	return 0;
}
