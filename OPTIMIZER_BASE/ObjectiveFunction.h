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

#include <Vector.h>
#include <Value.h>

//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class CObjectiveFunction
// 
// base class template for all objective functions.  allows the 
//		objective function to define a gradient, but a flag is provided
//		in the case that no gradient is available
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
class CObjectiveFunction
{
public:
	// constructs an objective function; gets flag to indicate
	//		whether gradient information is available
	CObjectiveFunction(BOOL bHasGradientInfo)
		: m_bHasGradientInfo(bHasGradientInfo)
	{
	}

	// evaluates the objective function
	virtual TYPE operator()(const CVector<DIM, TYPE>& vInput) = 0;

	// whether gradient information is available
	BOOL HasGradientInfo()
	{
		return m_bHasGradientInfo;
	}

	// evaluates the gradient of the objective function
	virtual CVector<DIM, TYPE> Grad(const CVector<DIM, TYPE>& vInput)
	{
		return CVector<DIM, TYPE>();
	}

private:
	// flag to indicate that gradient information is available
	BOOL m_bHasGradientInfo;
};

#endif
