// ClusterDlgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClusterDlg.h"
#include "ClusterDlgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CClusterDlgDlg dialog

CClusterDlgDlg::CClusterDlgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClusterDlgDlg::IDD, pParent),
		m_pCluster(NULL)
{
	//{{AFX_DATA_INIT(CClusterDlgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClusterDlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClusterDlgDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClusterDlgDlg, CDialog)
	//{{AFX_MSG_MAP(CClusterDlgDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClusterDlgDlg message handlers

BOOL CClusterDlgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClusterDlgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClusterDlgDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// now draw the diagram
		CPaintDC dc(this); // device context for painting

		CStatic *pRect = (CStatic *) GetDlgItem(IDC_DIAGRAM);
		CRect rect;
		pRect->GetWindowRect(&rect);
		ScreenToClient(&rect);
		dc.Rectangle(rect);

		// now draw the diagram within the rectangle
		rect.DeflateRect(20, 20, 150, 20);
		if (m_pCluster)
		{
			m_pCluster->Draw(&dc, &rect);
		}

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClusterDlgDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CClusterDlgDlg::OnOK() 
{
	ReadData();

	Invalidate(TRUE);
}

void CClusterDlgDlg::OnCancel() 
{
	// delete the cluster
	if (m_pCluster != NULL)
	{
		delete m_pCluster;
	}

	// and process cancel
	CDialog::OnCancel();
}

void CClusterDlgDlg::OnSize(UINT nType, int cx, int cy) 
{
	CStatic *pStatic = (CStatic *) GetDlgItem(IDC_DIAGRAM);

	CDialog::OnSize(nType, cx, cy);
}

void ParseVector(CString& strValues, CVectorN<>& vValues)
{
	CString strLabel;

	int nComma = strValues.Find(_T(','));
	strLabel = strValues.Left(nComma);

	while (nComma > -1)
	{
		strValues = strValues.Mid(nComma+1);

		nComma = strValues.Find(_T(','));
		
		vValues.SetDim(vValues.GetDim()+1);
		sscanf(strValues.GetBuffer(255), "%lf", &vValues[vValues.GetDim()-1]);
	}

	strValues = strLabel;
}

void CClusterDlgDlg::OnOpen() 
{
	CFileDialog dlgFile(TRUE, "csv");
	
	if (dlgFile.DoModal() == IDOK)
	{
		m_strFileName = dlgFile.GetFileName();
		ReadData();
	}

	Invalidate(TRUE);
}

void CClusterDlgDlg::ReadData()
{
	CStdioFile file(m_strFileName, 
		CFile::modeRead | CFile::shareDenyNone | CFile::typeText);

	// read the header
	CString strHeader;
	file.ReadString(strHeader);

	// read the weights
	CString strLabel;
	CVectorN<> vWeights;
	file.ReadString(strLabel);
	ParseVector(strLabel, vWeights);

	// stores the vectors
	CArray< CVectorN<>, CVectorN<>& > arrVectors;

	// stores the labels;
	CStringArray arrLabels;

	// read the vectors
	while (file.ReadString(strLabel))
	{
		// parse the vector from the line
		CVectorN<> vVector;
		ParseVector(strLabel, vVector);

		for (int nAt = 0; nAt < vVector.GetDim(); nAt++)
		{
			vVector[nAt] *= vWeights[nAt];
		}

		// add to the arrays
		arrVectors.Add(vVector);
		arrLabels.Add(strLabel);
	}

	if (arrVectors.GetSize() > 0)
	{
		CEuclideanVectorDistance< CVectorN<> > distFunc;
		m_pCluster = CCluster< CVectorN<> >::Analyze(&arrVectors[0], 
			arrVectors.GetSize(), distFunc, &arrLabels);
	}
}
