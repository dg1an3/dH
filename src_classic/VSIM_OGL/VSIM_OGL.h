// VSIM_OGL.h : main header file for the VSIM_OGL application
//

#if !defined(AFX_VSIM_OGL_H__3E80E024_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_VSIM_OGL_H__3E80E024_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <GUI_BASE_resource.h>	// GUI_BASE symbols
#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CVSIM_OGLApp:
// See VSIM_OGL.cpp for the implementation of this class
//

class CVSIM_OGLApp : public CWinApp
{
public:
	CVSIM_OGLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSIM_OGLApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CVSIM_OGLApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CSingleDocTemplate * m_pSeriesDocTemplate;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VSIM_OGL_H__3E80E024_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
