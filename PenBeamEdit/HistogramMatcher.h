// HistogramMatcher.h: interface for the CHistogramMatcher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_)
#define AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ObjectiveFunction.h>

#include <Plan.h>

#include "Histogram.h"

class CHistogramMatcher : public CObjectiveFunction  
{
public:
	CHistogramMatcher();
	virtual ~CHistogramMatcher();

	// accessors for the plan
	CPlan *GetPlan();
	void SetPlan(CPlan *pPlan);

	// accessors for the structures
	void AddStructure(CSurface *pStructure);

	// DVH accessors
	CHistogram *GetActualDVH(CSurface *pStructure);
	CHistogram *GetTargetDVH(CSurface *pStructure);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);

private:
	// the plan to be optimized
	CPlan *m_pPlan;

	// the actual DVHs
	CMap<CSurface *, CSurface *, CHistogram *, CHistogram *>
		m_mapActualDVHs;

	// the target DVHs
	CMap<CSurface *, CSurface *, CHistogram *, CHistogram *>
		m_mapTargetDVHs;
};

#endif // !defined(AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_)
