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

	void SetAutoObserver(CObserver *pObserver)
	{
		// TODO: check this logic
		if (m_pAutoObserver 
				&& m_value 
				&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
			((CObservableObject *)m_value)->RemoveObserver(m_pAutoObserver);

		m_pAutoObserver = pObserver;

		if (m_pAutoObserver 
				&& m_value 
				&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
			((CObservableObject *)m_value)->AddObserver(m_pAutoObserver);
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
				((CObservableObject *)m_value)->RemoveObserver(m_pAutoObserver);

			m_value = fromValue;

			if (m_pAutoObserver 
					&& m_value 
					&& m_value->IsKindOf(RUNTIME_CLASS(CObservableObject)))
				((CObservableObject *)m_value)->AddObserver(m_pAutoObserver);

			FireChange(pOldValue);
		}
	}

protected:
	// stores a pointer to an observer (usually the parent of this association)
	//		that is always registered as a change listener on the association
	//		target.
	CObserver *m_pAutoObserver;
};

#endif // !defined(AFX_ASSOCIATION_H__76008148_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
