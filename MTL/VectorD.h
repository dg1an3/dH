//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVectorD template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

// math utilities
#include "MathUtil.h"

// common operations
#include "VectorOps.h"


//////////////////////////////////////////////////////////////////////
// class CVectorD<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = REAL>
class CVectorD
{
	// the vector's elements
	TYPE m_arrElements[DIM];

public:
	// constructors / destructor
	CVectorD();
	CVectorD(TYPE x);
	CVectorD(TYPE x, TYPE y);
	CVectorD(TYPE x, TYPE y, TYPE z);
	CVectorD(TYPE x, TYPE y, TYPE z, TYPE w);
	CVectorD(const CVectorD& vFrom);

#ifdef __AFXWIN_H__
	CVectorD(const CPoint& pt);
#endif

	~CVectorD();

	// assignment
	CVectorD& operator=(const CVectorD& vFrom);

	// conversion
#ifdef __AFXWIN_H__
	operator CPoint() const;
#endif

	// template helper for element type
	typedef TYPE ELEM_TYPE;

	// element accessors
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// dimensional accessors
	int GetDim() const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// tests for approximate equality using the EPS defined at the 
	//		top of this file
	BOOL IsApproxEqual(const CVectorD& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// assignment operators -- requires proper type for strict
	//		compile-time check
	CVectorD& operator+=(const CVectorD& vRight);
	CVectorD& operator-=(const CVectorD& vRight);
	CVectorD& operator*=(const TYPE& scalar);

};	// class CVectorD<DIM, TYPE>


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 
//
// default constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 
{
	// clear to all zeros
	memset(&(*this)[0], 0, DIM * sizeof(TYPE));

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x) 
//
// construct from one element
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x) 
{
	// set the given elements
	(*this)[0] = x;

	// clear to all zeros
	if (DIM > 1)
	{
		memset(&(*this)[1], 0, (DIM-1) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 
//
// construct from two elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 
{
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;

	// clear to all zeros
	if (DIM > 2)
	{
		memset(&(*this)[2], 0, (DIM-2) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 
//
// construct from three elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 
{
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;

	// clear to all zeros
	if (DIM > 3)
	{
		memset(&(*this)[3], 0, (DIM-3) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 
//
// construct from four elements
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 
{ 
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;
	(*this)[3] = w;

	// clear to all zeros
	if (DIM > 4)
	{
		memset(&(*this)[4], 0, (DIM-4) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 
{
	// assign to copy 
	(*this) = vFrom;

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 


#ifdef __AFXWIN_H__
//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)
//
// construct from a windows CPoint
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)
{
	// set the given elements
	(*this)[0] = pt.x;
	(*this)[1] = pt.y;

	// clear to all zeros
	if (DIM > 2)
	{
		memset(&(*this)[2], 0, (DIM-2) * sizeof(TYPE));
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)
#endif


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()
//
// destructor
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()
{
}	// CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()


#ifdef __AFXWIN_H__
//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator CPoint() const
//
// Windows CPoint conversion 
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::operator CPoint() const
{
	return CPoint((int) floor((*this)[0] + 0.5), 
		(int) floor((*this)[1] + 0.5));

}	// CVectorD<DIM, TYPE>::operator CPoint() const
#endif


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=(const CVectorD<DIM, TYPE>& vFrom)
{
	// copy the elements
	AssignValues(&(*this)[0], &vFrom[0], GetDim());

	// return a reference to this
	return (*this);

}	// CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::GetDim
//
// returns the dimensionality of the vector
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
int CVectorD<DIM, TYPE>::GetDim() const
{
	return DIM;

}	// CVectorD<DIM, TYPE>::GetDim


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator[]
//
// returns a reference to the specified element.
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
TYPE& CVectorD<DIM, TYPE>::operator[](int nAtRow)
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_arrElements[nAtRow];

}	// CVectorD<DIM, TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator[] const
//
// returns a const reference to the specified element.
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
const TYPE& CVectorD<DIM, TYPE>::operator[](int nAtRow) const
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_arrElements[nAtRow];

}	// CVectorD<DIM, TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator TYPE *
//
// TYPE * conversion -- returns a pointer to the first element
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::operator TYPE *()
{
	return &(*this)[0];

}	// CVectorD<DIM, TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator const TYPE *
//
// const TYPE * conversion -- returns a pointer to the first 
//		element.
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>::operator const TYPE *() const
{
	return &(*this)[0];

}	// CVectorD<DIM, TYPE>::operator const TYPE *



//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::GetLength
//
// returns the euclidean length of the vector
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
TYPE CVectorD<DIM, TYPE>::GetLength() const
{
	return VectorLength((*this));

}	// CVectorD<DIM, TYPE>::GetLength


#ifdef USE_IPP
#define DECLARE_VECTORD_GETLENGTH(TYPE, TYPE_IPP, DIM) \
	template<> __forceinline							\
	TYPE CVectorD<DIM, TYPE>::GetLength() const			\
	{													\
		TYPE length;									\
		ippmL2Norm_v_##TYPE_IPP##_##DIM##x1(&(*this)[0],\
			&length);									\
		return length;									\
	}

DECLARE_VECTORD_GETLENGTH(float, 32f, 3);
DECLARE_VECTORD_GETLENGTH(float, 32f, 4);
DECLARE_VECTORD_GETLENGTH(float, 32f, 5);
DECLARE_VECTORD_GETLENGTH(float, 32f, 6);
DECLARE_VECTORD_GETLENGTH(double, 64f, 3);
DECLARE_VECTORD_GETLENGTH(double, 64f, 4);
DECLARE_VECTORD_GETLENGTH(double, 64f, 5);
DECLARE_VECTORD_GETLENGTH(double, 64f, 6);
#endif

//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::Normalize
//
// scales the vector so its length is 1.0
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void CVectorD<DIM, TYPE>::Normalize()
{
	ScaleValues(&(*this)[0], (TYPE) 1.0 / GetLength(), GetDim());

}	// CVectorD<DIM, TYPE>::Normalize



//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::IsApproxEqual
//
// tests for approximate equality using the given epsilon
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
BOOL CVectorD<DIM, TYPE>::IsApproxEqual(const CVectorD<DIM, TYPE>& v, 
										TYPE epsilon) const
{
	// form the difference vector
	CVectorD<DIM, TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorD<DIM, TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator+=
//
// in-place vector addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator+=(const CVectorD<DIM, TYPE>& vRight)
{
	SumValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorD<DIM, TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CVectorD<DIM, TYPE>::operator-=
//
// in-place vector subtraction; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator-=(const CVectorD<DIM, TYPE>& vRight)
{
	DiffValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorD<DIM, TYPE>::operator-=


//////////////////////////////////////////////////////////////////
// CVectorD<TYPE>::operator*=
//
// in-place scalar multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator*=(const TYPE& scalar)
{
	ScaleValues(&(*this)[0], scalar, GetDim());

	return (*this);

}	// CVectorD<TYPE>::operator*=



//////////////////////////////////////////////////////////////////////
// operator==(const CVectorD, const CVectorD)
//
// friend function to provide equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
bool operator==(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
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

}	// operator==(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator!=(const CVectorD, const CVectorD)
//
// friend function to provide inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
bool operator!=(const CVectorD<DIM, TYPE>& vLeft, 
					   const CVectorD<DIM, TYPE>& vRight)
{
	return !(vLeft == vRight);

}	// operator!=(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator+(const CVectorD, const CVectorD)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE> operator+(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum += vRight;

	return vSum;

}	// operator+(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator-(const CVectorD, const CVectorD)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE> operator-(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;

	return vSum;

}	// operator-(const CVectorD, const CVectorD)




//////////////////////////////////////////////////////////////////////
// operator*(const CVectorD, const CVectorD)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
TYPE operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	return DotProduct(vLeft, vRight);

}	// operator*(const CVectorD, const CVectorD)


#ifdef USE_IPP
#define DECLARE_VECTORD_DOTPRODUCT(TYPE, TYPE_IPP, DIM) \
	template<> __forceinline									\
	TYPE operator*(const CVectorD<DIM, TYPE>& vLeft,			\
					const CVectorD<DIM, TYPE>& vRight)			\
	{															\
		TYPE prod;												\
		ippmDotProduct_vv_##TYPE_IPP##_##DIM##x1(&vLeft[0],		\
			&vRight[0], &prod);									\
		return prod;											\
	}

DECLARE_VECTORD_DOTPRODUCT(float, 32f, 3);
DECLARE_VECTORD_DOTPRODUCT(float, 32f, 4);
DECLARE_VECTORD_DOTPRODUCT(float, 32f, 5);
DECLARE_VECTORD_DOTPRODUCT(float, 32f, 6);
DECLARE_VECTORD_DOTPRODUCT(double, 64f, 3);
DECLARE_VECTORD_DOTPRODUCT(double, 64f, 4);
DECLARE_VECTORD_DOTPRODUCT(double, 64f, 5);
DECLARE_VECTORD_DOTPRODUCT(double, 64f, 6);
#endif


//////////////////////////////////////////////////////////////////////
// operator*(TYPE scalar, const CVectorD)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE> operator*(TYPE scalar, 
							  const CVectorD<DIM, TYPE>& vRight)
{
	CVectorD<DIM, TYPE> vProd = vRight;
	vProd *= scalar;

	return vProd;

}	// operator*(TYPE scalar, const CVectorD)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorD, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE> operator*(const CVectorD<DIM, TYPE>& vLeft, 
							  TYPE scalar)
{
	CVectorD<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;

	return vProd;

}	// operator*(const CVectorD, TYPE scalar)


//////////////////////////////////////////////////////////////////////
// Cross(const CVectorD<2>&, const CVectorD<2>&)
//
// friend function for vector cross product of 2-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
TYPE Cross(const CVectorD<2, TYPE>& vLeft, 
					const CVectorD<2, TYPE>& vRight)
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

}	// Cross(const CVectorD<2>&, const CVectorD<2>&)


//////////////////////////////////////////////////////////////////////
// Cross(const CVectorD<3>&, const CVectorD<3>&)
//
// friend function for vector cross product of 3-d vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorD<3, TYPE> Cross(const CVectorD<3, TYPE>& vLeft, 
						      const CVectorD<3, TYPE>& vRight)
{
	CVectorD<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;

}	// Cross(const CVectorD<3>&, const CVectorD<3>&)


//////////////////////////////////////////////////////////////////////
// ToHG(const CVectorD&)
//
// converts an N-dimensional vector to an N+1-dimensional homogeneous
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM + 1, TYPE> ToHG(const CVectorD<DIM, TYPE>& v) 
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

}	// ToHG(const CVectorD&)

#define ToHomogeneous ToHG

//////////////////////////////////////////////////////////////////////
// FromHG(const CVectorD&)
//
// converts an N+1-dimensional homogeneous vector to an N-dimensional 
//		vector
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> __forceinline
CVectorD<DIM, TYPE> FromHG(const CVectorD<DIM + 1, TYPE>& vh) 
{
	// create the non-homogeneous vector
	CVectorD<DIM, TYPE> v;

	// normalize the first DIM-1 elements by the last element
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		v[nAt] = vh[nAt] / vh[DIM];
	}

	return v;

}	// FromHG(const CVectorD&)

#define FromHomogeneous FromHG

#ifdef __AFX_H__
//////////////////////////////////////////////////////////////////////
// operator<<(CArchive &ar, CVectorD)
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

}	// operator<<(CArchive &ar, CVectorD)


//////////////////////////////////////////////////////////////////////
// operator>>(CArchive &ar, CVectorD)
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

}	// operator>>(CArchive &ar, CVectorD)
#endif

//////////////////////////////////////////////////////////////////////
// TraceVector
//
// helper function to output a vector for debugging
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void TraceVector(const CVectorD<DIM, TYPE>& vTrace)
{
#ifdef _DEBUG
	TRACE("<");
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		TRACE("%lf\t", vTrace[nAt]);
	}
	TRACE(">\n");
#endif

}	// TraceVector


//////////////////////////////////////////////////////////////////////
// TRACE_VECTOR
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



// template<class TYPE>
// void AssignValues(TYPE *pTo, int nSize, TYPE *pFrom);


#endif	// !defined(VECTOR_H)