// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C7A553A2_1418_11D5_9E53_00B0D0609AB0__INCLUDED_)
#define AFX_STDAFX_H__C7A553A2_1418_11D5_9E53_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>

#include <afxtempl.h>

#define CREATE_TAB(LENGTH) \
	CString TAB; \
	while (TAB.GetLength() < LENGTH * 2) \
		TAB += "\t\t"

#define DC_TAB(DUMP_CONTEXT) \
	DUMP_CONTEXT << TAB

#define PUSH_DUMP_DEPTH(DUMP_CONTEXT) \
	int OLD_DUMP_DEPTH = DUMP_CONTEXT.GetDepth(); \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH + 1)

#define POP_DUMP_DEPTH(DUMP_CONTEXT) \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH)


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C7A553A2_1418_11D5_9E53_00B0D0609AB0__INCLUDED_)
