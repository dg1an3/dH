//////////////////////////////////////////////////////////////////////
// ConjGradOptimizer.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "ConjGradOptimizer.h"


///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

// maximum iterations
const int ITER_MAX = 100;		

// z-epsilon -- small number to protect against fractional accuracy for 
//		a minimum that happens to be exactly zero.
const REAL ZEPS = 1.0e-10;	

///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::CConjGradOptimizer
// 
// construct a gradient optimizer for an objective function
///////////////////////////////////////////////////////////////////////////////
CConjGradOptimizer::CConjGradOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_lineFunction(pFunc),
		m_pLineOptimizer(&m_optimizeBrent),
		m_optimizeBrent(&m_lineFunction),
		m_optimizeCubic(&m_lineFunction)
{
	// set the Brent optimizer to use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);

	// set the cubic interpolation optimizer to use gradient
	m_optimizeCubic.SetUseGradientInfo(TRUE);
}


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::Optimize
// 
// performs the optimization given the initial value vector
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CConjGradOptimizer::Optimize(const CVectorN<>& vInit)
{
	// set the tolerance for the line optimizer
	m_pLineOptimizer->SetTolerance(GetTolerance());

	// store the initial parameter vector
	m_vFinalParam = vInit;

	// set the dimension of the current direction
	m_vXi.SetDim(vInit.GetDim());

	// evaluate the function at the initial point, storing
	//		the gradient as the current direction
	REAL fp = (*m_pFunc)(m_vFinalParam, &m_vXi);

	// if we are too short,
	if (m_vXi.GetLength() < GetTolerance())
	{
		// add some length
		for (int nAt = 0; nAt < m_vXi.GetDim(); nAt++)
		{
			// by adding random elements
			m_vXi[nAt] += GetTolerance() -
				(REAL) rand() * 2.0 * GetTolerance() / (REAL) RAND_MAX;
		}
	}

	// initialize the G vector
	m_vG = m_vXi;
	m_vG *= -1.0;

	// initialize the H vector
	m_vH = m_vXi = m_vG;

	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vXi);

		// now launch a line optimization
		REAL lambda = m_pLineOptimizer->Optimize(CBrentOptimizer::GetInitZero())[0];

		// check for failure if cubic
		if (m_pLineOptimizer == &m_optimizeCubic && !m_optimizeCubic.IsSuccess())
		{
			// try a brent optimization
			lambda = m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];
		}

		// update the final parameter value
		m_vFinalParam += lambda * m_lineFunction.GetDirection();

		// store the final value from the line optimizer
		m_finalValue = m_pLineOptimizer->GetFinalValue();

		// test for convergence
		if (2.0 * fabs(lambda - fp) <= GetTolerance() * fabs(lambda) 
				+ fabs(fp) + ZEPS)
		{
			return m_vFinalParam;
		}

		// store the previous lambda value
		fp = lambda;

		// compute the gradient at the current parameter value
		(*m_pFunc)(m_vFinalParam, &m_vXi);

		// gg measures the direction
		REAL gg = m_vG * m_vG;

#ifdef USE_FLETCHER_REEVES
		// use this for Fletcher-Reeves
		REAL dgg = m_vXi * m_vXi;
#else
		// use this for Polak-Ribiere
		REAL dgg = (m_vXi + m_vG) * m_vXi;
#endif

		// are we done yet?
		if (gg == 0.0)
		{ 
			// and return the final parameter vector
			return m_vFinalParam;
		}

		// otherwise, update the direction
		REAL gam = dgg / gg;

		m_vG = m_vXi;
		m_vG *= -1.0;
		m_vXi = m_vG + gam * m_vH;
		m_vH = m_vXi;
	}

	// Too many iterations
	TRACE0("Too many iterations\n");
	// ASSERT(FALSE);

	// return the last parameter vector
	return m_vFinalParam;
}

