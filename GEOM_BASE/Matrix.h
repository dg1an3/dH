//////////////////////////////////////////////////////////////////////
// Matrix.h: declaration and definition of the CMatrix template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIX_H)
#define MATRIX_H

#include <math.h>

#include "ScalarFunction.h"
#include "Vector.h"

//////////////////////////////////////////////////////////////////////
// constant for finding the pivot during a matrix inversion
//////////////////////////////////////////////////////////////////////
#define MAX_TO_PIVOT (1e-6)

//////////////////////////////////////////////////////////////////////
// class CMatrix<DIM, TYPE>
//
// represents a square matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CMatrix
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- initializes to an identity matrix
	//////////////////////////////////////////////////////////////////
	CMatrix()
		: m_arrRows(new CVector<DIM, TYPE>[DIM])
	{
		SetIdentity();
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrix(const CMatrix& fromMatrix)
		: m_arrRows(new CVector<DIM, TYPE>[DIM])
	{
		(*this) = fromMatrix;
	}

	//////////////////////////////////////////////////////////////////
	// destructor -- frees the row vectors
	//////////////////////////////////////////////////////////////////
	~CMatrix()
	{
		delete [] m_arrRows;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	operator=(const CMatrix& fromMatrix)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrRows[nAt] = fromMatrix.m_arrRows[nAt];
	}

	//////////////////////////////////////////////////////////////////
	// operator[] -- retrieves a reference to a row vector
	//////////////////////////////////////////////////////////////////
	CVector<DIM, TYPE>& operator[](int nAtRow)
	{
		// bounds check on the index
		ASSERT(nAtRow >= 0 && nAtRow < DIM);

		// return a reference to the row vector
		return m_arrRows[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// operator[] const -- retrieves a const reference to a row 
	//		vector
	//////////////////////////////////////////////////////////////////
	const CVector<DIM, TYPE>& operator[](int nAtRow) const
	{
		// bounds check on the index
		ASSERT(nAtRow >= 0 && nAtRow < DIM);

		// return a reference to the row vector
		return m_arrRows[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place matrix addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrix& operator+=(const CMatrix& mRight)
	{
		// element-by-element sum of the matrix
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nMid] += mRight[nMid][nCol];

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place matrix subtraction; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrix& operator-=(const CMatrix& mRight)
	{
		// element-by-element difference of the matrix
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nMid] -= mRight[nMid][nCol];

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place matrix multiplication; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrix& operator*=(const CMatrix& mRight)
	{
		// holds the final product
		CMatrix mProduct;

		// compute the matrix product
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
			{
				mProduct[nRow][nCol] = 0.0;
				for (int nMid = 0; nMid < DIM; nMid++)
				{
					mProduct[nRow][nCol] +=
						(*this)[nRow][nMid] * mRight[nMid][nCol];
				}
			}
		}

		// assign final product to this
		(*this) = mProduct;

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// SetIdentity -- sets the matrix to an identity matrix
	//////////////////////////////////////////////////////////////////
	void SetIdentity()
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nCol] = 
					(TYPE)((nRow == nCol) ? 1.0 : 0.0);
	}

	//////////////////////////////////////////////////////////////////
	// ToArray -- fills an OpenGL-format (column-major) double array 
	//		from this matrix
	//////////////////////////////////////////////////////////////////
	// 
	void ToArray(double *pArray) const
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				pArray[nCol * DIM + nRow] = (*this)[nRow][nCol];
	}

	//////////////////////////////////////////////////////////////////
	// FromArray -- fills this matrix from an OpenGL-format 
	//		(column-major) double array from this matrix
	//////////////////////////////////////////////////////////////////
	void FromArray(const double *pArray)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nCol] = pArray[nCol * DIM + nRow];
	}

	// Invert -- inverts the matrix using the Gauss-Jordan 
	//		elimination
	void Invert();

