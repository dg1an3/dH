// CoordSys.cpp: implementation of the CCoordSys class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CoordSys.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCoordSys::CCoordSys()
{

}

CCoordSys::~CCoordSys()
{
	// iterate over other fields, removing the transform to this
	POSITION pos = m_mapToCS.GetStartPosition();
	while (NULL != pos)
	{
		CCoordSys *pTo = NULL;
		IField *pField = NULL;
		m_mapToCS.GetNextAssoc(pos, (void *&) pTo, (void *&) pField);

		// delete the connection to this
		// TODO: ensure that the inverse field is released
		pTo->m_mapToCS.RemoveKey((void *) this);
	}
}


STDMETHODIMP CCoordSys::GetTransformTo(CCoordSys *pTo, IField **pField)
{
	if (!m_mapToCS.Lookup((void *) pTo, (void *&) (*pField)))
	{
		// attempt to search and composite fields

		// failed, so 
		(*pField) = NULL;
		return S_OK; // S_NOTFOUND;
	}

	return S_OK;
}

STDMETHODIMP CCoordSys::SetTransformTo(CCoordSys *pTo, IField *pField)
{
	// set this field
	m_mapToCS.SetAt((void *) pTo, (void *) pField);

	// set the inverse field
	IField *pInvField = NULL;
	pField->GetInverse(&pInvField);
	pTo->m_mapToCS.SetAt((void *) this, (void *) pInvField);

	// set transform in other direction
	return S_OK;
}

STDMETHODIMP CCoordSys::GetTransformFrom(CCoordSys *pTo, IField **pField)
{
	return pTo->GetTransformTo(this, pField);
}