//////////////////////////////////////////////////////////////////////
// MatrixNxM.h: declaration and definition of the CMatrixNxM template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXNXM_H)
#define MATRIXNXM_H

#include "MathUtil.h"

#include "MatrixBase.h"
#include "VectorN.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixNxM<TYPE>
//
// represents a square matrix with GetDim()ension and type given.
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CMatrixNxM : public CMatrixBase<TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- initializes to 0x0 matrix
	//////////////////////////////////////////////////////////////////
	CMatrixNxM()
	{
	}

	//////////////////////////////////////////////////////////////////
	// constructs a specific-dimensioned matrix
	//////////////////////////////////////////////////////////////////
	CMatrixNxM(int nCols, int nRows)
	{
		Reshape(nCols, nRows);
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixNxM(const CMatrixNxM& fromMatrix)
	{
		// sets the dimensions
		Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

		// SetIdentity to fill unoccupied parts of matrix
		SetIdentity();

		// copy the elements
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrixNxM(const CMatrixBase<TYPE>& fromMatrix)
	{
		// sets the dimensions
		Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

		// SetIdentity to fill unoccupied parts of matrix
		SetIdentity();

		// copy the elements
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// assignment operator
	//////////////////////////////////////////////////////////////////
	CMatrixNxM& operator=(const CMatrixBase<TYPE>& fromMatrix)
	{
		// sets the dimensions
		Reshape(fromMatrix.GetCols(), fromMatrix.GetRows());

		// SetIdentity to fill unoccupied parts of matrix
		SetIdentity();

		// copy the elements
		for (int nCol = 0; nCol < GetCols(); nCol++)
		{
			for (int nRow = 0; nRow < GetRows(); nRow++)
			{
				(*this)[nCol][nRow] = fromMatrix[nCol][nRow];
			}
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// Reshape -- sets the dimension of the matrix
	//////////////////////////////////////////////////////////////////
	void Reshape(int nCols, int nRows)
	{
		CMatrixBase<TYPE>::Reshape(nCols, nRows);
	}
};

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

	// serialize the individual row vectors
	for (int nAt = 0; nAt < m.GetCols(); nAt++)
	{
		ar << m[nAt];
	}

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

	// serialize the individual row vectors
	for (int nAt = 0; nAt < m.GetCols(); nAt++)
	{
		ar >> m[nAt];
	}

	// return the archive object
	return ar;
}

#endif

#endif