//////////////////////////////////////////////////////////////////////
// Value.h: interface for the CValue class.
//
// Copyright (C) 1999-2001 DGLane
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"

//////////////////////////////////////////////////////////////////////
// class CValue<TYPE>
// 
// a template class that wraps a (non CObservableObject-derived) 
// value.
//
// includes a synchronization mechanism that lets two CValue objects
// be synchronized (if they have the same formal type).
//
// includes a list of derived objects that are automatically destroyed
// when this object is destroyed.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CValue : public CObservableObject
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CValue() 
		: m_pSyncTo(NULL) 
	{ 
	}

	//////////////////////////////////////////////////////////////////
	// construct from an existing TYPE value
	//////////////////////////////////////////////////////////////////
	CValue(const TYPE& fromValue) 
		: m_value(fromValue), 
			m_pSyncTo(NULL) 
	{ 
	}

	//////////////////////////////////////////////////////////////////
	// destructor
	//////////////////////////////////////////////////////////////////
	virtual ~CValue() 
	{ 
		// destroy all derived objects.
		for (int nAt = 0; nAt < m_arrDerivedObjects.GetSize(); nAt++)
		{
			delete m_arrDerivedObjects[nAt];
		}
	}

	//////////////////////////////////////////////////////////////////
	// Get -- returns the stored value
	//////////////////////////////////////////////////////////////////
	TYPE& Get() 
	{
		// if synced, 
		if (m_pSyncTo)
		{
			// then return the synced value
			return m_pSyncTo->Get();
		}

		return m_value;
	}

	//////////////////////////////////////////////////////////////////
	// Get const -- returns a const version of the stored value
	//////////////////////////////////////////////////////////////////
	const TYPE& Get() const
	{
		// if synced, 
		if (m_pSyncTo)
		{
			// then return the synced value
			return m_pSyncTo->Get();
		}

		return m_value;
	}

	//////////////////////////////////////////////////////////////////
	// Set -- sets the stored value
	//		checks for equality using the defined comparison function
	//////////////////////////////////////////////////////////////////
	void Set(const TYPE& fromValue)
	{
		// if synced,
		if (m_pSyncTo)
		{
			// then set the synced value
			m_pSyncTo->Set(fromValue);
		}
		// only assign if different from the current value
		else if (m_value != fromValue)
		{
			// store the old value
			TYPE oldValue = m_value;

			// assign the new value
			m_value = fromValue;

			// and fire a change
			FireChange(&oldValue);
		}
	}

	//////////////////////////////////////////////////////////////////
	// SyncTo -- sets the other CValue to which this is synced
	//////////////////////////////////////////////////////////////////
	void SyncTo(CValue *pToValue)
	{
		// check for self-sync
		if (pToValue != this)
		{
			// change to an un-sync if so
			pToValue = NULL;
		}

		// check for a re-sync to the same target
		if (m_pSyncTo == pToValue)
		{
			return;
		}

		// if we are already synced,
		if (m_pSyncTo)
		{
			// remove us as an observer
			::RemoveObserver<CValue>(m_pSyncTo, this, OnChange);

			// make sure the value we are leaving is the same
			m_value = m_pSyncTo->Get();
		}

		// now set the sync pointer
		m_pSyncTo = pToValue;

		// if we have just synced to a real object,
		if (m_pSyncTo)
		{
			// add us an observer on the new object
			::AddObserver<CValue>(m_pSyncTo, this, OnChange);

			// fire a change, since the effective value has changed
			FireChange();
		}
	}

	//////////////////////////////////////////////////////////////////
	// OnChange -- simply propagates the change to all observers
	//////////////////////////////////////////////////////////////////
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue)
	{
		FireChange();
	}

	
	//////////////////////////////////////////////////////////////////
	// AddDerivedObject -- rudimentary memory management lets us add
	//		a list of objects that will be deleted when this one is.
	//////////////////////////////////////////////////////////////////
	void AddDerivedObject(CObject *pDerivedObjects)
	{
		m_arrDerivedObjects.Add(pDerivedObjects);
	}


	//////////////////////////////////////////////////////////////////
	// Serialize -- reads/writes this object; requires that 
	//		operator>> and operator<< are defined for the TYPE
	//////////////////////////////////////////////////////////////////
	virtual void Serialize(CArchive &ar)
	{
		// if writing,
		if (ar.IsStoring())
		{
			// then write,
			ar << Get();
		}
		else 
		{
			// otherwise, read
			TYPE inValue;
			ar >> inValue;

			// fires a change when set
			Set(inValue);
		}
	}

protected:
	// holds the value (if not synced)
	TYPE m_value;

	// points to the synced-to value
	CValue *m_pSyncTo;

	// array of derived function objects to be deleted with this one
	CObArray m_arrDerivedObjects;
};

#endif // !defined(AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
