// HistogramMatcher.cpp: implementation of the CHistogramMatcher class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HistogramMatcher.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHistogramMatcher::CHistogramMatcher()
	: CObjectiveFunction(FALSE)
{

}

CHistogramMatcher::~CHistogramMatcher()
{

}


// evaluates the objective function
REAL CHistogramMatcher::operator()(const CVectorN<>& vInput, 
	CVectorN<> *pGrad)
{
	return 0.0;
}

