// BufferField.cpp : Implementation of CBufferField
#include "stdafx.h"
#include "FieldCOM.h"
#include "BufferField.h"


/////////////////////////////////////////////////////////////////////////////
// CBufferField

// CField methods

STDMETHODIMP CBufferField::get_DimIn(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_DimIn(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_DimOut(/*[out, retval]*/ long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_DimOut(/*[in]*/ long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::Evaluate(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::Jacobian(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut)
{
	// TODO: Add your implementation code here

	return S_OK;
}

// CBufferField methods

STDMETHODIMP CBufferField::get_Extent(long nDim, long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_Extent(long nDim, long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_Basis(IMatrix **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_Basis(IMatrix *newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferShort1D(short **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferShort2D(short ***pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferShort3D(short ****pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferLong1D(long **pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferLong2D(long ***pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BufferLong3D(long ****pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_Scale(double *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_Scale(double newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::get_BytesPerSample(long *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CBufferField::put_BytesPerSample(long newVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}
