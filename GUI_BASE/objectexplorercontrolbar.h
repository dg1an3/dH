#if !defined(AFX_OBJECTEXPLORERCONTROLBAR_H__22285D46_0756_11D5_9E4A_00B0D0609AB0__INCLUDED_)
#define AFX_OBJECTEXPLORERCONTROLBAR_H__22285D46_0756_11D5_9E4A_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectExplorerControlBar.h : header file
//

#include "ObjectExplorer.h"

/////////////////////////////////////////////////////////////////////////////
// CObjectExplorerControlBar dialog

class CObjectExplorerControlBar : public CDialogBar
{
// Construction
public:
	CObjectExplorerControlBar();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectExplorerControlBar)
	enum { IDD = IDD_OBJECTEXPLORERCONTROLBAR };
	//}}AFX_DATA

	CObjectExplorer	m_ExplorerCtrl;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectExplorerControlBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CObjectExplorerControlBar)
    afx_msg LONG OnInitDialog( UINT wParam, LONG lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTEXPLORERCONTROLBAR_H__22285D46_0756_11D5_9E4A_00B0D0609AB0__INCLUDED_)
