// VOITerm.cpp: implementation of the CVOITerm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VOITerm.h"

#include <Structure.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CVOITerm::CVOITerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVOITerm::CVOITerm(CStructure *pStructure, REAL weight)
	: m_pVOI(pStructure), 
		m_nLevel(0),
		m_histogram(NULL, NULL),
		m_weight(weight),
		m_pNextScale(NULL)
{
//	pStructure->GetRegion(0);

}	// CVOITerm::CVOITerm


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::~CVOITerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVOITerm::~CVOITerm()
{
	// The associated Prescription will take care of this
	// delete m_pNextScale;

}	// CVOITerm::~CVOITerm


IMPLEMENT_SERIAL(CVOITerm, CModelObject, 1);


CVOITerm& CVOITerm::operator=(const CVOITerm& otherTerm)
{
	SetWeight(otherTerm.GetWeight());

	return (*this);
}

///////////////////////////////////////////////////////////////////////////////
// CVOITerm::Subcopy
// 
// helper to create pyramid - constructs a copy, except with nScale + 1
///////////////////////////////////////////////////////////////////////////////
CVOITerm *CVOITerm::Subcopy(CVOITerm *pSubcopy)
{
	pSubcopy->m_nLevel = m_nLevel + 1;
	pSubcopy->m_weight = m_weight;

	ASSERT(m_histogram.GetRegion() != NULL);

	CVolume<VOXEL_REAL> convRegion;
	convRegion.ConformTo(m_histogram.GetRegion());

	CVolume<VOXEL_REAL> kernel;
	kernel.SetDimensions(5, 5, 1);
	::CalcBinomialFilter(&kernel);

	::Convolve(m_histogram.GetRegion(), &kernel, &convRegion);

	CVolume<VOXEL_REAL> *pDecRegion = new CVolume<VOXEL_REAL>();
	::Decimate(&convRegion, pDecRegion);

	pSubcopy->m_histogram.SetRegion(pDecRegion);

	// pSubcopy->m_histogram.SetRegion(m_pVOI->GetRegion(pSubcopy->m_nLevel));

	m_pNextScale = pSubcopy;
	
	return pSubcopy;

}	// CVOITerm::Subcopy


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetSubcopy
// 
// returns the subcopy at the given scale (relative to the current scale)
///////////////////////////////////////////////////////////////////////////////
CVOITerm *CVOITerm::GetLevel(int nLevel, BOOL bCreate)
{
	if (nLevel == 0)
	{
		return this;
	}
	else if (m_pNextScale)
	{
		return m_pNextScale->GetLevel(nLevel-1);
	}
	else if (bCreate)
	{
		return Subcopy()->GetLevel(nLevel-1);
	}

	return NULL;

}	// CVOITerm::GetSubcopy

void CVOITerm::SetWeight(REAL weight)
{
	m_weight = weight;
	if (m_pNextScale)
		m_pNextScale->SetWeight(weight);
}

REAL CVOITerm::GetWeight() const
{
	return m_weight;
}
