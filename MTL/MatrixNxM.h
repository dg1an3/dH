//////////////////////////////////////////////////////////////////////
// MatrixNxM.h: declaration and definition of the CMatrixNxM template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXNXM_H)
#define MATRIXNXM_H

#include "MathUtil.h"

#include "VectorN.h"

#include "MatrixOps.h"

// constant for pseudo-inverse
const REAL PSEUDO_EPS = (REAL) 1e-8;

//////////////////////////////////////////////////////////////////////
// class CMatrixNxM<TYPE>
//
// represents a non-square matrix with type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class CMatrixNxM
{
	// counts number of columns
	int m_nColumns;

	// the row vectors of the matrix
	CVectorN<TYPE> *m_pColumns;

	// the elements of the matrix
	TYPE *m_pElements;

	// flag to indicate whether elements should be freed
	BOOL m_bFreeElements;

public:
	// constructors / destructor
	CMatrixNxM();
	explicit CMatrixNxM(int nCols, int nRows);
	CMatrixNxM(const CMatrixNxM& fromMatrix);
	~CMatrixNxM();

	// assignment operator
	CMatrixNxM& operator=(const CMatrixNxM<TYPE>& fromMatrix);

	// SetIdentity -- sets the matrix to an identity matrix
	void SetIdentity();

	typedef TYPE ELEM_TYPE;
	typedef CVectorN<TYPE> COL_TYPE;

	// element access
	COL_TYPE& operator[](int nAtCol);
	const COL_TYPE& operator[](int nAtCol) const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// matrix size
	int GetCols() const;
	int GetRows() const;

	// sets the dimension of the matrix -- optional preserves
	//		elements
	void Reshape(int nCols, int nRows, BOOL bPreserve = TRUE);

	// row-vector access
	void GetRow(int nAtRow, CVectorN<TYPE>& vRow) const;
	void SetRow(int nAtRow, const CVectorN<TYPE>& vRow);

	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	BOOL IsApproxEqual(const CMatrixNxM& m, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place operators
	CMatrixNxM& operator+=(const CMatrixNxM& mRight);
	CMatrixNxM& operator-=(const CMatrixNxM& mRight);
	CMatrixNxM& operator*=(const TYPE& scale);
	CMatrixNxM& operator*=(const CMatrixNxM& mRight);

	// Transpose -- transposes elements of the matrix
	void Transpose();

	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	BOOL Invert(BOOL bFullPivot = FALSE);

	//////////////////////////////////////////////////////////////////
	// Pseudoinvert -- in-place Moore-Penrose pseudoinversion
	//////////////////////////////////////////////////////////////////
//	BOOL Pseudoinvert();

protected:
	// SetElements -- sets the elements to an external pointer
	void SetElements(int nCols, int nRows, TYPE *pElements, BOOL bFreeElements);

};	// class CMatrixNxM


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>
//
// default constructor -- initializes to 0x0 matrix
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::CMatrixNxM()
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(int nCols, int nRows)
//
// constructs a specific-dimensioned matrix
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::CMatrixNxM(int nCols, int nRows)
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	Reshape(nCols, nRows);

}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(int nCols, int nRows)


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(
//		const CMatrixNxM<TYPE>& fromMatrix)
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::CMatrixNxM(const CMatrixNxM<TYPE>& fromMatrix)
	: m_nColumns(0),
		m_pColumns(NULL),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// sets the dimensions
	Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

	(*this) = fromMatrix;

}	// CMatrixNxM<TYPE>::CMatrixNxM<TYPE>(
	//		const CMatrixNxM<TYPE>& fromMatrix)


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::~CMatrixNxM<TYPE>
//
// destructor
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::~CMatrixNxM()
{
	// frees any elements, if needed
	SetElements(0, 0, NULL, TRUE);

}	// CMatrixNxM<TYPE>::~CMatrixNxM<TYPE>




//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator=(const CMatrixNxM<TYPE>& fromMatrix)
{
	// checks the dimensions
	ASSERT(GetCols() == fromMatrix.GetCols());
	ASSERT(GetRows() == fromMatrix.GetRows());

	// assign values
	AssignValues(&(*this)[0][0], &fromMatrix[0][0], GetCols() * GetRows());

	return (*this);

}	// CMatrixNxM<TYPE>::operator=

