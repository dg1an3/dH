//////////////////////////////////////////////////////////////////////
// VectorN.h: declaration and definition of the CVectorN dynamic 
//		vector template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORN_H)
#define VECTORN_H

#include <ippm.h>

#include <VectorOps.h>

//////////////////////////////////////////////////////////////////////
// class CVectorN<TYPE>
//
// represents a dynamically sizable mathematical vector with element
//		type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class CVectorN
{
	// dimension of vector
	int m_nDim;

	// vector elements
	TYPE *m_pElements;

	// flag to indicate whether the elements should be freed
	BOOL m_bFreeElements;

public:
	// constructors / destructor
	CVectorN();
	explicit CVectorN(int nDim);
	CVectorN(const CVectorN& vFrom);
	~CVectorN();

	// assignment operator
	CVectorN& operator=(const CVectorN& vFrom);

	// template helper for element type
	typedef TYPE ELEM_TYPE;

	// initializes all elements to zero
	void SetZero() { memset(&(*this)[0], 0, sizeof(TYPE) * GetDim()); }

	// element accessors
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// the dimensionality of this vector
	int GetDim() const;
	void SetDim(int nDim);

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// approximate equality using the epsilon
	BOOL IsApproxEqual(const CVectorN& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place vector arithmetic
	CVectorN& operator+=(const CVectorN& vRight);
	CVectorN& operator-=(const CVectorN& vRight);
	CVectorN& operator*=(const TYPE& scalar);

	//////////////////////////////////////////////////////////////////////
	// low-level element management

	// external element management
	void SetElements(int nDim, TYPE *pElements, BOOL bFreeElements);

	// copy elements from v, starting at start, for length elements, 
	//		and copy them to the destination position
	int CopyElements(const CVectorN<TYPE>& v, int nStart, int nLength, 
		int nDest = 0);

};	// class CVectorN<TYPE>


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::CVectorN<TYPE>
//
// default constructor
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::CVectorN<TYPE>() 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
}	// CVectorN<TYPE>::CVectorN<TYPE>


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::CVectorN<TYPE>(int nDim) 
//
// construct an arbitrary dimensioned vector
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::CVectorN<TYPE>(int nDim) 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// set the dimensionality of the vector
	SetDim(nDim);

}	// CVectorN<TYPE>::CVectorN<TYPE>(int nDim) 


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::CVectorN<TYPE>(const CVectorN<TYPE>& vFrom)
//
// copy constructor
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::CVectorN<TYPE>(const CVectorN<TYPE>& vFrom)
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
{
	// set the dimensionality of the vector
	SetDim(vFrom.GetDim());

	// copy the elements
	(*this) = vFrom;

}	// CVectorN<TYPE>::CVectorN<TYPE>(const CVectorN<TYPE>& vFrom)


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::~CVectorN<TYPE>
//
// destructor 
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::~CVectorN<TYPE>()
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		// free elements
		FreeValues(m_pElements);
	}

}	// CVectorN<TYPE>::~CVectorN<TYPE>


//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator=
//
// assignment operator
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>& CVectorN<TYPE>::operator=(const CVectorN<TYPE>& vFrom)
{
	// check the dimensionality of the vector
	ASSERT(GetDim() == vFrom.GetDim());

	if (GetDim() > 0)
	{
		AssignValues(&(*this)[0], &vFrom[0], __min(GetDim(), vFrom.GetDim()));
	}

	// set remainder of elements to 0
	if (GetDim() > vFrom.GetDim())
	{
		memset(&(*this)[vFrom.GetDim()], 0, 
			(GetDim() - vFrom.GetDim()) * sizeof(TYPE));
	}

	// return a reference to this
	return (*this);

}	// CVectorN<TYPE>::operator=


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator[]
//
// returns a reference to the specified element.
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
TYPE& CVectorN<TYPE>::operator[](int nAtRow)
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorN<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator[] const
//
// returns a const reference to the specified element.
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
const TYPE& CVectorN<TYPE>::operator[](int nAtRow) const
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// CVectorN<TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator TYPE *
//
// TYPE * conversion -- returns a pointer to the first element
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::operator TYPE *()
{
	return &m_pElements[0];

}	// CVectorN<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator const TYPE *
//
// const TYPE * conversion -- returns a pointer to the first 
//		element.
//		WARNING: this allows for no-bounds-checking access
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>::operator const TYPE *() const
{
	return &m_pElements[0];

}	// CVectorN<TYPE>::operator const TYPE *


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::GetDim
//
// returns the dimensionality of the vector
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
int CVectorN<TYPE>::GetDim() const
{
	return m_nDim;

}	// CVectorN<TYPE>::GetDim


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::SetDim
//
// sets the dimensionality of this vector
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CVectorN<TYPE>::SetDim(int nDim)
{
	// do nothing if the dim is already correct
	if (m_nDim != nDim)
	{
		// store pointer to old elements
		TYPE *pOldElements = m_pElements;
		m_pElements = NULL;

		// set the new dimensionality
		int nOldDim = m_nDim;
		m_nDim = nDim;

		// allocate new elements, if needed
		if (m_nDim > 0)
		{
			m_pElements = NULL;
			AllocValues(m_nDim, m_pElements);

			if (pOldElements)
			{
				// copy the elements
				memcpy(&(*this)[0], pOldElements, __min(GetDim(), nOldDim) * sizeof(TYPE));
			}

			// set remainder of elements to 0
			if (GetDim() > nOldDim)
			{
				memset(&(*this)[nOldDim], 0, 
					(GetDim() - nOldDim) * sizeof(TYPE));
			}
		}

		// free the elements, if needed
		if (pOldElements != NULL)
		{
			FreeValues(pOldElements);
		}
	}

}	// CVectorN<TYPE>::SetDim



