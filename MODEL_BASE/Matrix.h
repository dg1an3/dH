#if !defined(MATRIX_H)
#define MATRIX_H

#include <math.h>
#include "Vector.h"

template<int DIM = 4, class TYPE = double>
class CMatrix
{
public:
	CMatrix()
	{
		SetIdentity();
	}

	CVector<DIM, TYPE>& operator[](int nAtRow)
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrRows[nAtRow];
	}

	const CVector<DIM, TYPE>& operator[](int nAtRow) const
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrRows[nAtRow];
	}

	CMatrix& operator+=(const CMatrix& mRight)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nMid] += mRight[nMid][nCol];

		return (*this);
	}

	CMatrix& operator-=(const CMatrix& mRight)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
			{
				(*this)[nRow][nMid] -= mRight[nMid][nCol];
			}
		}

		return (*this);
	}

	CMatrix& operator*=(const CMatrix& mRight)
	{
		CMatrix mProduct;

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

		(*this) = mProduct;

		return (*this);
	}

	void SetIdentity()
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nCol] = (nRow == nCol) ? 1.0 : 0.0;
	}

	// fills an OpenGL-format double array from this matrix
	void ToArray(double *pArray) const
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				pArray[nCol * DIM + nRow] = (*this)[nRow][nCol];
	}

	// fills this matrix from an OpenGL-format double array
	void FromArray(const double *pArray)
	{
		for (int nRow = 0; nRow < DIM; nRow++)
			for (int nCol = 0; nCol < DIM; nCol++)
				(*this)[nRow][nCol] = pArray[nCol * DIM + nRow];
	}

	// matrix inversion -- defined below
	void Invert();

protected:

	// helper functions for Invert -- defined below
	void InterchangeRows(int nRow1, int nRow2);
	int FindPivot(int nDiag);

private:
	CVector<DIM> m_arrRows[DIM];
};

template<int DIM, class TYPE>
void CMatrix<DIM, TYPE>::InterchangeRows(int nRow1, int nRow2) 
{
	if (nRow1 != nRow2)
	{
		CVector<DIM, TYPE> vTemp = (*this)[nRow1];	// temporary vector storage
		(*this)[nRow1] = (*this)[nRow2];
		(*this)[nRow2] = vTemp;
	}
}
    
#define MAX_TO_PIVOT (1e-6)

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

template<int DIM, class TYPE>
inline bool operator==(const CMatrix<DIM, TYPE>& mLeft, 
					   const CMatrix<DIM, TYPE>& mRight)
{
	for (int nRow = 0; nRow < DIM; nRow++)
		for (int nCol = 0; nCol < DIM; nCol++)
			if (mLeft[nRow][nCol] != mRight[nRow][nCol])
				return false;

	return true;
}

template<int DIM, class TYPE>
inline bool operator!=(const CMatrix<DIM, TYPE>& mLeft, 
					   const CMatrix<DIM, TYPE>& mRight)
{
	return !(mLeft == mRight);
}

template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(const CMatrix<DIM, TYPE>& mat,
									const CVector<DIM, TYPE>& v)
{
	CVector<DIM, TYPE> vProduct;

	for (int nRow = 0; nRow < DIM; nRow++)
	{
		vProduct[nRow] = 0.0;
		for (int nMid = 0; nMid < DIM; nMid++)
		{
			vProduct[nRow] +=
				mat[nRow][nMid] * v[nMid];
		}
	}

	return vProduct;
}

template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> operator*(const CMatrix<DIM, TYPE>& mLeft, 
									const CMatrix<DIM, TYPE>& mRight)
{
	CMatrix<DIM, TYPE> mProduct = mLeft;

	mProduct *= mRight;

	return mProduct;
}

template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CMatrix<DIM, TYPE> m)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar << m[nAt];
	return ar;
}

template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CMatrix<DIM, TYPE>& m)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar >> m[nAt];
	return ar;
}

#define PI (atan(1.0) * 4.0)

inline CMatrix<4> CreateTranslate(const double& s, const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<4> mTranslate;

	mTranslate[0][3] = s * vAxis[0];
	mTranslate[1][3] = s * vAxis[1];
	mTranslate[2][3] = s * vAxis[2];

	return mTranslate;
}

inline CMatrix<4> CreateTranslate(const CVector<3>& vAxis)
{
	return CreateTranslate(1.0, vAxis);
}

inline CMatrix<4> CreateRotate(const double& theta, const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<4> mRotate; // SetIdentity();

	CVector<3> vNormAxis = vAxis; //(vAxis[0], vAxis[1], vAxis[2]);

	// normalize the axis
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

inline CMatrix<4> CreateScale(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<4> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];
	mScale[2][2] = vScale[2];

	return mScale;
}

template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> Invert(const CMatrix<DIM, TYPE>& m)
{
	CMatrix<DIM, TYPE> inv(m);
	inv.Invert();
	return inv;
}


#endif