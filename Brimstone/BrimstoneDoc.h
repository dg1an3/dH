// BrimstoneDoc.h : interface of the CBrimstoneDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_)
#define AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_

#include <Series.h>
#include <Plan.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPrescription;

class COptThread : public CWinThread
{
public:
	COptThread() 
		: m_pPresc(NULL),
		m_evtParamChanged(FALSE),
		m_evtNewResult(FALSE)
	{ 
	}

	DECLARE_DYNCREATE(COptThread);

	virtual BOOL InitInstance() { return TRUE; }

	virtual int Run();

	virtual BOOL Callback(REAL value, const CVectorN<>& vDir);

	// external prescription object (used to transfer parameters)
	CCriticalSection m_csPrescParam;
	CPrescription *m_pPrescParam;
	BOOL m_evtParamChanged;

	void UpdatePlan();

	int nIteration;
	BOOL m_bDone;
private:
	// results
	CCriticalSection m_csResult;
	REAL m_bestValue;
	CVectorN<> m_vResult;
	BOOL m_evtNewResult;

public:
	// internal prescription object
	CPrescription *m_pPresc;
};

class CBrimstoneDoc : public CDocument
{
protected: // create from serialization only
	CBrimstoneDoc();
	DECLARE_DYNCREATE(CBrimstoneDoc)

// Attributes
public:
	BOOL SelectStructure(const CString& strName);
	CStructure * GetSelectedStructure();

	CSeries * m_pSeries;
	CStructure * m_pSelectedStruct;
	CPlan * m_pPlan;

	COptThread *m_pOptThread;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnParamChange(CObservableEvent *, void *);
	void UpdateFromOptimizer();

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
	afx_msg void OnOptimize();
	afx_msg void OnScanbeamlets();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRIMSTONEDOC_H__30B2DDAA_E6C9_429D_9CF2_E66B43815B01__INCLUDED_)
