//////////////////////////////////////////////////////////////////////
// Optimizer.h: interface for the COptimizer template
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(OPTIMIZER_H)
#define OPTIMIZER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// include for the objective function
#include "ObjectiveFunction.h"

//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class COptimizer
// 
// base template class for all optimizers
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
class COptimizer
{
public:
	// construct a new COptimizer object
	COptimizer(CObjectiveFunction<DIM, TYPE> *pFunc)
		: m_tolerance((TYPE) 0.5),
			m_nIteration(0),
			m_pFunc(pFunc),
			m_bUseGradientInfo(FALSE)
	{
	}

	// destroy the optimizer
	virtual ~COptimizer()
	{
	}

	// returns the flag to indicate that gradient information should
	//		be used
	BOOL UseGradientInfo()
	{
		return m_bUseGradientInfo;
	}

	// sets the flag to indicate that gradient information should
	//		be used
	void SetUseGradientInfo(BOOL bUseGradientInfo)
	{
		m_bUseGradientInfo = bUseGradientInfo;
	}

	// defines the tolerance for exit from optimization loop
	TYPE GetTolerance()
	{
		return m_tolerance;
	}

	void SetTolerance(TYPE tol)
	{
		m_tolerance = tol;
	}

	// holds the number of iterations needed for the optimization
	int GetIterations()
	{
		return m_nIteration;
	}

	// holds the final value of the optimization
	TYPE GetFinalValue()
	{
		return m_finalValue;
	}

	// holds the final value of the parameters for the minimum f
	CVector<DIM, TYPE> GetFinalParameter()
	{
		return m_vFinalParam;
	}

	// function to actually perform the optimization
	virtual CVector<DIM, TYPE> Optimize(const CVector<DIM, TYPE>& vInit) = 0;

protected:
	// the objective function over which optimization is to occur
	CObjectiveFunction<DIM, TYPE> *m_pFunc;

protected:
	// holds the tolerance for the optimization
	TYPE m_tolerance;

	// holds the number of iterations for the optimization
	int m_nIteration;

	// holds the value of the objective function at the final point
	TYPE m_finalValue;

	// holds the final input parameter to the objective function (same as
	//		returned by Optimize)
	CVector<DIM, TYPE> m_vFinalParam;

private:
	// flag to indicate whether gradient information should be used
	BOOL m_bUseGradientInfo;
};

#endif // !defined(OPTIMIZER_H)
