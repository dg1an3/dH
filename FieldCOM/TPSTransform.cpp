// TPSTransform.cpp : Implementation of CTPSTransform
#include "stdafx.h"
#include "FieldCOM.h"
#include "TPSTransform.h"

/////////////////////////////////////////////////////////////////////////////
// CTPSTransform

// CField methods

STDMETHODIMP CTPSTransform::get_DimIn(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CTPSTransform::put_DimIn(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CTPSTransform::get_DimOut(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CTPSTransform::put_DimOut(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CTPSTransform::Evaluate(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CTPSTransform::Jacobian(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

// CTPSTransform methods


STDMETHODIMP CTPSTransform::get_Landmarks(IObjects **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}
