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
const int ITER_MAX = 500;		

// z-epsilon -- small number to protect against fractional accuracy for 
//		a minimum that happens to be exactly zero.
const REAL ZEPS = (REAL) 1.0e-10;	

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
		m_bSetLineToleranceEqual(true)
{
	// set the Brent optimizer to use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);

}	// CConjGradOptimizer::CConjGradOptimizer


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::GetBrentOptimizer
// 
// returns the embedded line optimizer
///////////////////////////////////////////////////////////////////////////////
CBrentOptimizer& CConjGradOptimizer::GetBrentOptimizer()
{
	return m_optimizeBrent;

}	// CConjGradOptimizer::GetBrentOptimizer


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::Optimize
// 
// performs the optimization given the initial value vector
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CConjGradOptimizer::Optimize(const CVectorN<>& vInit)
{
	BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize);
	LOG_EXPR_EXT(vInit);

	// set the tolerance for the line optimizer
	if (m_bSetLineToleranceEqual)
	{
		m_pLineOptimizer->SetTolerance(GetTolerance());
	}
	LOG_EXPR(GetTolerance());

	// store the initial parameter vector
	m_vFinalParam.SetDim(vInit.GetDim());
	m_vFinalParam = vInit;

	// set the dimension of the current direction
	m_vGrad.SetDim(vInit.GetDim());

	// evaluate the function at the initial point, storing
	//		the gradient as the current direction
	m_finalValue = (*m_pFunc)(m_vFinalParam, &m_vGrad);
	m_vGrad *= R(-1.0);
	LOG_EXPR(m_finalValue);
	LOG_EXPR_EXT(m_vGrad);

	// if we are too short,
	if (m_vGrad.GetLength() < 1e-8) // GetTolerance())
	{
		LOG("Gradient too small -- adding length");
		RandomVector(GetTolerance(), &m_vGrad[0], m_vGrad.GetDim());
	}

	// set the initial (steepest descent) direction
	m_vDirPrev.SetDim(m_vGrad.GetDim());
	m_vDir.SetDim(m_vGrad.GetDim());
	m_vDirPrev = m_vDir = m_vGrad;

	BOOL bConvergence = FALSE;
	for (m_nIteration = 0; (m_nIteration < ITER_MAX) && !bConvergence; m_nIteration++)
	{
		LOG(FMT("Iteration %i", m_nIteration));

		///////////////////////////////////////////////////////////////////////////////
		// line minimization

		BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize!Line_Minimization);

		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vDir); // m_vGrad);

		// now launch a line optimization
		REAL lambda = m_pLineOptimizer->Optimize(CBrentOptimizer::GetInitZero())[0];
		LOG_EXPR(lambda);

		// update the final parameter value
		m_vLambdaScaled.SetDim(m_lineFunction.GetDirection().GetDim());
		m_vLambdaScaled = m_lineFunction.GetDirection();
		m_vLambdaScaled *= lambda;
		m_vFinalParam += m_vLambdaScaled;
		LOG_EXPR_EXT(m_vFinalParam);

		// store the final value from the line optimizer
		REAL new_fv = m_pLineOptimizer->GetFinalValue();

		// test for convergence on line minimalization
		bConvergence = (2.0 * fabs(m_finalValue - new_fv) 
			<= GetTolerance() * (fabs(m_finalValue) + fabs(new_fv) + ZEPS));

		// store the previous lambda value
		m_finalValue = new_fv;
		LOG_EXPR(m_finalValue);

		END_LOG_SECTION();	// Line Minimization

		// need to call-back?
		if (m_pCallbackFunc)
		{
			if (!(*m_pCallbackFunc)(this, m_pCallbackParam)) 
			{
				// request to terminate
				m_nIteration = -1;
				return m_vFinalParam;
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Update Direction

		BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize!Update_Direction);
		
		// compute denominator for gamma
		REAL gg = m_vGrad * m_vGrad;
		bConvergence = bConvergence || (gg == 0.0);

		// must have performed at least one full iteration
		if (!bConvergence)
		{
			// store gradient for 
			m_vGradPrev.SetDim(m_vGrad.GetDim());
			m_vGradPrev = m_vGrad;

			// compute the gradient at the current parameter value
			(*m_pFunc)(m_vFinalParam, &m_vGrad);
			m_vGrad *= -1.0;
			LOG_EXPR_EXT(m_vGrad);

			// compute numerator for gamma (Polak-Ribiera formula)
			REAL dgg = m_vGrad * m_vGrad - m_vGradPrev * m_vGrad;

			// otherwise, update the direction
			m_vDir *= dgg / gg;
			m_vDir += m_vGrad;
			LOG_EXPR_EXT(m_vDir);

			BEGIN_LOG_ONLY(Test Direction Conjugacy);
			
			// output orthogonality conditions
			LOG_EXPR(m_vGrad * m_vGradPrev);
			LOG_EXPR(m_vGrad * m_vDirPrev);

			// output vector lengths for comparison
			LOG_EXPR(m_vDir.GetLength());
			LOG_EXPR(m_vGrad.GetLength());

			// store direction for next iteration
  			m_vDirPrev = m_vDir;

			END_LOG_SECTION();	// Test Conjugacy
		}
		else
		{
			TRACE("Convergence in %i iterations\n", m_nIteration);
		}

		END_LOG_SECTION();	// Update Direction
	}

	if (!bConvergence)
	{
		// Too many iterations
		LOG("Too many iterations");

		m_nIteration = -1;
	}

	END_LOG_SECTION();	// CConjGradOptimizer

	// return the last parameter vector
	return m_vFinalParam;

}	// CConjGradOptimizer::Optimize
