// CAutoSyncValue.h: interface for the CSyncConfigurator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYNCCONFIGURATOR_H__CF990024_FD24_11D4_9E44_00B0D0609AB0__INCLUDED_)
#define AFX_SYNCCONFIGURATOR_H__CF990024_FD24_11D4_9E44_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"
#include "Association.h"

// TODO: incorporate this function into the CValue base class
template<class TYPE, class MEMBER_TYPE>
class CAutoSyncValue : public CValue<MEMBER_TYPE>
{
public:
	CAutoSyncValue(CAssociation<TYPE> *pAssoc, 
		CValue<MEMBER_TYPE> TYPE::*pMember)
		: m_pAssoc(pAssoc),
			m_pMember(pMember)
	{
		m_pAssoc->AddObserver(this);

		// trigger an update
		OnChange(m_pAssoc, NULL);
	}

	virtual ~CAutoSyncValue()
	{
		m_pAssoc->RemoveObserver(this);
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

#endif // !defined(AFX_SYNCCONFIGURATOR_H__CF990024_FD24_11D4_9E44_00B0D0609AB0__INCLUDED_)
