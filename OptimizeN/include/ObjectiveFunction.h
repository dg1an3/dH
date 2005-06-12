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

	// whether gradient information is available
	BOOL HasGradientInfo() const;

	// transform function from linear to other parameter space
	virtual void Transform(CVectorN<> *pvInOut) const;
	virtual void dTransform(CVectorN<> *pvInOut) const;
	virtual void InvTransform(CVectorN<> *pvInOut) const;

	// function to evaluate gradiant at a point (uses difference method)
	void Gradient(const CVectorN<>& vIn, REAL epsilon, 
				CVectorN<>& vGrad_out) const;

	// function to evaluate hessian at a point (uses difference method)
	void Hessian(const CVectorN<>& vIn, REAL epsilon, 
				CMatrixNxM<>& mHess_out) const;

private:
	// flag to indicate that gradient information is available
	BOOL m_bHasGradientInfo;

};	// class CObjectiveFunction

#endif
