//////////////////////////////////////////////////////////////////////
// VectorN.h: declaration and definition of the CVectorN dynamic 
//		vector template class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORN_H)
#define VECTORN_H

// base class include
#include "VectorBase.h"


//////////////////////////////////////////////////////////////////////
// class CVectorN<TYPE>
//
// represents a dynamically sizable mathematical vector with element
//		type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class CVectorN : public CVectorBase<TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVectorN() 
	{
	}

	//////////////////////////////////////////////////////////////////
	// construct an arbitrary dimensioned vector
	//////////////////////////////////////////////////////////////////
	explicit CVectorN(int nDim) 
	{
		// set the dimensionality of the vector
		SetDim(nDim);
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CVectorN(const CVectorN& vFrom)
	{
		// set the dimensionality of the vector
		SetDim(vFrom.GetDim());

		// copy the elements
		(*this) = vFrom;
	}

	//////////////////////////////////////////////////////////////////
	// copy constructor
	//////////////////////////////////////////////////////////////////
	CVectorN(const CVectorBase<TYPE>& vFrom)
	{
		// set the dimensionality of the vector
		SetDim(vFrom.GetDim());

		// copy the elements
		(*this) = vFrom;
	}

	//////////////////////////////////////////////////////////////////
	// assignment operator
	//////////////////////////////////////////////////////////////////
	CVectorN& operator=(const CVectorN& vFrom)
	{
		// set the dimensionality of the vector
		SetDim(vFrom.GetDim());

		// set the given elements
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = 
				(nAt < vFrom.GetDim()) ? vFrom[nAt] : (TYPE) 0.0;
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// assignment operator
	//////////////////////////////////////////////////////////////////
	CVectorN& operator=(const CVectorBase<TYPE>& vFrom)
	{
		// set the dimensionality of the vector
		SetDim(vFrom.GetDim());

		// set the given elements
		for (int nAt = 0; nAt < GetDim(); nAt++)
		{
			(*this)[nAt] = 
				(nAt < vFrom.GetDim()) ? vFrom[nAt] : (TYPE) 0.0;
		}

		// return a reference to this
		return (*this);
	}

	//////////////////////////////////////////////////////////////////
	// SetDim -- sets the dimensionality of this vector
	//////////////////////////////////////////////////////////////////
	void SetDim(int nDim)
	{
		// do nothing if the dim is already correct
		if (m_nDim == nDim)
		{
			return;
		}

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
		{
			m_pElements = new TYPE[m_nDim];

			for (int nAt = 0; nAt < m_nDim; nAt++)
			{
				m_pElements[nAt] = (TYPE) 0.0;
			}
		}
	}
};

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

#endif	// _WINDOWS

#endif	// !defined(VECTOR_H)