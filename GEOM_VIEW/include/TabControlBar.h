#if !defined(AFX_TABCONTROLBAR_H__76008144_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_TABCONTROLBAR_H__76008144_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabControlBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabControlBar dialog

class CTabControlBar : public CDialogBar
{
// Construction
public:
	CTabControlBar(); // CWnd* pParent = NULL);   // standard constructor

	// adds a new tab to the tab control bar
	int AddTab(const CString &strName, CDialog *pNewTab);
		//CRuntimeClass *pRuntimeClass, UINT nIDTemplate);

	CObArray m_arrTabs;

// Dialog Data
	//{{AFX_DATA(CTabControlBar)
	enum { IDD = IDD_TABCONTROLBAR };
	CTabCtrl	m_tabCtrl;
	CButton	m_btnRect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabControlBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabControlBar)
    afx_msg LONG OnInitDialog( UINT wParam, LONG lParam );
	afx_msg void OnSelchangeTabctrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABCONTROLBAR_H__76008144_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
