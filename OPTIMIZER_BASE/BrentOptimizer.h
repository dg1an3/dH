//////////////////////////////////////////////////////////////////////
// BrentOptimizer.h: interface for the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(BRENTOPTIMIZER_H)
#define BRENTOPTIMIZER_H

// include the optimizer base class
#include "Optimizer.h"

//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class CBrentOptimizer
// 
// optimizer that implements the Brent algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CBrentOptimizer : public COptimizer<1, TYPE>
{
public:
	// construct a new Brent Optimizer
	CBrentOptimizer(CObjectiveFunction<1, TYPE> *pFunc)
		: COptimizer<1, TYPE>(pFunc)
	{
	}

	// optimizes the initial input vector
	virtual CVector<1, TYPE> Optimize(const CVector<1, TYPE>& vInit);

protected:
	// finds a bracket for the minimum value of the objective function
	void BracketMinimum(TYPE& ax, TYPE& bx, TYPE& cx);

	// finds the minimum of the objective function
	TYPE FindMinimum(TYPE ax, TYPE bx, TYPE cx);

	// finds the minimum of the objective function, using gradient information
	TYPE FindMinimumGrad(TYPE ax, TYPE bx, TYPE cx);
};

#endif