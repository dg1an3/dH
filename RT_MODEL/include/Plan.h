//////////////////////////////////////////////////////////////////////
// Plan.h: declaration of the CPlan class
//
// Copyright (C) 2000-2002  Derek G. Lane
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(PLAN_H)
#define PLAN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// series upon which plan is based
#include "Series.h"

// beams belonging to the plan
#include "Beam.h"

// histograms for the plan
#include <Histogram.h>

#include <VectorN.h>
#include <MatrixNxM.h>

//////////////////////////////////////////////////////////////////////
// class CPlan
//
// represents a treatment plan
//////////////////////////////////////////////////////////////////////
class CPlan : public CDocument
{
protected: // create from serialization only
	// constructor
	CPlan();

	// dynamic create
	DECLARE_SERIAL(CPlan)

// Attributes
public:
	// series accessor
	CSeries * GetSeries();
	void SetSeries(CSeries *pSeries);

	// histogram accessor
	CHistogram *GetHistogram(CStructure *pSurface);

	// the beams for this plan
	int GetBeamCount() const;
	CBeam * GetBeamAt(int nAt);
	int AddBeam(CBeam *pBeam);

	int m_nFields;

	// the beam weights, as a vector
	void GetBeamWeights(CVectorN<>& vWeights);
	void SetBeamWeights(const CVectorN<>& vWeights);

	// DVH accessors
	const CMatrixNxM<> *GetTargetDVH(CMesh *pStructure);
	void SetTargetDVH(CMesh *pStructure, CMatrixNxM<> *pmTarget);

	// the computed dose for this plan (NULL if no dose exists)
	CVolume<double> *GetDoseMatrix(int nScale = 0);

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
	void OnBeamChange(CObservableEvent *, void *);

// Generated message map functions
protected:
	//{{AFX_MSG(CPlan)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// the plan's beams
	CObArray m_arrBeams;

private:
	// the series upon which this plan is based
	CSeries * m_pSeries;

	// the target DVHs
	CMapStringToPtr m_mapTargetDVHs;

	// the dose matrix for the plan
	CVolume<double> m_arrDose[MAX_SCALES];

	// flag to indicate the total dose is to be recomputed
	BOOL m_bRecomputeTotalDose;

	// the histograms
	CMapPtrToPtr m_mapHistograms;

	// static variable that stores a pointer to the 
	//		CSeries document template
	static CDocTemplate * m_pSeriesDocTemplate;

	// stores the filename for the corresponding series
	CString m_strSeriesFilename;

};	// class CPlan


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_)