//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::GetLength
//
// returns the euclidean length of the vector
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
TYPE CVectorN<TYPE>::GetLength() const
{
	return VectorLength((*this));

}	// CVectorN<TYPE>::GetLength


#ifdef USE_IPP
#define DECLARE_VECTORN_GETLENGTH(TYPE, TYPE_IPP) \
	template<> __forceinline							\
	TYPE CVectorN<TYPE>::GetLength() const				\
	{													\
		TYPE length;									\
		ippmL2Norm_v_##TYPE_IPP(&(*this)[0], &length, GetDim());	\
		return length;									\
	}

DECLARE_VECTORN_GETLENGTH(float, 32f);
DECLARE_VECTORN_GETLENGTH(double, 64f);
#endif


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::Normalize
//
// scales the vector so its length is 1.0
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
void CVectorN<TYPE>::Normalize()
{
	ScaleValues(&(*this)[0], (TYPE) 1.0 / GetLength(), GetDim());

}	// CVectorN<TYPE>::Normalize


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::IsApproxEqual
//	
// tests for approximate equality using the EPS defined at 
//		the top of this file
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
BOOL CVectorN<TYPE>::IsApproxEqual(const CVectorN<TYPE>& v, 
									  TYPE epsilon) const
{
	// form the difference vector
	CVectorN<TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorN<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator+=
//
// in-place vector addition; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>& CVectorN<TYPE>::operator+=(const CVectorN<TYPE>& vRight)
{
	SumValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator-=
//
// in-place vector subtraction; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>& CVectorN<TYPE>::operator-=(const CVectorN<TYPE>& vRight)
{
	DiffValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::operator*=
//
// in-place scalar multiplication; returns a reference to this
//////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE>& CVectorN<TYPE>::operator*=(const TYPE& scalar)
{
	ScaleValues(&(*this)[0], scalar, GetDim());

	return (*this);

}	// CVectorN<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
// CVectorN<TYPE>::SetElements
//
// management for external elements
//////////////////////////////////////////////////////////////////
template<class TYPE>
void CVectorN<TYPE>::SetElements(int nDim, TYPE *pElements,
									BOOL bFreeElements)
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	m_nDim = nDim;
	m_pElements = pElements;
	m_bFreeElements = bFreeElements;

}	// CVectorN<TYPE>::SetElements



//////////////////////////////////////////////////////////////////////
// operator==(const CVectorN, const CVectorN)
//
// friend function to provide exact equality comparison for vectors.
// use IsApproxEqual for approximate equality.
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
bool operator==(const CVectorN<TYPE>& vLeft, 
					   const CVectorN<TYPE>& vRight)
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

}	// operator==(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
// operator!=(const CVectorN, const CVectorN)
//
// friend function to provide exact inequality comparison for vectors.
// use !IsApproxEqual for approximate inequality.
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
bool operator!=(const CVectorN<TYPE>& vLeft, 
					   const CVectorN<TYPE>& vRight)
{
	return !(vLeft == vRight);

}	// operator!=(const CVectorN, const CVectorN)



//////////////////////////////////////////////////////////////////////
// operator+(const CVectorN, const CVectorN)
//
// friend function to add two vectors, returning the sum as a new 
//		vector
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE> operator+(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	CVectorN<TYPE> vSum(vLeft);
	vSum += vRight;

	return vSum;

}	// operator+(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
// operator-(const CVectorN, const CVectorN)
//
// friend function to subtract one vector from another, returning 
//		the difference as a new vector
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE> operator-(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	CVectorN<TYPE> vDiff(vLeft);
	vDiff -= vRight;

	return vDiff;

}	// operator-(const CVectorN, const CVectorN)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorN, const CVectorN)
//
// friend function for vector inner product
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
TYPE operator*(const CVectorN<TYPE>& vLeft, 
							  const CVectorN<TYPE>& vRight)
{
	return DotProduct(vLeft, vRight);

}	// operator*(const CVectorN, const CVectorN)


#ifdef USE_IPP
#define DECLARE_VECTORN_DOTPRODUCT(TYPE, TYPE_IPP) \
	template<> __forceinline									\
	TYPE operator*(const CVectorN<TYPE>& vLeft,					\
					 const CVectorN<TYPE>& vRight)				\
	{															\
		TYPE prod;												\
		ippmDotProduct_vv_##TYPE_IPP(&vLeft[0],					\
			&vRight[0], &prod, vLeft.GetDim());					\
		return prod;											\
	}

