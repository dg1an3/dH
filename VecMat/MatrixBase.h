//////////////////////////////////////////////////////////////////////
// Matrix.h: declaration and definition of the CMatrixBase template class.
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

#include "VectorN.h"

//////////////////////////////////////////////////////////////////////
// constant for finding the pivot during a matrix inversion
//////////////////////////////////////////////////////////////////////
#define MAX_TO_PIVOT (1.0)

//////////////////////////////////////////////////////////////////////
// class CMatrixBase<TYPE>
//
// represents a square matrix with GetDim()ension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixBase
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- initializes to 0x0 matrix
	//////////////////////////////////////////////////////////////////
	CMatrixBase()
		: m_ppRows(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixBase(const CMatrixBase& fromMatrix)
		: m_ppRows(NULL)
	{
		SetDim(fromMatrix.GetDim());

		(*this) = fromMatrix;
	}

	//////////////////////////////////////////////////////////////////
	// destructor -- frees the row vectors
	//////////////////////////////////////////////////////////////////
	~CMatrixBase()
	{
		if (NULL != m_ppRows)
		{
			delete [] m_ppRows[0];
			delete [] m_ppRows;
		}
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixBase& operator=(const CMatrixBase& fromMatrix)
	{
		ASSERT(GetDim() == fromMatrix.GetDim());

		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = fromMatrix[nAt];
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator[] -- retrieves a reference to a row vector
	//////////////////////////////////////////////////////////////////
	CVectorBase<TYPE>& operator[](int nAtRow)
	{
		// bounds check on the index
		ASSERT(nAtRow >= 0 && nAtRow < m_ppRows[0]->GetDim());

		// return a reference to the row vector
		return (*m_ppRows[nAtRow]);
	}

	//////////////////////////////////////////////////////////////////
	// operator[] const -- retrieves a const reference to a row 
	//		vector
	//////////////////////////////////////////////////////////////////
	const CVectorBase<TYPE>& operator[](int nAtRow) const
	{
		// bounds check on the index
		ASSERT(nAtRow >= 0 && nAtRow < m_ppRows[0]->GetDim());

		// return a reference to the row vector
		return (*m_ppRows[nAtRow]);
	}

	//////////////////////////////////////////////////////////////////
	// GetDim -- returns the dimension of the matrix
	//////////////////////////////////////////////////////////////////
	int GetDim() const
	{
		if (NULL != m_ppRows)
		{
			return (*this)[0].GetDim();
		}
		else
		{
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CMatrixBase& m, TYPE epsilon = EPS) const
	{
		ASSERT(GetDim() == m.GetDim());

		for (int nAtRow = 0; nAtRow < GetDim(); nAtRow++)
		{
			if (!(*this)[nAtRow].IsApproxEqual(m[nAtRow], epsilon))
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
		ASSERT(GetDim() == mRight.GetDim());

		// element-by-element sum of the matrix
		for (int nRow = 0; nRow < GetDim(); nRow++)
		{
			for (int nCol = 0; nCol < GetDim(); nCol++)
			{
				(*this)[nRow][nCol] += mRight[nRow][nCol];
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
		ASSERT(GetDim() == mRight.GetDim());

		// element-by-element difference of the matrix
		for (int nRow = 0; nRow < GetDim(); nRow++)
		{
			for (int nCol = 0; nCol < GetDim(); nCol++)
			{
				(*this)[nRow][nCol] -= mRight[nRow][nCol];
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
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
			{
				(*this)[nRow][nMid] *= scale;
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
		ASSERT(GetDim() == mRight.GetDim());

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
		for (int nRow = 0; nRow < GetDim(); nRow++)
		{
			for (int nCol = 0; nCol < GetDim(); nCol++)
			{
				pArray[nCol * GetDim() + nRow] = (*this)[nRow][nCol];
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// FromArray -- fills this matrix from an OpenGL-format 
	//		(column-major) double array from this matrix
	//////////////////////////////////////////////////////////////////
	void FromArray(const double *pArray)
	{
		for (int nRow = 0; nRow < GetDim(); nRow++)
		{
			for (int nCol = 0; nCol < GetDim(); nCol++)
			{
				(*this)[nRow][nCol] = pArray[nCol * GetDim() + nRow];
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// SetIdentity -- sets the matrix to an identity matrix
	//////////////////////////////////////////////////////////////////
	void SetIdentity()
	{
		for (int nRow = 0; nRow < GetDim(); nRow++)
		{
			for (int nCol = 0; nCol < GetDim(); nCol++)
			{
				(*this)[nRow][nCol] = 
					(TYPE)((nRow == nCol) ? 1.0 : 0.0);
			}
		}
	}

	// Transpose -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	void Transpose();

	// Orthogonalize --
	void Orthogonalize();

	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	void Invert(BOOL bFullPivot = FALSE);

protected:
	//////////////////////////////////////////////////////////////////
	// SetDim -- sets the dimension of the matrix
	//////////////////////////////////////////////////////////////////
	void SetDim(int nDim)
	{
		if (GetDim() == nDim)
		{
			return;
		}

		// delete previous data
		if (NULL != m_ppRows)
		{
			delete [] m_ppRows[0];
			delete [] m_ppRows;
			m_ppRows = NULL;
		}

		// set up the row vectors
		if (0 != nDim)
		{
			// allocate new data
			m_ppRows = new CVectorBase<TYPE>*[nDim];
			CVectorN<TYPE> *pRows = new CVectorN<TYPE>[nDim];
			for (int nAt = 0; nAt < nDim; nAt++)
			{
				pRows[nAt].SetDim(nDim);
				m_ppRows[nAt] = &pRows[nAt];
			}
		}

		// populate as an identity matrix
		SetIdentity();
	}

	// helper functions for Invert -- defined below

	// InterchangeRows -- swaps two rows of the matrix
	void InterchangeRows(int nRow1, int nRow2);

	// InterchangeCols -- swaps two cols of the matrix
	void InterchangeCols(int nCol1, int nCol2);

	// FindPivotRow -- determines the best other row to be 
	//		exchanged so that no zero elements occur on the diagonal
	int FindPivotRow(int nDiag);

	// FindPivotElem -- determines the best pivot element
	void FindPivotElem(int nDiag, int *nRow, int *nCol);

protected:
	// the row vectors of the matrix
	CVectorBase<TYPE> **m_ppRows;
};

//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::Transpose
//
// transposes the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Transpose()
{
	for (int nCol = 0; nCol < GetDim(); nCol++)
	{
		for (int nRow = 0; nRow < nCol; nRow++)
		{
			TYPE temp = (*this)[nRow][nCol];
			(*this)[nRow][nCol] = (*this)[nCol][nRow];
			(*this)[nCol][nRow] = temp;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// function Orthogonalize
//
// orthogonalizes the input matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Orthogonalize()
{
	// Gramm-Schmidt orthogonalization

	// begin by setting the first orthogonal row vector
	CMatrixBase<TYPE> mOrtho(*this);
	mOrtho[0] = (*this)[0];

	// apply to each row vector after the zero-th
	for (int nAtRow = 1; nAtRow < GetDim(); nAtRow++)
	{
		mOrtho[nAtRow] = (*this)[nAtRow];
		for (int nAtOrthoRow = nAtRow-1; nAtOrthoRow >= 0; nAtOrthoRow--)
		{
			double scalar = ((*this)[nAtRow] * mOrtho[nAtOrthoRow])
				/ (mOrtho[nAtOrthoRow] * mOrtho[nAtOrthoRow]);
			mOrtho[nAtRow] -= scalar * mOrtho[nAtOrthoRow];
			TRACE("Dot product = %lf\n", (mOrtho[nAtRow] * mOrtho[nAtOrthoRow]));
		}
	}

	// now normalize all rows
	for (nAtRow = 0; nAtRow < GetDim(); nAtRow++)
	{
		mOrtho[nAtRow].Normalize();
	}

#ifdef _DEBUG
	// test for orthogonality
	for (nAtRow = 1; nAtRow < GetDim(); nAtRow++)
	{
		for (int nAtOrthoRow = nAtRow-1; nAtOrthoRow >= 0; nAtOrthoRow--)
		{
			ASSERT((mOrtho[nAtRow] * mOrtho[nAtOrthoRow]) < EPS);
		}
	}
#endif

	// assign the result
	(*this) = mOrtho;
}

//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::Invert
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::Invert(BOOL bFullPivot)
{
	// Gauss-Jordan elimination
	CMatrixBase<TYPE> mCopy(*this);		// the copy of this matrix
	CMatrixBase<TYPE> mInv(*this);		// stores the built inverse
	mInv.SetIdentity();

	// stores the sequence of column swaps for a full pivot
	valarray<int> arrColumnSwaps(GetDim());

	for (int nCol = 0; nCol < GetDim(); nCol++)
	{
		if (bFullPivot)
		{
			// CHECK THE FULL PIVOT
			// find the full pivot element
			int nPivotRow, nPivotCol;
			mCopy.FindPivotElem(nCol, &nPivotRow, &nPivotCol);

			// interchange the rows
			mCopy.InterchangeRows(nCol, nPivotRow);
			mCopy.InterchangeCols(nCol, nPivotCol);

			// interchange the columns
			mInv.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeCols(nCol, nPivotCol);
			
			// store the column interchange for later restoring
			arrColumnSwaps[nCol] = nPivotCol;
		}
		else	// partial pivot
		{
			// pivot if necessary
			int nPivotRow = mCopy.FindPivotRow(nCol);
			mCopy.InterchangeRows(nCol, nPivotRow);
			mInv.InterchangeRows(nCol, nPivotRow);
		}

    	// obtain a 1 in the diagonal position
		//		(the pivot ensures that copy[nCol][nCol] is not zero)
		// make sure we are numerically stable
		ASSERT(fabs(mCopy[nCol][nCol]) > 1e-8);
    	double scale = 1.0 / mCopy[nCol][nCol];	// computed scale factor
    	mCopy[nCol] *= scale;
		mInv[nCol] *= scale;

		// obtain zeros in the off-diagonal elements
		int nRow;	// for index
	  	for (nRow = 0; nRow < GetDim(); nRow++) 
		{
    		if (nRow != nCol) {
    			scale = -mCopy[nRow][nCol];
				mCopy[nRow] += mCopy[nCol] * scale;
    			mInv[nRow] += mInv[nCol] * scale;
    		}
    	}
	}

	if (bFullPivot)
	{
		// restores the sequence of columns
		for (int nAtCol = GetDim()-1; nAtCol >= 0; nAtCol--)
		{
			mInv.InterchangeCols(nAtCol, arrColumnSwaps[nAtCol]);
		}
	}

	// assign this to inverse
	(*this) = mInv; 
}

//////////////////////////////////////////////////////////////////////
// function CMatrixBase<, TYPE>::InterchangeRows
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeRows(int nRow1, int nRow2) 
{
	// check that the rows are not the same
	if (nRow1 != nRow2)
	{
		// temporary vector storage
		CVectorN<TYPE> vTemp = (*this)[nRow1];	

		// and swap the two rows
		(*this)[nRow1] = (*this)[nRow2];
		(*this)[nRow2] = vTemp;
	}
}
    
//////////////////////////////////////////////////////////////////////
// function CMatrixBase<, TYPE>::InterchangeRows
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::InterchangeCols(int nCol1, int nCol2) 
{
	// check that the cols are not the same
	if (nCol1 != nCol2)
	{
		for (int nAtRow = 0; nAtRow < GetDim(); nAtRow++)
		{
			TYPE temp = (*this)[nAtRow][nCol1];
			(*this)[nAtRow][nCol1] = (*this)[nAtRow][nCol2];
			(*this)[nAtRow][nCol2] = temp;
		}
	}
}
    
//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::FindPivotRow
//
// finds the pivot element (returns the row of the element in the
//		the given column)
//////////////////////////////////////////////////////////////////////
template<class TYPE>
int CMatrixBase<TYPE>::FindPivotRow(int nDiag)
{
	int nBestRow = nDiag;	// stores the current best row

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		int nRow;	// for index
    	for (nRow = nDiag + 1; nRow < GetDim(); nRow++)
		{
			if (fabs((*this)[nRow][nDiag]) > fabs((*this)[nBestRow][nDiag]))
			{
    			nBestRow = nRow;
			}
		}
	}

	return nBestRow;
}

//////////////////////////////////////////////////////////////////////
// function CMatrixBase<TYPE>::FindPivotElem
//
// finds the pivot element, returning the row and column
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CMatrixBase<TYPE>::FindPivotElem(int nDiag, 
		int *pBestRow, int *pBestCol)
{
	(*pBestRow) = nDiag;
	(*pBestCol) = nDiag;

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
    	for (int nRow = nDiag; nRow < GetDim(); nRow++)
		{
			for (int nCol = nDiag; nCol < GetDim(); nCol++)
			{
				if (fabs((*this)[nRow][nCol]) 
					> fabs((*this)[*pBestRow][*pBestCol]))
				{
					(*pBestRow) = nRow;
					(*pBestCol) = nCol;
				}
			}
		}
	}
}

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
	for (int nRow = 0; nRow < mLeft.GetDim(); nRow++)
	{
		for (int nCol = 0; nCol < mLeft.GetDim(); nCol++)
		{
			if (mLeft[nRow][nCol] != mRight[nRow][nCol])
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
// function operator*(const CMatrixBase<TYPE>&, const CVectorN<TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator*(const CMatrixBase<TYPE>& mat,
									const CVectorBase<TYPE>& v)
{
	// stored the product
	CVectorN<TYPE> vProduct(v.GetDim());

	// step through the rows
	for (int nRow = 0; nRow < mat.GetDim(); nRow++)
	{
		ASSERT(vProduct[nRow] == 0.0);

		for (int nMid = 0; nMid < mat.GetDim(); nMid++)
		{
			vProduct[nRow] += mat[nRow][nMid] * v[nMid];
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
	// create the product
	CMatrixBase<TYPE> mProduct(mLeft);

	// use in-place multiplication
	mProduct *= mRight;

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

	for (int nAtRow = 0; nAtRow < m.GetDim(); nAtRow++)
	{
		cout << "\t<";
		for (int nAtCol = 0; nAtCol < m.GetDim(); nAtCol++)
		{
			cout << (double) m[nAtRow][nAtCol] << "\t";
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

	for (int nAtRow = 0; nAtRow < m.GetDim(); nAtRow++)
	{
		TRACE("\t<");
		for (int nAtCol = 0; nAtCol < m.GetDim(); nAtCol++)
		{
			TRACE("%10.4lf\t", (double) m[nAtRow][nAtCol]);
		}
		TRACE(">\n");
	} 
}


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


#endif