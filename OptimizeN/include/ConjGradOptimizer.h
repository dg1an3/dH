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

// cubic interpolation optimizer
#include "CubicInterpOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"


typedef void OptimizerCallback(REAL lambda, const CVectorN<>& vDir, 
							 void *pParam);

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

	// returns a reference to the embedded Brent optimizer
	CBrentOptimizer& GetBrentOptimizer();

	// sets the callback function
	void SetCallback(OptimizerCallback *pCallback, void *pParam = NULL);

	// optimize the objective function
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

private:
	// line function that projects the objective function along 
	//		a given line
	CLineFunction m_lineFunction;

	// points to the line optimizer to be used
	COptimizer *m_pLineOptimizer;

	// brent optimizer along the line function
	CBrentOptimizer m_optimizeBrent;

	// stores the callback info
	OptimizerCallback *m_pCallbackFunc;
	void *m_pCallbackParam;

	// "statics" for the optimization routine
	CVectorN<> m_vGrad;
	CVectorN<> m_vGradPrev;
	CVectorN<> m_vDir;
	CVectorN<> m_vDirPrev;
};

#endif
