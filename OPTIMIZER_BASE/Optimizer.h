////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

// Optimizer.h: interface for the COptimizer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(OPTIMIZER_H)
#define OPTIMIZER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>
#include <Value.h>
#include "ObjectiveFunction.h"

template<int DIM, class TYPE>
class COptimizer
{
public:
	COptimizer(CObjectiveFunction<DIM, TYPE> *pFunc)
		: m_pFunc(pFunc),
			tolerance((TYPE) 0.1),
			iteration(0)
	{
	}

	virtual ~COptimizer()
	{
	}

	// defines the tolerance for exit from optimization loop
	CValue< TYPE > tolerance;

	// holds the number of iterations needed for the optimization
	CValue< int > iteration;

	// holds the final value of the optimization
	CValue< TYPE > finalValue;

	// holds the final value of the parameters for the minimum f
	CValue< CVector<DIM, TYPE> > finalParam;

	// function to actually perform the optimization
	virtual CVector<DIM, TYPE> Optimize(const CVector<DIM, TYPE>& vInit) = 0;

protected:
	CObjectiveFunction<DIM, TYPE> *m_pFunc;
};

#endif // !defined(OPTIMIZER_H)
