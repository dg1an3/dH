//////////////////////////////////////////////////////////////////////
// VectorFunction.h: declarations of CVector-typed function factories,
// as well as some generic scalar functions
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORFUNCTION_H)
#define VECTORFUNCTION_H

#include <Function.h>

#include "Vector.h"

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
// declare function factories for matrix arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(operator+, CVector<4>)
// FUNCTION_FACTORY2(operator-, CVector<4>)
// FUNCTION_FACTORY2(operator*, CVector<4>)

#endif // !defined(VECTORFUNCTION_H)
