// Function.cpp: implementation of the CFunction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Function.h"

#include "Matrix.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix<4> CreateXRot(const double& angle)
{
	return CMatrix<4>();
}

CMatrix<4> CreateRot(const double& angle, const CVector<3>& vAxis)
{
	return CMatrix<4>();
}

double sine(const double& value)
{
	return (double) sin(value);
}

CValue< double > inValue;

CValue< CVector<3> > rotAxis;

CFunction1< double, double > sineFunc(&sine, &inValue);

CFunction1< CMatrix<4>, double > rotXFunc(&CreateXRot, &inValue);

CFunction2< CMatrix<4>, double, CVector<3> > rotFunc(&CreateRot, &inValue, &rotAxis);
