//////////////////////////////////////////////////////////////////////
// PowellOptimizer.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// set up XML logging
#include <XMLLogging.h>

// include the Powell optimizer
#include "PowellOptimizer.h"

const int ITER_MAX = 1000;		// maximum iteration

//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::CPowellOptimizer
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
CPowellOptimizer::CPowellOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_lineFunction(pFunc),
		m_optimizeBrent(&m_lineFunction)
{
	// set the Brent optimizer to not use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);

}	// CPowellOptimizer::CPowellOptimizer


//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::~CPowellOptimizer
// 
// destroy the optimizer
//////////////////////////////////////////////////////////////////////
CPowellOptimizer::~CPowellOptimizer()
{
}	// CPowellOptimizer::~CPowellOptimizer


//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CPowellOptimizer::Optimize(const CVectorN<>& vInit)
{
	BEGIN_LOG_SECTION(CPowellOptimizer::Optimize);

	LOG_EXPR_EXT(vInit);
	LOG_EXPR(GetTolerance());

	// grab the dimensions for the optimization
	int nDim = vInit.GetDim();

	// reshape direction set
	m_mDirections.Reshape(nDim, nDim);
	m_mDirections.SetIdentity();

	// set the tolerance for the Brent optimizer
	m_optimizeBrent.SetTolerance(GetTolerance());

	// sync the point in the line function to the current parameter value "finalParam"
	m_vFinalParam = vInit;
	m_vPt = vInit;	// save the initial point 

	m_finalValue = (*m_pFunc)(vInit);

	BOOL bConvergence = FALSE;
	for (m_nIteration = 0; m_nIteration < ITER_MAX && !bConvergence; m_nIteration++)
	{
		BEGIN_LOG_SECTION_(FMT("Iteration %i", m_nIteration));

		// store the current final value prior to iteration over directoin set
		REAL fp = m_finalValue;

		// perform one complete iteration over the direction set
		REAL del;         
		int nBigDir = IterateOverDirectionSet(del);

		// termination criterion 
		if (2.0 * fabs(fp - m_finalValue) >
			m_optimizeBrent.GetTolerance() * (fabs(fp) + fabs(m_finalValue)))
		{
			// static CVectorN<> ptt = 2.0f * m_vFinalParam - pt;
			m_vPtt = m_vFinalParam;
			m_vPtt *= (REAL) 2.0;
			m_vPtt -= m_vPt;
			LOG_EXPR_EXT_DESC(m_vPtt, "m_vPtt = 2.0 * m_vFinalParam - m_vPt");

			// static CVectorN<> xit = m_vFinalParam - pt;
			m_vXit = m_vFinalParam;
			m_vXit -= m_vPt;
			LOG_EXPR_EXT_DESC(m_vXit, "m_vXit = m_vFinalParam - m_vPt");

			m_vPt = m_vFinalParam;
			REAL fptt = (*m_pFunc)(m_vPtt);

			// see if the new parameters is less
			if (fptt < fp)
			{
				// yes, so set a new direction
				LOG("fptt(%lf) &lt; fp(%lf)", fptt, fp);

				FindNewDirection(fp, fptt, del, nBigDir);		
			}
			else LOG("fptt(%lf) &gt;= fp(%lf)", fptt, fp);			 
		}
		else
		{
			LOG("Convergence at %lf, fp = %lf", m_finalValue, fp);
			bConvergence = TRUE;
		}

		END_LOG_SECTION();	// Iteration
	}
	
	END_LOG_SECTION();	// CPowellOptimizer::Optimize

	return m_vFinalParam;

}	// CPowellOptimizer::Optimize



