//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "ObjectiveFunction.h"

CObjectiveFunction::CObjectiveFunction(BOOL bHasGradientInfo)
	: m_bHasGradientInfo(bHasGradientInfo)
{
}

// whether gradient information is available
BOOL CObjectiveFunction::HasGradientInfo() const
{
	return m_bHasGradientInfo;
}
