//////////////////////////////////////////////////////////////////////
// MatrixBase.h: declaration and definition of the CMatrixBase 
//		template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXBASE_H)
#define MATRIXBASE_H

#undef min
#undef max

#include <valarray>
using namespace std;

#include "MathUtil.h"

#include "VectorBase.h"

//////////////////////////////////////////////////////////////////////
// constant for finding the pivot during a matrix inversion
//////////////////////////////////////////////////////////////////////
#define MAX_TO_PIVOT (1.0)


//////////////////////////////////////////////////////////////////////
// forward definition of trace function
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixBase;

template<class TYPE>
void TraceMatrix(const char *pszMessage, const CMatrixBase<TYPE> m);

//////////////////////////////////////////////////////////////////////
// macro TRACE_MATRIX
//
// macro to trace matrix -- only compiles in debug version
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define TRACE_MATRIX(strMessage, m) \
	TraceMatrix(strMessage, m);	
#else
#define TRACE_MATRIX(strMessage, m)
#endif


//////////////////////////////////////////////////////////////////////
// class CMatrixBase<TYPE>
//
// represents a square matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixBase
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- initializes to 0x0 matrix
	//////////////////////////////////////////////////////////////////
	CMatrixBase()
		: m_arrColumns(NULL),
			m_pElements(NULL),
			m_bFreeElements(TRUE)
	{
		m_nCols = 0;
		m_nRows = 0;

	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixBase(const CMatrixBase& fromMatrix)
		: m_arrColumns(NULL),
			m_pElements(NULL),
			m_bFreeElements(TRUE)
	{
		m_nCols = 0;
		m_nRows = 0;

		Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

		(*this) = fromMatrix;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixBase(int nCols, int nRows)
		: m_arrColumns(NULL),
			m_pElements(NULL),
			m_bFreeElements(TRUE)
	{
		m_nCols = 0;
		m_nRows = 0;

		Reshape(nCols, nRows);
	}

	//////////////////////////////////////////////////////////////////
	// destructor -- frees the row vectors
	//////////////////////////////////////////////////////////////////
	~CMatrixBase()
	{
		if (m_bFreeElements && NULL != m_pElements)
		{
			delete [] m_pElements;
		}

		if (NULL != m_arrColumns)
		{
			delete [] m_arrColumns;
		}
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator=(const CMatrixBase& fromMatrix)
	{
		Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

		// assign successive column vectors
		for (int nAt = 0; nAt < GetCols(); nAt++)
		{
			(*this)[nAt] = fromMatrix[nAt];
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator[] -- retrieves a reference to a column vector
	//////////////////////////////////////////////////////////////////
	CVectorBase<TYPE>& operator[](int nAtCol)
	{
		// bounds check on the index
		ASSERT(nAtCol >= 0 && nAtCol < GetCols());

		// return a reference to the column vector
		return m_arrColumns[nAtCol];
	}

	//////////////////////////////////////////////////////////////////
	// operator[] const -- retrieves a const reference to a column 
	//		vector
	//////////////////////////////////////////////////////////////////
	const CVectorBase<TYPE>& operator[](int nAtCol) const
	{
		// bounds check on the index
		ASSERT(nAtCol >= 0 && nAtCol < GetCols());

		// return a reference to the column vector
		return m_arrColumns[nAtCol];
	}

	//////////////////////////////////////////////////////////////////
	// GetCols -- returns the number of columns of the matrix
	//////////////////////////////////////////////////////////////////
	int GetCols() const
	{
		return m_nCols;
	}

	//////////////////////////////////////////////////////////////////
	// GetRows -- returns the number of rows of the matrix
	//////////////////////////////////////////////////////////////////
	int GetRows() const
	{
		return m_nRows;
	}

	//////////////////////////////////////////////////////////////////
	// GetColumn -- constructs and returns a row vector
	//////////////////////////////////////////////////////////////////
	void GetRow(int nAtRow, CVectorBase<TYPE>& vRow) const
	{
		// make the row vector the same size
		// vRow.SetDim(GetCols());

		// populate the row vector
		for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
		{
			vRow[nAtCol] = (*this)[nAtCol][nAtRow];
		}
	}

	//////////////////////////////////////////////////////////////////
	// SetColumn -- constructs and returns a column vector
	//////////////////////////////////////////////////////////////////
	void SetRow(int nAtRow, const CVectorBase<TYPE>& vRow)
	{
		if (vRow.GetDim() == GetCols())
		{
			// de-populate the row vector
			for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
			{
				(*this)[nAtCol][nAtRow] = vRow[nAtCol];
			}
		}
	}

	// InterchangeRows -- swaps two rows of the matrix
	void InterchangeRows(int nRow1, int nRow2);

	// InterchangeCols -- swaps two cols of the matrix
	void InterchangeCols(int nCol1, int nCol2);

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CMatrixBase& m, TYPE epsilon = EPS) const
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
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place matrix addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator+=(const CMatrixBase& mRight)
	{
		ASSERT(GetCols() == mRight.GetCols());
		ASSERT(GetRows() == mRight.GetRows());

		// element-by-element sum of the matrix
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] += mRight[nCol][nRow];
			}
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place matrix subtraction; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator-=(const CMatrixBase& mRight)
	{
		ASSERT(GetCols() == mRight.GetCols());
		ASSERT(GetRows() == mRight.GetRows());

		// element-by-element difference of the matrix
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] -= mRight[nCol][nRow];
			}
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place scalar multiplication
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator*=(double scale)
	{
		// element-by-element difference of the matrix
		for (int nRow = 0; nRow < GetRows(); nRow++)
		{
			for (int nCol = 0; nCol < GetCols(); nCol++)
			{
				(*this)[nCol][nRow] *= scale;
			}
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place matrix multiplication; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator*=(const CMatrixBase& mRight)
	{
		// multiply this by right
		(*this) = (*this) * mRight;

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// ToArray -- fills an OpenGL-format (column-major) double array 
	//		from this matrix
	//////////////////////////////////////////////////////////////////
	void ToArray(double *pArray) const
	{
		// element position in the array
		long nAtElement = 0;

		// for each element in the matrix,
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				// assign the element
				pArray[nAtElement] = (*this)[nCol][nRow];
				
				// increment the element position
				nAtElement++;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// FromArray -- fills this matrix from an OpenGL-format 
	//		(column-major) double array from this matrix
	//////////////////////////////////////////////////////////////////
	void FromArray(const double *pArray)
	{
		// element position in the array
		long nAtElement = 0;

		// for each element in the matrix,
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				// assign the element
				(*this)[nCol][nRow] = pArray[nAtElement];
				
				// increment the element position
				nAtElement++;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// SetIdentity -- sets the matrix to an identity matrix
	//////////////////////////////////////////////////////////////////
	void SetIdentity()
	{
		// for each element in the matrix,
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] = 
					(TYPE)((nRow == nCol) ? 1.0 : 0.0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// SetElements -- sets the elements to an external pointer
	//////////////////////////////////////////////////////////////////
	void SetElements(int nCols, int nRows, TYPE *pElements);

	//////////////////////////////////////////////////////////////////
	// Transpose -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	//////////////////////////////////////////////////////////////////
	void Transpose();

	//////////////////////////////////////////////////////////////////
	// orthogonality test
	//////////////////////////////////////////////////////////////////
	BOOL IsOrthogonal() const;

	//////////////////////////////////////////////////////////////////
	// Orthogonalize 
	//////////////////////////////////////////////////////////////////
	void Orthogonalize();

	//////////////////////////////////////////////////////////////////
	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	//////////////////////////////////////////////////////////////////
	void Invert(BOOL bFullPivot = FALSE);

	//////////////////////////////////////////////////////////////////
	// SVD -- singular-valued decomposition -- returns the singular
	//		value matrix as a vector of diagonal values w, and 
	//		returns V (not V^T)
	//////////////////////////////////////////////////////////////////
	BOOL SVD(CVectorBase<TYPE>& w, CMatrixBase& v);

	//////////////////////////////////////////////////////////////////
	// Pseudoinvert -- in-place Moore-Penrose pseudoinversion
	//////////////////////////////////////////////////////////////////
	BOOL Pseudoinvert();

protected:
	//////////////////////////////////////////////////////////////////
	// Reshape -- sets the rows and columns of the matrix
	//////////////////////////////////////////////////////////////////
	void Reshape(int nCols, int nRows);

	// helper functions for Invert -- defined below

	// FindPivotRow -- determines the best other row to be 
	//		exchanged so that no zero elements occur on the diagonal
	int FindPivotRow(int nDiag);

	// FindPivotElem -- determines the best pivot element
	void FindPivotElem(int nDiag, int *nRow, int *nCol);

	// helper function for SVD to compute the Householder decomposition
	//		of this matrix, and to accumulate the rh and lh singular
	//		values
	TYPE Householder(CVectorBase<TYPE>& w, CVectorBase<TYPE>& rv1);
	void AccumulateRH(CMatrixBase& v, const CVectorBase<TYPE>& rv1);
	void AccumulateLH(CVectorBase<TYPE>& w);

protected:
	// the matrix shape
	int m_nRows;
	int m_nCols;

	// the row vectors of the matrix
	CVectorBase<TYPE> *m_arrColumns;

	// the elements of the matrix
	TYPE *m_pElements;

	// flag to indicate whether elements should be freed
	BOOL m_bFreeElements;
};

//////////////////////////////////////////////////////////////////////
// function operator==(const CMatrixBase<TYPE>&, 
//		const CMatrixBase<TYPE>&)
//
// exact matrix equality
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator==(const CMatrixBase<TYPE>& mLeft, 
					   const CMatrixBase<TYPE>& mRight)
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
}

//////////////////////////////////////////////////////////////////////
// function operator!=(const CMatrixBase<TYPE>&, 
//		const CMatrixBase<TYPE>&)
//
// exact matrix inequality
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator!=(const CMatrixBase<TYPE>& mLeft, 
					   const CMatrixBase<TYPE>& mRight)
{
	return !(mLeft == mRight);
}

//////////////////////////////////////////////////////////////////////
// function operator+(const CMatrixBase<TYPE>&, 
//		const CMatrixBase<TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixBase<TYPE> operator+(const CMatrixBase<TYPE>& mLeft, 
									const CMatrixBase<TYPE>& mRight)
{
	// create the product
	CMatrixBase<TYPE> mSum(mLeft);

	// use in-place multiplication
	mSum += mRight;

	// return the product
	return mSum;
}

//////////////////////////////////////////////////////////////////////
// function operator+(const CMatrixBase<TYPE>&, 
//		const CMatrixBase<TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixBase<TYPE> operator-(const CMatrixBase<TYPE>& mLeft, 
									const CMatrixBase<TYPE>& mRight)
{
	// create the difference
	CMatrixBase<TYPE> mDiff(mLeft);

	// use in-place subtraction
	mDiff -= mRight;

	// return the product
	return mDiff;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrixBase<TYPE>&, double scale)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixBase<TYPE> operator*(const CMatrixBase<TYPE>& mat,
									double scale)
{
	// stored the product
	CMatrixBase<TYPE> mProduct(mat);

	// scalar multiplication
	mProduct *= scale;

	// return product
	return mProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(double scale, const CMatrixBase<TYPE>&)
//
// matrix scalar multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixBase<TYPE> operator*(double scale, 
								   const CMatrixBase<TYPE>& mat)
{
	// stored the product
	CMatrixBase<TYPE> mProduct(mat);

	// scalar multiplication
	mProduct *= scale;

	// return product
	return mProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrixBase<TYPE>&, const CVectorBase<TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator*(const CMatrixBase<TYPE>& mat,
									const CVectorBase<TYPE>& v)
{
	ASSERT(mat.GetRows() == v.GetDim());

	// stored the product
	CVectorBase<TYPE> vProduct(v);

	// step through the rows of the matrix
	for (int nRow = 0; nRow < mat.GetRows(); nRow++)
	{
		ASSERT(vProduct[nRow] == 0.0);

		// step through the columns of the matrix
		for (int nCol = 0; nCol < mat.GetCols(); nCol++)
		{
			vProduct[nRow] += mat[nCol][nRow] * v[nCol];
		}
	}

	// return the product
	return vProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrixBase<TYPE>&, 
//		const CMatrixBase<TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CMatrixBase<TYPE> operator*(const CMatrixBase<TYPE>& mLeft, 
									const CMatrixBase<TYPE>& mRight)
{
	ASSERT(mLeft.GetCols() == mRight.GetRows());

	// create the product
	CMatrixBase<TYPE> mProduct(mRight.GetCols(), mLeft.GetRows());

	// compute the matrix product
	for (int nRow = 0; nRow < mProduct.GetRows(); nRow++)
	{
		for (int nCol = 0; nCol < mProduct.GetCols(); nCol++)
		{
			mProduct[nCol][nRow] = 0.0;

			for (int nMid = 0; nMid < mLeft.GetCols(); nMid++)
			{
				mProduct[nCol][nRow] +=
					mLeft[nMid][nRow] * mRight[nCol][nMid];
			}
		}
	}

	// return the product
	return mProduct;
}


//////////////////////////////////////////////////////////////////////
// function operator<<(ostream& ar, CMatrixBase)
//
// stream output of a matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
ostream& operator<<(ostream& os, CMatrixBase<TYPE> v)
{
	cout << strMessage << " = \n";

	for (int nAtRow = 0; nAtRow < m.GetRows(); nAtRow++)
	{
		cout << "\t<";
		for (int nAtCol = 0; nAtCol < m.GetCols(); nAtCol++)
		{
			cout << (double) m[nAtCol][nAtRow] << "\t";
		}
		cout << ">\n";
	} 

	return os;
}


//////////////////////////////////////////////////////////////////////
// function TraceMatrix
//
// outputs the matrix for tracing
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TraceMatrix(const char *pszMessage, const CMatrixBase<TYPE> m)
{
	TRACE("%s = \n", pszMessage);

	// MATLAB output
	TRACE("\t[");

	for (int nAtRow = 0; nAtRow < m.GetRows(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < m.GetCols(); nAtCol++)
		{
			TRACE("%10.4lf\t", (double) m[nAtCol][nAtRow]);
		}

		if (nAtRow < m.GetRows()-1)
		{
			// MATLAB output
			TRACE(";\n\t");
		}
	} 

	// MATLAB output
	TRACE("]\n");
}

#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////
// function MatrixValid
//
// asserts that the matrix elements are valid values
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void MatrixValid(const CMatrixBase<TYPE>& m)
{
	for (int nAt = 0; nAt < m.GetCols(); nAt++)
	{
		VectorValid(m[nAt]);
	}
}
#endif

#endif