//////////////////////////////////////////////////////////////////////
// LineFunction.h: interface for the CLineFunction
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(LINEFUNCTION_H)
#define LINEFUNCTION_H

// base class
#include "ObjectiveFunction.h"

//////////////////////////////////////////////////////////////////////
// class CLineFunction
// 
// defines a line function given another objective function and
//		a line in the vector-space domain of that objective function
//////////////////////////////////////////////////////////////////////
class CLineFunction : public CObjectiveFunction
{
public:
	// construct a new line function object, given an DIM-dimensional
	//		objective function
	CLineFunction(CObjectiveFunction *pProjFunc);

	// returns the point on the line
	const CVectorN<>& GetPoint() const;

	// returns the direction of the line
	const CVectorN<>& GetDirection() const;

	// sets the line parameters
	void SetLine(const CVectorN<>& vPoint, const CVectorN<>& vDir);

	// evaluates the line function
	virtual REAL operator()(const CVectorN<>& vInput,
		CVectorN<> *pGrad = NULL);

private:
	// pointer to the objective function upon which this line function
	//		is based
	CObjectiveFunction *m_pProjFunc;

	// stores a point on the line
	CVectorN<> m_vPoint;

	// stores the direction of the line
	CVectorN<> m_vDirection;

	// temporary store of evaluation point
	CVectorN<> m_vEvalPoint;

	// stores the gradient at the last evaluated point
	CVectorN<> m_vGrad;
};

#endif
