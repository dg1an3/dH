//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.h: interface for the CObjectiveFunction
//		template base class
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(OBJECTIVEFUNCTION_H)
#define OBJECTIVEFUNCTION_H

// objective functions are vector-domained functions
#include <VectorN.h>

#include <MatrixNxM.h>

//////////////////////////////////////////////////////////////////////
// class CObjectiveFunction
// 
// base class template for all objective functions.  allows the 
//		objective function to define a gradient, but a flag is provided
//		in the case that no gradient is available
//////////////////////////////////////////////////////////////////////
class CObjectiveFunction
{
public:
	// constructs an objective function; gets flag to indicate
	//		whether gradient information is available
	CObjectiveFunction(BOOL bHasGradientInfo);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const = 0;

	// function to evaluate gradiant at a point
	virtual void Gradient(const CVectorN<>& vIn, CVectorN<>& vGrad_out) const 
	{ 
		vGrad_out.SetDim(vIn.GetDim());
		operator()(vIn, &vGrad_out);
	};

	// function to evaluate hessian at a point
	virtual void Hessian(const CVectorN<>& vIn, CMatrixNxM<>& mHess_out) const;

	// whether gradient information is available
	BOOL HasGradientInfo() const;

	// function to test the gradient calculation
	REAL TestGradient(const CVectorN<>& vAtParam, double epsilon) const;

	// function to test hessian calculation
	void TestHessian(const CVectorN<>& vAtParam, double epsilon) const;

private:
	// flag to indicate that gradient information is available
	BOOL m_bHasGradientInfo;
};

#endif
