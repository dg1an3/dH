// CubicInterpOptimizer.cpp: implementation of the CCubicInterpOptimizer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CubicInterpOptimizer.h"

#include <Matrix.h>

// holds the initial value for the brent optimization
const CVectorN<> CCubicInterpOptimizer::m_vInit(1);

const double BRACKET = 10.0;		// initial bracket size


//////////////////////////////////////////////////////////////////////
// SolveQuadratic
// 
// solves a quadratic equation.  returns FALSE if no real roots.
//////////////////////////////////////////////////////////////////////
BOOL SolveQuadratic(REAL a, REAL b, REAL c, REAL *r1, REAL *r2)
{
	// compute determinant terms
	REAL b_sq = b * b;
	REAL ac_4 = 4.0 * a * c;

	// test for real roots
	if (b_sq < ac_4)
	{
		// no real roots, return FALSE
		return FALSE;
	}

	// compute determinant
	REAL det = sqrt(b_sq - ac_4);

	// compute roots
	(*r1) = (-b + det) / (2.0 * a);
	(*r2) = (-b - det) / (2.0 * a);

	// roots found
	return TRUE;
}

void FindMinimumFast(REAL x1, REAL fx1, REAL dx1, 
					 REAL x2, REAL fx2, REAL dx2,
					 REAL *min_guess)
{
	// update constants
	REAL beta1 = dx1 + dx2 - 3.0 * (fx1 - fx2) / (x1 - x2);
	REAL beta2 = sqrt(beta1 * beta1 - dx1 * dx2);

	// compute the next guess for minimum
	(*min_guess) = x2 - (x2 - x1) * (dx2 + beta2 - beta1) 
		/ (dx2 - dx1 + 2.0 * beta2);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CCubicInterpOptimizer::CCubicInterpOptimizer
// 
// constructs the optimizer
//////////////////////////////////////////////////////////////////////
CCubicInterpOptimizer::CCubicInterpOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc)
{
}

//////////////////////////////////////////////////////////////////////
// CCubicInterpOptimizer::~CCubicInterpOptimizer
// 
// destroys the optimizer
//////////////////////////////////////////////////////////////////////
CCubicInterpOptimizer::~CCubicInterpOptimizer()
{
}

//////////////////////////////////////////////////////////////////////
// CCubicInterpOptimizer::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
const CVectorN<>& CCubicInterpOptimizer::Optimize(const CVectorN<>& vInit)
{
	// initialize the first point
	m_vFinalParam = vInit;
	m_vGrad.SetDim(1);
	m_finalValue = (*m_pFunc)(m_vFinalParam, &m_vGrad);

	// initialize the second point
	CVectorN<> v2 = vInit;
	if (m_vGrad[0] < 0.0)
	{
		v2[0] += BRACKET;
	}
	else
	{
		v2[0] -= BRACKET;
	}
	CVectorN<> vGrad2(1);
	REAL fx2;

	// iterations
	for (m_nIteration = 0; fabs(v2[0] - m_vFinalParam[0]) > GetTolerance() 
			&& m_nIteration < 100; m_nIteration++)
	{
		// evaluate the second function point
		fx2 = (*m_pFunc)(v2, &vGrad2);

		// solve for the constants
		BOOL bResult = SolveConst(m_vFinalParam[0], m_finalValue, m_vGrad[0], 
			v2[0], fx2, vGrad2[0]);
		if (!bResult)
		{
			m_bSuccess = FALSE;
			return m_vFinalParam;
		}

		// perform fit
		REAL min_guess;
		bResult = FindMinimum(&min_guess);
		if (!bResult)
		{
			m_bSuccess = FALSE;
			return m_vFinalParam;
		}

		// update the first point
		m_vFinalParam = v2;
		m_vGrad = vGrad2;
		m_finalValue = fx2;

		// update the second
		v2[0] = min_guess;
	} 

	if (100 == m_nIteration)
	{
		m_bSuccess = FALSE;
		return m_vFinalParam;
	}

	// evaluate the second function point a final time
	fx2 = (*m_pFunc)(v2, &vGrad2);

	// see which answer is smaller
	if (fx2 < m_finalValue)
	{
		// swap the last two guesses
		m_vFinalParam = v2;
		m_vGrad = vGrad2;
		m_finalValue = fx2;
	}

	// return the answer
	m_bSuccess = TRUE;
	return m_vFinalParam;
}

//////////////////////////////////////////////////////////////////////
// CCubicInterpOptimizer::SolveConst
// 
// solves for cubic constants, given two sets of function evaluations
//////////////////////////////////////////////////////////////////////
BOOL CCubicInterpOptimizer::SolveConst(REAL x1, REAL fx1, REAL dx1, 
									   REAL x2, REAL fx2, REAL dx2)
{
	// solve for the cubic a, b, c, and d parameters

	// set up the system matrix
	CMatrix<4> mS;

	mS[0][0] = 3.0 * x1 * x1;
	mS[0][1] = 2.0 * x1;
	mS[0][2] = 1.0;
	mS[0][3] = 0.0;

	mS[1][0] = 3.0 * x2 * x2;
	mS[1][1] = 2.0 * x2;
	mS[1][2] = 1.0;
	mS[1][3] = 0.0;

	mS[2][0] = x1 * x1 * x1;
	mS[2][1] = x1 * x1;
	mS[2][2] = x1;
	mS[2][3] = 1.0;

	mS[3][0] = x2 * x2 * x2;
	mS[3][1] = x2 * x2;
	mS[3][2] = x2;
	mS[3][3] = 1.0;

	// trace out the matrix
	TRACE_MATRIX("mS = ", mS);

	// invert
	mS.Invert();

	// and solve
	CVector<4> v;
	v[0] = dx1;
	v[1] = dx2;
	v[2] = fx1;
	v[3] = fx2;

	// set the constant vector
	m_vK = mS * v;

	// return TRUE
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CCubicInterpOptimizer::FindMinimum
// 
// interpolates the minimum from the constants
//////////////////////////////////////////////////////////////////////
BOOL CCubicInterpOptimizer::FindMinimum(REAL *min_guess)
{
	// minimum is the root of the first derivative so that the
	//		second derivative is positive

	// find roots of first derivative
	REAL r1, r2;
	BOOL bReal = SolveQuadratic(3.0 * m_vK[0], 2.0 * m_vK[1], m_vK[2], 
		&r1, &r2);

	// find value of second derivative 
	REAL ddx1 = 6.0 * m_vK[0] * r1 + 2.0 * m_vK[1];
	if (ddx1 > 0.0)
	{
		// root1 is the minimum
		(*min_guess) = r1;

		// return TRUE;
		return TRUE;
	}

	REAL ddx2 = 6.0 * m_vK[0] * r2 + 2.0 * m_vK[1];
	if (ddx2 > 0.0)
	{
		// root2 is the minimum
		(*min_guess) = r2;

		// return TRUE;
		return TRUE;
	}

	// otherwise, no minimum
	return FALSE;
}
