//////////////////////////////////////////////////////////////////////
// PowellOptimizer.h: interface for the CPowellOptimizer 
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(POWELLOPTIMIZER_H)
#define POWELLOPTIMIZER_H

#include <MatrixNxM.h>

// optimizer base class
#include "Optimizer.h"

// the underlying Brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"

//////////////////////////////////////////////////////////////////////
// class CPowellOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class CPowellOptimizer : public COptimizer
{
public:
	// construct a new Powell optimizer
	CPowellOptimizer(CObjectiveFunction *pFunc);
	virtual ~CPowellOptimizer();

	// performs the optimization
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

protected:
	// helper functions for optimization
	int IterateOverDirectionSet(REAL& del);
	void FindNewDirection(REAL fp, REAL fptt, REAL del, int nBigDir);

private:
	// line function that projects the objective function along a given line
	CLineFunction m_lineFunction;

	// brent optimizer along the line function
	CBrentOptimizer m_optimizeBrent;

	// the current direction set for the optimization
	CMatrixNxM<> m_mDirections;

	// "locals" for the optimization, declared members to avoid initializing
	CVectorN<> m_vScaledDir;
	CVectorN<> m_vPt;
	CVectorN<> m_vPtt;
	CVectorN<> m_vXit;
};

#endif
