//////////////////////////////////////////////////////////////////////
// ConjGradOptimizer.h: interface for the CConjGradOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

// base class includes
#include "Optimizer.h"

// subordinate brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"


//////////////////////////////////////////////////////////////////////
// class CConjGradOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class CConjGradOptimizer : public COptimizer
{
public:
	// construct a gradient optimizer for an objective function
	CConjGradOptimizer(CObjectiveFunction *pFunc);

	// optimize the objective function
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

private:
	// line function that projects the objective function along 
	//		a given line
	CLineFunction m_lineFunction;

	// brent optimizer along the line function
	CBrentOptimizer m_optimizeBrent;

	// "statics" for the optimization routine
	CVectorN<> m_vG;
	CVectorN<> m_vXi;
	CVectorN<> m_vH;
};

#endif
