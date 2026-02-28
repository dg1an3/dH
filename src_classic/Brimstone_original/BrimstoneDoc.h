// BrimstoneDoc.h : interface of the CBrimstoneDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_)
#define AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_

#include <Series.h>
#include <Plan.h>
#include <Prescription.h>
#include "OptThread.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBrimstoneDoc : public CDocument
{
protected: // create from serialization only
	CBrimstoneDoc();
	DECLARE_DYNCREATE(CBrimstoneDoc)

// Attributes
public:
	BOOL SelectStructure(const CString& strName);
	CStructure * GetSelectedStructure();

	// pointer to selected structure
	CStructure * m_pSelectedStruct;

	CAutoPtr<CSeries> m_pSeries;
	CAutoPtr<CPlan> m_pPlan;

	// stores the prescription object
	CAutoPtr<CPrescription> m_pPresc;

	// generates a histogram for the specified structure
	void AddHistogram(CStructure * pStruct);

	// removes histogram for designated structure
	void RemoveHistogram(CStructure * pStruct);

public:
	// add new structure term
	void AddStructTerm(CVOITerm * pVOIT);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:

	void OnDoseChange(CObservableEvent*, void*);

	virtual ~CBrimstoneDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBrimstoneDoc)
	// afx_msg void OnOptimize();
	afx_msg void OnGenbeamlets();
	afx_msg void OnFileImportDcm();
	afx_msg void OnPlanAddPresc();
	afx_msg void OnOptDashboard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_)
