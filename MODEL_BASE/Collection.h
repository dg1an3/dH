//////////////////////////////////////////////////////////////////////
// Collection.h: interface for the CCollection class.
//
// Copyright (C) 1999-2000 DGLane
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_)
#define AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"

//////////////////////////////////////////////////////////////////////
// template class CCollection
//
// Represents a collection of objects of TYPE.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CCollection : public CObservableObject
{
public:
	CCollection() 
		: m_pSyncTo(NULL)
	{ 
	}

	virtual ~CCollection() 
	{ 
		// destroy elements
		int nAt;
		for (nAt = 0; nAt < m_arrElements.GetSize(); nAt++)
			delete m_arrElements[nAt];

		// remove all array elements (pointers)
		m_arrElements.RemoveAll();
	}

	int GetSize() const
	{
		return m_arrElements.GetSize();
	}

	TYPE *Get(int nIndex)
	{
		return (TYPE *)m_arrElements[nIndex];
	}

	const TYPE *Get(int nIndex) const
	{
		return (TYPE *)m_arrElements[nIndex];
	}

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

	void SyncTo(CCollection<TYPE> *pToCollection)
	{
		m_pSyncTo = pToCollection;
	}

	virtual void Serialize(CArchive &ar)
	{
		m_arrElements.Serialize(ar);

		if (!ar.IsStoring())
			FireChange();
	}

private:
	CObArray m_arrElements;

	CCollection<TYPE> *m_pSyncTo;
};


#endif // !defined(AFX_COLLECTION_H__4CA62889_01F1_11D5_9E46_00B0D0609AB0__INCLUDED_)
