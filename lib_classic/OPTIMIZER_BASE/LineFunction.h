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

	// returns the point on the line
	CVector<DIM, TYPE> GetPoint()
	{
		return m_vPoint;
	}

	// returns the direction of the line
	CVector<DIM, TYPE> GetDirection()
	{
		return m_vDirection;
	}

	// sets the line parameters
	void SetLine(const CVector<DIM, TYPE>& vPoint, 
		const CVector<DIM, TYPE>& vDir)
	{
		m_vPoint = vPoint;
		m_vDirection = vDir;
	}

	// evaluates the line function
	virtual TYPE operator()(const CVector<1, TYPE>& vInput)
	{
		return (*m_pProjFunc)(GetPoint() + vInput[0] * GetDirection());
	}

	// evaluates the gradient of the line function
	virtual CVector<1, TYPE> Grad(const CVector<1, TYPE>& vInput)
	{
		return m_pProjFunc->Grad(GetPoint() + vInput[0] * GetDirection())
			* GetDirection();
	}

private:
	// pointer to the objective function upon which this line function
	//		is based
	CObjectiveFunction<DIM, TYPE> *m_pProjFunc;

	// stores a point on the line
	CVector<DIM, TYPE> m_vPoint;

	// stores the direction of the line
	CVector<DIM, TYPE> m_vDirection;
};

#endif
