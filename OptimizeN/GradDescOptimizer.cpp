//////////////////////////////////////////////////////////////////////
// CGradDescOptimizer.cpp: implementation of the CGradDescOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "GradDescOptimizer.h"


///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

const REAL STEP_SCALE = 0.99;	// scale for each step

const int ITER_MAX = 1500;		// maximum iteration


//////////////////////////////////////////////////////////////////////
// CGradDescOptimizer::CGradDescOptimizer
// 
// constructs a new gradient descent optimizer
//////////////////////////////////////////////////////////////////////
CGradDescOptimizer::CGradDescOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc)
{
}

//////////////////////////////////////////////////////////////////////
// CGradDescOptimizer::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CGradDescOptimizer::Optimize(const CVectorN<>& vInit)
{
	// initialize starting value
	m_vFinalParam = vInit;

	// first evaluation of function
	REAL prevValue = (*m_pFunc)(m_vFinalParam, &m_vGrad);
	if (m_vGrad.GetLength() < GetTolerance())
	{
		// add some length
		for (int nAt = 0; nAt < m_vGrad.GetDim(); nAt++)
		{
			m_vGrad[nAt] += GetTolerance() -
				(REAL) rand() * 2.0 * GetTolerance() / (REAL) RAND_MAX;
		}
	}

	// Main function loop. 
	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		// move along the gradient
		m_vFinalParam -= STEP_SCALE * m_vGrad;

		// compute the value at the new point
		m_finalValue = (*m_pFunc)(m_vFinalParam, &m_vGrad);

		// test for convergence
		if ((REAL) fabs(m_finalValue - prevValue) < GetTolerance())
		{
			return m_vFinalParam;
		}

		// store the computed value
		prevValue = m_finalValue;
	}

	ASSERT("Too many iterations\n");

	return m_vFinalParam;
}
