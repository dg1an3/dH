// ObjectExplorerControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "GUI_BASE_resource.h"
#include "ObjectExplorerControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CObjectExplorerControlBar dialog


CObjectExplorerControlBar::CObjectExplorerControlBar()
{
	//{{AFX_DATA_INIT(CObjectExplorerControlBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CObjectExplorerControlBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectExplorerControlBar)
	// DDX_Control(pDX, IDC_TREE1, m_ExplorerCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectExplorerControlBar, CDialogBar)
	//{{AFX_MSG_MAP(CObjectExplorerControlBar)
    ON_MESSAGE( WM_INITDIALOG, OnInitDialog )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectExplorerControlBar message handlers

LONG CObjectExplorerControlBar::OnInitDialog(UINT wParam, LONG lParam) 
{
	CRect rect;
	GetDlgItem(IDC_TREECTRLRECT)->GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (!m_ExplorerCtrl.Create(WS_VISIBLE | WS_CHILD 
		| TVS_HASLINES // | TVS_LINESATROOT 
		| TVS_HASBUTTONS | TVS_CHECKBOXES, 
		rect, this, IDC_TREECTRLRECT+1))
		return FALSE;

    if ( !HandleInitDialog( wParam, lParam ) || !UpdateData( FALSE ) )
    {
        TRACE0( "Warning:  UpdateData failed during CWinLevCtrl dialog "
                "initialization\n" );
        return FALSE;
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
