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
// CObjectiveFunction::TestGradient
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
REAL CObjectiveFunction::TestGradient(const CVectorN<>& vAtParam, REAL epsilon) const
{
	REAL res = 0.0;
	BEGIN_LOG_SECTION(CObjectiveFunction::TestGradient());

	CVectorN<> vEvalGrad;
	vEvalGrad.SetDim(vAtParam.GetDim());
	Gradient(vAtParam, vEvalGrad);
	LOG_EXPR_EXT(vEvalGrad);

	// numerically evaluate the gradiant
	CVectorN<> vParam = vAtParam;
	const REAL fp = (*this)(vParam);

	CVectorN<> vDiffGrad;
	vDiffGrad.SetDim(vParam.GetDim());
	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
	{
		vParam[nAt] += epsilon;

		const REAL fp_del = (*this)(vParam);

		vDiffGrad[nAt] = fp_del - fp;
		vDiffGrad[nAt] /= epsilon;

		vParam[nAt] -= epsilon;
	}
	LOG_EXPR_EXT(vDiffGrad);

	res = (vEvalGrad - vDiffGrad).GetLength();
	LOG_EXPR(res);
	END_LOG_SECTION();	// CObjectiveFunction::TestGradient

	return res;

}	// CObjectiveFunction::TestGradient


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::Hessian
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::Hessian(const CVectorN<>& vAtParam, CMatrixNxM<>& mHess_out) const
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
		vParam[nAtCol] += DEFAULT_EPSILON;
		const REAL fp_del_col = (*this)(vParam);

		for (int nAtRow = 0; nAtRow < mHess_out.GetRows(); nAtRow++)
		{
			BEGIN_LOG_SECTION_(FMT("Row %i", nAtRow));

			vParam = vAtParam;
			vParam[nAtRow] += DEFAULT_EPSILON;
			const REAL fp_del_row = (*this)(vParam);
			vParam[nAtCol] += DEFAULT_EPSILON;
			const REAL fp_del_row_del_col = (*this)(vParam);

			mHess_out[nAtCol][nAtRow] = fp_del_row_del_col - fp_del_row;
			mHess_out[nAtCol][nAtRow] -= fp_del_col - fp;
			mHess_out[nAtCol][nAtRow] /= DEFAULT_EPSILON * DEFAULT_EPSILON;
			LOG_EXPR(mHess_out[nAtCol][nAtRow]);

			END_LOG_SECTION();	// FormatString("Row %i", nAtRow);
		}
		END_LOG_SECTION();	// FormatString("Row %i", nAtCol);
	}
	LOG_EXPR_EXT(mHess_out);

	END_LOG_SECTION();	// CObjectiveFunction::Hessian

}	// CObjectiveFunction::Hessian


///////////////////////////////////////////////////////////////////////////////
// CObjectiveFunction::TestHessian
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CObjectiveFunction::TestHessian(const CVectorN<>& vAtParam, REAL epsilon) const
{
	BEGIN_LOG_SECTION(CObjectiveFunction::TestHessian());

	CMatrixNxM<> mEvalHess;
	Hessian(vAtParam, mEvalHess);
	LOG_EXPR_EXT(mEvalHess);

	const REAL fp = (*this)(vAtParam);

	// numerically evaluate the hessian
	CVectorN<> vParam = vAtParam;

	CMatrixNxM<> vDiffHess;
	vDiffHess.Reshape(vParam.GetDim(), vParam.GetDim());
	for (int nAtCol = 0; nAtCol < vParam.GetDim(); nAtCol++)
	{
		BEGIN_LOG_SECTION_(FMT("Col %i", nAtCol));

		vParam = vAtParam;
		vParam[nAtCol] += epsilon;
		const REAL fp_del_col = (*this)(vParam);

		for (int nAtRow = 0; nAtRow < vDiffHess.GetRows(); nAtRow++)
		{
			BEGIN_LOG_SECTION_(FMT("Row %i", nAtRow));

			vParam = vAtParam;
			vParam[nAtRow] += epsilon;
			const REAL fp_del_row = (*this)(vParam);
			vParam[nAtCol] += epsilon;
			const REAL fp_del_row_del_col = (*this)(vParam);

			vDiffHess[nAtCol][nAtRow] = fp_del_row_del_col - fp_del_row;
			vDiffHess[nAtCol][nAtRow] -= fp_del_col - fp;
			vDiffHess[nAtCol][nAtRow] /= epsilon * epsilon;
			LOG_EXPR(vDiffHess[nAtCol][nAtRow]);

			END_LOG_SECTION();	// FormatString("Row %i", nAtRow);
		}
		END_LOG_SECTION();	// FormatString("Row %i", nAtCol);
	}
	LOG_EXPR_EXT(vDiffHess);

	END_LOG_SECTION();	// CObjectiveFunction::TestHessian

}	// CObjectiveFunction::TestHessian
