// Association.h: interface for the CAssociation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"

template<class TYPE>
class CAssociation : public CValue<TYPE *>
{
public:
	CAssociation() 
		: CValue<TYPE *>(NULL),
			m_pAutoObserver(NULL)
	{ 
	}

	CAssociation(TYPE *pFromPtr) 
		: CValue<TYPE *>(pFromPtr),
			m_pAutoObserver(NULL)
	{
	}

	virtual ~CAssociation() 
	{ 
	}

	void SetAutoObserver(CObject *pObserver, ChangeFunction func)
	{
		// TODO: check this logic
		if (m_pAutoObserver 
				&& m_value 
				&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
			((CObservableObject *)m_value)->RemoveObserver(m_pAutoObserver, func);

		m_pAutoObserver = pObserver;

		if (m_pAutoObserver 
				&& m_value 
				&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
			::AddObserver<CObject>((CObservableObject *)m_value, m_pAutoObserver, func);
	}

	TYPE *operator->() 
	{ 
		return (Get()); 
	}

	void Set(TYPE *fromValue)
	{
		if (m_pSyncTo)
		{
			m_pSyncTo->Set(fromValue);
		}
		else if (m_value != fromValue)
		{
			// store the old value for sending to observers
			TYPE *pOldValue = m_value;

			// TODO: check this logic
			if (m_pAutoObserver 
					&& m_value 
					&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
				((CObservableObject *)m_value)->RemoveObserver(m_pAutoObserver, 
					m_autoObserverFunction);

			m_value = fromValue;

			if (m_pAutoObserver 
					&& m_value 
					&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
				::AddObserver<CObject>((CObservableObject *)m_value, m_pAutoObserver,
					m_autoObserverFunction);

			FireChange(pOldValue);
		}
	}

protected:
	// stores a pointer to an observer (usually the parent of this association)
	//		that is always registered as a change listener on the association
	//		target.
	CObject *m_pAutoObserver;
	ChangeFunction m_autoObserverFunction;
};

//////////////////////////////////////////////////////////////////////
// template class CAutoSyncValue
//
// Special type of CValue that automatically syncs to a member 
// variable of a specified CAssociation.
//////////////////////////////////////////////////////////////////////
template<class TYPE, class MEMBER_TYPE>
class CAutoSyncValue : public CValue<MEMBER_TYPE>
{
public:
	CAutoSyncValue(CAssociation<TYPE> *pAssoc, 
		CValue<MEMBER_TYPE> TYPE::*pMember)
		: m_pAssoc(pAssoc),
			m_pMember(pMember)
	{
		::AddObserver< CAutoSyncValue >(m_pAssoc, this, OnChange);

		// trigger an update
		OnChange(m_pAssoc, NULL);
	}

	virtual ~CAutoSyncValue()
	{
		m_pAssoc->RemoveObserver(this, (ChangeFunction) OnChange);
	}

	virtual void OnChange(CObservableObject *pSource, void *pOldValue)
	{
		if (pSource == m_pAssoc)
		{
			TYPE *pObj = (TYPE *) m_pAssoc->Get();
			
			// if we are pointing to something
			if (pObj != NULL)
				// sync the member
				SyncTo(&(pObj->*m_pMember));
			else
				// sync to nothing
				SyncTo(NULL);
		}
		else
			CValue<MEMBER_TYPE>::OnChange(pSource, pOldValue);
	}

private:
	CAssociation<TYPE> *m_pAssoc;
	CValue<MEMBER_TYPE> TYPE::*m_pMember;
};

#endif // !defined(AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
