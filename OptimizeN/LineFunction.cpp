//////////////////////////////////////////////////////////////////////
// BrentOptimizer.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "LineFunction.h"

// construct a new line function object, given an DIM-dimensional
//		objective function
CLineFunction::CLineFunction(CObjectiveFunction *pProjFunc)
	: CObjectiveFunction(pProjFunc->HasGradientInfo()),
		m_pProjFunc(pProjFunc)
{
}

// returns the point on the line
const CVectorN<>& CLineFunction::GetPoint() const
{
	return m_vPoint;
}

// returns the direction of the line
const CVectorN<>& CLineFunction::GetDirection() const
{
	return m_vDirection;
}

// sets the line parameters
void CLineFunction::SetLine(const CVectorN<>& vPoint, const CVectorN<>& vDir)
{
	ASSERT(vPoint.GetDim() == vDir.GetDim());

	m_vPoint = vPoint;
	m_vDirection = vDir;
}

// evaluates the line function
REAL CLineFunction::operator()(const CVectorN<>& vInput, CVectorN<> *pGrad)
{
	ASSERT(vInput.GetDim() == 1);

	m_vGrad.SetDim(GetPoint().GetDim());

	REAL value = (*m_pProjFunc)(GetPoint() + vInput[0] * GetDirection(), 
			pGrad ? &m_vGrad : NULL);

	if (pGrad)
	{
		ASSERT(pGrad->GetDim() == 1);

		(*pGrad)[0] = m_vGrad * GetDirection();
	}

	return value;
}
