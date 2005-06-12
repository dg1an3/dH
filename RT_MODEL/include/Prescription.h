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

///////////////////////////////////////////////////////////////////////////////
// class CPrescription
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CPrescription : public CObjectiveFunction  
{
public:
	// constructors / destructors
	CPrescription(CPlan *pPlan, int nLevel);
	virtual ~CPrescription();

	// plan accessor
	CPlan * GetPlan();

	// pyramid levels for objective function
	CPrescription *GetLevel(int nLevel);
	const CPrescription *GetLevel(int nLevel) const;

	// independent terms of the function
	CVOITerm *GetStructureTerm(CStructure *pStruct);
	void AddStructureTerm(CVOITerm *pST);
	void RemoveStructureTerm(CStructure *pStruct);
	
	// updates term parameters from another CPrescription object
	void UpdateTerms(CPrescription *pPresc);

	// sets the entropy weight parameter
	void SetEntropyWeight(REAL weight);

	// sets the GBinSigma for all histograms
	void SetGBinSigma(REAL binSigma);

	//////////////////////////////////////////////////////////////////////////
	// optimization and helpers

	// returns embedded optimizer
	COptimizer * GetOptimizer(void);

	// performs the optimization (calls sub-levels first)
	BOOL Optimize(CVectorN<>& vInit, OptimizerCallback *pFunc, void *pParam);

	// initial step in objective function -- forming sum and histogram
	void CalcSumSigmoid(CHistogram *pHisto, const CVectorN<>& vInput, 
		const CArray<BOOL, BOOL>& arrInclude) const;

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

	// evaluates the total entropy part of the objective function
	REAL Eval_TotalEntropy(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const;

	// transform function from linear to other parameter space
	virtual void Transform(CVectorN<> *pvInOut) const;
	virtual void dTransform(CVectorN<> *pvInOut) const;
	virtual void InvTransform(CVectorN<> *pvInOut) const;

	//////////////////////////////////////////////////////////////////////////
	// state vector functions

	// initial state vector
	void GetInitStateVector(CVectorN<>& vInit);

	// transfers state vector from plan
	void GetStateVectorFromPlan(CVectorN<>& vState);
	void SetStateVectorToPlan(const CVectorN<>& vState);

protected:
	// helper functions for state vector
	void StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights);
	void BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState);

	// mux / demux of vector elements
	void GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet) const;

	// transfers state vector from level n+1 to level n
	void InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg);

public:
	// helper to set up element include flags
	void SetElementInclude();

private:  
	// my plan
	CPlan *m_pPlan;

	// my optimizer
	COptimizer * m_pOptimizer;

	// the level of this
	int m_nLevel;

	// the next level
	CPrescription *m_pNextLevel;
	
	// sigmoid for parameter transform
	REAL m_inputScale;

	// the sum volume used for histogram
	CVolume<VOXEL_REAL> m_sumVolume;

	// helper (temporaries) for objective function eval
	mutable CVolume<VOXEL_REAL> m_volGroup;
	mutable CVolume<VOXEL_REAL> m_volGroupMain;

	// stores the VOITs
	CTypedPtrMap<CMapPtrToPtr, CStructure*, CVOITerm*> m_mapVOITs;

	// parameters for objective and optimization
	REAL m_GBinSigma;
	REAL m_tolerance;

	// more params
	REAL m_totalEntropyWeight;
	REAL m_intensityMapSumWeight;

	// helpers for Eval_TotalEntropy
	mutable CVectorN<double> m_vBeamWeights;
	mutable CVectorN<> m_vPartGrad;

	// array of flags for element inclusion
	CArray<BOOL, BOOL> m_arrIncludeElement;

};	// class CPrescription


#endif // !defined(AFX_PRESCRIPTION_H__014398FA_0683_4935_A57E_2785989E71C3__INCLUDED_)
