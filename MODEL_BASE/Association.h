//////////////////////////////////////////////////////////////////////
// Association.h: interface for the CAssociation class and the 
//		CAutoSyncValue accessory class
//
// Copyright (C) 1999-2001 DGLane
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"

//////////////////////////////////////////////////////////////////////
// class CAssociation<TYPE>
// 
// a template class that wraps a pointer to a CObject-derived object.
//
// includes an auto-observer mechanism that will be registered as an
// observer on any object that this association is pointed to.
//
// includes a list of derived objects that are automatically destroyed
// when this object is destroyed.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CAssociation : public CValue<TYPE *>
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CAssociation() 
		: CValue<TYPE *>(NULL),
			m_pAutoObserver(NULL)
	{ 
	}

	//////////////////////////////////////////////////////////////////
	// construct from an existing pointer
	//////////////////////////////////////////////////////////////////
	CAssociation(TYPE *pFromPtr) 
		: CValue<TYPE *>(pFromPtr),
			m_pAutoObserver(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////
	// destructor
	//////////////////////////////////////////////////////////////////
	virtual ~CAssociation() 
	{ 
	}

	//////////////////////////////////////////////////////////////////
	// SetAutoObserver -- sets the auto observer for this association
	//////////////////////////////////////////////////////////////////
	void SetAutoObserver(CObject *pObserver, ChangeFunction func)
	{
		// if we currently have an auto observer observing the object,
		//		and the object actually is observable
		if (m_pAutoObserver != NULL
				&& Get() != NULL
				&& Get()->IsKindOf(RUNTIME_CLASS(CObservableObject)))
		{
			// then remove the auto observer
			((CObservableObject *)Get())->RemoveObserver(m_pAutoObserver, func);
		}

		// set the new auto observer
		m_pAutoObserver = pObserver;

		// if we now have an auto observer observing the object,
		//		and the object actually is observable
		if (m_pAutoObserver != NULL
				&& Get() != NULL
				&& Get()->IsKindOf(RUNTIME_CLASS(CObservableObject)))
		{
			// add the new auto observer
			((CObservableObject *)Get())->AddObserver(m_pAutoObserver, func);
		}
	}

	//////////////////////////////////////////////////////////////////
	// operator-> -- returns a dereferenceable pointer to the target
	//		of this association
	//////////////////////////////////////////////////////////////////
	TYPE *operator->() 
	{ 
		return (Get()); 
	}

	//////////////////////////////////////////////////////////////////
	// Set -- over-ride to handle the auto-observer
	//////////////////////////////////////////////////////////////////
	void Set(TYPE *fromValue)
	{
		// store the auto-observer values
		CObject *pAutoObserver = m_pAutoObserver;
		ChangeFunction autoObserverFunction = m_autoObserverFunction;
			
		// if synced,
		if (m_pSyncTo)
		{
			// remove the current auto-observer
			SetAutoObserver(NULL, NULL);

			// set the synced value
			m_pSyncTo->Set(fromValue);

			// and restore the auto-observer
			SetAutoObserver(pAutoObserver, autoObserverFunction);
		}
		// otherwise, only set if the value is changing
		else if (m_value != fromValue)
		{
			// remove the current auto-observer
			SetAutoObserver(NULL, NULL);

			// store the old value for sending to observers
			TYPE *pOldValue = m_value;

			// set the value
			m_value = fromValue;

			// fire a change for the new value
			FireChange(pOldValue);

			// and restore the auto-observer
			SetAutoObserver(pAutoObserver, autoObserverFunction);
		}
	}

protected:
	// stores a pointer to an observer (usually the parent of this association)
	//		that is always registered as a change listener on the association
	//		target.
	CObject *m_pAutoObserver;

	// stores the function to be called on the auto observer
	ChangeFunction m_autoObserverFunction;
};

//////////////////////////////////////////////////////////////////////
// class CAutoSyncValue<TYPE, MEMBER_TYPE>
//
// special type of CValue that automatically syncs to a member 
// variable of a specified CAssociation.
//////////////////////////////////////////////////////////////////////
template<class TYPE, class MEMBER_TYPE>
class CAutoSyncValue : public CValue<MEMBER_TYPE>
{
public:
	//////////////////////////////////////////////////////////////////
	// construct from an association and a pointer-to-member
	//////////////////////////////////////////////////////////////////
	CAutoSyncValue(CAssociation<TYPE> *pAssoc, 
			CValue<MEMBER_TYPE> TYPE::*pMember)
		: m_pAssoc(pAssoc),
			m_pMember(pMember)
	{
		// add this as an observer of the association
		::AddObserver< CAutoSyncValue >(m_pAssoc, this, OnAssocChange);

		// trigger an update
		OnChange(m_pAssoc, NULL);
	}

	//////////////////////////////////////////////////////////////////
	// destructore
	//////////////////////////////////////////////////////////////////
	virtual ~CAutoSyncValue()
	{
		::RemoveObserver< CAutoSyncValue >(m_pAssoc, this, OnAssocChange);
	}

	//////////////////////////////////////////////////////////////////
	// OnAssocChange -- listener for when the association changes
	//////////////////////////////////////////////////////////////////
	void OnAssocChange(CObservableObject *pSource, void *pOldValue)
	{
		// get the value of the association
		TYPE *pObj = (TYPE *) m_pAssoc->Get();
		
		// if we are pointing to something
		if (pObj != NULL)
		{
			// sync the member
			SyncTo(&(pObj->*m_pMember));
		}
		else
		{
			// sync to nothing
			SyncTo(NULL);
		}
	}

private:
	// stores a pointer to the association upon which syncing occurs
	CAssociation<TYPE> *m_pAssoc;

	// pointer-to-member 
	CValue<MEMBER_TYPE> TYPE::*m_pMember;
};

#endif // !defined(AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
