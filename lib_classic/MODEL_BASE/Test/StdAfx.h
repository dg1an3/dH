// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__09C4CC82_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_)
#define AFX_STDAFX_H__09C4CC82_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>

#include <afxtempl.h>

#define BEGIN_TEST_BANNER(class_name) \
	cout << "/////////////////////////////////////////////////////" << endl; \
	cout << "// Testing the " << class_name << " class" << endl; \
	cout << "/////////////////////////////////////////////////////" << endl; \

#define END_TEST_BANNER(class_name) \
	cout << "/////////////////////////////////////////////////////" << endl; \
	cout << "// Done testing the " << class_name << " class" << endl; \
	cout << "/////////////////////////////////////////////////////" << endl; \

#define PERFORM_STEP(msg, step) \
	cout << '\t' << msg << "..."; \
	step; \
	cout << "done." << endl; \

#define ASSERT_RESULT(number, bResult) \
	if (bResult) \
		{ cout << "---- TEST (" << number << ") SUCCEEDED ----" << endl; } \
	else \
		{ cout << "**** TEST (" << number << ") FAILED ****" << endl; }

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__09C4CC82_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_)
