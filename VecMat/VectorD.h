//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVectorD template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

#include "MathUtil.h"

#include "VectorBase.h"


//////////////////////////////////////////////////////////////////////
// class CVectorD<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = double>
class CVectorD : public CVectorBase<TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVectorD() 
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
	CVectorD(TYPE x) 
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
	CVectorD(TYPE x, TYPE y) 
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
	CVectorD(TYPE x, TYPE y, TYPE z) 
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
	CVectorD(TYPE x, TYPE y, TYPE z, TYPE w) 
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
	CVectorD(const CVectorD& vFrom) 
	{
		m_nDim = DIM;
		m_pElements = &m_arrElements[0];

		(*this) = vFrom;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor from base class
	//////////////////////////////////////////////////////////////////
	explicit CVectorD(const CVectorBase<TYPE>& vFrom) 
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
	CVectorD(const CPoint& pt)
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

	~CVectorD()
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
	CVectorD& operator=(const CVectorD& vFrom)
	{
		// ASSERT(m_pElements == &m_arrElements[0]);

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
	BOOL IsApproxEqual(const CVectorD& v, TYPE epsilon = EPS) const
	{
		// form the difference vector
		CVectorD vDiff(*this);
		vDiff -= v;

		return (vDiff.GetLength() < epsilon);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place vector addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CVectorD& operator+=(const CVectorD& vRight)
	{
		CVectorBase<TYPE>::operator+=((const CVectorBase<TYPE>&)vRight);
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place vector subtraction; returns a reference 
	//		to this
	//////////////////////////////////////////////////////////////////
	CVectorD& operator-=(const CVectorD& vRight)
	{
		CVectorBase<TYPE>::operator-=((const CVectorBase<TYPE>&)vRight);
		return (*this);
	}

private:
	// the vector's elements
	TYPE m_arrElements[DIM];
};


//////////////////////////////////////////////////////////////////////
// function operator==(const CVectorD, const CVectorD)
//
// friend function to provide equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator==(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
{
	return operator==((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
}

//////////////////////////////////////////////////////////////////////
// function operator!=(const CVectorD, const CVectorD)
//
// friend function to provide inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline bool operator!=(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
{
	return operator!=((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
}

//////////////////////////////////////////////////////////////////////
// function operator+(const CVectorD, const CVectorD)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator+(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum += vRight;

	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator-(const CVectorD, const CVectorD)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator-(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;

	return vSum;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVectorD, const CVectorD)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline TYPE operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	return operator*((const CVectorBase<TYPE>&) vLeft,
		(const CVectorBase<TYPE>&) vRight);
}

//////////////////////////////////////////////////////////////////////
// function operator*(TYPE scalar, const CVectorD)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator*(TYPE scalar, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vProd = vRight;
	vProd *= scalar;

	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function operator*(const CVectorD, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  TYPE scalar)
{
	CVectorD<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;

	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function Cross(const CVectorD<2>&, const CVectorD<2>&)
//
// friend function for vector cross product of 2-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE Cross(const CVectorD<2, TYPE>& vLeft, 
					const CVectorD<2, TYPE>& vRight)
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];
}

//////////////////////////////////////////////////////////////////////
// function Cross(const CVectorD<3>&, const CVectorD<3>&)
//
// friend function for vector cross product of 3-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorD<3, TYPE> Cross(const CVectorD<3, TYPE>& vLeft, 
						      const CVectorD<3, TYPE>& vRight)
{
	CVectorD<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;
}

//////////////////////////////////////////////////////////////////////
// function ToHomogeneous(const CVectorD&)
//
// converts an N-dimensional vector to an N+1-dimensional homogeneous
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM + 1, TYPE> ToHomogeneous(const CVectorD<DIM, TYPE>& v) 
{
	// create the homogeneous vector
	CVectorD<DIM + 1, TYPE> vh;

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
// function FromHomogeneous(const CVectorD&)
//
// converts an N+1-dimensional homogeneous vector to an N-dimensional 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CVectorD<DIM, TYPE> FromHomogeneous(const CVectorD<DIM + 1, TYPE>& vh) 
{
	// create the non-homogeneous vector
	CVectorD<DIM, TYPE> v;

	// normalize the first DIM-1 elements by the last element
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		v[nAt] = vh[nAt] / vh[DIM];
	}

	return v;
}

#ifdef _WINDOWS

//////////////////////////////////////////////////////////////////////
// function operator<<(CArchive &ar, CVectorD)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator<<(CArchive &ar, CVectorD<DIM, TYPE> v)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << v[nAt];
	}

	return ar;
}

//////////////////////////////////////////////////////////////////////
// function operator>>(CArchive &ar, CVectorD)
//
// CArchive de-serialization of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& operator>>(CArchive &ar, CVectorD<DIM, TYPE>& v)
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
class CPackedVectorD
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CPackedVectorD()
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = 0.0;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CPackedVectorD(const CVectorD<DIM, TYPE>& vFrom)
	{
		for (int nAt = 0; nAt < DIM; nAt++)
			m_arrElements[nAt] = vFrom[nAt];
	}

	//////////////////////////////////////////////////////////////////
	// cast to CVectorD
	//////////////////////////////////////////////////////////////////
	operator CVectorD<DIM, TYPE>() const
	{
		CVectorD<DIM, TYPE> vConv;
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