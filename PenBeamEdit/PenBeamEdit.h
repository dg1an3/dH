// PenBeamEdit.h : main header file for the PENBEAMEDIT application
//

#if !defined(AFX_PENBEAMEDIT_H__34348F40_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_PENBEAMEDIT_H__34348F40_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditApp:
// See PenBeamEdit.cpp for the implementation of this class
//

class CPenBeamEditApp : public CWinApp
{
public:
	CPenBeamEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPenBeamEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CPenBeamEditApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PENBEAMEDIT_H__34348F40_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
