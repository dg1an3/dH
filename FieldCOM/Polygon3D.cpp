// Polygon.cpp : Implementation of CPolygon

#include "stdafx.h"
#include "FieldCOM.h"

#include "resource.h"       // main symbols

#include "Polygon3D.h"

/////////////////////////////////////////////////////////////////////////////
// CPolygon

HRESULT CPolygon3D::FinalConstruct()
{
	// create and initialize the basis matrix
	CHECK_RESULT(m_pBasis.CoCreateInstance(CLSID_Matrix));
	CHECK_RESULT(m_pBasis->Reshape(4, 4));
	CHECK_RESULT(m_pBasis->Identity());

	// create and initialize the matrix of vertices
	CHECK_RESULT(m_pVertices.CoCreateInstance(CLSID_Matrix));
	CHECK_RESULT(m_pVertices->Reshape(0, 2));

	// get the property notify sink interface for this object
	CComQIPtr<IPropertyNotifySink, &IID_IPropertyNotifySink> pSink(this);

	// holds the cookire returned by the Advise call
	DWORD dwCookie;

	// advise on property notify for vertices
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> 
		pVerticesCPC(m_pVertices);
	CHECK_RESULT(pVerticesCPC.Advise(pSink, IID_IPropertyNotifySink, &dwCookie));

	// advise on property notify for basis
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> 
		pBasisCPC(m_pBasis);
	CHECK_RESULT(pBasisCPC.Advise(pSink, IID_IPropertyNotifySink, &dwCookie));

	return S_OK;
}

STDMETHODIMP CPolygon3D::get_Label(VARIANT *pVal)
{
	// assign the receiving variant
	(*pVal) = m_varLabel;

	return S_OK;
}

STDMETHODIMP CPolygon3D::put_Label(VARIANT newVal)
{
	return m_varLabel.Copy(&newVal);
}

STDMETHODIMP CPolygon3D::get_Vertices(IMatrix **pVertices)
{
	// query for the IMatrix interface
	return m_pVertices->QueryInterface(IID_IMatrix, (void **) pVertices);
}

STDMETHODIMP CPolygon3D::put_Vertices(IMatrix *pVertices)
{
	// add a reference during this call
	pVertices->AddRef();

	// check the matrix size
	long nRows;
	CHECK_RESULT(pVertices->get_Rows(&nRows));
	CHECK_CONDITION(2 == nRows);

	// copy the vertices matrix
	m_pVertices->Copy(pVertices);

	// release the reference
	pVertices->Release();

	return S_OK;
}

STDMETHODIMP CPolygon3D::get_Basis(IMatrix **pBasis)
{
	return m_pBasis->QueryInterface(IID_IMatrix, (void **) &pBasis);
}

STDMETHODIMP CPolygon3D::put_Basis(IMatrix *pBasis)
{
	// add a reference during this call
	pBasis->AddRef();

	// check the matrix size
	long nCols, nRows;
	CHECK_RESULT(pBasis->get_Rows(&nRows));
	CHECK_CONDITION(4 == nRows);
	CHECK_RESULT(pBasis->get_Cols(&nCols));
	CHECK_CONDITION(4 == nCols);

	// copy the basis matrix
	m_pBasis->Copy(pBasis);

	// release the reference
	pBasis->Release();

	return S_OK;
}

STDMETHODIMP CPolygon3D::SignedArea(double *pVal)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::IntersectLine(IMatrix *pLine, double *pLambda)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::AddVertex(double x, double y)
{
	// allow the vertex matrix shape to change
	Lock();
	m_bShapeLocked = FALSE;

	// reshape the matrix
	long nCols;
	CHECK_RESULT(m_pVertices->get_Cols(&nCols));
	CHECK_RESULT(m_pVertices->Reshape(nCols + 1, 2));

	// lock the matrix shape again
	m_bShapeLocked = FALSE;
	Unlock();

	// set the new element value
	CHECK_RESULT(m_pVertices->put_Element(nCols, 0, x));
	CHECK_RESULT(m_pVertices->put_Element(nCols, 1, y));

	return S_OK;
}

STDMETHODIMP CPolygon3D::get_DimIn(LONG * pVal)
{
	if (pVal == NULL)
	{
		return E_POINTER;
	}
	
	// two dimensions in for a Polygon3D
	(*pVal) = 2;

	return S_OK;
}

STDMETHODIMP CPolygon3D::put_DimIn(LONG pVal)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::get_DimOut(LONG * pVal)
{
	if (pVal == NULL)
	{
		return E_POINTER;
	}
	
	// one dimension out
	(*pVal) = 1;

	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::put_DimOut(LONG pVal)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::Eval(IMatrix * pIn, IMatrix * * pOut)
{
	if (pOut == NULL)
	{
		return E_POINTER;
	}

	// evaluate if the points are in the polygon
		
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::EvalJac(IMatrix * pIn, IMatrix * * pOut, IMatrix * * pJac)
{
	// no jacobian implemented for polygon			
	return E_NOTIMPL;
}

STDMETHODIMP CPolygon3D::OnChanged(DISPID dispID)
{
	return S_OK;
}

STDMETHODIMP CPolygon3D::OnRequestEdit(DISPID dispID)
{
	// see if an attempt is made to change the matrix rows
	if (DISPID_MATRIX_ROWS == dispID
		|| DISPID_MATRIX_COLS == dispID)
	{
		return m_bShapeLocked ? S_FALSE : S_OK;
	}

	return S_OK;
}
