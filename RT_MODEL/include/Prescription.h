// Prescription.h: interface for the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_)
#define AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_

#include <ModelObject.h>

#include <ObjectiveFunction.h>
#include <Optimizer.h>

#include <Histogram.h>
#include <HistogramMatcher.h>

#include <Structure.h>

#include <Plan.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class COptimizer;
typedef void (*CCallbackFunc)(COptimizer *pOpt, int nIter);

///////////////////////////////////////////////////////////////////////////////
// class CVOITerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CVOITerm
{
public:
	CVOITerm(CStructure *pStructure, int nScale)
		: m_pVOI(pStructure), 
			m_nScale(nScale),
			m_histogram(NULL, pStructure->GetRegion(nScale))
	{	
	}	// CVOITerm

	// helper to create pyramid - constructs a copy, except with nScale + 1
	virtual CVOITerm *Subcopy() = 0;

	// accessors for structure and histogram
	CStructure *GetVOI() { return m_pVOI; }
	CHistogram *GetHistogram() { return &m_histogram; }

	virtual REAL Eval(const CVectorN<>& d_vInput, CVectorN<> *pvGrad = NULL) = 0;

protected:
	// the structure and scale
	CStructure *m_pVOI;
	int m_nScale;

	// the histogram for the term
	CHistogram m_histogram;

	// the weight
	REAL m_weight;

};	// class CVOITerm


///////////////////////////////////////////////////////////////////////////////
// class CKLDivTerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CKLDivTerm : public CVOITerm
{
public:
	CKLDivTerm(CStructure *pStructure, int nScale)
		: CVOITerm(pStructure, nScale)
	{	
	}	// CKLDivTerm

	virtual CVOITerm *Subcopy() 
	{
		CKLDivTerm *pSubcopy = new CKLDivTerm(GetVOI(), m_nScale+1);
		pSubcopy->m_vTargetBins = m_vTargetBins;
		pSubcopy->m_vTargetGBins = m_vTargetGBins;

		return pSubcopy;
	}

	// accessor for target bins
	CVectorN<>& GetTargetBins() { return m_vTargetBins; }
	const CVectorN<>& GetTargetBins() const { return m_vTargetBins; }
	const CVectorN<>& GetTargetGBins() const { return m_vTargetGBins; }

	// sets the bins to an interval distribution
	void SetInterval(long nAt, double low, double high, double fraction);
	void SetRamp(long nAt, double low, double low_frac,
			double high, double high_frac);

	virtual REAL Eval(const CVectorN<>& d_vInput, CVectorN<> *pvGrad = NULL);

protected:
	CVectorN<> m_vTargetBins;
	CVectorN<> m_vTargetGBins;

};	// class CKLDivTerm


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



///////////////////////////////////////////////////////////////////////////////
// class CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CPrescription : public CObjectiveFunction  
{
public:
	CPrescription(CPlan *pPlan);
	virtual ~CPrescription();

	void SetPlan(CPlan *pPlan, int nLevels = 3);

	int AddStructure(CStructure *pStruct, REAL weight);
	CHistogram * GetHistogram(CStructure *pStruct, int nScale);

	void AddStructureTerm(CVOITerm *pST);
	void RemoveStructureTerm(CStructure *pStruct);
	CVOITerm *GetStructureTerm(CStructure *pStruct);

	//////////////////////////

	void GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet);

	void GetStateVector(int nScale, CVectorN<>& vState);
	void SetStateVector(int nScale, const CVectorN<>& vState);

	void InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg);

	void StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights);
	void BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState);

	///////////////////////////////////

	BOOL Optimize(CCallbackFunc func);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

public:
	CPlan *m_pPlan;
	COptimizer * m_pOptimizer;
	CPrescription *m_pNextLevel;

	CVolume<double> *m_pSumVolume;

	CTypedPtrMap<CMapPtrToPtr, CStructure*, CVOITerm*> m_mapVOITerms;

	CTypedPtrMap<CMapPtrToPtr, CStructure*, CHistogram*> m_mapHistograms[MAX_SCALES];

	CTypedPtrArray<CPtrArray, CHistogramMatcher*> m_arrHistoMatchers;

};	// class CPrescription


#endif // !defined(AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_)