//////////////////////////////////////////////////////////////////
// CMatrixD<TYPE>::SetIdentity
//
// sets the matrix to an identity matrix
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
void CMatrixNxM<TYPE>::SetIdentity()
{
	memset(m_pElements, 0, GetRows() * GetCols() * sizeof(TYPE));

	// for each element in the matrix,
	for (int nAt = 0; nAt < __min(GetCols(), GetRows()); nAt++)
	{
		(*this)[nAt][nAt] = TYPE(1.0);
	}

}	// CMatrixD<TYPE>::SetIdentity



//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Reshape
//
// sets the dimension of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixNxM<TYPE>::Reshape(int nCols, int nRows, BOOL bPreserve)
{
	// check if we need to reshape
	if (GetRows() == nRows 
		&& GetCols() == nCols)
	{
		// no reshape, but if we aren't preserving elements
		if (!bPreserve)
		{
			// then set to identity
			SetIdentity();
		}

		return;
	}

	// preserve existing elements
	int nOldRows = GetRows();
	int nOldCols = GetCols();

	// store pointer to old elements, if we are preserving
	if (!bPreserve)
	{
		FreeValues(m_pElements);
	}
	TYPE *pOldElements = m_pElements;

	// allocate and set the new elements, but do not free the old
	TYPE *pNewElements = NULL;
	AllocValues(nCols * nRows, pNewElements);

	// don't free the existing elements, as they will be copied
	m_bFreeElements = FALSE;

	// set the new elements
	SetElements(nCols, nRows, pNewElements, TRUE);

	// if there were old elements, 
	if (pOldElements)
	{
		// set the new elements to 0 initially
		memset(pNewElements, 0, sizeof(TYPE) * nCols * nRows);

		// create a temporary matrix to hold the old elements
		CMatrixNxM<TYPE> mTemp;
		mTemp.SetElements(nOldCols, nOldRows, pOldElements, TRUE);

		// and assign
		for (int nAtCol = 0; nAtCol < __min(GetCols(), mTemp.GetCols()); nAtCol++)
		{
			for (int nAtRow = 0; nAtRow < __min(GetRows(), mTemp.GetRows()); nAtRow++)
			{
				(*this)[nAtCol][nAtRow] = mTemp[nAtCol][nAtRow];
			}
		}
	}
	else
	{
		// populate as an identity matrix
		SetIdentity();
	}

}	// CMatrixNxM<TYPE>::Reshape



//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator[]
//
// retrieves a reference to a column vector
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>& CMatrixNxM<TYPE>::operator[](int nAtCol)
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixNxM<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator[]
//
// retrieves a reference to a column vector
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
const CVectorN<TYPE>& CMatrixNxM<TYPE>::operator[](int nAtCol) const
{
	// bounds check on the index
	ASSERT(nAtCol >= 0 && nAtCol < GetCols());

	// return a reference to the column vector
	return m_pColumns[nAtCol];

}	// CMatrixNxM<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::GetCols
//
// returns the number of columns of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
int CMatrixNxM<TYPE>::GetCols() const
{
	return m_nColumns;

}	// CMatrixNxM<TYPE>::GetCols


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::GetRows
//
// returns the number of rows of the matrix
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
int CMatrixNxM<TYPE>::GetRows() const
{
	if (m_pColumns)
	{
		return m_pColumns->GetDim();
	}

	return 0;

}	// CMatrixNxM<TYPE>::GetRows


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator TYPE *
//
// TYPE * conversion -- returns a pointer to the first element
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::operator TYPE *()
{
	return &m_pElements[0];

}	// CMatrixNxM<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator const TYPE *
//
// const TYPE * conversion -- returns a pointer to the first 
//		element.
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>::operator const TYPE *() const
{
	return &m_pElements[0];

}	// CMatrixNxM<TYPE>::operator const TYPE *



//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::GetRow
//
// constructs and returns a row vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixNxM<TYPE>::GetRow(int nAtRow, CVectorN<TYPE>& vRow) const
{
	// make the row vector the same size
	ASSERT(vRow.GetDim() == GetCols());

	// populate the row vector
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		vRow[nAtCol] = (*this)[nAtCol][nAtRow];
	}

}	// CMatrixNxM<TYPE>::GetRow


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::SetRow
// 
// sets the rows vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixNxM<TYPE>::SetRow(int nAtRow, const CVectorN<TYPE>& vRow)
{
	if (vRow.GetDim() == GetCols())
	{
		// de-populate the row vector
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			(*this)[nAtCol][nAtRow] = vRow[nAtCol];
		}
	}

}	// CMatrixNxM<TYPE>::SetRow




