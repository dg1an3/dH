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

#include <VectorN.h>
#include <MatrixNxM.h>

// series upon which plan is based
#include <Series.h>

// beams belonging to the plan
#include <Beam.h>

// histograms for the plan
#include <Histogram.h>

class CEnergyDepKernel;

//////////////////////////////////////////////////////////////////////
// class CPlan
//
// represents a treatment plan
//////////////////////////////////////////////////////////////////////
class CPlan : public CModelObject
{
public:
	// constructor
	CPlan();
	virtual ~CPlan();

	// dynamic create
	DECLARE_SERIAL(CPlan)

	// series accessor
	CSeries * GetSeries();
	void SetSeries(CSeries *pSeries);

	// histogram accessor
	CHistogram *GetHistogram(CStructure *pSurface);

	// the beams for this plan
	int GetBeamCount() const;
	CBeam * GetBeamAt(int nAt);
	int AddBeam(CBeam *pBeam);

	// helper functions
	int GetTotalBeamletCount(int nLevel);
	void SetBeamCount(int nCount);

	// helper to get formatted mass density volume
	CVolume<REAL> * GetMassDensity();

	// the computed dose for this plan (NULL if no dose exists)
	CVolume<REAL> *GetDoseMatrix();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlan)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:

	// stores the energy dep kernel
	CEnergyDepKernel * m_pKernel;


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void OnBeamChange(CObservableEvent *, void *);

	// the plan's beams
	CTypedPtrArray<CObArray, CBeam*> m_arrBeams;

private:
	// the series upon which this plan is based
	CSeries * m_pSeries;

	// pyramid storing resampled mass density
	CVolume<REAL> m_massDensity;
	BOOL m_bCalcMassDensity;

public:
	// the dose matrix for the plan
	CVolume<REAL> m_dose;

private:
	// flag to indicate the total dose is to be recomputed
	BOOL m_bRecomputeTotalDose;

	// the histograms
	CMapPtrToPtr m_mapHistograms;

};	// class CPlan


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAN_H__71D1495A_EE39_11D4_9E36_00B0D0609AB0__INCLUDED_)
