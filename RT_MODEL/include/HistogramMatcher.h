//////////////////////////////////////////////////////////////////////
// HistogramMatcher.h: interface for the CHistogramMatcher class.
//
// Copyright (C) 2000-2004
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_)
#define AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

#include <MatrixNxM.h>

#include <ObjectiveFunction.h>

#include "Histogram.h"

class CPrescription;

//////////////////////////////////////////////////////////////////////
// class CHistogramMatcher
//
// objective function for histogram matching
//////////////////////////////////////////////////////////////////////
class CHistogramMatcher : public CObjectiveFunction
{
public:
	// construction / destruction
	CHistogramMatcher(CPrescription *pPresc, int nScale);
	virtual ~CHistogramMatcher();

	// accessors for collection of histograms being optimized
	long GetHistogramCount() const;
	CHistogram *GetHistogram(long nAt) const;
	double GetHistogramWeight(long nAt) const;
	long AddHistogram(CHistogram *pHisto, double weight = 1.0);

	// accessor for target bins
	CVectorN<>& GetTargetBins(long nAt);
	const CVectorN<>& GetTargetBins(long nAt) const;
	const CVectorN<>& GetTargetGBins(long nAt) const;

	// sets the bins to an interval distribution
	void SetInterval(long nAt, double low, double high, double fraction);
	void SetRamp(long nAt, double low, double low_frac,
			double high, double high_frac);

	double Eval_RelativeEntropy(long nAt, const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const;
	double Eval_DiffSq(long nAt, const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const;
	REAL Eval_TotalEntropy(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const;

	void StateVectorToParam(const CVectorN<>& vState, CVectorN<>& vParam);
	void ParamToStateVector(const CVectorN<>& vParam, CVectorN<>& vState);

	REAL Sigmoid(REAL x) const;
	REAL dSigmoid(REAL x) const;
	REAL InvSigmoid(REAL x) const;

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

private:
	// the array of historgrams
	CObArray m_arrHisto;

	// histogram weights
	CVectorN<> m_arrHistoWeights;

	// target bins
	std::vector< CVectorN<> > m_arrTargetBins;
	mutable std::vector< CVectorN<> > m_arrTargetGBins;

	// flag to indicate use of rel. entropy
	BOOL m_bUseRelativeEntropy;

	BOOL m_bUseTotalEntropy;
	REAL m_totalEntropyWeight;

	// stores the plan, and scale
	// CPlanIMRT * m_pPlan;
	CPrescription *m_pPresc;
	int m_nScale;

public:
	// scales input prior to exponentiation
	REAL m_inputScale;

};	// class CHistogramMatcher


#endif // !defined(AFX_HISTOGRAMMATCHER_H__246CC43C_43BF_49F4_B80C_5510F7C570BE__INCLUDED_)
