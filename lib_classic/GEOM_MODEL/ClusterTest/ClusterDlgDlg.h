// ClusterDlgDlg.h : header file
//
//{{AFX_INCLUDES()
//}}AFX_INCLUDES

#if !defined(AFX_CLUSTERDLGDLG_H__F8023DFA_09C1_4369_B97F_FE67590F6CA9__INCLUDED_)
#define AFX_CLUSTERDLGDLG_H__F8023DFA_09C1_4369_B97F_FE67590F6CA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Cluster.h"
#include <VectorN.h>

/////////////////////////////////////////////////////////////////////////////
// CClusterDlgDlg dialog

class CClusterDlgDlg : public CDialog
{
// Construction
public:
	CClusterDlgDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CClusterDlgDlg)
	enum { IDD = IDD_CLUSTERDLG_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClusterDlgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	// reads the data
	void ReadData();

	// stores the file name
	CString m_strFileName;

	// stores the analyzed cluster
	const CCluster< CVectorN<> > *m_pCluster;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClusterDlgDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLUSTERDLGDLG_H__F8023DFA_09C1_4369_B97F_FE67590F6CA9__INCLUDED_)
