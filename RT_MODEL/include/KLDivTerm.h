// KLDivTerm.h: interface for the CKLDivTerm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KLDIVTERM_H__03B2D862_D441_437B_9CB9_05602208F0E4__INCLUDED_)
#define AFX_KLDIVTERM_H__03B2D862_D441_437B_9CB9_05602208F0E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VOITerm.h>


///////////////////////////////////////////////////////////////////////////////
// class CKLDivTerm
// 
// represents a Kullback-Liebler divergence match of a DVH to a target curve
///////////////////////////////////////////////////////////////////////////////
class CKLDivTerm : public CVOITerm
{
public:
	// constructor / destructor
	CKLDivTerm(CStructure *pStructure = NULL, REAL weight = 0.0);
	virtual ~CKLDivTerm();

	// serializable
	DECLARE_SERIAL(CKLDivTerm)
	virtual void Serialize(CArchive& ar);

	// sets the bins to an interval distribution
	const CMatrixNxM<>& GetDVPs() const;
	void SetDVPs(const CMatrixNxM<>& mDVP);

	// special-purpose
	void SetInterval(REAL low, REAL high, REAL fraction);

	// returns minimum / maximum target dose to the structure
	REAL GetMinDose(void) const;
	REAL GetMaxDose(void) const;	
	
	// accessor for target bins
	const CVectorN<>& GetTargetBins() const;
	const CVectorN<>& GetTargetGBins() const;

	// evaluates the term
	virtual REAL Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude);

protected:
	// over-ride to create subcopy
	virtual CVOITerm *Subcopy(CVOITerm *pSubcopy = NULL);

	// change handler for when the volume or region changes
	void OnHistogramBinningChange(CObservableEvent *pSource = NULL, void *pVoid = NULL);

private:
	// DVPs
	CMatrixNxM<> m_mDVPs;

	// the target bins
	mutable CVectorN<> m_vTargetBins;
	mutable bool m_bRecompTarget;

	// the convolved bins
	mutable CVectorN<> m_vTargetGBins;
	mutable bool m_bReconvolve;

	// use if cross entropy of calc w.r.t. target is needed
	bool m_bTargetCrossEntropy;

};	// class CKLDivTerm



#endif // !defined(AFX_KLDIVTERM_H__03B2D862_D441_437B_9CB9_05602208F0E4__INCLUDED_)
