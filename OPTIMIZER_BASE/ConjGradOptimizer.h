////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

#include <Matrix.h>

#include "Optimizer.h"
#include "BrentOptimizer.h"

template<int DIM, class TYPE>
class CConjGradOptimizer : public COptimizer<DIM, TYPE>
{
public:
	CConjGradOptimizer(CGradObjectiveFunction<DIM, TYPE> *pFunc)
		: COptimizer<DIM, TYPE>(pFunc),
			m_lineFunction(pFunc),
			m_optimizeBrent(&m_lineFunction)
	{
		finalValue.SyncTo(&m_optimizeBrent.finalValue);
		tolerance.SyncTo(&m_optimizeBrent.tolerance);
	}

	CValue< CMatrix<DIM, TYPE> > directionSet;

	virtual CVector<DIM, TYPE> Optimize(const CVector<DIM, TYPE>& vInit)
	{
		CGradObjectiveFunction<DIM, TYPE> *pGradFunc =
			(CGradObjectiveFunction<DIM, TYPE> *) m_pFunc;

		finalParam.Set(vInit);
		m_lineFunction.point.SyncTo(&finalParam);

		TYPE fp = (*pGradFunc)(finalParam.Get());
		CVector<DIM, TYPE> xi = pGradFunc->Grad(finalParam.Get());

		CVector<DIM, TYPE> g = -1.0 * xi;
		CVector<DIM, TYPE> h = xi = g;

		for (iteration.Set(0); iteration.Get() < ITMAX; 
				iteration.Set(iteration.Get()+1))
		{
			// set up the direction for the line minimization
			m_lineFunction.direction.Set(xi);

			// now launch a Brent optimization
			TYPE lambda = m_optimizeBrent.Optimize(0.0)[0];

			// update the final parameter value
			finalParam.Set(finalParam.Get() + lambda * m_lineFunction.direction.Get());

			// test for convergence
			if (2.0 * fabs(lambda - fp) <= tolerance.Get() * fabs(lambda) + fabs(fp) + ZEPS)
			{
				return finalParam.Get();
			}

			// store the previous lambda value
			fp = lambda;

			// compute the gradient at the current parameter value
			xi = pGradFunc->Grad(finalParam.Get());

			TYPE gg = g * g;

			// use this for Polak-Ribiere
			TYPE dgg = (xi + g) * xi;

			// use this for Fletcher-Reeves
			// TYPE dgg = xi * xi;

			if (gg == 0.0)
			{ 
				return finalParam.Get();
			}

			TYPE gam = dgg / gg;

			g = -1.0 * xi;
			xi = g + gam * h;
			h = xi;
		}

		ASSERT(FALSE); // "Too many iterations\n");

		return finalParam.Get();
	}

private:
	// line function that projects the objective function along a given line
	CGradLineFunction<DIM, TYPE> m_lineFunction;

	// brent optimizer along the line function
	// CGradBrentOptimizer<TYPE> m_optimizeBrent;
	CBrentOptimizer<TYPE> m_optimizeBrent;
};

#endif
