//////////////////////////////////////////////////////////////////////
// ScalarFunction.h: declarations of double-typed function factories,
// as well as some generic scalar functions
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(SCALARFUNCTION_H)
#define SCALARFUNCTION_H

#include <Function.h>

//////////////////////////////////////////////////////////////////////
// declare PI for later use
//////////////////////////////////////////////////////////////////////
#define PI (atan(1.0) * 4.0)

//////////////////////////////////////////////////////////////////////
// declare functions to represent basic real arithmetic
//////////////////////////////////////////////////////////////////////
inline double add(const double& l, const double& r) { return l + r; }
inline double sub(const double& l, const double& r) { return l - r; }
inline double mul(const double& l, const double& r) { return l * r; }
inline double div(const double& l, const double& r) { return l / r; }

//////////////////////////////////////////////////////////////////////
// declare function factories for real arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2_RENAME(operator+, add, double)
FUNCTION_FACTORY2_RENAME(operator-, sub, double)
// FUNCTION_FACTORY2_RENAME(operator*, mul, double)
FUNCTION_FACTORY2_RENAME(operator/, div, double)

//////////////////////////////////////////////////////////////////////
// declare functions to represent gaussian distributions
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Gauss(TYPE x, TYPE s)
{
	// compute the exponent
	TYPE d = (x * x) / ((TYPE) 2.0 * s * s); 

	// return the exponential
	return (TYPE) exp(-d)
		/ (TYPE) (sqrt((TYPE) 2.0 * PI * s));
}

template<class TYPE>
inline TYPE Gauss2D(TYPE x, TYPE y, TYPE sx, TYPE sy)
{
	// compute the exponent
	TYPE d = (x * x) / ((TYPE) 2.0 * sx * sx) 
		+ (y * y) / ((TYPE) 2.0 * sy * sy);

	// return the exponential
	return (TYPE) exp(-d)
		/ (TYPE) (sqrt(2.0 * PI * sx * sy));
}

#endif // !defined(SCALAR_FUNCTION)
