// AffineTransform.cpp : Implementation of CAffineTransform
#include "stdafx.h"
#include "FieldCOM.h"
#include "AffineTransform.h"

/////////////////////////////////////////////////////////////////////////////
// CAffineTransform

// CField methods

STDMETHODIMP CAffineTransform::get_DimIn(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::put_DimIn(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::get_DimOut(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::put_DimOut(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::Evaluate(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::Jacobian(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

// CAffineTransform methods

STDMETHODIMP CAffineTransform::get_Matrix(IMatrix **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::put_Matrix(IMatrix *newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::get_IsRigid(BOOL *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

