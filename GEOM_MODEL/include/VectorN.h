//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVector template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORN_H)
#define VECTORN_H

#include <math.h>

#include "ScalarFunction.h"

//////////////////////////////////////////////////////////////////////
// macros to assist in approximate evaluation of vectors
//////////////////////////////////////////////////////////////////////
#undef EPS
#define EPS (1e-6)

#undef IS_APPROX_EQUAL
#define IS_APPROX_EQUAL(v1, v2) \
	(fabs(v1- v2) < (EPS))

//////////////////////////////////////////////////////////////////////
// class CVectorN<TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CVectorN
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVectorN() 
		: m_nDim(0),
			m_pElements(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////
	// construct from one element
	//////////////////////////////////////////////////////////////////
	CVectorN(TYPE x) 
		: m_nDim(0),
			m_pElements(NULL)
	{
		// set the dimensionality of the vector
		SetDim(1);

		// set the given elements
		m_arrElements[0] = x;
	}

	//////////////////////////////////////////////////////////////////
	// construct from two elements
	//////////////////////////////////////////////////////////////////
	CVectorN(TYPE x, TYPE y) 
		: m_nDim(0),
			m_pElements(NULL)
	{
		// set the dimensionality of the vector
		SetDim(2);

		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;
	}

	//////////////////////////////////////////////////////////////////
	// construct from three elements
	//////////////////////////////////////////////////////////////////
	CVectorN(TYPE x, TYPE y, TYPE z) 
		: m_nDim(0),
			m_pElements(NULL)
	{
		// set the dimensionality of the vector
		SetDim(3);

		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;
	}

	//////////////////////////////////////////////////////////////////
	// construct from four elements
	//////////////////////////////////////////////////////////////////
	CVectorN(TYPE x, TYPE y, TYPE z, TYPE w) 
	{ 
		// set the dimensionality of the vector
		SetDim(4);

		// set the given elements
		m_arrElements[0] = x;
		m_arrElements[1] = y;
		m_arrElements[2] = z;
		m_arrElements[3] = w;
	}

#ifdef _WINDOWS
	//////////////////////////////////////////////////////////////////
	// construct from a windows CPoint
	//////////////////////////////////////////////////////////////////
	CVectorN(const CPoint& pt)
	{
		// set the dimensionality of the vector
		SetDim(2);

		// set the given elements
		m_arrElements[0] = pt.x;
		m_arrElements[1] = pt.y;
	}
#endif

	//////////////////////////////////////////////////////////////////
	// GetDim -- returns the dimensionality of the vector
	//////////////////////////////////////////////////////////////////
	int GetDim()
	{
		return m_nDim;
	}

	//////////////////////////////////////////////////////////////////
	// SetDim -- sets the dimensionality of this vector
	//////////////////////////////////////////////////////////////////
	void SetDim(int nDim)
	{
		// free the elements, if needed
		if (m_pElements != NULL)
		{
			delete [] m_pElements;
			m_pElements = NULL;
		}

		// set the new dimensionality
		m_nDim = nDim;

		// allocate new elements, if needed
		if (nDim > 0)
			m_pElements = new TYPE[m_nDim];
	}

	//////////////////////////////////////////////////////////////////
	// bracket operator -- returns a reference to the specified 
	//		element.
	//////////////////////////////////////////////////////////////////
	TYPE& operator[](int nAtRow)
	{
		ASSERT(nAtRow >= 0 && nAtRow < GetDim());
		return m_arrElements[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// const bracket operator -- returns a const reference to the 
	//		specified element.
	//////////////////////////////////////////////////////////////////
	const TYPE& operator[](int nAtRow) const
	{
		ASSERT(nAtRow >= 0 && nAtRow < GetDim());
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
	BOOL IsApproxEqual(const CVectorN& v, TYPE epsilon = EPS) const
	{
		return ((*this - v).GetLength() < epsilon);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place vector addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CVectorN& operator+=(const CVectorN& vRight)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
			(*this)[nAt] += vRight[nAt];

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place vector subtraction; returns a reference 
	//		to this
	//////////////////////////////////////////////////////////////////
	CVectorN& operator-=(const CVectorN& vRight)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
			(*this)[nAt] -= vRight[nAt];

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place scalar multiplication; returns a 
	//		reference to this
	//////////////////////////////////////////////////////////////////
	CVectorN& operator*=(TYPE scalar)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
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
		for (int nAt = 0; nAt < GetDim(); nAt++)
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

		for (int nAt = 0; nAt < GetDim(); nAt++)
			m_arrElements[nAt] /= len;
	}

private:
	// the dimesions of the vector
	int m_nDim;

	// the elements of the vector
	TYPE *m_pElements;
};

//////////////////////////////////////////////////////////////////////
// function operator==(const CVector, const CVector)
//
// friend function to provide equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator==(const CVectorN<TYPE>& vLeft, 
					   const CVectorN<TYPE>& vRight)
{
	for (int nAt = 0; nAt < vLeft.GetLength(); nAt++)
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
template<class TYPE>
inline bool operator!=(const CVectorN<TYPE>& vLeft, 
					   const CVectorN<TYPE>& vRight)
{
	return !(vLeft == vRight);
}

//////////////////////////////////////////////////////////////////////
// function operator+(const CVector, const CVector)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> operator+(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	CVectorN<TYPE> vSum = vLeft;
	vSum += vRight;
	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator-(const CVector, const CVector)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> operator-(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	CVectorN<TYPE> vSum = vLeft;
	vSum -= vRight;
	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVector, const CVector)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE operator*(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	TYPE prod = 0.0;
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
		prod += vLeft[nAt] * vRight[nAt];

	return prod;
}

//////////////////////////////////////////////////////////////////////
// function operator*(TYPE scalar, const CVector)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> operator*(TYPE scalar, 
							  const CVectorN<TYPE>& vRight)
{
	CVectorN<TYPE> vProd = vRight;
	vProd *= scalar;
	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVector, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> operator*(const CVectorN<TYPE>& vLeft, 
							  TYPE scalar)
{
	CVectorN<TYPE> vProd = vLeft;
	vProd *= scalar;
	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function Cross(const CVector<3>&, const CVector<3>&)
//
// friend function for vector cross product of 3-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorN<TYPE> Cross(const CVectorN<TYPE>& vLeft, 
						      const CVectorN<TYPE>& vRight)
{
	ASSERT(vLeft.GetDim() == 3);
	ASSERT(vRight.GetDim() == 3);

	CVectorN<TYPE> vProd;
	vProd.SetDim(3);

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function operator<<(CArchive &ar, CVector)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator<<(CArchive &ar, CVectorN<TYPE> v)
{
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
		ar << v[nAt];
	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>(CArchive &ar, CVector)
//
// CArchive de-serialization of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator>>(CArchive &ar, CVectorN<TYPE>& v)
{
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
		ar >> v[nAt];
	return ar;
}

#endif	// !defined(VECTOR_H)