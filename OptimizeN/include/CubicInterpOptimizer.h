// CubicInterpOptimizer.h: interface for the CCubicInterpOptimizer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUBICINTERPOPTIMIZER_H__C66C7716_4CB4_4DF0_B2EC_3D15DFA5EEDE__INCLUDED_)
#define AFX_CUBICINTERPOPTIMIZER_H__C66C7716_4CB4_4DF0_B2EC_3D15DFA5EEDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Optimizer.h"
#include <Vector.h>

//////////////////////////////////////////////////////////////////////
// class CCubicInterpOptimizer
// 
// optimizes a function by fitting to a cubic.  only for functions
//		with gradient information
//////////////////////////////////////////////////////////////////////
class CCubicInterpOptimizer : public COptimizer  
{
public:
	// construction/destruction
	CCubicInterpOptimizer(CObjectiveFunction *pFunc);
	virtual ~CCubicInterpOptimizer();

	// optimizes the initial input vector
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit = m_vInit);

	// returns flag to indicate whether previous optimization was 
	//	successful
	BOOL IsSuccess()
	{
		return m_bSuccess;
	}

	// solves for the constants, given two points, the function value,
	//		and the derivative at the points
	BOOL SolveConst(REAL x1, REAL fx1, REAL dx1, 
		REAL x2, REAL fx2, REAL dx2);

	// finds the minimum of the cubic described by the constants
	BOOL FindMinimum(REAL *min_guess);

private:
	// holds the gradient
	CVectorN<> m_vGrad;

	// vector holding constants for cubic fit
	CVector<4> m_vK;

	// success flag
	BOOL m_bSuccess;

	// starting value for the optimizer
	static const CVectorN<> m_vInit;
};

#endif // !defined(AFX_CUBICINTERPOPTIMIZER_H__C66C7716_4CB4_4DF0_B2EC_3D15DFA5EEDE__INCLUDED_)
