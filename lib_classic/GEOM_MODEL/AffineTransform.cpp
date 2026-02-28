// AffineTransform.cpp : Implementation of CAffineTransform
#include "stdafx.h"
#include "FieldCOM.h"
#include "AffineTransform.h"

/////////////////////////////////////////////////////////////////////////////
// CAffineTransform


STDMETHODIMP CAffineTransform::Identity()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::get_MatrixHG(VARIANT *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::put_MatrixHG(VARIANT newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::Translate(VARIANT varAmount)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::Rotate(double angle, VARIANT varAxis)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CAffineTransform::Scale(VARIANT varScale)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here

	return S_OK;
}
