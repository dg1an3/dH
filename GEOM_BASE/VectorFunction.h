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
// declare function factories for vector arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(operator+, CVector<4>)
// FUNCTION_FACTORY2(operator-, CVector<4>)
// FUNCTION_FACTORY2(operator*, CVector<4>)

#endif // !defined(VECTORFUNCTION_H)
