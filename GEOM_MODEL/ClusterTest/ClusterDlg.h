// ClusterDlg.h : main header file for the CLUSTERDLG application
//

#if !defined(AFX_CLUSTERDLG_H__A65B88CE_5F72_42F8_987F_90AC4AF0D7FC__INCLUDED_)
#define AFX_CLUSTERDLG_H__A65B88CE_5F72_42F8_987F_90AC4AF0D7FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CClusterDlgApp:
// See ClusterDlg.cpp for the implementation of this class
//

class CClusterDlgApp : public CWinApp
{
public:
	CClusterDlgApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClusterDlgApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClusterDlgApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLUSTERDLG_H__A65B88CE_5F72_42F8_987F_90AC4AF0D7FC__INCLUDED_)
