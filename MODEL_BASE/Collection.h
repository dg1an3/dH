//////////////////////////////////////////////////////////////////////
// Collection.h: interface for the CCollection class.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_)
#define AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"

//////////////////////////////////////////////////////////////////////
// class CCollection<TYPE>
//
// represents a collection of objects of TYPE.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CCollection : public CObservableObject
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CCollection() 
		: m_pSyncTo(NULL)
	{ 
	}

	//////////////////////////////////////////////////////////////////
	// destructor
	//////////////////////////////////////////////////////////////////
	virtual ~CCollection() 
	{ 
		// destroy elements
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
		{
			delete m_arrElements[nAt];
		}

		// remove all array elements (pointers)
		m_arrElements.RemoveAll();
	}

	//////////////////////////////////////////////////////////////////
	// GetSize -- returns the number of members in the collection
	//////////////////////////////////////////////////////////////////
	int GetSize() const
	{
		return m_arrElements.GetSize();
	}

	//////////////////////////////////////////////////////////////////
	// Get -- returns a pointer to the requested member
	//////////////////////////////////////////////////////////////////
	TYPE *Get(int nIndex)
	{
		return (TYPE *)m_arrElements[nIndex];
	}

	//////////////////////////////////////////////////////////////////
	// Get const -- returns a const pointer to the requested member
	//////////////////////////////////////////////////////////////////
	const TYPE *Get(int nIndex) const
	{
		return (TYPE *)m_arrElements[nIndex];
	}

	//////////////////////////////////////////////////////////////////
	// Set -- sets the indexed element
	//////////////////////////////////////////////////////////////////
	void Set(int nIndex, TYPE *pElement)
	{
		m_arrElements[nIndex] = pElement;
	}

	//////////////////////////////////////////////////////////////////
	// Add -- a new element to the collection, returning the new 
	//		index
	//////////////////////////////////////////////////////////////////
	int Add(TYPE *pElement)
	{
		// check to ensure the element is not already present
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
			if (m_arrElements[nAt] == pElement)
				return nAt;

		// add the new element to the array
		int nIndex = m_arrElements.Add(pElement);

		// fire a change
		FireChange(pElement);

		// return the index of the newly added element
		return nIndex;
	}

	//////////////////////////////////////////////////////////////////
	// Release -- an element in the collection.  The CCollection is
	//		no longer responsible for this element, and it does not
	//		delete the element
	//////////////////////////////////////////////////////////////////
	void Release(TYPE *pElement)
	{
		// find the element
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
			if (m_arrElements[nAt] == pElement)
			{
				// remove the array element (pointer)
				m_arrElements.RemoveAt(nAt);

				// and notify of a change
				FireChange(); // pElement);
			}
	}

	//////////////////////////////////////////////////////////////////
	// Remove -- an element from the collection, deleting it along
	//		the way
	//////////////////////////////////////////////////////////////////
	void Remove(TYPE *pElement)
	{
		// find the element
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
			if (m_arrElements[nAt] == pElement)
			{
				// remove the array element (pointer)
				m_arrElements.RemoveAt(nAt);

				// and notify of a change
				FireChange(); // pElement);

				// delete the object
				delete pElement;
			}
	}

	//////////////////////////////////////////////////////////////////
	// RemoveAll -- elements from the collection, deleting them all
	//////////////////////////////////////////////////////////////////
	void RemoveAll()
	{
		// delete each of the objects
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
			delete m_arrElements[nAt];

		// remove the elements of the array (pointers)
		m_arrElements.RemoveAll();

		// and fire a change
		FireChange();
	}

	//////////////////////////////////////////////////////////////////
	// SyncTo -- another collection of like type
	//////////////////////////////////////////////////////////////////
	void SyncTo(CCollection<TYPE> *pToCollection)
	{
		m_pSyncTo = pToCollection;
	}

	//////////////////////////////////////////////////////////////////
	// Serialize -- the collection to an archive
	//////////////////////////////////////////////////////////////////
	virtual void Serialize(CArchive &ar)
	{
		m_arrElements.Serialize(ar);

		if (!ar.IsStoring())
			FireChange();
	}

private:
	// array of elements in this collection
	CObArray m_arrElements;

	// another collection to which this one is synchronized
	CCollection<TYPE> *m_pSyncTo;
};


#endif // !defined(AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_)
