//////////////////////////////////////////////////////////////////////
// VectorBase.h: declaration and definition of the CVectorBase 
//		template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORBASE_H)
#define VECTORBASE_H

// for iostream support for vectors
#include <iostream>
using namespace std;

// math utilities
#include "MathUtil.h"


//////////////////////////////////////////////////////////////////////
// class CVectorBase<TYPE>
//
// base class for mathematical vectors with element type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = double>
class CVectorBase
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor -- public can not construct a CVectorBase
	//////////////////////////////////////////////////////////////////
	CVectorBase()
		: m_nDim(0),
			m_pElements(NULL),
			m_bFreeElements(TRUE)
	{
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CVectorBase(const CVectorBase& vFrom)
		: m_nDim(0),
			m_pElements(NULL),
			m_bFreeElements(TRUE)
	{
		// set up the elements
		m_nDim = vFrom.GetDim();
		m_pElements = new TYPE[m_nDim];

		// and copy them
		(*this) = vFrom;
	}

	//////////////////////////////////////////////////////////////////
	// destructor 
	//////////////////////////////////////////////////////////////////
	~CVectorBase()
	{
		if (m_bFreeElements && NULL != m_pElements)
		{
			// free elements
			delete [] m_pElements;
		}
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor -- copies arbitrary dimensioned vector to
	//		this, zero-filling or truncating the elements
	//////////////////////////////////////////////////////////////////
	CVectorBase& operator=(const CVectorBase& vFrom)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = 
				(nAt < vFrom.GetDim()) ? vFrom[nAt] : (TYPE) 0.0;
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// SetZero -- sets all elements to zero
	//////////////////////////////////////////////////////////////////
	void SetZero() 
	{
		// zero all elements
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = (TYPE) 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////
	// GetDim -- returns the dimensionality of the vector
	//////////////////////////////////////////////////////////////////
	int GetDim() const
	{
		return m_nDim;
	}

	//////////////////////////////////////////////////////////////////
	// bracket operator -- returns a reference to the specified 
	//		element.
	//////////////////////////////////////////////////////////////////
	TYPE& operator[](int nAtRow)
	{
		// check dimensions
		ASSERT(nAtRow >= 0 && nAtRow < GetDim());

		return m_pElements[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// const bracket operator -- returns a const reference to the 
	//		specified element.
	//////////////////////////////////////////////////////////////////
	const TYPE& operator[](int nAtRow) const
	{
		// check dimensions
		ASSERT(nAtRow >= 0 && nAtRow < GetDim());

		return m_pElements[nAtRow];
	}

	//////////////////////////////////////////////////////////////////
	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	//////////////////////////////////////////////////////////////////
	operator TYPE *()
	{
		return &m_pElements[0];
	}

	//////////////////////////////////////////////////////////////////
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		WARNING: this allows for no-bounds-checking access
	//////////////////////////////////////////////////////////////////
	operator const TYPE *() const
	{
		return &m_pElements[0];
	}

	//////////////////////////////////////////////////////////////////
	// GetLength -- returns the euclidean length of the vector
	//////////////////////////////////////////////////////////////////
	TYPE GetLength() const
	{
		// form the sum of the square of each element
		TYPE len = 0.0;
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			len += (*this)[nAt] * (*this)[nAt];
		}
		
		// return the square root of this
		return static_cast<TYPE>(sqrt(len));
	}

	//////////////////////////////////////////////////////////////////
	// Normalize -- scales the vector so its length is 1.0
	//////////////////////////////////////////////////////////////////
	void Normalize()
	{
		// store the computed lendth
		TYPE len = GetLength();

		// don't normalize zero vectors
		if (len > 0.0)
		{
			// normalize each element by length
			for (int nAt = 0; nAt < GetDim(); nAt++)
			{
				(*this)[nAt] /= len;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	//////////////////////////////////////////////////////////////////
	BOOL IsApproxEqual(const CVectorBase& v, TYPE epsilon = EPS) const
	{
		// form the difference vector
		CVectorBase vDiff(*this);
		vDiff -= v;

		return (vDiff.GetLength() < epsilon);
	}

	//////////////////////////////////////////////////////////////////
	// operator+= -- in-place vector addition; returns a reference to 
	//		this
	//////////////////////////////////////////////////////////////////
	CVectorBase& operator+=(const CVectorBase& vRight)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] += vRight[nAt];
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator-= -- in-place vector subtraction; returns a reference 
	//		to this
	//////////////////////////////////////////////////////////////////
	CVectorBase& operator-=(const CVectorBase& vRight)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] -= vRight[nAt];
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// operator*= -- in-place scalar multiplication; returns a 
	//		reference to this
	//////////////////////////////////////////////////////////////////
	CVectorBase& operator*=(TYPE scalar)
	{
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] *= scalar;
		}

		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// from a TYPE pointer
	//////////////////////////////////////////////////////////////////
	void SetElements(int nDim, TYPE *pElements)
	{
		m_nDim = nDim;
		m_pElements = pElements;
		m_bFreeElements = FALSE;
	}

protected:
	// dimension of vector
	int m_nDim;

	// vector elements
	TYPE *m_pElements;

	// flag to indicate whether the elements should be freed
	BOOL m_bFreeElements;
};


//////////////////////////////////////////////////////////////////////
// function operator==(const CVectorBase, const CVectorBase)
//
// friend function to provide exact equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator==(const CVectorBase<TYPE>& vLeft, 
					   const CVectorBase<TYPE>& vRight)
{
	// test for element-wise equality
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
	{
		if (vLeft[nAt] != vRight[nAt])
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// function operator!=(const CVectorBase, const CVectorBase)
//
// friend function to provide exact inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline bool operator!=(const CVectorBase<TYPE>& vLeft, 
					   const CVectorBase<TYPE>& vRight)
{
	return !(vLeft == vRight);
}


//////////////////////////////////////////////////////////////////////
// function operator+(const CVectorBase, const CVectorBase)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator+(const CVectorBase<TYPE>& vLeft, 
							  const CVectorBase<TYPE>& vRight)
{
	// copy left vector to intermediate sum value
	CVectorBase<TYPE> vSum(vLeft);

	// add the right vector
	vSum += vRight;

	// return formed sum value
	return vSum;
}


//////////////////////////////////////////////////////////////////////
// function operator-(const CVectorBase, const CVectorBase)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator-(const CVectorBase<TYPE>& vLeft, 
							  const CVectorBase<TYPE>& vRight)
{
	// copy left vector to intermediate difference value
	CVectorBase<TYPE> vDiff(vLeft);

	// subtract the right vector
	vDiff -= vRight;

	// return formed difference value
	return vDiff;
}


//////////////////////////////////////////////////////////////////////
// function operator*(const CVectorBase, const CVectorBase)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline TYPE operator*(const CVectorBase<TYPE>& vLeft, 
							  const CVectorBase<TYPE>& vRight)
{
	// stores the dot product
	TYPE prod = (TYPE) 0.0;

	// sum the element-wise multiplication
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
	{
		prod += vLeft[nAt] * vRight[nAt];
	}

	// return the product
	return prod;
}


