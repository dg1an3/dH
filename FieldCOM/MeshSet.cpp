// MeshSet.cpp : Implementation of CMeshSet
#include "stdafx.h"
#include "FieldCOM.h"
#include "MeshSet.h"

/////////////////////////////////////////////////////////////////////////////
// CMeshSet

STDMETHODIMP CMeshSet::get_Label(VARIANT *pVal)
{
	// assign the receiving variant
	(*pVal) = m_varLabel;

	return S_OK;
}

STDMETHODIMP CMeshSet::put_Label(VARIANT newVal)
{
	return m_varLabel.Copy(&newVal);
}

STDMETHODIMP CMeshSet::get__NewEnum(IUnknown **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::get_Count(long *pCount)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::Item(long nIndex, IDispatch **pItem)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::Union(IMesh *pPoly)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::Intersect(IMesh *pPoly)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::Diff(IMesh *pPoly)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CMeshSet::IntersectLine(IMatrix *pLine, double *pLambda)
{
	// TODO: Add your implementation code here

	return S_OK;
}
