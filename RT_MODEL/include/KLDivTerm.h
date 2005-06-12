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
// <description>
///////////////////////////////////////////////////////////////////////////////
class CKLDivTerm : public CVOITerm
{
public:
	CKLDivTerm(CStructure *pStructure, REAL weight);
	virtual ~CKLDivTerm();

	// accessor for target bins
	CVectorN<>& GetTargetBins();
	const CVectorN<>& GetTargetBins() const;
	const CVectorN<>& GetTargetGBins() const;

	// sets the bins to an interval distribution
	// NOTE: these must be called AFTER SetBinning for histogram
	void SetInterval(REAL low, REAL high, REAL fraction);
	void SetRamp(REAL low, REAL low_frac,
			REAL high, REAL high_frac);

	virtual REAL Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude);

protected:
	friend class CPrescription;

	// over-ride to create subcopy
	virtual CVOITerm *Subcopy(CVOITerm *pSubcopy = NULL);

	// change handler for when the volume or region changes
	void OnHistogramChange(CObservableEvent *pSource = NULL, void *pVoid = NULL);

private:
	// the target bins
	CVectorN<> m_vTargetBins;

	// the convolved bins
	mutable CVectorN<> m_vTargetGBins;
	mutable bool m_bReconvolve;

// public:
	// use if cross entropy of calc w.r.t. target is needed
	bool m_bTargetCrossEntropy;

};	// class CKLDivTerm



#endif // !defined(AFX_KLDIVTERM_H__03B2D862_D441_437B_9CB9_05602208F0E4__INCLUDED_)