//////////////////////////////////////////////////////////////////////
// function operator*(TYPE scalar, const CVectorBase)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator*(TYPE scalar, 
							  const CVectorBase<TYPE>& vRight)
{
	// copy vector to intermediate product value
	CVectorBase<TYPE> vProd(vRight);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;
}


//////////////////////////////////////////////////////////////////////
// function operator*(const CVectorBase, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVectorBase<TYPE> operator*(const CVectorBase<TYPE>& vLeft, 
							  TYPE scalar)
{
	// copy vector to intermediate product value
	CVectorBase<TYPE> vProd(vLeft);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;
}


//////////////////////////////////////////////////////////////////////
// function operator<<(ostream& ar, CVectorBase)
//
// stream output of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
ostream& operator<<(ostream& os, CVectorBase<TYPE> v)
{
	os << "<";
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		os << v[nAt] << "\t";
	}
	os << ">\n";

	return os;
}


//////////////////////////////////////////////////////////////////////
// function TraceVector
//
// helper function to output a vector for debugging
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TraceVector(const CVectorBase<TYPE>& vTrace)
{
#ifdef _DEBUG
	TRACE("<");
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		TRACE("%lf\t", vTrace[nAt]);
	}
	TRACE(">\n");
#endif
}

//////////////////////////////////////////////////////////////////////
// macro TRACE_VECTOR
//
// macro to trace matrix -- only compiles in debug version
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define TRACE_VECTOR(strMessage, v) \
	TRACE(strMessage);				\
	TraceVector(v);					\
	TRACE("\n");
#else
#define TRACE_VECTOR(strMessage, v)
#endif


#ifdef _DEBUG
//////////////////////////////////////////////////////////////////////
// function VectorValid
//
// asserts that the vector has valid values
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void VectorValid(const CVectorBase<TYPE>& v)
{
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ASSERT(_finite(v[nAt]) && !_isnan(v[nAt]));
	}
}
#endif


#endif	// !defined(VECTORBASE_H)