// Matrix.cpp : Implementation of CMatrix
#include "stdafx.h"
#include "FieldCOM.h"

#include "Matrix.h"

#include <MatrixWrapper.h>

// static to hold the initial size of the array
const long CMatrix::INIT_SIZE[2] = { 4, 4 };


///////////////////////////////////////////////////////////////////////////////
// CMatrix


///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Label(VARIANT *pVal)
{
	// assign the name
	pVal = &m_varLabel;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::put_Label
//
// sets the label for the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::put_Label(VARIANT newVal)
{
	// assign the name
	m_varLabel = newVal;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Rows
//
// returns the number of rows in the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Rows(long *pVal)
{
	// return the row count
	(*pVal) = m_Matrix.GetRows();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::put_Rows
//
// sets the number of rows in the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::put_Rows(long newVal)
{
	// reshape the matrix
	return Reshape(m_Matrix.GetCols(), newVal);
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Cols
//
// returns the number of columns in the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Cols(long *pVal)
{
	// default value
	(*pVal) = m_Matrix.GetCols();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::put_Cols
//
// sets the number of columns in the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::put_Cols(long newVal)
{
	// reshape the matrix
	return Reshape(newVal, m_Matrix.GetRows());
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::Reshape
//
// reshapes the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Reshape(long nCols, long nRows)
{
	// see if we can reshape
	CHECK_CONDITION(m_Matrix.GetCols() == nCols 
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_COLS));
	CHECK_CONDITION(m_Matrix.GetRows() == nRows 
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_ROWS));

	// reshape the matrix
	m_Matrix.Reshape(nCols, nRows);

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Element(long nCol, long nRow, double *pVal)
{
	// return the element
	(*pVal) = m_Matrix[(int) nCol][(int) nRow];

	// set OK
	return S_OK; 
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::put_Element(long nCol, long nRow, double newVal)
{
	// return the element
	m_Matrix[(int) nCol][(int) nRow] = newVal;

	// set OK
	return S_OK; 
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Elements
//
// safe access to the matrix elements
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Elements(VARIANT *pElements)
{
	// initialize the variant
	VariantInit(pElements);

	// set the variant type
	pElements->vt = VT_ARRAY | VT_R8;

	SAFEARRAYBOUND bounds[] = 
	{
		{ m_Matrix.GetCols(), 0 },
		{ m_Matrix.GetRows(), 0 }
	};
	pElements->parray = ::SafeArrayCreate(VT_R8, 2, bounds);

	if (m_Matrix.GetCols() * m_Matrix.GetRows() > 0)
	{
		double *pNewElements = NULL;
		CHECK_RESULT(::SafeArrayAccessData(pElements->parray, (void **) &pNewElements));

		memcpy(pNewElements, &m_Matrix[0][0], 
			m_Matrix.GetRows() * m_Matrix.GetCols() * sizeof(double));
	}

	// set the pointer to the elements
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// function CMatrix::put_Elements
//
// safe access to the matrix elements
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::put_Elements(VARIANT elements)
{
	// check the variant type
	ATLASSERT((VT_ARRAY | VT_R8) == elements.vt);

	// initialize the safe array wrapper class
	CSafeArray<double> safeElements(elements);

	CHECK_RESULT(Reshape(safeElements.GetSize(0), safeElements.GetSize(1)));

	// copy the elements
	memcpy(&m_Matrix[0][0], safeElements.GetElements(), 
		m_Matrix.GetRows() * m_Matrix.GetCols() * sizeof(double));

	// set the pointer to the elements
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// function CMatrix::AccessElements
//
// accesses the matrix elements
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::AccessElements(double **pElements)
{
	if (m_Matrix.GetCols() == 0)
	{
		return S_FALSE;
	}

	(*pElements) = &m_Matrix[0][0];

	// lock the object for thread safety
	Lock();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::UnaccessElements
//
// unaccesses the matrix elements
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::UnaccessElements(BOOL bChanged)
{
	// lock the object for thread safety
	Unlock();

	if (bChanged)
	{
		// fire a change
		FireOnChanged(DISPID_MATRIX_ELEMENTS);
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Identity()
{
	m_Matrix.SetIdentity();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::IsApproxEqual(IMatrix *pMatrix, double epsilon, BOOL *pVal)
{
	CMatrixWrapper m(pMatrix);

	// not approximately equal
	(*pVal) = m_Matrix.IsApproxEqual(m, epsilon);

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::Copy
//
// copies the given matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Copy(IMatrix *pMatrix)
{
	// check for the same matrix
	IUnknown *pThisUnknown;
	CHECK_RESULT(QueryInterface(IID_IUnknown, (void **) &pThisUnknown));
	if (pThisUnknown == (IUnknown *) pMatrix)
	{
		return S_OK;
	}
	pThisUnknown->Release();

	CMatrixWrapper m(pMatrix);

	// see if we can reshape
	CHECK_CONDITION(m_Matrix.GetCols() == m.GetCols()
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_COLS));
	CHECK_CONDITION(m_Matrix.GetRows() == m.GetRows() 
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_ROWS));

	m_Matrix = m;

	// return OK
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Det
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_Det(double *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::Add
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Add(IMatrix *pMatrix)
{
	CMatrixWrapper m(pMatrix);

	m_Matrix += m;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Sub(IMatrix *pMatrix)
{
	CMatrixWrapper m(pMatrix);

	m_Matrix -= m;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Mult(IMatrix *pLeft, IMatrix *pRight)
{
	CMatrixWrapper mLeft(pLeft);
	CMatrixWrapper mRight(pRight);

	// see if we can reshape
	CHECK_CONDITION(m_Matrix.GetCols() == mRight.GetCols()
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_COLS));
	CHECK_CONDITION(m_Matrix.GetRows() == mLeft.GetRows() 
		|| S_OK == FireOnRequestEdit(DISPID_MATRIX_ROWS));

	m_Matrix = mLeft * mRight;

	// TODO: fire change notifications

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::MultScalar
//
// multiplies the matrix by a scalar
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::MultScalar(double scalar)
{
	m_Matrix *= scalar;

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::Transpose
//
// sets to the transpose of the given matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Transpose()
{
	m_Matrix.Transpose();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Orthogonalize()
{
	m_Matrix.Orthogonalize();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_IsOrthogonal(BOOL *pVal)
{
	(*pVal) = m_Matrix.IsOrthogonal();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_Name
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Invert()
{
	m_Matrix.Invert();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::Dot
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::Dot(IMatrix *pMatrix, double *pVal)
{
	// TODO: Add your implementation code here

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// function CMatrix::get_IsReshapeable
//
// returns the name of the matrix
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CMatrix::get_DimIn(long *pVal)
{
	return get_Cols(pVal);
}

STDMETHODIMP CMatrix::put_DimIn(long newVal)
{
	long nRows;
	CHECK_RESULT(get_Rows(&nRows));

	return Reshape(newVal, nRows);
}

STDMETHODIMP CMatrix::get_DimOut(long *pVal)
{
	return get_Rows(pVal);
}

STDMETHODIMP CMatrix::put_DimOut(long newVal)
{
	long nCols;
	CHECK_RESULT(get_Cols(&nCols));

	return Reshape(nCols, newVal);
}

STDMETHODIMP CMatrix::Eval(IMatrix *pIn, IMatrix **pOut)
{
	// if we need to create the result matrix,
	if (NULL == (*pOut))
	{
		// create one
		CComObject<CMatrix> *pProduct;
		CHECK_RESULT(CComObject<CMatrix>::CreateInstance(&pProduct));

		// assign the interface
		CHECK_RESULT(pProduct->QueryInterface(IID_IMatrix, (void **)(*pOut)));
	}

	// multiply
	return (*pOut)->Mult(this, pIn);
}

STDMETHODIMP CMatrix::EvalJac(IMatrix *pIn, IMatrix **pOut, IMatrix **pJac)
{
	// if we need to create the result matrix,
	if (NULL == (*pOut))
	{
		// create one
		CComObject<CMatrix> *pProduct;
		CHECK_RESULT(CComObject<CMatrix>::CreateInstance(&pProduct));

		// assign the interface
		CHECK_RESULT(pProduct->QueryInterface(IID_IMatrix, (void **)(*pOut)));
	}

	// assign the jacobian to this matrix
	CHECK_RESULT(QueryInterface(IID_IMatrix, (void **)(*pJac)));

	// multiply
	return (*pOut)->Mult(this, pIn);
}