//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::IsApproxEqual
//
// tests for approximate equality using the EPS 
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
BOOL CMatrixNxM<TYPE>::IsApproxEqual(const CMatrixNxM& m, TYPE epsilon) const
{
	ASSERT(GetCols() == m.GetCols());

	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		if (!(*this)[nAtCol].IsApproxEqual(m[nAtCol], epsilon))
		{
			return FALSE;
		}
	}

	return TRUE;

}	// CMatrixNxM<TYPE>::IsApproxEqual



//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator+=
//
// in-place matrix addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator+=(const CMatrixNxM<TYPE>& mRight)
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	SumValues(&(*this)[0][0], &mRight[0][0], GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator-=
//
// in-place matrix subtraction; returns a reference to 	this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator-=(const CMatrixNxM<TYPE>& mRight)
{
	ASSERT(GetCols() == mRight.GetCols());
	ASSERT(GetRows() == mRight.GetRows());

	DiffValues(&(*this)[0][0], &mRight[0][0], GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator*=
//
// in-place matrix multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator*=(const CMatrixNxM<TYPE>& mRight)
{
	CMatrixNxM<TYPE> mProduct = (*this) * mRight;

	// and assign
	Reshape(mProduct.GetCols(), mProduct.GetRows());
	(*this) = mProduct;

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::operator*=
//
// in-place matrix multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE>& CMatrixNxM<TYPE>::operator*=(const TYPE& scale)
{
	ScaleValues(&(*this)[0][0], scale, GetCols() * GetRows());

	// return a reference to this
	return (*this);

}	// CMatrixNxM<TYPE>::operator*=


/*
//////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Pseudoinvert
// 
// in-place Moore-Penrose pseudoinversion
//////////////////////////////////////////////////////////////////
template<class TYPE>
BOOL CMatrixNxM<TYPE>::Pseudoinvert()
{
	CVectorN<TYPE> w(GetCols());
	CMatrixNxM<TYPE> v(GetCols(), GetCols());
	if (!SVD(w, v))
	{
		return FALSE;
	}

	// using the formula (A+)^T = U * {1/w} * V^T

	// form the S matrix (S^T * S)^-1 * S^T
	CMatrixNxM<TYPE> s(GetCols(), GetCols());
	for (int nAt = 0; nAt < GetCols(); nAt++)
	{
		s[nAt][nAt] = (w[nAt] > PSEUDO_EPS) ? 1.0 / w[nAt] : 0.0;
	}
	(*this) *= s;

	// multiply by V^T
	v.Transpose();
	(*this) *= v;

	// and transpose the result to finish with A+
	Transpose();

	return TRUE;

}	// CMatrixNxM<TYPE>::Pseudoinvert
*/

#ifdef USE_IPP
//////////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
#define DECLARE_MATRIXN_TRANSPOSE(TYPE, TYPE_IPP) \
template<> __forceinline									\
void CMatrixNxM<TYPE>::Transpose()							\
{															\
	TYPE *pElements = NULL;									\
	AllocValues(GetRows() * GetCols(), pElements);			\
	ippmTranspose_m_##TYPE_IPP(&(*this)[0][0],				\
		GetCols() * sizeof(TYPE),							\
		GetCols(), GetRows(),								\
		pElements, GetCols() * sizeof(TYPE));				\
	SetElements(GetRows(), GetCols(), pElements, TRUE);		\
}	// CMatrixNxM<TYPE>::Transpose

// declare specific class functions
DECLARE_MATRIXN_TRANSPOSE(float, 32f);
DECLARE_MATRIXN_TRANSPOSE(double, 64f);

#else
//////////////////////////////////////////////////////////////////////
// CMatrixD<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
void CMatrixNxM<TYPE>::Transpose()
{
	// allocate the elements of the transposed matrix
	TYPE *pElements = NULL;
	AllocValues(GetRows() * GetCols(), pElements);

	// make the transposed matrix
	CMatrixNxM<TYPE> mTranspose;
	mTranspose.SetElements(GetRows(), GetCols(), pElements, FALSE);
	
	for (int nCol = 0; nCol < GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			mTranspose[nRow][nCol] = (*this)[nCol][nRow];
		}
	}

	// set the elements of this to the transposed elements
	SetElements(GetRows(), GetCols(), pElements, TRUE);

}	// CMatrixD<TYPE>::Transpose
#endif


#ifdef USE_IPP
//////////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Invert
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
#define DECLARE_MATRIXN_INVERT(TYPE, TYPE_IPP) \
	template<> __forceinline									\
	BOOL CMatrixNxM<TYPE>::Invert(BOOL bFlag)					\
	{															\
		ASSERT(GetRows() == GetCols());							\
		TYPE *pElements = NULL;									\
		AllocValues(GetRows() * GetCols(), pElements);			\
		static TYPE *arrBuffer = NULL;							\
		static int nLength = 0;									\
		if (nLength < GetRows() * GetCols())					\
		{														\
			FreeValues(arrBuffer);								\
			nLength = GetRows() * GetCols();					\
			/* TODO: fix this memory leak */					\
			AllocValues(2 * nLength, arrBuffer);				\
		}														\
		IppStatus stat = ippmInvert_m_##TYPE_IPP(&(*this)[0][0],	\
			GetCols() * sizeof(TYPE),							\
			GetCols(), 											\
			arrBuffer,											\
			pElements, GetCols() * sizeof(TYPE));				\
		if (stat == ippStsOk)									\
		{														\
			SetElements(GetCols(), GetRows(), pElements, TRUE);	\
		}														\
		return (stat == ippStsOk);								\
	}	// CMatrixNxM<TYPE>::Invert

// declare specific class functions
DECLARE_MATRIXN_INVERT(float, 32f);
DECLARE_MATRIXN_INVERT(double, 64f);
#endif

//////////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::SetElements
//
// sets the elements of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixNxM<TYPE>::SetElements(int nCols, int nRows, 
									TYPE *pElements, BOOL bFreeElements)
{
	// delete previous data
	if (m_bFreeElements && NULL != m_pElements)
	{
		FreeValues(m_pElements);
	}

	if (nCols != m_nColumns 
		&& NULL != m_pColumns)
	{
		delete [] m_pColumns;
		m_pColumns = NULL;
	}

	m_nColumns = nCols;
	// m_nRows = nRows;

	m_pElements = pElements;

	// set up the column vectors
	if (0 != m_nColumns)
	{
		if (m_pColumns == NULL)
		{
			// allocate column vectors
			m_pColumns = new COL_TYPE[GetCols()];
		}

		// initialize the column vectors and the pointers
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			// initialize the column vector
			m_pColumns[nAt].SetElements(nRows, &m_pElements[nAt * GetRows()],
				FALSE);
		}
	}

	m_bFreeElements = bFreeElements;

}	// CMatrixNxM<TYPE>::SetElements




