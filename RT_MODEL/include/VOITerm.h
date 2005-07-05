// VOITerm.h: interface for the CVOITerm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOITERM_H__BC410BA9_C9C4_472B_8984_0AB4C818ECA9__INCLUDED_)
#define AFX_VOITERM_H__BC410BA9_C9C4_472B_8984_0AB4C818ECA9__INCLUDED_

#include <Histogram.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStructure;

///////////////////////////////////////////////////////////////////////////////
// class CVOITerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CVOITerm : public CModelObject
{
public:
	CVOITerm(CStructure *pStructure = NULL, REAL weight = 1.0);
	virtual ~CVOITerm();

	DECLARE_SERIAL(CVOITerm);

	virtual CVOITerm& operator=(const CVOITerm& otherTerm);

	void SetStructure(CStructure *pStructure);

	// returns the specified subcopy 
	virtual CVOITerm *GetLevel(int nLevel, BOOL bCreate = FALSE);

	// accessors for structure and histogram
	CStructure *GetVOI() { return m_pVOI; }
	CHistogram *GetHistogram() { return &m_histogram; }
	const CHistogram *GetHistogram() const { return &m_histogram; }

	virtual REAL Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude) { return 0; }

	REAL GetWeight() const;
	void SetWeight(REAL weight);

protected:
	friend class CPrescription;

	// helper to create pyramid - constructs a copy, except with nScale + 1
	virtual CVOITerm *Subcopy(CVOITerm *pSubcopy = NULL);

	// the structure and scale
	CStructure *m_pVOI;
	int m_nLevel;

	// the histogram for the term
	CHistogram m_histogram;

// public:
	// the weight
	REAL m_weight;

	CVOITerm *m_pNextScale;

};	// class CVOITerm


/*
///////////////////////////////////////////////////////////////////////////////
// class CTCPTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CTCPTerm : public CVOITerm
{
public:
	virtual REAL Eval(const CVectorN<>& d_vInput, CVectorN<> *pvGrad = NULL);

};	// class CTCPTerm


///////////////////////////////////////////////////////////////////////////////
// class CNTCPTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CNTCPTerm : public CVOITerm
{
public:
	virtual REAL Eval(const CVectorN<>& d_vInput, CVectorN<> *pvGrad = NULL);

};	// class CNTCPTerm
*/



#endif // !defined(AFX_VOITERM_H__BC410BA9_C9C4_472B_8984_0AB4C818ECA9__INCLUDED_)
