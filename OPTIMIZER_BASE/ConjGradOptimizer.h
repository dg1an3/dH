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
#include "BrentOptimizer.h"

///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

const int ITER_MAX = 1500;		// maximum iterations

const double ZEPS = 1.0e-10;	// z-epsilon -- small number to protect against 
								// fractional accuracy for a minimum that
								// happens to be exactly zero;  used in
								//    function FindMinimum

//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class CPowellOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
class CConjGradOptimizer : public COptimizer<DIM, TYPE>
{
public:
	// construct a gradient optimizer for an objective function
	CConjGradOptimizer(CObjectiveFunction<DIM, TYPE> *pFunc)
		: COptimizer<DIM, TYPE>(pFunc),
			m_lineFunction(pFunc),
			m_optimizeBrent(&m_lineFunction)
	{
		// set the Brent optimizer to use the gradient information
		m_optimizeBrent.SetUseGradientInfo(TRUE);
	}

	// optimize the objective function
	virtual CVector<DIM, TYPE> Optimize(const CVector<DIM, TYPE>& vInit);

private:
	// line function that projects the objective function along a given line
	CLineFunction<DIM, TYPE> m_lineFunction;

	// brent optimizer along the line function
	CBrentOptimizer<TYPE> m_optimizeBrent;
};

//////////////////////////////////////////////////////////////////////
// CConjGradOptimizer<DIM, TYPE>::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVector<DIM, TYPE> CConjGradOptimizer<DIM, TYPE>::Optimize(const CVector<DIM, TYPE>& vInit)
{
	// set the tolerance for the Brent optimizer
	m_optimizeBrent.SetTolerance(GetTolerance());

	m_vFinalParam = vInit;

	TYPE fp = (*m_pFunc)(m_vFinalParam);
	CVector<DIM, TYPE> xi = m_pFunc->Grad(m_vFinalParam);

	CVector<DIM, TYPE> g = -1.0 * xi;
	CVector<DIM, TYPE> h = xi = g;

	for (m_nIteration = 0; m_nIteration < ITER_MAX; m_nIteration++)
	{
		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, xi);

		// now launch a Brent optimization
		TYPE lambda = m_optimizeBrent.Optimize(0.0)[0];

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
		xi = m_pFunc->Grad(m_vFinalParam);

		TYPE gg = g * g;

		// use this for Polak-Ribiere
		TYPE dgg = (xi + g) * xi;

		// use this for Fletcher-Reeves
		// TYPE dgg = xi * xi;

		if (gg == 0.0)
		{ 
			m_finalValue = m_optimizeBrent.GetFinalValue();
			return m_vFinalParam;
		}

		TYPE gam = dgg / gg;

		g = -1.0 * xi;
		xi = g + gam * h;
		h = xi;
	}

	// Too many iterations
	TRACE0("Too many iterations\n");
	ASSERT(FALSE);

	// dummy return
	m_finalValue = m_optimizeBrent.GetFinalValue();
	return m_vFinalParam;
}

#endif
