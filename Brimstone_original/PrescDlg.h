#if !defined(AFX_PRESCDLG_H__454F1B0A_DB37_48E8_96EC_25583F397316__INCLUDED_)
#define AFX_PRESCDLG_H__454F1B0A_DB37_48E8_96EC_25583F397316__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrescDlg.h : header file
//
 
class CSeries;
class CPrescription;
class CStructure;

/////////////////////////////////////////////////////////////////////////////
// CPrescDlg dialog

class CPrescDlg : public CDialog
{
// Construction
public:
	CPrescDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrescDlg)
	enum { IDD = IDD_ADDPRESC };
	CComboBox	m_StructCombo;
	double	m_Dose2;
	double	m_Volume1;
	double	m_Dose1;
	//}}AFX_DATA

	CPrescription * m_pPrescParam;
	CPrescription * m_pPresc;
	CSeries * m_pSeries;
	CStructure * m_pStruct;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrescDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrescDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESCDLG_H__454F1B0A_DB37_48E8_96EC_25583F397316__INCLUDED_)
