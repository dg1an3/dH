//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVector template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

#include <math.h>

#include "ScalarFunction.h"

//////////////////////////////////////////////////////////////////////
// macros to assist in approximate evaluation of vectors
//////////////////////////////////////////////////////////////////////
#define EPS (1e-6)
#define IS_APPROX_EQUAL(v1, v2) \
	(fabs(v1- v2) < (EPS))

//////////////////////////////////////////////////////////////////////
// class CVector<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CVector
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVector() 
	{
		// initialize the elements to 0.0
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// construct from one element
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x) 
	{
		// set the given elements
		m_arrElements[0] = x;

		// initialize the other elements to 0.0
		for (int nAt = 1; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// construct from two elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y) 
	{
		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;

		// initialize the other elements to 0.0
		for (int nAt = 2; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// construct from three elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y, TYPE z) 
	{
		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;

		// initialize the other elements to 0.0
		for (int nAt = 3; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// construct from four elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y, TYPE z, TYPE w) 
	{ 
		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;
		m_arrElements[3] = w;

		// initialize the other elements to 0.0
		for (int nAt = 4; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}

#ifdef _WINDOWS
	//////////////////////////////////////////////////////////////////
	// construct from a windows CPoint
	//////////////////////////////////////////////////////////////////
	CVector(const CPoint& pt)
	{
		// set the given elements
		m_arrElements[0] = pt.x;
		m_arrElements[1] = pt.y;

		// initialize the other elements to 0.0
		for (int nAt = 2; nAt < DIM; nAt++)
			m_arrElements[nAt] = (TYPE) 0.0;
	}
#endif

	//////////////////////////////////////////////////////////////////
	// bracket operator -- returns a reference to the specified 
	//		element.
	//////////////////////////////////////////////////////////////////
	TYPE& operator[](int nAtRow)
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrElements[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// const bracket operator -- returns a const reference to the 
	//		specified element.
	//////////////////////////////////////////////////////////////////
	const TYPE& operator[](int nAtRow) const
	{
		ASSERT(nAtRow >= 0 && nAtRow < DIM);
		return m_arrElements[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// TYPE * conversion -- returns a pointer to the first element
	//		BEWARE: this allows for no-bounds-checking access
	//////////////////////////////////////////////////////////////////
	operator TYPE *()
	{
		return &m_arrElements[0];
	}

	//////////////////////////////////////////////////////////////////
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		BEWARE: this allows for no-bounds-checking access
	//////////////////////////////////////////////////////////////////
	operator const TYPE *() const
	{
		return &m_arrElements[0];
	}

#ifdef _WINDOWS
	//////////////////////////////////////////////////////////////////
	// Windows CPoint conversion 
	//////////////////////////////////////////////////////////////////
	operator CPoint()
	{
		return CPoint((int)(*this)[0], (int)(*this)[1]);
	}
#endif

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CVector& v) const
	{
		return ((*this - v).GetLength() < EPS);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place vector addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CVector& operator+=(const CVector& vRight)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] += vRight[nAt];

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place vector subtraction; returns a reference 
	//		to this
	//////////////////////////////////////////////////////////////////
	CVector& operator-=(const CVector& vRight)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] -= vRight[nAt];

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place scalar multiplication; returns a 
	//		reference to this
	//////////////////////////////////////////////////////////////////
	CVector& operator*=(TYPE scalar)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			(*this)[nAt] *= scalar;

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// GetLength -- returns the euclidean length of the vector
	//////////////////////////////////////////////////////////////////
	TYPE GetLength() const
	{
		// form the sum of the square of each element
		TYPE len = 0.0;
		for (int nAt = 0; nAt < DIM; nAt++)
			len += (*this)[nAt] * (*this)[nAt];
		
		// return the square root of this
		return static_cast<TYPE>(sqrt(len));
	}

	//////////////////////////////////////////////////////////////////
	// Normalize -- scales the vector so its length is 1.0
	//////////////////////////////////////////////////////////////////
	void Normalize()
	{
		TYPE len = GetLength();

		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] /= len;
	}

private:
	// the vector's elements
	TYPE m_arrElements[DIM];
};

//////////////////////////////////////////////////////////////////////
// function operator==(const CVector, const CVector)
//
// friend function to provide equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator==(const CVector<DIM, TYPE>& vLeft, 
					   const CVector<DIM, TYPE>& vRight)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		if (vLeft[nAt] != vRight[nAt])
			return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// function operator!=(const CVector, const CVector)
//
// friend function to provide inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator!=(const CVector<DIM, TYPE>& vLeft, 
					   const CVector<DIM, TYPE>& vRight)
{
	return !(vLeft == vRight);
}

//////////////////////////////////////////////////////////////////////
// function operator+(const CVector, const CVector)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator+(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vSum = vLeft;
	vSum += vRight;
	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator-(const CVector, const CVector)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator-(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;
	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVector, const CVector)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline TYPE operator*(const CVector<DIM, TYPE>& vLeft, 
							  const CVector<DIM, TYPE>& vRight)
{
	TYPE prod = 0.0;
	for (int nAt = 0; nAt < DIM; nAt++)
		prod += vLeft[nAt] * vRight[nAt];

	return prod;
}

//////////////////////////////////////////////////////////////////////
// function operator*(TYPE scalar, const CVector)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(TYPE scalar, 
							  const CVector<DIM, TYPE>& vRight)
{
	CVector<DIM, TYPE> vProd = vRight;
	vProd *= scalar;
	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVector, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> operator*(const CVector<DIM, TYPE>& vLeft, 
							  TYPE scalar)
{
	CVector<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;
	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function Cross(const CVector<3>&, const CVector<3>&)
//
// friend function for vector cross product of 3-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVector<3, TYPE> Cross(const CVector<3, TYPE>& vLeft, 
						      const CVector<3, TYPE>& vRight)
{
	CVector<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function Cross(const CVector<2>&, const CVector<2>&)
//
// friend function for vector cross product of 2-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Cross(const CVector<2, TYPE>& vLeft, 
					const CVector<2, TYPE>& vRight)
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];
}

//////////////////////////////////////////////////////////////////////
// function ToHomogeneous(const CVector&)
//
// converts an N-dimensional vector to an N+1-dimensional homogeneous
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM + 1, TYPE> ToHomogeneous(const CVector<DIM, TYPE>& v) 
{
	// create the homogeneous vector
	CVector<DIM + 1, TYPE> vh;

	// fill with the given vector's elements
	for (int nAt = 0; nAt < DIM; nAt++)
		vh[nAt] = v[nAt];

	// set last element to 1.0
	vh[DIM] = (TYPE) 1.0;

	return vh;
}

//////////////////////////////////////////////////////////////////////
// function FromHomogeneous(const CVector&)
//
// converts an N+1-dimensional homogeneous vector to an N-dimensional 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVector<DIM, TYPE> FromHomogeneous(const CVector<DIM + 1, TYPE>& vh) 
{
	// create the non-homogeneous vector
	CVector<DIM, TYPE> v;

	// normalize the first DIM-1 elements by the last element
	for (int nAt = 0; nAt < DIM; nAt++)
		v[nAt] = vh[nAt] / vh[DIM];

	return v;
}

//////////////////////////////////////////////////////////////////////
// function operator<<(CArchive &ar, CVector)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CVector<DIM, TYPE> v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar << v[nAt];
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>(CArchive &ar, CVector)
//
// CArchive de-serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CVector<DIM, TYPE>& v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
		ar >> v[nAt];
	return ar;
}

//////////////////////////////////////////////////////////////////////
// declare function factories for vector arithmetic
//////////////////////////////////////////////////////////////////////
FUNCTION_FACTORY2(operator+, CVector<4>)
// FUNCTION_FACTORY2(operator-, CVector<4>)
// FUNCTION_FACTORY2(operator*, CVector<4>)

//////////////////////////////////////////////////////////////////////
// macro TRACE_VECTOR3
//
// helper macro to output a vector for debugging
//////////////////////////////////////////////////////////////////////
#define TRACE_VECTOR3(message, vector) \
	TRACE1("%s = ", message); \
	TRACE3("<%lf, %lf, %lf>\n", (vector)[0], (vector)[1], (vector)[2]);

#endif	// !defined(VECTOR_H)