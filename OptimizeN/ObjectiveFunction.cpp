//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "ObjectiveFunction.h"

CObjectiveFunction::CObjectiveFunction(BOOL bHasGradientInfo)
	: m_bHasGradientInfo(bHasGradientInfo)
{
}

// whether gradient information is available
BOOL CObjectiveFunction::HasGradientInfo() const
{
	return m_bHasGradientInfo;
}


REAL CObjectiveFunction::TestGradient(const CVectorN<>& vAtParam, REAL epsilon) const
{
	CVectorN<> vParam = vAtParam;
	CVectorN<> vEvalGrad;

	REAL fp = (*this)(vParam, &vEvalGrad);

	// test the gradient
	CVectorN<> vDiffGrad;
	vDiffGrad.SetDim(vParam.GetDim());

	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
	{
		vParam[nAt] += epsilon;

		REAL fp_del = (*this)(vParam);

		vDiffGrad[nAt] = (fp_del - fp);
		vDiffGrad[nAt] /= epsilon;

		vParam[nAt] -= epsilon;
	}

	TRACE_VECTOR("Eval Grad = ", vEvalGrad);
	TRACE_VECTOR("Diff Grad = ", vDiffGrad);

	return (vEvalGrad - vDiffGrad).GetLength();
}
