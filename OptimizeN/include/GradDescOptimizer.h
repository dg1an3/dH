//////////////////////////////////////////////////////////////////////
// GradDescOptimizer.h: interface for the CGradDescOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(GRADDESCOPTIMIZER_H)
#define GRADDESCOPTIMIZER_H

// include the optimizer base class
#include "Optimizer.h"

//////////////////////////////////////////////////////////////////////
// class CGradDescOptimizer
// 
// optimizer that implements 1D gradient optimizer
//////////////////////////////////////////////////////////////////////
class CGradDescOptimizer : public COptimizer
{
public:
	// construct a new Gradient Descent Optimizer
	CGradDescOptimizer(CObjectiveFunction *pFunc);

	// optimizes the initial input vector
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

private:
	// holds the gradient
	CVectorN<> m_vGrad;
};

#endif