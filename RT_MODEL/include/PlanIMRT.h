// PlanIMRT.h: interface for the CPlanIMRT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLANIMRT_H__75AC54E0_9379_42C2_A808_69ED320E7A57__INCLUDED_)
#define AFX_PLANIMRT_H__75AC54E0_9379_42C2_A808_69ED320E7A57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>
#include <BeamIMRT.h>
#include <Histogram.h>

#include <Plan.h>

class CPlanIMRT : public CPlan  
{
public:
	CPlanIMRT();
	virtual ~CPlanIMRT();

	int AddStructure(CStructure *pStruct);

	CHistogram *GetRegionHisto(int nAt, int nScale);

	void SetBeamCount(int nCount);
	CBeamIMRT * GetBeam(int nAt);

	int GetTotalBeamletCount(int nScale);
	void GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet);

	void GetStateVector(int nScale, CVectorN<>& vState);
	void SetStateVector(int nScale, const CVectorN<>& vState);
	void InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg);

	void StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights);
	void BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState);

private:
	CObArray m_arrHistograms[MAX_SCALES];
};

#endif // !defined(AFX_PLANIMRT_H__75AC54E0_9379_42C2_A808_69ED320E7A57__INCLUDED_)