//////////////////////////////////////////////////////////////////////
// operator==(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// exact matrix equality
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
bool operator==(const CMatrixNxM<TYPE>& mLeft, 
					   const CMatrixNxM<TYPE>& mRight)
{
	// element-by-element comparison
	for (int nCol = 0; nCol < mLeft.GetCols(); nCol++)
	{
		for (int nRow = 0; nRow < mLeft.GetRows(); nRow++)
		{
			if (mLeft[nCol][nRow] != mRight[nCol][nRow])
			{
				return false;
			}
		}
	}

	return true;

}	// operator==(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator!=(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// exact matrix inequality
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
bool operator!=(const CMatrixNxM<TYPE>& mLeft, 
					   const CMatrixNxM<TYPE>& mRight)
{
	return !(mLeft == mRight);

}	// operator!=(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// operator+(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix addition
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE> operator+(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	// create the product
	CMatrixNxM<TYPE> mSum(mLeft);
	mSum += mRight;

	// return the product
	return mSum;

}	// operator+(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator-(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix subtraction
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE> operator-(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	// create the difference
	CMatrixNxM<TYPE> mDiff(mLeft);
	mDiff -= mRight;

	// return the difference
	return mDiff;

}	// operator-(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, double scale)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE> operator*(const CMatrixNxM<TYPE>& mat,
									double scale)
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, double scale)


//////////////////////////////////////////////////////////////////////
// operator*(double scale, const CMatrixNxM<TYPE>&)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE> operator*(double scale, 
								   const CMatrixNxM<TYPE>& mat)
{
	// stored the product
	CMatrixNxM<TYPE> mProduct(mat);
	mProduct *= scale;

	// return product
	return mProduct;

}	// operator*(double scale, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, const CVectorN<TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE> operator*(const CMatrixNxM<TYPE>& mat,
									const CVectorN<TYPE>& v)
{
	// stored the product
	CVectorN<TYPE> vProduct(mat.GetRows());
	MultMatrixVector(vProduct, mat, v);

	// return the product
	return vProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CVectorN<TYPE>&)


