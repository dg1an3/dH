#if !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
#define AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_

#include "BrimstoneDoc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrescriptionToolbar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar window

class CPrescriptionToolbar : public CDialogBar
{
// Construction
public:
	CPrescriptionToolbar();

// Attributes
public:
	void SetDocument(CBrimstoneDoc *pDoc);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrescriptionToolbar)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPrescriptionToolbar();

protected:
	CBrimstoneDoc * m_pDoc;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrescriptionToolbar)
	afx_msg void OnSelchangeStructselectcombo();
	afx_msg void OnDropdownStructselectcombo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
