// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "VSIM_OGL.h"

#include "MainFrm.h"
#include "SimView.h"

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
	ON_COMMAND_EX(ID_VIEW_BEAMPARAM, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BEAMPARAM, OnUpdateControlBarMenu)
	ON_WM_CLOSE()
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
: m_bViewBeam(TRUE),
	m_bViewLightpatch(FALSE)
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
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

	if (!m_wndBeamParamCtrl	.Create(this, IDD_TABCONTROLBAR, 
		WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_RIGHT 
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, 
		ID_VIEW_BEAMPARAM))
	{
		return -1;
	}

	m_wndPosCtrl.Create(CBeamParamPosCtrl::IDD, &m_wndBeamParamCtrl);
	m_wndBeamParamCtrl.AddTab("Position", &m_wndPosCtrl);

	m_wndCollimCtrl.Create(CBeamParamCollimCtrl::IDD, &m_wndBeamParamCtrl);
	m_wndBeamParamCtrl.AddTab("Collim", &m_wndCollimCtrl);

//	if (!m_wndStructCtrl.Create(this, IDD_STRUCTUREDIALOG, 
//		WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_RIGHT 
//		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, 
//		ID_VIEW_STRUCTURE))
//	{
//		return -1;
//	}

	if (!m_wndExplorerCtrl.Create(this, IDD_OBJECTEXPLORERCONTROLBAR, 
		WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_RIGHT 
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, 
		ID_VIEW_STRUCTURE))
	{
		return -1;
	}
	m_wndExplorerCtrl.m_ExplorerCtrl.SetBkColor(RGB(196, 196, 196));
	// m_wndExplorerCtrl.m_ExplorerCtrl.SetTextColor(RGB(255, 255, 255));
	CFont *pFont = new CFont();
	pFont->CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 0, 
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		"Arial");
	m_wndExplorerCtrl.m_ExplorerCtrl.SetFont(pFont);

	// free the font
	// delete pFont;

	EnableDocking(CBRS_ALIGN_ANY);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	m_wndBeamParamCtrl.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBar(&m_wndBeamParamCtrl, AFX_IDW_DOCKBAR_RIGHT);

//	m_wndStructCtrl.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
//	DockControlBar(&m_wndStructCtrl, AFX_IDW_DOCKBAR_LEFT);

	m_wndExplorerCtrl.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBar(&m_wndExplorerCtrl, AFX_IDW_DOCKBAR_LEFT);

	// now restore the state of the control bars
	LoadBarState("ControlBars");

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
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

void CMainFrame::OnClose() 
{
	// save the state of the control bars
	SaveBarState("ControlBars");

	CFrameWnd::OnClose();
}
