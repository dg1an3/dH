//////////////////////////////////////////////////////////////////////
// ComVariantMatrix.h: declaration and definition of the 
//		CComVariantMatrix template
//
// Copyright (C) 2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(COMVARIANTMATRIX_H)
#define COMVARIANTMATRIX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MatrixNxM.h"

//////////////////////////////////////////////////////////////////////
// class CComVariantMatrix<TYPE>
//
// represents a square matrix with GetDim()ension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CComVariantMatrix : public CMatrixNxM<TYPE>, public CComVariant
{
public:
	// construction / destruction
	CComVariantMatrix();
	virtual ~CComVariantMatrix();

	// assignment
	CMatrixNxM<TYPE>& operator=(const CMatrixBase<TYPE>& mOther);

	// helper function to retrieve one of the sizes of the safe array
	int GetSafeArraySize(int nDim);

	// attaching / detaching VARIANTs -- use default to attach to
	//		self
	STDMETHOD(Attach)( VARIANT* pSrc = NULL);
	STDMETHOD(Detach)( VARIANT* pSrc = NULL);

	// Reshape -- sets the dimension of the matrix
	STDMETHOD(Reshape)(int nCols, int nRows);

	// static to return VARTYPE for the template expansion
	static inline VARTYPE GetVarType();

};	// class CComVariantMatrix


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::CComVariantMatrix<TYPE>()
//
// default constructor -- initializes to empty variant
//////////////////////////////////////////////////////////////////
template<class TYPE>	
CComVariantMatrix<TYPE>::CComVariantMatrix<TYPE>()
{ 
}	// CComVariantMatrix<TYPE>::CComVariantMatrix<TYPE>()


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::~CComVariantMatrix<TYPE>()
//
// destructor
//////////////////////////////////////////////////////////////////
template<class TYPE>
CComVariantMatrix<TYPE>::~CComVariantMatrix<TYPE>() 
{ 
}	// CComVariantMatrix<TYPE>::~CComVariantMatrix<TYPE>()


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::operator=
//
// assignment
//////////////////////////////////////////////////////////////////
template<class TYPE>
CMatrixNxM<TYPE>& CComVariantMatrix<TYPE>::operator=(const CMatrixBase<TYPE>& mOther)
{
	return CMatrixNxM<TYPE>::operator=(mOther);

}	// CComVariantMatrix<TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::GetSafeArraySize
//
// returns size of one of the dimensions of safe array
//////////////////////////////////////////////////////////////////
template<class TYPE>
int CComVariantMatrix<TYPE>::GetSafeArraySize(int nDim)
{
	// initialize to zero to give correct default if not
	//		a valid dimension
	long nLBound = 0;
	long nUBound = 0;

	// is it a valid dimension?
	if (::SafeArrayGetDim(parray) >= nDim)
	{
		CHECK_HRESULT(::SafeArrayGetLBound(parray, nDim, &nLBound));
		CHECK_HRESULT(::SafeArrayGetUBound(parray, nDim, &nUBound));
	}

	// return offset
	return nUBound - nLBound + 1;

}	// CComVariantMatrix<TYPE>::GetSafeArraySize


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::Attach
//
// attaching to a variant -- NULL to attach to self
//////////////////////////////////////////////////////////////////
template<class TYPE>
inline STDMETHODIMP CComVariantMatrix<TYPE>::Attach( VARIANT* pSrc )
{
	// if no variant to attach to, attach to self
	if (pSrc == NULL)
	{
		pSrc = this;
	}

	// check constraints
	ATLASSERT(pSrc->vt & VT_ARRAY != 0);
	ATLASSERT(::SafeArrayGetDim(pSrc->parray) <= 2);

	// copy elements
	vt = pSrc->vt;
	parray = pSrc->parray;

	// get the array data
	TYPE *pElements = NULL;
	CHECK_HRESULT(::SafeArrayAccessData(parray, (void **) &pElements));

	// set the matrix elements
	SetElements(GetSafeArraySize(2), GetSafeArraySize(1), pElements, FALSE);

	return S_OK;

}	// CComVariantMatrix<TYPE>::Attach


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::Detach
//
// detaching to a variant -- NULL to detach to self
//////////////////////////////////////////////////////////////////
template<class TYPE>
inline HRESULT CComVariantMatrix<TYPE>::Detach( VARIANT* pSrc )
{
	// if no variant to attach to, attach to self
	if (pSrc == NULL)
	{
		pSrc = this;
	}

	// release elements
	SetElements(NULL, 0, 0, FALSE);

	// unaccess the data
	CHECK_HRESULT(::SafeArrayUnaccessData(parray));

	// if we were not detaching from self
	if (pSrc != this)
	{
		// detach contained variant
		vt = VT_EMPTY;
		parray = NULL;
	}

	return S_OK;

}	// CComVariantMatrix<TYPE>::Detach


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<TYPE>::Reshape
//
// sets the dimension of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
inline STDMETHODIMP CComVariantMatrix<TYPE>::Reshape(int nCols, int nRows)
{
	// must have some rows and cols
	ASSERT(nCols > 0 && nRows > 0);
	
	// destroy existing elements
	Clear();

	// change to array type
	vt = GetVarType() | VT_ARRAY;

	// set up the array size
	SAFEARRAYBOUND rgsabound[2];
	rgsabound[ 0 ].lLbound = 0;
	rgsabound[ 0 ].cElements = nCols;
	rgsabound[ 1 ].lLbound = 0;
	rgsabound[ 1 ].cElements = nRows;

	// create the safearray
    parray = ::SafeArrayCreate(GetVarType(), 2, rgsabound);

	return S_OK;

}	// CComVariantMatrix<TYPE>::Reshape


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<double>::GetVarType
//
// specific implementation for double
//////////////////////////////////////////////////////////////////
template<>
inline VARTYPE CComVariantMatrix<double>::GetVarType()
{
	return VT_R8;

}	// CComVariantMatrix<double>::GetVarType


//////////////////////////////////////////////////////////////////
// CComVariantMatrix<float>::GetVarType
//
// specific implementation for float
//////////////////////////////////////////////////////////////////
template<>
inline VARTYPE CComVariantMatrix<float>::GetVarType()
{
	return VT_R4;

}	// CComVariantMatrix<float>::GetVarType


#endif // !defined(COMVARIANTMATRIX_H)
