// VOITerm.cpp: implementation of the CVOITerm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\include\voiterm.h"

#include <UtilMacros.h>
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
	: m_pVOI(pStructure)
		, m_nLevel(0)
		, m_histogram(NULL, NULL)
		, m_weight(weight)
		, m_pNextScale(NULL)
{

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



#define VOITERM_SCHEMA 2
	// 1 - initial
	// 2 - histogram serialization

IMPLEMENT_SERIAL(CVOITerm, CModelObject, VERSIONABLE_SCHEMA | VOITERM_SCHEMA);

void CVOITerm::Serialize(CArchive& ar)
{
	// schema for the voiterm object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : VOITERM_SCHEMA;

	// base class
	CModelObject::Serialize(ar);

	// serialize attributes
	SERIALIZE_VALUE(ar, m_weight);
	SERIALIZE_VALUE(ar, m_pVOI);

	if (nSchema >= 2)
	{
		GetHistogram()->Serialize(ar);
	}
}


CVOITerm& CVOITerm::operator=(const CVOITerm& otherTerm)
{
	SetWeight(otherTerm.GetWeight());

	return (*this);
}

///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetLevel
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

}	// CVOITerm::GetLevel


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetVOI
// 
// accessors for structure 
///////////////////////////////////////////////////////////////////////////////
CStructure *CVOITerm::GetVOI() 
{ 
	return m_pVOI; 

}	// CVOITerm::GetVOI


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetHistogram
// 
// accessors for histogram
///////////////////////////////////////////////////////////////////////////////
CHistogram *CVOITerm::GetHistogram() 
{ 
	return &m_histogram; 

}	// CVOITerm::GetHistogram


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetHistogram
// 
// accessors for histogram
///////////////////////////////////////////////////////////////////////////////
const CHistogram *CVOITerm::GetHistogram() const 
{ 
	return &m_histogram; 

}	// CVOITerm::GetHistogram


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::GetWeight
// 
// accessors for term weight
///////////////////////////////////////////////////////////////////////////////
REAL CVOITerm::GetWeight() const
{
	return m_weight;

}	// CVOITerm::GetWeight


///////////////////////////////////////////////////////////////////////////////
// CVOITerm::SetWeight
// 
// accessors for term weight
///////////////////////////////////////////////////////////////////////////////
void CVOITerm::SetWeight(REAL weight)
{
	m_weight = weight;

	if (m_pNextScale)
	{
		m_pNextScale->SetWeight(weight);
	}

}	// CVOITerm::SetWeight



///////////////////////////////////////////////////////////////////////////////
// CVOITerm::Eval
// 
// Evaluation of term over-ride for real terms
///////////////////////////////////////////////////////////////////////////////
REAL CVOITerm::Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude) 
{ 
	return 0;

}	// CVOITerm::Eval


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