protected:

	// helper functions for Invert -- defined below

	// InterchangeRows -- swaps two rows of the matrix
	void InterchangeRows(int nRow1, int nRow2);

	// FindPivot -- determines the best other row to be exchanged
	//		so that no zero elements occur on the diagonal
	int FindPivot(int nDiag);

private:
	// the row vectors of the matrix
	CVector<DIM, TYPE> *m_arrRows; // [DIM];
};

//////////////////////////////////////////////////////////////////////
// function CMatrix<DIM, TYPE>::InterchangeRows
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void CMatrix<DIM, TYPE>::InterchangeRows(int nRow1, int nRow2) 
{
	// check that the rows are not the same
	if (nRow1 != nRow2)
	{
		// temporary vector storage
		CVector<DIM, TYPE> vTemp = (*this)[nRow1];	

		// and swap the two rows
		(*this)[nRow1] = (*this)[nRow2];
		(*this)[nRow2] = vTemp;
	}
}
    
//////////////////////////////////////////////////////////////////////
// function CMatrix<DIM, TYPE>::FindPivot
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
int CMatrix<DIM, TYPE>::FindPivot(int nDiag)
{
	int nBestRow = nDiag;	// stores the current best row

	if (fabs((*this)[nDiag][nDiag]) < MAX_TO_PIVOT) 
	{
		int nRow;	// for index
    	for (nRow = nDiag + 1; nRow < DIM; nRow++)
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
// function CMatrix<DIM, TYPE>::Invert
//
// swaps two rows of the matrix
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void CMatrix<DIM, TYPE>::Invert()
{
	// Gauss-Jordan elimination
	CMatrix<DIM, TYPE> copy(*this);	// the copy of this matrix
	CMatrix<DIM, TYPE> inv;	// stores the built inverse

	int nCol;	// for index
	for (nCol = 0; nCol < DIM; nCol++)
	{
		// pivot if necessary
		int nPivotRow = copy.FindPivot(nCol);
		copy.InterchangeRows(nCol, nPivotRow);
		inv.InterchangeRows(nCol, nPivotRow);

    	// obtain a 1 in the diagonal position
		//		(the pivot ensures that copy[nCol][nCol] is not zero)
    	double scale = 1.0 / copy[nCol][nCol];	// computed scale factor
    	copy[nCol] = copy[nCol] * scale;
		inv[nCol] = inv[nCol] * scale;

		// obtain zeros in the off-diagonal elements
		int nRow;	// for index
    	for (nRow = 0; nRow < DIM; nRow++) {
    		if (nRow != nCol) {
    			scale = -copy[nRow][nCol];
				copy[nRow] = copy[nRow] + copy[nCol] * scale;
    			inv[nRow] = inv[nRow] + inv[nCol] * scale;
    		}
    	}
	}

	(*this) = inv; 
}

//////////////////////////////////////////////////////////////////////
// function operator==(const CMatrix<DIM, TYPE>&, 
//		const CMatrix<DIM, TYPE>&)
//
// exact matrix equality
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator==(const CMatrix<DIM, TYPE>& mLeft, 
					   const CMatrix<DIM, TYPE>& mRight)
{
	// element-by-element comparison
	for (int nRow = 0; nRow < DIM; nRow++)
		for (int nCol = 0; nCol < DIM; nCol++)
			if (mLeft[nRow][nCol] != mRight[nRow][nCol])
				return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// function operator==(const CMatrix<DIM, TYPE>&, 
//		const CMatrix<DIM, TYPE>&)
//
// exact matrix inequality
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator!=(const CMatrix<DIM, TYPE>& mLeft, 
					   const CMatrix<DIM, TYPE>& mRight)
{
	return !(mLeft == mRight);
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrix<DIM, TYPE>&, 
//		const CVector<DIM, TYPE>&)
//
// matrix-vector multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(const CMatrix<DIM, TYPE>& mat,
									const CVector<DIM, TYPE>& v)
{
	// stored the product
	CVector<DIM, TYPE> vProduct;

	// step through the rows
	for (int nRow = 0; nRow < DIM; nRow++)
	{
		vProduct[nRow] = 0.0;
		for (int nMid = 0; nMid < DIM; nMid++)
		{
			vProduct[nRow] +=
				mat[nRow][nMid] * v[nMid];
		}
	}

	// return the product
	return vProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrix<DIM, TYPE>&, 
//		const CMatrix<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> operator*(const CMatrix<DIM, TYPE>& mLeft, 
									const CMatrix<DIM, TYPE>& mRight)
{
	// create the product
	CMatrix<DIM, TYPE> mProduct = mLeft;

	// use in-place multiplication
	mProduct *= mRight;

	// return the product
	return mProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator<<
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CMatrix<DIM, TYPE> m)
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
		ar << m[nAt];

	// return the archive object
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>
//
// matrix serialization
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CMatrix<DIM, TYPE>& m)
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
		ar >> m[nAt];

	// return the archive object
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function CreateTranslate
//
// creates a translation matrix from a vector and a scalar
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateTranslate(const double& s, const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<4> mTranslate;

	mTranslate[0][3] = s * vAxis[0];
	mTranslate[1][3] = s * vAxis[1];
	mTranslate[2][3] = s * vAxis[2];

	return mTranslate;
}

//////////////////////////////////////////////////////////////////////
// function CreateTranslate
//
// creates a translation matrix from an offset vector
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateTranslate(const CVector<3>& vAxis)
{
	return CreateTranslate(1.0, vAxis);
}

//////////////////////////////////////////////////////////////////////
// function CreateRotate
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateRotate(const double& theta, 
							   const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<4> mRotate;

	// normalize the axis
	CVector<3> vNormAxis = vAxis;
	vNormAxis.Normalize();

	// now compute the rotation matrix

	// angle helper values
	double st = sin(theta);
	double ct = cos(theta);
	double vt = 1.0 - cos(theta);

	mRotate[0][0] = vNormAxis[0] * vNormAxis[0] * vt + ct;
	mRotate[0][1] = vNormAxis[0] * vNormAxis[1] * vt - vNormAxis[2] * st;
	mRotate[0][2] = vNormAxis[0] * vNormAxis[2] * vt + vNormAxis[1] * st;

	mRotate[1][0] = vNormAxis[0] * vNormAxis[1] * vt + vNormAxis[2] * st;
	mRotate[1][1] = vNormAxis[1] * vNormAxis[1] * vt + ct;
	mRotate[1][2] = vNormAxis[1] * vNormAxis[2] * vt - vNormAxis[0] * st;

	mRotate[2][0] = vNormAxis[0] * vNormAxis[2] * vt - vNormAxis[1] * st;
	mRotate[2][1] = vNormAxis[1] * vNormAxis[2] * vt + vNormAxis[0] * st;
	mRotate[2][2] = vNormAxis[2] * vNormAxis[2] * vt + ct;

	return mRotate;
}

//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a scaling matrix from a vector whose element's lengths 
//		are scale factors
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateScale(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<4> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];
	mScale[2][2] = vScale[2];

	return mScale;
}

//////////////////////////////////////////////////////////////////////
// function Invert
//
// stand-alone matrix inversion
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> Invert(const CMatrix<DIM, TYPE>& m)
{
	CMatrix<DIM, TYPE> inv(m);
	inv.Invert();
	return inv;
}

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(operator+, CMatrix<4>)
// FUNCTION_FACTORY2(operator-, CMatrix<4>)
// FUNCTION_FACTORY2(operator*, CMatrix<4>)
FUNCTION_FACTORY2_ARG(operator*, CMatrix<4>, CMatrix<4>, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix creation from parameters
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(CreateRotate, CMatrix<4>)
FUNCTION_FACTORY1(CreateTranslate, CMatrix<4>)
FUNCTION_FACTORY2(CreateTranslate, CMatrix<4>)
FUNCTION_FACTORY1(CreateScale, CMatrix<4>)

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix inversion
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY1(Invert, CMatrix<4>)

#endif