//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::IterateOverDirectionSet
// 
// iterates over direction set once, returning the direction of
//		biggest change
// 
//////////////////////////////////////////////////////////////////////
int CPowellOptimizer::IterateOverDirectionSet(REAL &del)
{
	int nBigDir = 0;
	del = 0.0;

	BEGIN_LOG_SECTION_("CPowellOptimizer::IterateOverDirectionSet");

	// in each iteration, loop over all directions in the set
	int nDim = m_mDirections.GetCols();
	for (int nAtDir = 0; nAtDir < nDim; nAtDir++)	
	{												
		BEGIN_LOG_SECTION_(FMT("Direction %i of %i", nAtDir, nDim));

		// store the current final value of the objective function
		REAL fptt = m_finalValue;

		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_mDirections[nAtDir]);
		LOG_EXPR_EXT_DESC(m_vFinalParam, "Line origin");
		LOG_EXPR_EXT_DESC(m_mDirections[nAtDir], "Line direction");

		// now launch a Brent optimization
		REAL lambda = m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];
		LOG_EXPR(lambda);

		// set the final value
		m_finalValue = m_optimizeBrent.GetFinalValue();

		// update the current point
		// m_vFinalParam += lambda * m_lineFunction.GetDirection();
		// static CVectorN<> m_vScaledDir;
		m_vScaledDir = m_lineFunction.GetDirection();
		m_vScaledDir *= (fabs(lambda) > DEFAULT_EPSILON) 
			? lambda : DEFAULT_EPSILON;
		m_vFinalParam += m_vScaledDir;
		LOG_EXPR_EXT_DESC(m_vFinalParam, "Next Param");

		if (fabs(fptt - m_finalValue) > del)
		{
			del = (REAL) fabs(fptt - m_finalValue);
			nBigDir = nAtDir;
		}

		END_LOG_SECTION(); // Direction
	}
	LOG("Dimension of biggest change = %i", nBigDir);
	LOG_EXPR(del);
	LOG_EXPR(m_finalValue);
	LOG_EXPR_EXT_DESC(m_mDirections[nBigDir], "Direction of Biggest Change");

	END_LOG_SECTION();

	return nBigDir;

}	// CPowellOptimizer::IterateOverDirectionSet


//////////////////////////////////////////////////////////////////////
// CPowellOptimizer::FindNewDirection
// 
// determines a new direction to add to the direction set
//////////////////////////////////////////////////////////////////////
void CPowellOptimizer::FindNewDirection(REAL fp, REAL fptt, REAL del, int nBigDir)
{
	BEGIN_LOG_SECTION_("CPowellOptimizer::FindNewDirection");

	// compute the t parameter to determine the brent optimization parameter
	REAL t = (REAL) 2.0 
		* (fp - (REAL)2.0*m_finalValue + fptt)
		* ( (fp-m_finalValue - del) * (fp-m_finalValue - del) ) 
			- del * ( (fp-fptt) * (fp-fptt) );
	LOG_EXPR(t);

	// if t is less than zero,
	if (t < 0.0)
	{
		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vXit);
		LOG_EXPR_EXT_DESC(m_vFinalParam, "Line origin (t &lt; 0.0)");
		LOG_EXPR_EXT_DESC(m_vXit, "Line direction (t &lt; 0.0)");

		// now launch a Brent optimization
		REAL lambda = m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];
		LOG_EXPR(lambda);

		// update the current point
		// m_vFinalParam += lambda * m_lineFunction.GetDirection();
		m_vScaledDir = m_lineFunction.GetDirection();
		m_vScaledDir *= (fabs(lambda) > DEFAULT_EPSILON) 
			? lambda : DEFAULT_EPSILON;
		m_vFinalParam += m_vScaledDir;

		int nDim = m_mDirections.GetCols();
		m_mDirections[nBigDir] = m_mDirections[nDim - 1];
		LOG("Setting new big direction to dim %i", nBigDir);

		// set up the direction for the line minimization
		m_mDirections[nDim - 1] = m_vXit;
		LOG("Setting least dimension to m_vXit");
		
		LOG_EXPR_EXT_DESC(m_mDirections, "New Direction Set");
	}

	END_LOG_SECTION();

}	// CPowellOptimizer::FindNewDirection

