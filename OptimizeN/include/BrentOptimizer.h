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
// class CBrentOptimizer
// 
// optimizer that implements the Brent algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class CBrentOptimizer : public COptimizer
{
public:
	// construct a new Brent Optimizer
	CBrentOptimizer(CObjectiveFunction *pFunc);

	// optimizes the initial input vector
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit = m_vBrentInit);

	// helper function to return an initial Zero vector
	static const CVectorN<>& GetInitZero();

protected:
	// finds a bracket for the minimum value of the objective function
	void BracketMinimum(REAL& ax, REAL& bx, REAL& cx);

	// finds the minimum of the objective function
	REAL FindMinimum(REAL ax, REAL bx, REAL cx);

	// finds the minimum of the objective function, using gradient information
	REAL FindMinimumGrad(REAL ax, REAL bx, REAL cx);

private:
	// these are "local" variables that are initialized here so that we do not
	//		need to re-allocate them during optimization
	CVectorN<> m_vAx, m_vBx, m_vCx;
	CVectorN<> m_vX, m_vU;

	// holds the gradient
	CVectorN<> m_vGrad;

	// starting value for the brent optimizer
	//		initialized to 
	static const CVectorN<> m_vBrentInit;
};

#endif