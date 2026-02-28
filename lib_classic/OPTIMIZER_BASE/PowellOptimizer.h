//////////////////////////////////////////////////////////////////////
// PowellOptimizer.h: interface for the CPowellOptimizer 
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(POWELLOPTIMIZER_H)
#define POWELLOPTIMIZER_H

#include <Matrix.h>

// optimizer base class
#include "Optimizer.h"

// the underlying Brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"


//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class CPowellOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
class CPowellOptimizer : public COptimizer<DIM, TYPE>
{
public:
	// construct a new Powell optimizer
	CPowellOptimizer(CObjectiveFunction<DIM, TYPE> *pFunc)
		: COptimizer<DIM, TYPE>(pFunc),
			m_lineFunction(pFunc),
			m_optimizeBrent(&m_lineFunction)
	{
		// set the Brent optimizer to use the gradient information,
		//		if available
		m_optimizeBrent.SetUseGradientInfo(TRUE);
	}

	// performs the optimization
	virtual CVector<DIM, TYPE> Optimize(const CVector<DIM, TYPE>& vInit);

private:
	// line function that projects the objective function along a given line
	CLineFunction<DIM, TYPE> m_lineFunction;

	// brent optimizer along the line function
	CBrentOptimizer<TYPE> m_optimizeBrent;

	// the current direction set for the optimization
	CMatrix<DIM, TYPE> m_directionSet;
};

//////////////////////////////////////////////////////////////////////
// CPowellOptimizer<DIM, TYPE>::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CVector<DIM, TYPE> CPowellOptimizer<DIM, TYPE>::Optimize(const CVector<DIM, TYPE>& vInit)
{
	// set the tolerance for the Brent optimizer
	m_optimizeBrent.SetTolerance(GetTolerance());

	int i,ibig;
	TYPE del,fp,fptt,t; //,*pt,*ptt,*xit;  

	// sync the point in the line function to the current parameter value "finalParam"
	m_vFinalParam = vInit;
	CVector<DIM, TYPE> pt = vInit;	// save the initial point 
	CMatrix<DIM, TYPE> xi = m_directionSet;

	m_finalValue = (*m_pFunc)(vInit);
	for (m_nIteration = 0; ; m_nIteration++)
	{
		fp = m_finalValue;
		ibig = 0;
		del = 0.0;                     /* will the biggest function decrease? */
		for (i = 0; i < DIM; i++)             /* in each iteration, loop over all */
		{                            /* directions in the set */ 
			// store the current final value of the objective function
			fptt = m_finalValue;

			// set up the direction for the line minimization
			m_lineFunction.SetLine(m_vFinalParam, xi[i]);

			// now launch a Brent optimization
			TYPE lambda = m_optimizeBrent.Optimize(0.0)[0];

			// set the final value
			m_finalValue = m_optimizeBrent.GetFinalValue();

			// update the current point
			m_vFinalParam += lambda * m_lineFunction.GetDirection();

			if (fabs(fptt - m_finalValue) > del)
			{
				del = (TYPE) fabs(fptt - m_finalValue);
				ibig = i;
			}
		}

		 /* termination criterion */
		if (2.0 * fabs(fp - m_finalValue) <= 
			m_optimizeBrent.GetTolerance() * (fabs(fp) + fabs(m_finalValue)))
		{
			m_directionSet = xi;

			// update the final value of the objective function
			ASSERT(GetFinalValue() == m_optimizeBrent.GetFinalValue());

			return GetFinalParameter();
		}

		CVector<DIM, TYPE> ptt = (TYPE)2.0 * m_vFinalParam - pt;
		CVector<DIM, TYPE> xit = m_vFinalParam - pt;
		pt = m_vFinalParam;
		fptt = (*m_pFunc)(ptt);

		if (fptt < fp)
		{
			/*      t=2.0*(fp-2.0*finalValue.Get()+fptt)*SQR(fp-finalValue.Get()-del)-del*SQR(fp-fptt); */
			t = (TYPE)2.0 * (fp - (TYPE)2.0 * m_finalValue + fptt)
				* ( (fp-m_finalValue - del)*(fp-m_finalValue - del) ) - del
				* ( (fp-fptt) * (fp-fptt) );
			if (t < 0.0)
			{
				// set up the direction for the line minimization
				m_lineFunction.SetLine(m_vFinalParam, xit);

				// now launch a Brent optimization
				TYPE lambda = m_optimizeBrent.Optimize((TYPE)0.0)[0];

				// update the current point
				m_vFinalParam += lambda * m_lineFunction.GetDirection();

				xi[ibig] = xi[DIM-1];
				xi[DIM-1] = xit;
			}
		}
	}
}

#endif
