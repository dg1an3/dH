// Prescription.h: interface for the CPrescription class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_)
#define AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_

#include <ModelObject.h>

#include <ObjectiveFunction.h>
#include <Optimizer.h>

#include <Histogram.h>

#include <KLDivTerm.h>

#include <Structure.h>

#include <Plan.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class COptimizer;
typedef void (*CCallbackFunc)(COptimizer *pOpt, int nIter);

///////////////////////////////////////////////////////////////////////////////
// class CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CPrescription : public CObjectiveFunction  
{
public:
	CPrescription(CPlan *pPlan, int nLevel);
	virtual ~CPrescription();

	CPrescription *GetLevel(int nLevel)
	{
		if (nLevel == 0) return this; else return m_pNextLevel->GetLevel(nLevel-1);
	}

	const CPrescription *GetLevel(int nLevel) const
	{
		if (nLevel == 0) return this; else return m_pNextLevel->GetLevel(nLevel-1);
	}


	void AddStructureTerm(CVOITerm *pST);
	void RemoveStructureTerm(CStructure *pStruct);
	CVOITerm *GetStructureTerm(CStructure *pStruct);

	// sets the GBinSigma for all histograms
	void SetGBinSigma(REAL binSigma);

	//////////////////////////

	void GetInitStateVector(CVectorN<>& vInit);

	void GetStateVectorFromPlan(CVectorN<>& vState);
	void SetStateVectorToPlan(const CVectorN<>& vState);

	///////////////////////////////////

	BOOL Optimize(CVectorN<>& vInit, CCallbackFunc func);

	void CalcSumSigmoid(CHistogram *pHisto, const CVectorN<>& vInput) const;

	REAL Eval_TotalEntropy(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const;

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

protected:
	void StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights);
	void BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState);

	void GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet) const;
	void InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg);

private:
	CPlan *m_pPlan;
	COptimizer * m_pOptimizer;
	CPrescription *m_pNextLevel;
	int m_nLevel;

	CVolume<REAL> m_sumVolume;

	CTypedPtrMap<CMapPtrToPtr, CStructure*, CVOITerm*> m_mapVOITs;

	REAL m_GBinSigma;
	REAL m_tolerance;

	REAL m_totalEntropyWeight;

	// scales input prior to exponentiation
	REAL m_inputScale;

};	// class CPrescription


#endif // !defined(AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_)
