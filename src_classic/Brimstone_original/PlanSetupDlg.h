#if !defined(AFX_PLANSETUPDLG_H__A3A51FE4_09F1_45E1_89BE_EC8A744CBF41__INCLUDED_)
#define AFX_PLANSETUPDLG_H__A3A51FE4_09F1_45E1_89BE_EC8A744CBF41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlanSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg dialog

class CPlanSetupDlg : public CDialog
{
// Construction
public:
	CPlanSetupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlanSetupDlg)
	enum { IDD = IDD_PLANSETUPDLG };
	UINT	m_nBeamCount;
	double	m_isoX;
	double	m_isoY;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlanSetupDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLANSETUPDLG_H__A3A51FE4_09F1_45E1_89BE_EC8A744CBF41__INCLUDED_)
