// PenBeamEdit.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PenBeamEdit.h"

#include "MainFrm.h"
// #include "PenBeamEditDoc.h"
#include "PenBeamEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

template<class VOXEL_TYPE>
BOOL ReadAsciiImage(LPCTSTR lpszPathName, CVolume<VOXEL_TYPE> *pImage, double weight = 1.0)
{
	CStdioFile inFile;
	
	if (!inFile.Open(lpszPathName, CFile::modeRead | CFile::typeText))
		return FALSE;

	int nSize = 0;
	CArray<VOXEL_TYPE, VOXEL_TYPE> arrIntensity;
	CString strLine;
	while (inFile.ReadString(strLine))
	{
		strLine.TrimLeft();
		strLine.TrimRight();
		if (strLine == "")
		{
			if (nSize == 0)
				nSize = arrIntensity.GetSize();
		}
		else
		{
			double density;
			sscanf(strLine.GetBuffer(100), "%lf", &density);
			arrIntensity.Add((VOXEL_TYPE) (density * weight));
		}
	}

	pImage->SetDimensions(nSize, nSize, 1);

	int nAtVoxel = 0;
	for (int nAtY = 0; nAtY < nSize; nAtY++)
	{
		for (int nAtX = 0; nAtX < nSize; nAtX++)
		{
			pImage->GetVoxels()[0][nAtY][nAtX] = arrIntensity[nAtVoxel++];
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditApp

BEGIN_MESSAGE_MAP(CPenBeamEditApp, CWinApp)
	//{{AFX_MSG_MAP(CPenBeamEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditApp construction

CPenBeamEditApp::CPenBeamEditApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPenBeamEditApp object

CPenBeamEditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditApp initialization

BOOL CPenBeamEditApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

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
#if OLD
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPenBeamEditDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CPenBeamEditView));
	AddDocTemplate(pDocTemplate);
#else
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPlan),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CPenBeamEditView));
	AddDocTemplate(pDocTemplate);

	// create the series doc template
	//		don't add it as a template to the application, because we
	//		don't want series to be created new
	m_pSeriesDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSeries),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CPenBeamEditView));
	CPlan::SetSeriesDocTemplate(m_pSeriesDocTemplate);
#endif
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

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
void CPenBeamEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditApp message handlers


void CPenBeamEditApp::OnFileImport() 
{
	// ask the user for the import directory
	CFileDialog dlgFile(TRUE, "dat", "density.dat");
	if (dlgFile.DoModal() != IDOK)
		return;

	// set a pointer to the selected path
	CString strPath = dlgFile.GetPathName();
	strPath = strPath.Left(strPath.ReverseFind('\\'));

	// get the current plan
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate *pTemplate = GetNextDocTemplate(pos);
	pos = pTemplate->GetFirstDocPosition();
	CPlan *pPlan = 
		(CPlan *)pTemplate->GetNextDoc(pos);
	pPlan->SetPathName("ImportedPlan.pln");

	// set the plan path name
	pPlan->SetPathName("IMPORT.PLN");
	pPlan->SetModifiedFlag();

	// create an empty series
	CSeries *pSeries = 
		(CSeries *)m_pSeriesDocTemplate->CreateNewDocument();

	// set the series path name
	pSeries->SetPathName("IMPORT.SER");
	pSeries->SetModifiedFlag();

	// now read the data
	BOOL bResult = ReadAsciiImage(strPath + "\\density.dat", 
		&pSeries->volume, 1000.0);

	// initialize the structure
	CSurface *pStructure = new CSurface();
	CVolume<int> *pRegion = new CVolume<int>;
	pStructure->m_pRegion = pRegion;
	pSeries->m_arrStructures.Add(pStructure);

	pRegion->SetDimensions(pSeries->volume.GetWidth(),
		pSeries->volume.GetHeight(),
		pSeries->volume.GetDepth());

	// initialize the region to all ones
	for (int nAtY = 0; nAtY < pRegion->GetHeight(); nAtY++)
	{
		for (int nAtX = 0; nAtX < pRegion->GetWidth(); nAtX++)
		{
			if (nAtX > 45 && nAtX < 55)
			{
				pRegion->GetVoxels()[0][nAtY][nAtX] = 1;
			}
		}
	}

	// set the series for the plan
	pPlan->SetSeries(pSeries);

#define PI (atan(1.0) * 4.0)
#define SIGMA 7.0

	// and form the total dose
	pPlan->GetDoseMatrix()->SetDimensions(pSeries->volume.GetWidth(),
		pSeries->volume.GetHeight(),
		pSeries->volume.GetDepth());
	pPlan->SetDoseValid(TRUE);

	// read the pencil beams, forming the summed dose
	double maxDose = 0.0;
	for (int nAt = 1; nAt < 100; nAt++)
	{
		CString strPencilBeamFilename;
		strPencilBeamFilename.Format("\\dose%i.dat", nAt);

		CBeam *pPencilBeam = new CBeam();
		bResult = bResult && ReadAsciiImage(strPath + strPencilBeamFilename, 
			pPencilBeam->GetDoseMatrix());
		pPencilBeam->SetDoseValid(TRUE);
		pPlan->AddBeam(pPencilBeam);

		// set the weights to a gaussian distribution
		double weight = 1.0 / sqrt(2 * PI * SIGMA) 
			* exp(- (double)((50 - nAt) * (50 - nAt)) / (SIGMA * SIGMA));
		pPencilBeam->SetWeight(weight);

		for (int nAtY = 0; nAtY < pPlan->GetDoseMatrix()->GetHeight(); nAtY++)
		{
			for (int nAtX = 0; nAtX < pPlan->GetDoseMatrix()->GetWidth(); nAtX++)
			{
				pPlan->GetDoseMatrix()->GetVoxels()[0][nAtY][nAtX] += 
					weight * pPencilBeam->GetDoseMatrix()->GetVoxels()[0][nAtY][nAtX];
				maxDose = __max(maxDose, pPlan->GetDoseMatrix()->GetVoxels()[0][nAtY][nAtX]);
			}
		}
	}

	// normalize the total dose
	for (nAtY = 0; nAtY < pPlan->GetDoseMatrix()->GetHeight(); nAtY++)
	{
		for (int nAtX = 0; nAtX < pPlan->GetDoseMatrix()->GetWidth(); nAtX++)
		{
			pPlan->GetDoseMatrix()->GetVoxels()[0][nAtY][nAtX] /= maxDose;
		}
	}

	// update the views
	pPlan->UpdateAllViews(NULL);
	m_pMainWnd->RedrawWindow();
}
