// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Brimstone.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndPrescToolBar.Create(this, IDD_PRESCDLG, 
		CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT | CBRS_GRIPPER | CBRS_TOOLTIPS, 
		IDC_PRESCTOOLBAR))
	{
		TRACE0("Failed to create mainbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	EnableDocking(CBRS_ALIGN_ANY);

	m_wndPrescToolBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBar(&m_wndPrescToolBar, AFX_IDW_DOCKBAR_RIGHT);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	SetTimer(8, 50, NULL);

	return 0;
}


void CMainFrame::ActivateFrame(int nCmdShow) 
{
	CDocument *pDoc = GetActiveDocument();
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CBrimstoneDoc)));
	m_wndPrescToolBar.SetDocument((CBrimstoneDoc *) pDoc);

	CFrameWnd::ActivateFrame(nCmdShow);
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CBrimstoneDoc *pDoc = (CBrimstoneDoc *) GetActiveDocument();

	if (pDoc)
	{
		CString strMessage;
		if (pDoc->m_pOptThread->m_bDone)
		{
			strMessage = "Done.";
		}
		else
		{
			for (int nAt = 0; nAt < pDoc->m_pOptThread->nIteration; nAt++)
			{
				strMessage += "*";
			}
		}
		m_wndStatusBar.SetPaneText(0, strMessage);
	}
	
	CFrameWnd::OnTimer(nIDEvent);
}
