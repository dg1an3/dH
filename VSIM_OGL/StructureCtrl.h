#if !defined(AFX_STRUCTURECTRL_H__758C51B4_F88B_11D4_9E3F_00B0D0609AB0__INCLUDED_)
#define AFX_STRUCTURECTRL_H__758C51B4_F88B_11D4_9E3F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StructureCtrl.h : header file
//

// #include "Value.h"

/////////////////////////////////////////////////////////////////////////////
// CStructureCtrl dialog

class CStructureCtrl : public CDialogBar
{
// Construction
public:
	CStructureCtrl(); // CWnd* pParent = NULL);   // standard constructor

	BOOL m_arrEnabled[8];

// Dialog Data
	//{{AFX_DATA(CStructureCtrl)
	enum { IDD = IDD_STRUCTUREDIALOG };
	BOOL	m_bStructEnabled1;
	BOOL	m_bStructEnabled2;
	BOOL	m_bStructEnabled3;
	BOOL	m_bStructEnabled4;
	BOOL	m_bStructEnabled5;
	BOOL	m_bStructEnabled6;
	BOOL	m_bStructEnabled7;
	BOOL	m_bStructEnabled8;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStructureCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStructureCtrl)
	afx_msg void OnApplyChanges();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRUCTURECTRL_H__758C51B4_F88B_11D4_9E3F_00B0D0609AB0__INCLUDED_)
