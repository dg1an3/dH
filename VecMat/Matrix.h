//////////////////////////////////////////////////////////////////////
// Matrix.h: declaration and definition of the CMatrix template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIX_H)
#define MATRIX_H

#include <MathUtil.h>

#include "MatrixBase.h"
#include "Vector.h"

//////////////////////////////////////////////////////////////////////
// class CMatrix<DIM, TYPE>
//
// represents a square matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CMatrix : public CMatrixBase<TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- initializes to an identity matrix
	//////////////////////////////////////////////////////////////////
	CMatrix()
	{
		// initialize pointers
		m_ppRows = (CVectorBase<TYPE> **)&m_pRows;
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			m_ppRows[nAt] = &m_arrRows[nAt];
		}

		// populate as an identity matrix
		SetIdentity();
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CMatrix(const CMatrix& fromMatrix)
	{
		// initialize pointers
		m_ppRows = (CVectorBase<TYPE> **)&m_pRows;
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			m_ppRows[nAt] = &m_arrRows[nAt];
		}

		// populate from other matrix
		(*this) = fromMatrix;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	explicit CMatrix(const CMatrixBase<TYPE>& fromMatrix)
	{
		// initialize pointers
		m_ppRows = (CVectorBase<TYPE> **)&m_pRows;
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			m_ppRows[nAt] = &m_arrRows[nAt];
		}

		// set to identity (for partial fills)
		SetIdentity();

		// populate from other matrix
		for (nAt = 0; nAt < __min(GetDim(), fromMatrix.GetDim()); nAt++)
		{
			(*this)[nAt] = CVector<DIM, TYPE>(fromMatrix[nAt]);
		}
	}

	//////////////////////////////////////////////////////////////////
	// destructor -- frees the row vectors
	//////////////////////////////////////////////////////////////////
	~CMatrix()
	{
		// prevent base class from trying to free
		m_ppRows = NULL;
	}

	//////////////////////////////////////////////////////////////////
	// assignment operator
	//////////////////////////////////////////////////////////////////
	CMatrix& operator=(const CMatrix& fromMatrix)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			(*this)[nAt] = fromMatrix[nAt];
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator[] -- retrieves a reference to a row vector
	//////////////////////////////////////////////////////////////////
	CVector<DIM, TYPE>& operator[](int nAtRow)
	{
		// return a reference to the row vector
		return m_arrRows[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// operator[] const -- retrieves a const reference to a row 
	//		vector
	//////////////////////////////////////////////////////////////////
	const CVector<DIM, TYPE>& operator[](int nAtRow) const
	{
		// return a reference to the row vector
		return m_arrRows[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CMatrix& m, TYPE epsilon = EPS) const
	{
		return CMatrixBase<TYPE>::IsApproxEqual(
			(const CMatrixBase<TYPE>&) m, epsilon);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place matrix addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrix& operator+=(const CMatrix& mRight)
	{
		// element-by-element sum of the matrix
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
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
	CMatrix& operator-=(const CMatrix& mRight)
	{
		// element-by-element difference of the matrix
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
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
	CMatrix& operator*=(double scale)
	{
		// element-by-element difference of the matrix
		for (int nRow = 0; nRow < DIM; nRow++)
		{
			for (int nCol = 0; nCol < DIM; nCol++)
			{
				(*this)[nRow][nCol] *= scale;
			}
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place matrix multiplication; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CMatrix& operator*=(const CMatrix& mRight)
	{
		// assign final product to this
		(*this) = (*this) * mRight;

		// return a reference to this
		return (*this);
	}

private:
	// row pointers
	CVector<DIM, TYPE> *m_pRows[DIM];

	// rows
	CVector<DIM, TYPE> m_arrRows[DIM];
};

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
	// call CMatrixBase version
	return operator==((const CMatrixBase<TYPE>&) mLeft,
		(const CMatrixBase<TYPE>&) mRight);
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
	// call CMatrixBase version
	return operator!=((const CMatrixBase<TYPE>&) mLeft,
		(const CMatrixBase<TYPE>&) mRight);
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
		ASSERT(vProduct[nRow] == 0.0);
		for (int nMid = 0; nMid < DIM; nMid++)
		{
			vProduct[nRow] += mat[nRow][nMid] * v[nMid];
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
	// holds the final product
	CMatrix<DIM, TYPE> mProduct;

	// compute the matrix product
	for (int nRow = 0; nRow < DIM; nRow++)
	{
		for (int nCol = 0; nCol < DIM; nCol++)
		{
			mProduct[nRow][nCol] = 0.0;
			for (int nMid = 0; nMid < DIM; nMid++)
			{
				mProduct[nRow][nCol] +=
					mLeft[nRow][nMid] * mRight[nMid][nCol];
			}
		}
	}

	// return the product
	return mProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrix<DIM, TYPE>&, 
//		const CMatrix<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> operator*(const CMatrix<DIM, TYPE>& mLeft, 
									TYPE scalar)
{
	// create the product
	CMatrix<DIM, TYPE> mProduct(mLeft);

	// use in-place multiplication
	mProduct *= scalar;

	// return the product
	return mProduct;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CMatrix<DIM, TYPE>&, 
//		const CMatrix<DIM, TYPE>&)
//
// matrix multiplication
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> operator*(TYPE scalar, 
									const CMatrix<DIM, TYPE>& mRight)
{
	// create the product
	CMatrix<DIM, TYPE> mProduct(mLeft);

	// use in-place multiplication
	mProduct *= scalar;

	// return the product
	return mProduct;
}

#ifdef _WINDOWS

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
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CMatrix<DIM, TYPE>& m)
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar >> m[nAt];
	}

	// return the archive object
	return ar;
}

#endif 

//////////////////////////////////////////////////////////////////////
// function CreateTranslate
//
// creates a 3x3 homogeneous translation matrix from a vector and 
//		a scalar for a 2D translation
//////////////////////////////////////////////////////////////////////
inline CMatrix<3> CreateTranslate(const double& s, 
								  const CVector<2>& vAxis)
{
	// start with an identity matrix
	CMatrix<3> mTranslate;

	mTranslate[0][2] = s * vAxis[0];
	mTranslate[1][2] = s * vAxis[1];

	return mTranslate;
}

//////////////////////////////////////////////////////////////////////
// function CreateTranslate
//
// creates a 4x4 homogeneous translation matrix from a vector and 
//		a scalar for a 3D translation
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateTranslate(const double& s, 
								  const CVector<3>& vAxis)
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
// creates a 2D rotation matrix given an angle
//////////////////////////////////////////////////////////////////////
inline CMatrix<2> CreateRotate(const double& theta)
{
	// start with an identity matrix
	CMatrix<2> mRotate;

	// now compute the rotation matrix

	// angle helper values
	double st = sin(theta);
	double ct = cos(theta);

	mRotate[0][0] = ct;
	mRotate[0][1] = - st;

	mRotate[1][0] = st;
	mRotate[1][1] = ct;

	return mRotate;
}

//////////////////////////////////////////////////////////////////////
// function CreateRotate
//
// creates a 3D rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
inline CMatrix<3> CreateRotate(const double& theta, 
							   const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<3> mRotate;

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
// function CreateRotate
//
// creates a 3D rotation matrix given two positions: a pre-rotated
//		position and a post-rotated position. scale is a scale factor
//		for the resulting rotation.
//////////////////////////////////////////////////////////////////////
inline CMatrix<3> CreateRotate(const CVector<3>& vInitPt,
	const CVector<3>& vFinalPt, double scale)
{
	// compute the length of the two legs of the triangle (a & b)
	double a = vInitPt.GetLength(); 
	double b = vFinalPt.GetLength();
	
	// compute the length of the segment connecting the initial and final
	//		point (c)
	double c = (vFinalPt - vInitPt).GetLength();

	// compute the angle theta using the law of cosines
	double theta = 0.0;
	if (a * b != 0.0)
	{
		theta = (2.0 * PI - acos((a * a + b * b - c * c) 
			/ (2.0 * a * b)));
	}

	// compute the axis of rotation = normalized cross product of 
	//		initial and current drag points
	CVector<3> u = Cross(vFinalPt, vInitPt);
	u.Normalize();

	// now compute the rotation matrix
	return CreateRotate(scale * theta, u);
}


//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a scaling matrix from a vector whose element's lengths 
//		are scale factors
//////////////////////////////////////////////////////////////////////
inline CMatrix<2> CreateScale(const CVector<2>& vScale)
{
	// start with an identity matrix
	CMatrix<2> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];

	return mScale;
}

//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a scaling matrix from a vector whose element's lengths 
//		are scale factors
//////////////////////////////////////////////////////////////////////
inline CMatrix<3> CreateScale(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<3> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];
	mScale[2][2] = vScale[2];

	return mScale;
}

//////////////////////////////////////////////////////////////////////
// function CreateProjection
//
// creates a projection matrix from a near-plane and a far-plane
//////////////////////////////////////////////////////////////////////
inline CMatrix<4> CreateProjection(const double& n, const double& f)
{
	CMatrix<4> mProj;

	// set the near plane elements
	mProj[0][0] = n;
	mProj[1][1] = n;

	// set the ratio elements
	mProj[2][2] = -(f + n)	  / (f - n);
	mProj[2][3] = -2.0 * f * n / (f - n);

	// set the projection elements
	mProj[3][2] = -1.0;
	mProj[3][3] =  0.0;

	// return the created matrix
	return mProj;
}


//////////////////////////////////////////////////////////////////////
// function Eigenvector
//
// computes the eigenvector of a 2x2 matrix, or returns <0.0, 0.0> if
//		no real-valued eigenvectors exist
// order is either 1 or 2, depending on whether the principle or
//		remaining eigenvector is wanted
//////////////////////////////////////////////////////////////////////
inline REAL Eigen(CMatrix<2> m, int nOrder = 1, 
				  CVector<2> *pVector = NULL)
{
	// compute factors of the characteristic quadratic equation
	REAL b = -(m[0][0] + m[1][1]);
	REAL c = m[0][0] * m[1][1] - m[0][1] * m[1][0];

	// check to see if characteristic equation has real roots
	if (b * b < 4.0 * c)
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// and find the roots of the equation
	REAL r1 = -(b + sqrt(b * b - 4.0 * c)) / 2.0;
	REAL r2 = -(b - sqrt(b * b - 4.0 * c)) / 2.0;

	// choose root which is either largest or smallest, depending on order
	REAL r = ((fabs(r1) > fabs(r2)) && (nOrder <= 1)) 
		? r1 : r2;

	// find the larger off-diagonal element
	if (fabs(m[0][1]) > fabs(m[1][0]))
	{
		// and solve for the vector 
		(*pVector)[0] = 1.0;
		(*pVector)[1] = (r - m[0][0]) / m[0][1];
	}
	// otherwise use the other element if it is non-zero
	else if (fabs(m[1][0]) > 0.0)
	{
		// and solve for the vector 
		(*pVector)[0] = (r - m[1][1]) / m[1][0];
		(*pVector)[1] = 1.0;
	}
	// otherwise,
	else
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// normalize the computed eigenvector
	pVector->Normalize();

#ifdef _DEBUG
	// assert that our eigenvector is really an eigenvector
	CVector<2> vLeft = m * (*pVector);
	CVector<2> vRight = r * (*pVector);

	ASSERT(vLeft.IsApproxEqual(vRight));
#endif

	// return the computed eigenvalue
	return r;
}
#endif