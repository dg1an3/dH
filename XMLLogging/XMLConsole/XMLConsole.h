// XMLConsole.h : main header file for the XMLCONSOLE application
//

#if !defined(AFX_XMLCONSOLE_H__ADC9F310_D077_4194_86E2_5EDBE3BC2C93__INCLUDED_)
#define AFX_XMLCONSOLE_H__ADC9F310_D077_4194_86E2_5EDBE3BC2C93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleApp:
// See XMLConsole.cpp for the implementation of this class
//

class CXMLConsoleApp : public CWinApp
{
public:
	CXMLConsoleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLConsoleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CXMLConsoleApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XMLCONSOLE_H__ADC9F310_D077_4194_86E2_5EDBE3BC2C93__INCLUDED_)
