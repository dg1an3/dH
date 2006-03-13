//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.cpp: implementation of the CObjectiveFunction 
//		base class
//
// Copyright (C) 1996-2004  Derek G. Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "ObjectiveFunction.h"


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::CObjectiveFunction
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CObjectiveFunction::CObjectiveFunction(BOOL bHasGradientInfo)
	: m_bHasGradientInfo(bHasGradientInfo)
{
}	// CObjectiveFunction::CObjectiveFunction

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::HasGradientInfo
// 
// whether gradient information is available
///////////////////////////////////////////////////////////////////////////////
BOOL CObjectiveFunction::HasGradientInfo() const
{
	return m_bHasGradientInfo;

}	// CObjectiveFunction::HasGradientInfo


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Transform
// 
// over-ride for function to transform parameters
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::Transform(CVectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// CObjectiveFunction::Transform

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::dTransform
// 
// over-ride for derivative transform parameters
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::dTransform(CVectorN<> *pvInOut) const
{
	// set to all 1.0 for identity transform
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 1.0);

}	// CObjectiveFunction::dTransform

///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::InvTransform
// 
// inverse of parameter transform
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::InvTransform(CVectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// CObjectiveFunction::InvTransform


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Gradient
// 
// approximates gradient using difference method
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::Gradient(const CVectorN<>& vIn, REAL epsilon, 
				CVectorN<>& vGrad_out) const
{
	REAL res = 0.0;
	BEGIN_LOG_SECTION(CObjectiveFunction::Gradient());

	// get epsilon
	REAL elem_max = 0.0;
	for (int nAt = 0; nAt < vIn.GetDim(); nAt++)
	{
		elem_max = __max(elem_max, fabs(vIn[nAt]));
	}
	epsilon = elem_max * epsilon;

	// numerically evaluate the gradiant
	CVectorN<> vParam = vIn;
	const REAL fp = (*this)(vParam);

	vGrad_out.SetDim(vParam.GetDim());
	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
	{
		vParam[nAt] += epsilon;

		const REAL fp_del = (*this)(vParam);
		vGrad_out[nAt] = (fp_del - fp) / epsilon;

		vParam[nAt] -= epsilon;
	}
	LOG_EXPR_EXT(vGrad_out);

	END_LOG_SECTION();	// CObjectiveFunction::Gradient

}	// CObjectiveFunction::Gradient


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Hessian
// 
// approximates hessian using difference method
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::Hessian(const CVectorN<>& vAtParam, REAL epsilon,
								CMatrixNxM<>& mHess_out) const
{
	BEGIN_LOG_SECTION(CObjectiveFunction::Hessian());

	const REAL fp = (*this)(vAtParam);

	// numerically evaluate the hessian
	CVectorN<> vParam = vAtParam;
	mHess_out.Reshape(vParam.GetDim(), vParam.GetDim());
	for (int nAtCol = 0; nAtCol < vParam.GetDim(); nAtCol++)
	{
		BEGIN_LOG_SECTION_(FMT("Col %i", nAtCol));

		vParam = vAtParam;
		vParam[nAtCol] += epsilon;
		const REAL fp_del_col = (*this)(vParam);

		for (int nAtRow = 0; nAtRow < mHess_out.GetRows(); nAtRow++)
		{
			BEGIN_LOG_SECTION_(FMT("Row %i", nAtRow));

			vParam = vAtParam;
			vParam[nAtRow] += epsilon;
			const REAL fp_del_row = (*this)(vParam);
			vParam[nAtCol] += epsilon;
			const REAL fp_del_row_del_col = (*this)(vParam);

			mHess_out[nAtCol][nAtRow] = fp_del_row_del_col - fp_del_row;
			mHess_out[nAtCol][nAtRow] -= fp_del_col - fp;
			mHess_out[nAtCol][nAtRow] /= epsilon * epsilon;
			LOG_EXPR(mHess_out[nAtCol][nAtRow]);

			END_LOG_SECTION();	// FormatString("Row %i", nAtRow);
		}
		END_LOG_SECTION();	// FormatString("Row %i", nAtCol);
	}
	LOG_EXPR_EXT(mHess_out);

	END_LOG_SECTION();	// CObjectiveFunction::Hessian

}	// CObjectiveFunction::Hessian
