// VSIM_OGL.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VSIM_OGL.h"

#include "MainFrm.h"

#include <Plan.h>
#include "simview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVSIM_OGLApp

BEGIN_MESSAGE_MAP(CVSIM_OGLApp, CWinApp)
	//{{AFX_MSG_MAP(CVSIM_OGLApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSIM_OGLApp construction

CVSIM_OGLApp::CVSIM_OGLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVSIM_OGLApp object

CVSIM_OGLApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVSIM_OGLApp initialization

BOOL CVSIM_OGLApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	if (CoInitialize(NULL) != S_OK)
		return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;

// #define DEFAULT_VIEW
#ifdef DEFAULT_VIEW
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CVSIM_OGLDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CVSIM_OGLView));
	AddDocTemplate(pDocTemplate); 
#else
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPlan),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSimView));
	AddDocTemplate(pDocTemplate);

	// create the series doc template
	//		don't add it as a template to the application, because we
	//		don't want series to be created new
	m_pSeriesDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSeries),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSimView));
	CPlan::SetSeriesDocTemplate(m_pSeriesDocTemplate);
#endif

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// get a pointer to the main frame
	CMainFrame *pMainFrame = (CMainFrame *)AfxGetMainWnd();

	// The one and only window has been initialized, so show and update it.
	pMainFrame->ShowWindow(SW_SHOW);  // SW_MAXIMIZE);
	pMainFrame->UpdateWindow();

	// synchronize the current beam pointers
	CSimView *pView = (CSimView *)pMainFrame->GetActiveView();
	if (pView)
	{
		pMainFrame->m_wndPosCtrl.SetBeam(pView->GetCurrentBeam());
		pMainFrame->m_wndCollimCtrl.SetBeam(pView->GetCurrentBeam());
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CVSIM_OGLApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CVSIM_OGLApp message handlers


int CVSIM_OGLApp::ExitInstance() 
{
	// uninitialize COM
	CoUninitialize();

	// exit the application
	int nResult = CWinApp::ExitInstance();

	// close any opened series
	m_pSeriesDocTemplate->CloseAllDocuments(TRUE);	
	delete m_pSeriesDocTemplate;

	return nResult;
}

void CVSIM_OGLApp::OnFileOpen() 
{
	// close any open documents
	m_pDocManager->CloseAllDocuments(FALSE);
	m_pSeriesDocTemplate->CloseAllDocuments(FALSE);

	// now open the new one
	m_pDocManager->OnFileOpen();
}

CDocument* CVSIM_OGLApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	// open the plan file
	CPlan *pPlan = (CPlan *)CWinApp::OpenDocumentFile(lpszFileName);

	// check the type of the returned object
	ASSERT(pPlan->IsKindOf(RUNTIME_CLASS(CPlan)));

	// create a temporary sphere object
	pPlan->GetSeries()->CreateSphereStructure("Sphere");

	// return the plan file
	return pPlan;
}

BOOL CVSIM_OGLApp::OnIdle(LONG lCount) 
{
	if (lCount % 4 != 0)
		return TRUE;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CSimView *pView = (CSimView *)pFrame->GetActiveView();

	return TRUE; // CWinApp::OnIdle(lCount);
}

void CVSIM_OGLApp::OnFileSave() 
{
}
