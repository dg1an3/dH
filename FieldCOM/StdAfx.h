// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__A9461C33_FA60_4AF8_9FD7_440915AB55BC__INCLUDED_)
#define AFX_STDAFX_H__A9461C33_FA60_4AF8_9FD7_440915AB55BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

// include math, used all over
#include <math.h>

// undef for valarray include
#undef min
#undef max

// valarray
#include <valarray>

// redef for min, max
#define min __min
#define max __max

// standard macros for processing function return results
#include <results.h>

// ATL base include
#include <atlbase.h>

// custom CComVariantEx and CSafeArray classes
#include <ComVariantEx.h>

// the safearray wrapper class
#include <SafeArray.h>

// pull the old switcheroo
#define CComVariant CComVariantEx

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

// ATL COM classes
#include <atlcom.h>

// ATL ActiveX control classes
#include <atlctl.h>

// COM definitions
#include <comdef.h>

// extra definition of ASSERT for MFC-derived code
#define ASSERT ATLASSERT

// trace macro
#define TRACE ATLTRACE

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9461C33_FA60_4AF8_9FD7_440915AB55BC__INCLUDED)
