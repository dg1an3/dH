#if !defined(POWELLOPTIMIZER_H)
#define POWELLOPTIMIZER_H

#include <Matrix.h>

#include "Optimizer.h"
#include "BrentOptimizer.h"

template<int DIM, class TYPE>
class CPowellOptimizer : public COptimizer<DIM, TYPE>
{
public:
	CPowellOptimizer(CObjectiveFunction<DIM, TYPE> *pFunc)
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
		int i,ibig;
		TYPE del,fp,fptt,t; //,*pt,*ptt,*xit;  

		// sync the point in the line function to the current parameter value "finalParam"
		m_lineFunction.point.SyncTo(&finalParam);
		finalParam.Set(vInit);
		CVector<DIM, TYPE> pt = vInit;	// save the initial point 
		CMatrix<DIM, TYPE> xi = directionSet.Get();

		finalValue.Set((*m_pFunc)(vInit));
		for (iteration.Set(0); ; iteration.Set(iteration.Get()+1))
		{
			fp = finalValue.Get();
			ibig = 0;
			del = 0.0;                     /* will the biggest function decrease? */
			for (i = 0; i < DIM; i++)             /* in each iteration, loop over all */
			{                            /* directions in the set */ 
				// store the current final value of the objective function
				fptt = finalValue.Get();

				// set up the direction for the line minimization
				m_lineFunction.direction = xi[i];

				// now launch a Brent optimization
				TYPE lambda = m_optimizeBrent.Optimize(0.0)[0];

				// update the current point
				finalParam.Set(finalParam.Get() + lambda * m_lineFunction.direction.Get());

				if (fabs(fptt - finalValue.Get()) > del)
				{
					del = (TYPE) fabs(fptt - finalValue.Get());
					ibig = i;
				}
			}

			 /* termination criterion */
			if (2.0 * fabs(fp - finalValue.Get()) <= 
				m_optimizeBrent.tolerance.Get() * (fabs(fp) + fabs(finalValue.Get())))
			{
				directionSet.Set(xi);

				// update the final value of the objective function
				ASSERT(finalValue.Get() == m_optimizeBrent.finalValue.Get());

				return finalParam.Get();
			}

			CVector<DIM, TYPE> ptt = (TYPE)2.0 * finalParam.Get() - pt;
			CVector<DIM, TYPE> xit = finalParam.Get() - pt;
			pt = finalParam.Get();
			fptt = (*m_pFunc)(ptt);

			if (fptt < fp)
			{
				/*      t=2.0*(fp-2.0*finalValue.Get()+fptt)*SQR(fp-finalValue.Get()-del)-del*SQR(fp-fptt); */
				t = (TYPE)2.0 * (fp - (TYPE)2.0 * finalValue.Get() + fptt)
					* ( (fp-finalValue.Get() - del)*(fp-finalValue.Get() - del) ) - del
					* ( (fp-fptt) * (fp-fptt) );
				if (t < 0.0)
				{
					// set up the direction for the line minimization
					m_lineFunction.direction = xit;

					// now launch a Brent optimization
					TYPE lambda = m_optimizeBrent.Optimize((TYPE)0.0)[0];

					// update the current point
					finalParam.Set(finalParam.Get() + lambda * m_lineFunction.direction.Get());

					xi[ibig] = xi[DIM-1];
					xi[DIM-1] = xit;
				}
			}
		}
	}

private:
	// line function that projects the objective function along a given line
	CLineFunction<DIM, TYPE> m_lineFunction;

	// brent optimizer along the line function
	CBrentOptimizer<TYPE> m_optimizeBrent;
};

#endif
