// Value.h: interface for the CValue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"

template<class TYPE>
class CValue : public CObservableObject
{
public:
	CValue() 
		: m_pSyncTo(NULL) 
	{ 
	}

	CValue(const TYPE& fromValue) 
		: m_value(fromValue), 
			m_pSyncTo(NULL) 
	{ 
	}

	virtual ~CValue() 
	{ 
		for (int nAt = 0; nAt < m_arrDerivedObjects.GetSize(); nAt++)
			delete m_arrDerivedObjects[nAt];
	}

	TYPE& Get() 
	{
		if (m_pSyncTo)
		{
			return m_pSyncTo->Get();
		}

		return m_value;
	}

	const TYPE& Get() const
	{
		if (m_pSyncTo)
		{
			return m_pSyncTo->Get();
		}

		return m_value;
	}

	void Set(const TYPE& fromValue)
	{
		if (m_pSyncTo)
		{
			m_pSyncTo->Set(fromValue);
		}
		else if (m_value != fromValue)
		{
			m_value = fromValue;
			FireChange();
		}
	}

	void SyncTo(CValue *pToValue)
	{
		ASSERT(pToValue != this);

		if (m_pSyncTo == pToValue)
			return;

		if (m_pSyncTo)
		{
			m_pSyncTo->RemoveObserver(this, (ChangeFunction) OnChange);

			// make sure the value we are leaving is the same
			m_value = m_pSyncTo->Get();
		}

		m_pSyncTo = pToValue;

		if (m_pSyncTo)
		{
			m_pSyncTo->AddObserver(this, (ChangeFunction) OnChange);

			// fire a change, since the effective value has changed
			FireChange();
		}
	}

	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue)
	{
		FireChange();
	}

	// TODO: rename this
	void AddDerivedObject(CObject *pDerivedObjects)
	{
		m_arrDerivedObjects.Add(pDerivedObjects);
	}

	virtual void Serialize(CArchive &ar)
	{
		if (ar.IsStoring())
		{
			ar << Get();
		}
		else 
		{
			TYPE inValue;
			ar >> inValue;
			Set(inValue);
		}
	}

protected:
	// holds the value (if not synced)
	TYPE m_value;

	// points to the synced-to value
	CValue *m_pSyncTo;

	// array of derived function objects
	//		to be deleted
	CObArray m_arrDerivedObjects;
};

#endif // !defined(AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
