//////////////////////////////////////////////////////////////////////
// PowellOptimizer.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// include the Powell optimizer
#include "PowellOptimizer.h"

//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::CPowellOptimizer
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
CPowellOptimizer::CPowellOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_lineFunction(pFunc),
		m_optimizeBrent(&m_lineFunction),
		m_pDirections(NULL)
{
	// set the Brent optimizer to not use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);
}

//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::~CPowellOptimizer
// 
// destroy the optimizer
//////////////////////////////////////////////////////////////////////
CPowellOptimizer::~CPowellOptimizer()
{
	delete [] m_pDirections;
}

//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CPowellOptimizer::Optimize(const CVectorN<>& vInit)
{
	// grab the dimensions for the optimization
	int nDim = vInit.GetDim();

	if (NULL == m_pDirections 
		|| m_pDirections[0].GetDim() != nDim)
	{
		// clear the direction matrix
		delete [] m_pDirections;

		// initialize the directions
		m_pDirections = new CVectorN<>[nDim];
	}

	// set the directions to an identity matrix
	for (int nDir = 0; nDir < nDim; nDir++)
	{
		m_pDirections[nDir].SetDim(nDim);
		m_pDirections[nDir][nDir] = 1.0f;
	}

	// set the tolerance for the Brent optimizer
	m_optimizeBrent.SetTolerance(GetTolerance());

	int nAtDir, nBigDir;
	REAL del,fp,fptt,t;

	// sync the point in the line function to the current parameter value "finalParam"
	m_vFinalParam = vInit;
	m_vPt = vInit;	// save the initial point 

	m_finalValue = (*m_pFunc)(vInit);
	for (m_nIteration = 0; ; m_nIteration++)
	{
		fp = m_finalValue;
		nBigDir = 0;
		del = 0.0;                     // will the biggest function decrease? 
		for (nAtDir = 0; nAtDir < nDim; nAtDir++)	// in each iteration, loop over all 
		{											// directions in the set
			// store the current final value of the objective function
			fptt = m_finalValue;

			// set up the direction for the line minimization
			m_lineFunction.SetLine(m_vFinalParam, m_pDirections[nAtDir]);

			// now launch a Brent optimization
			REAL lambda = m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];

			// set the final value
			m_finalValue = m_optimizeBrent.GetFinalValue();

			// update the current point
			// m_vFinalParam += lambda * m_lineFunction.GetDirection();
			// static CVectorN<> m_vScaledDir;
			m_vScaledDir = m_lineFunction.GetDirection();
			m_vScaledDir *= (fabs(lambda) > EPS) ? lambda : EPS;
			m_vFinalParam += m_vScaledDir;

			if (fabs(fptt - m_finalValue) > del)
			{
				del = (REAL) fabs(fptt - m_finalValue);
				nBigDir = nAtDir;
			}
		}

		// termination criterion 
		if (2.0 * fabs(fp - m_finalValue) <= 
			m_optimizeBrent.GetTolerance() * (fabs(fp) + fabs(m_finalValue)))
		{
			// update the final value of the objective function
			ASSERT(GetFinalValue() == m_optimizeBrent.GetFinalValue());

			return m_vFinalParam;
		}

		// static CVectorN<> ptt = 2.0f * m_vFinalParam - pt;
		m_vPtt = m_vFinalParam;
		m_vPtt *= 2.0f;
		m_vPtt -= m_vPt;

		// static CVectorN<> xit = m_vFinalParam - pt;
		m_vXit = m_vFinalParam;
		m_vXit -= m_vPt;

		m_vPt = m_vFinalParam;
		fptt = (*m_pFunc)(m_vPtt);

		if (fptt < fp)
		{
			// compute the t parameter to determine the brent optimization parameter
			t = (REAL)2.0 * (fp - (REAL)2.0 * m_finalValue + fptt)
				* ( (fp-m_finalValue - del)*(fp-m_finalValue - del) ) - del
				* ( (fp-fptt) * (fp-fptt) );

			// if t is less than zero,
			if (t < 0.0)
			{
				// set up the direction for the line minimization
				m_lineFunction.SetLine(m_vFinalParam, m_vXit);

				// now launch a Brent optimization
				REAL lambda = m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];

				// update the current point
				// m_vFinalParam += lambda * m_lineFunction.GetDirection();
				m_vScaledDir = m_lineFunction.GetDirection();
				m_vScaledDir *= (fabs(lambda) > EPS) ? lambda : EPS;
				m_vFinalParam += m_vScaledDir;

				m_pDirections[nBigDir] = m_pDirections[nDim - 1];

				// set up the direction for the line minimization
				m_pDirections[nDim - 1] = m_vXit;
			}
		}
	}
}

