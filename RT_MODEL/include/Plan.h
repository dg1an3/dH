// Plan.h : interface of the CPlan class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Series.h"
#include "Beam.h"

/////////////////////////////////////////////////////////////////////////////
// CPlan document

class CPlan : public CDocument
{
protected: // create from serialization only
	CPlan();
	DECLARE_DYNCREATE(CPlan)

// Attributes
public:
	// series accessor
	CSeries * GetSeries();
	void SetSeries(CSeries *pSeries);

	// the beams for this plan
	int GetBeamCount() const;
	CBeam * GetBeamAt(int nAt);
	int AddBeam(CBeam *pBeam);

	// flag to indicate whether the plan's dose is valid
	BOOL IsDoseValid() const;
	void SetDoseValid(BOOL bValid = TRUE)
	{
		m_bDoseValid = bValid;
	}

	// the computed dose for this plan (NULL if no dose exists)
	CVolume<double> *GetDoseMatrix()
	{
		return &m_dose;
	}

// Operations
public:
	// sets the document template from which to obtain the
	//		CSeries
	static void SetSeriesDocTemplate(CDocTemplate *pTemplate);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlan)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetFileRoot();
	CString GetFileName();
	virtual ~CPlan();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPlan)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the series upon which this plan is based
	CSeries * m_pSeries;

	// the plan's beams
	CObArray m_arrBeams;

	// the dose matrix for the plan
	BOOL m_bDoseValid;
	CVolume<double> m_dose;

	// static variable that stores a pointer to the 
	//		CSeries document template
	static CDocTemplate * m_pSeriesDocTemplate;

	// stores the filename for the corresponding series
	CString m_strSeriesFilename;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_)