// TODO: move this to MultMatrixVector
#ifdef USE_IPP
#define DECLARE_MATRIXN_VECPRODUCT(TYPE, TYPE_IPP) \
	template<> __forceinline	\
	CVectorN<TYPE> operator*(const CMatrixNxM<TYPE>& mat,		\
							 const CVectorN<TYPE>& v)				\
	{																	\
		CVectorN<TYPE> vProduct(v.GetDim());							\
		ippmMul_mTv_##TYPE_IPP(&mat[0][0], mat.GetCols() * sizeof(TYPE),	\
			mat.GetCols(), mat.GetRows(),								\
			&v[0], v.GetDim(),											\
			&vProduct[0]);												\
		return vProduct;												\
	}

DECLARE_MATRIXN_VECPRODUCT(float, 32f);
DECLARE_MATRIXN_VECPRODUCT(double, 64f);
#endif



//////////////////////////////////////////////////////////////////////
// operator*(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CMatrixNxM<TYPE> operator*(const CMatrixNxM<TYPE>& mLeft, 
									const CMatrixNxM<TYPE>& mRight)
{
	// create the product
	CMatrixNxM<TYPE> mProduct(mRight.GetCols(), mLeft.GetRows());
	MultMatrixMatrix(mProduct, mLeft, mRight);

	// return the product
	return mProduct;

}	// operator*(const CMatrixNxM<TYPE>&, const CMatrixNxM<TYPE>&)


//////////////////////////////////////////////////////////////////////
// CMatrixNxM<TYPE>::Determinant
//
// computes the determinant of the matrix, for square matrices
//////////////////////////////////////////////////////////////////////
template<class TYPE> inline
TYPE Determinant(const CMatrixNxM<TYPE>& mMat) 
{
	ASSERT(mMat.GetCols() == mMat.GetRows());

	if (mMat.GetCols() > 2) 
	{
		TYPE det = 0.0;
		for (int nAtCol = 0; nAtCol < mMat.GetCols(); nAtCol++) 
		{
			CMatrixNxM<TYPE> mMinor(mMat.GetCols()-1, mMat.GetRows()-1);
			for (int nAtRow = 1; nAtRow < mMat.GetRows(); nAtRow++) 
			{
				int nAtMinorCol = 0;
				for (int nAtCol2 = 0; nAtCol2 < mMat.GetCols(); nAtCol2++) 
				{
				   if (nAtCol2 != nAtCol)
				   {
					   mMinor[nAtMinorCol][nAtRow-1] = mMat[nAtCol2][nAtRow];
					   nAtMinorCol++;
				   }
				}
			}
			det += ((nAtCol % 2 == 0) ? 1.0 : -1.0) 
				* mMat[nAtCol][0] * Determinant(mMinor);
		}

		return det;
	}
	else if (mMat.GetCols() > 1) 
	{
		return mMat[0][0] * mMat[1][1] - mMat[1][0] * mMat[0][1];
	}
	
	return mMat[0][0];

}	// CMatrixNxM<TYPE>::Determinant


// operator overloads for serialization
#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
// function operator<<
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator<<(CArchive &ar, CMatrixNxM<TYPE> m)
{
	// serialize the dimension
	ar << m.GetCols();
	ar << m.GetRows();

	// serialize the individual elements
	ar.Write((TYPE *) m, m.GetCols() * m.GetRows() * sizeof(TYPE));


	// return the archive object
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator>>(CArchive &ar, CMatrixNxM<TYPE>& m)
{
	// serialize the dimension
	int nCols, nRows;
	ar >> nCols >> nRows;
	m.Reshape(nCols, nRows);

	// serialize the individual elements
	ar.Read((TYPE *) m, nCols * nRows * sizeof(TYPE));

	// return the archive object
	return ar;
}

#endif	// __AFX_H__


#ifdef USE_XMLLOGGING
//////////////////////////////////////////////////////////////////////
// LogExprExt
//
// helper function for XML logging of matrices
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
void LogExprExt(const CMatrixNxM<TYPE>& mMat, const char *pszName, const char *pszModule)
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// create a new expression element
	CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

	// if there is a name,
	if (strlen(pszName) > 0)
	{
		// set it.
		pVarElem->Attribute("name", pszName);
	}

	// set type to generice "CVector"
	pVarElem->Attribute("type", "CMatrix");

	// stores each row
	CVectorN<TYPE> vRow(mMat.GetCols());
	for (int nAtRow = 0; nAtRow < mMat.GetRows(); nAtRow++)
	{
		// get each row
		mMat.GetRow(nAtRow, vRow);

		// and output as sub-element
		LogExprExt(vRow, "", pszModule);
	}

	// close the element
	pLog->CloseElement();

}	// LogExprExt

#endif	// USE_XMLLOGGING



#endif	// MATRIXNXM_H