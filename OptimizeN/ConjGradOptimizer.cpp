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

const int ITER_MAX = 1500;		// maximum iterations

const double ZEPS = 1.0e-10;	// z-epsilon -- small number to protect against 
								// fractional accuracy for a minimum that
								// happens to be exactly zero;  used in
								//    function FindMinimum

// construct a gradient optimizer for an objective function
CConjGradOptimizer::CConjGradOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_lineFunction(pFunc),
		m_optimizeBrent(&m_lineFunction)
{
	// set the Brent optimizer to use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);
}


//////////////////////////////////////////////////////////////////////
// CConjGradOptimizer<DIM, REAL>::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CConjGradOptimizer::Optimize(const CVectorN<>& vInit)
{
	// set the tolerance for the Brent optimizer
	m_optimizeBrent.SetTolerance(GetTolerance());

	m_vFinalParam = vInit;

	// CVectorN<> xi;
	m_vXi.SetDim(vInit.GetDim());
	REAL fp = (*m_pFunc)(m_vFinalParam, &m_vXi);
	if (m_vXi.GetLength() < GetTolerance())
	{
		// add some length
		for (int nAt = 0; nAt < m_vXi.GetDim(); nAt++)
		{
			m_vXi[nAt] += GetTolerance() -
				(REAL) rand() * 2.0 * GetTolerance() / (REAL) RAND_MAX;
		}
	}

	TraceVector(m_vXi);

	// CVectorN<> g;
	m_vG = m_vXi;
	m_vG *= -1.0;

	// CVectorN<> h;
	m_vH = m_vXi = m_vG;

	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vXi);
		TraceVector(m_vFinalParam);
		TraceVector(m_vXi);

		// now launch a Brent optimization
		// static CVectorN<> v0(1);
		// v0[0] = 0.0;
		REAL lambda = m_optimizeBrent.Optimize()[0];

		// update the final parameter value
		m_vFinalParam += lambda * m_lineFunction.GetDirection();

		// test for convergence
		if (2.0 * fabs(lambda - fp) <= GetTolerance() * fabs(lambda) + fabs(fp) + ZEPS)
		{
			m_finalValue = m_optimizeBrent.GetFinalValue();
			return m_vFinalParam;
		}

		// store the previous lambda value
		fp = lambda;

		// compute the gradient at the current parameter value
		(*m_pFunc)(m_vFinalParam, &m_vXi);
// 		ASSERT(m_vXi != -1.0 * m_vG);
		TraceVector(m_vXi);
		TraceVector(m_vG);

		REAL gg = m_vG * m_vG;

// #define USE_FLETCHER_REEVES
#ifdef USE_FLETCHER_REEVES
		// use this for Fletcher-Reeves
		REAL dgg = m_vXi * m_vXi;
#else
		// use this for Polak-Ribiere
		REAL dgg = (m_vXi + m_vG) * m_vXi;
#endif

		if (gg == 0.0)
		{ 
			m_finalValue = m_optimizeBrent.GetFinalValue();
			return m_vFinalParam;
		}

		REAL gam = dgg / gg;

		m_vG = m_vXi;
		m_vG *= -1.0;
		m_vXi = m_vG + gam * m_vH;
		m_vH = m_vXi;
	}

	// Too many iterations
	TRACE0("Too many iterations\n");
	ASSERT(FALSE);

	// dummy return
	m_finalValue = m_optimizeBrent.GetFinalValue();
	return m_vFinalParam;
}

