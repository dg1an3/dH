//////////////////////////////////////////////////////////////////////
// LineFunction.h: interface for the CLineFunction
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(LINEFUNCTION_H)
#define LINEFUNCTION_H

#include <Vector.h>
#include <Value.h>

// base class
#include "ObjectiveFunction.h"

//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// class CLineFunction
// 
// defines a line function given another objective function and
//		a line in the vector-space domain of that objective function
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
class CLineFunction : public CObjectiveFunction<1, TYPE>
{
public:
	// construct a new line function object, given an DIM-dimensional
	//		objective function
	CLineFunction(CObjectiveFunction<DIM, TYPE> *pProjFunc)
		: CObjectiveFunction<1, TYPE>(pProjFunc->HasGradientInfo()),
			m_pProjFunc(pProjFunc)
	{
	}

	// stores a point on the line
	CValue< CVector<DIM, TYPE> > point;

	// stores the direction of the line
	CValue< CVector<DIM, TYPE> > direction;

	// evaluates the line function
	virtual TYPE operator()(const CVector<1, TYPE>& vInput)
	{
		return (*m_pProjFunc)(point.Get() + vInput[0] * direction.Get());
	}

	// evaluates the gradient of the line function
	virtual CVector<1, TYPE> Grad(const CVector<1, TYPE>& vInput)
	{
		return m_pProjFunc->Grad(point.Get() + vInput[0] * direction.Get())
			* direction.Get();
	}

private:
	// pointer to the objective function upon which this line function
	//		is based
	CObjectiveFunction<DIM, TYPE> *m_pProjFunc;
};

#endif