DECLARE_VECTORN_DOTPRODUCT(float, 32f);
DECLARE_VECTORN_DOTPRODUCT(double, 64f);
#endif


//////////////////////////////////////////////////////////////////////
// operator*(TYPE scalar, const CVectorN)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE> operator*(const TYPE& scalar, 
							  const CVectorN<TYPE>& vRight)
{
	// copy vector to intermediate product value
	CVectorN<TYPE> vProd(vRight);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(TYPE scalar, const CVectorN)


//////////////////////////////////////////////////////////////////////
// operator*(const CVectorN, TYPE scalar)
//
// friend function for scalar multiplication of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE> __forceinline
CVectorN<TYPE> operator*(const CVectorN<TYPE>& vLeft, 
							  const TYPE& scalar)
{
	// copy vector to intermediate product value
	CVectorN<TYPE> vProd(vLeft);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(const CVectorN, TYPE scalar)


///////////////////////////////////////////////////////////////////////////////
// CalcBinomialCoeff
// 
// calculates the binomial coefficients, returns in the vector
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void CalcBinomialCoeff(CVectorN<TYPE>& vCoeff)
{
	CVectorN<TYPE> vAltCoeff;
	vAltCoeff.SetDim(vCoeff.GetDim());

	for (int nAt = 0; nAt < vCoeff.GetDim()-1; nAt++)
	{
		vCoeff[0] = 1.0;
		for (int nAtElem = 0; nAtElem < nAt; nAtElem++)
		{
			vCoeff[nAtElem+1] = vAltCoeff[nAtElem] + vAltCoeff[nAtElem+1];
		}
		vCoeff[nAt+1] = 1.0;

		vAltCoeff = vCoeff;
	}

}	// CalcBinomialCoeff



//////////////////////////////////////////////////////////////////////
// TraceVector
//
// helper function to output a vector for debugging
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TraceVector(const CVectorN<TYPE>& vTrace)
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



#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
// function operator<<(CArchive &ar, CVector)
//
// CArchive serialization of a vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& operator<<(CArchive &ar, CVectorN<TYPE> v)
{
	// store the dimension first
	ar << v.GetDim();

	// store the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
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
template<class TYPE>
CArchive& operator>>(CArchive &ar, CVectorN<TYPE>& v)
{
	// retrieve the dimension first
	int nDim;
	ar >> nDim;
	v.SetDim(nDim);

	// retrieve the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ar >> v[nAt];
	}

	return ar;
}

#endif	// __AFX_H__


//////////////////////////////////////////////////////////////////////
// LogExprExt
//
// helper function for XML logging of vectors
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void LogExprExt(const CVectorN<TYPE> & vVec, const char *pszName, const char *pszModule)
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// only if we are logging --
	if (pLog->IsLogging())
	{
		// create a new expression element
		CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

		// if there is a name,
		if (strlen(pszName) > 0)
		{
			// set it.
			pVarElem->Attribute("name", pszName);
		}

		// set type to generice "CVector"
		pVarElem->Attribute("type", "CVector");
		
		// get the current format for the element type
		const char *pszFormat = pLog->GetFormat((TYPE) 0);
		for (int nAt = 0; nAt < vVec.GetDim(); nAt++)
		{
			// format each element
			pLog->Format(pszFormat, vVec[nAt]);
		}

		// done.
		pLog->GetLogFile()->CloseElement();
	}

}	// LogExprExt


#endif	// !defined(VECTORN_H)