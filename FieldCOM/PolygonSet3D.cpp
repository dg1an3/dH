// PolygonSet3D.cpp : Implementation of CPolygonSet3D
#include "stdafx.h"
#include "FieldCOM.h"
#include "PolygonSet3D.h"

/////////////////////////////////////////////////////////////////////////////
// CPolygonSet3D

HRESULT CPolygonSet3D::FinalConstruct()
{
	// the basis matrix
	CHECK_RESULT(CComObject<CMatrix>::CreateInstance(&m_pBasis));
	CHECK_RESULT(m_pBasis->Reshape(4, 4));
	CHECK_RESULT(m_pBasis->Identity());

	return S_OK;
}

STDMETHODIMP CPolygonSet3D::get_Label(VARIANT *pVal)
{
	// assign the receiving variant
	(*pVal) = m_varLabel;

	return S_OK;
}

STDMETHODIMP CPolygonSet3D::put_Label(VARIANT newVal)
{
	return m_varLabel.Copy(&newVal);
}

STDMETHODIMP CPolygonSet3D::get__NewEnum(IUnknown **pVal)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}

STDMETHODIMP CPolygonSet3D::get_Count(long *pCount)
{
	// check for NULL pointer
	if (pCount == NULL)
	{
		return E_POINTER;
	}
		
	(*pCount) = m_arrPolygons.size();

	return S_OK;
}

STDMETHODIMP CPolygonSet3D::Item(long nIndex, IPolygon3D **pItem)
{
	// check bounds
	if (nIndex < 0 
		|| nIndex >= m_arrPolygons.size())
	{
		return E_FAIL;
	}

	// return the polygon's interface
	return m_arrPolygons[nIndex]->QueryInterface(
		IID_IPolygon3D, (void**)pItem);
}

STDMETHODIMP CPolygonSet3D::Union(IPolygon3D *pPoly)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}

STDMETHODIMP CPolygonSet3D::Intersect(IPolygon3D *pPoly)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}

STDMETHODIMP CPolygonSet3D::Diff(IPolygon3D *pPoly)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}

STDMETHODIMP CPolygonSet3D::get_Basis(IMatrix **pVal)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}

STDMETHODIMP CPolygonSet3D::IntersectLine(IMatrix *pLine, double *pLambda)
{
	// TODO: Add your implementation code here

	return E_NOTIMPL;
}
