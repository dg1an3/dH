//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVector template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

#include "MathUtil.h"

#include "VectorBase.h"


//////////////////////////////////////////////////////////////////////
// class CVector<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CVector : public CVectorBase<TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVector() 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// initialize the elements to 0.0
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// construct from one element
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		(*this)[0] = x;

		// initialize the other elements to 0.0
		for (int nAt = 1; nAt < DIM; nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// construct from two elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		(*this)[0] = x;
		(*this)[1] = y;

		// initialize the other elements to 0.0
		for (int nAt = 2; nAt < DIM; nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// construct from three elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y, TYPE z) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		(*this)[0] = x;
		(*this)[1] = y;
		(*this)[2] = z;

		// initialize the other elements to 0.0
		for (int nAt = 3; nAt < DIM; nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// construct from four elements
	//////////////////////////////////////////////////////////////////
	CVector(TYPE x, TYPE y, TYPE z, TYPE w) 
	{ 
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		(*this)[0] = x;
		(*this)[1] = y;
		(*this)[2] = z;
		(*this)[3] = w;

		// initialize the other elements to 0.0
		for (int nAt = 4; nAt < DIM; nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CVector(const CVector& vFrom) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		(*this) = vFrom;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor from base class
	//////////////////////////////////////////////////////////////////
	explicit CVector(const CVectorBase<TYPE>& vFrom) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = 
				(nAt < vFrom.GetDim()) ? vFrom[nAt] : (TYPE) 0.0;
		}
	}

#ifdef _WINDOWS
	//////////////////////////////////////////////////////////////////
	// construct from a windows CPoint
	//////////////////////////////////////////////////////////////////
	CVector(const CPoint& pt)
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		// set the given elements
		(*this)[0] = pt.x;
		(*this)[1] = pt.y;

		// initialize the other elements to 0.0
		for (int nAt = 2; nAt < DIM; nAt++)
			(*this)[nAt] = (TYPE) 0.0;
	}
#endif

	~CVector()
	{
		// change the pointer so the base class's destructor won't
		//		try to free it
		m_pElements = NULL;
	}

#ifdef _WINDOWS
	//////////////////////////////////////////////////////////////////
	// Windows CPoint conversion 
	//////////////////////////////////////////////////////////////////
	operator CPoint() const
	{
		return CPoint((int)(*this)[0], (int)(*this)[1]);
	}
#endif

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CVector& operator=(const CVector& vFrom)
	{
		ASSERT(m_pElements == &m_arrElements[0]);

		// set the given elements
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = vFrom[nAt];
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CVector& v, TYPE epsilon = EPS) const
	{
		// form the difference vector
		CVector vDiff(*this);
		vDiff -= v;

		return (vDiff.GetLength() < epsilon);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place vector addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CVector& operator+=(const CVector& vRight)
	{
		CVectorBase<TYPE>::operator+=((const CVectorBase<TYPE>&)vRight);
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place vector subtraction; returns a reference 
	//		to this
	//////////////////////////////////////////////////////////////////
	CVector& operator-=(const CVector& vRight)
	{
		CVectorBase<TYPE>::operator-=((const CVectorBase<TYPE>&)vRight);
		return (*this);
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
	return operator==((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
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
	return operator!=((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
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
	return operator*((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
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
	{
		vh[nAt] = v[nAt];
	}

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
	{
		v[nAt] = vh[nAt] / vh[DIM];
	}

	return v;
}

#ifdef _WINDOWS

//////////////////////////////////////////////////////////////////////
// function operator<<(CArchive &ar, CVector)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CVector<DIM, TYPE> v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << v[nAt];
	}

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
	{
		ar >> v[nAt];
	}

	return ar;
}

#endif

//////////////////////////////////////////////////////////////////////
// class CPackedVector<DIM, TYPE>
//
// helper class for a packed static vector
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CPackedVector
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CPackedVector()
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CPackedVector(const CVector<DIM, TYPE>& vFrom)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = vFrom[nAt];
	}

	//////////////////////////////////////////////////////////////////
	// cast to CVector
	//////////////////////////////////////////////////////////////////
	operator CVector<DIM, TYPE>() const
	{
		CVector<DIM, TYPE> vConv;
		for (int nAt = 0; nAt < DIM; nAt++)
			vConv[nAt] = m_arrElements[nAt];

		return vConv;
	}

	//////////////////////////////////////////////////////////////////
	// cast to TYPE*
	//////////////////////////////////////////////////////////////////
	operator TYPE*()
	{
		return &m_arrElements[0];
	}

protected:
	// the vector's elements
	TYPE m_arrElements[DIM];
};

#endif	// !defined(VECTOR_H)