#if !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
#define AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_

#include "BrimstoneDoc.h"

#include <Prescription.h>

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
	CPrescriptionToolbar(CWnd *pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CPrescriptionToolbar)
	enum { IDD = IDD_PRESCDLG };
	CComboBox m_cbSSelect;
	CButton m_btnVisible;
	CButton m_btnHistogram;
	CButton m_btnPrescNone;
	CButton m_btnPrescTarget;
	CButton m_btnPrescOAR;
	CSliderCtrl m_sliderWeight;
	CEdit m_editDose1;
	CEdit m_editDose2;
	//}}AFX_DATA

// Attributes
public:
	void SetDocument(CBrimstoneDoc *pDoc);

	// returns the currently selected structure
	CStructure * GetSelectedStruct(void);

	// returns presc for currently selected structure (if possible)
	CVOITerm * GetSelectedPresc(void);

	// updates the associated prescription object
	void UpdatePresc(void);

// Operations
public:
	virtual BOOL OnInitDialog();

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
	afx_msg void OnVisibleCheck();
	afx_msg void OnHistogramCheck();
	afx_msg void OnPrescriptionChange();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDose1Changed();
	afx_msg void OnDose2Changed();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();
};	// class CPrescriptionToolbar


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
