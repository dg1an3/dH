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
class CValue : public CObservable, public CObserver
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
		for (int nAt = 0; nAt < m_arrFunctions.GetSize(); nAt++)
			delete m_arrFunctions[nAt];
	}

	CValue& operator=(const CValue& fromValue) 
	{ 
		if (m_pSyncTo)
		{
			return (*m_pSyncTo) = fromValue;
		}

		Set(fromValue.Get());

		return (*this);
	}

//	operator const TYPE&() const
//	{
//		if (m_pSyncTo)
//		{
//			return m_pSyncTo->Get();
//		}
//
//		return Get();
//	}
//
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
		if (m_pSyncTo == pToValue)
			return;

		if (m_pSyncTo)
		{
			m_pSyncTo->RemoveObserver(this);

			// make sure the value we are leaving is the same
			m_value = m_pSyncTo->Get();
		}

		m_pSyncTo = pToValue;

		if (m_pSyncTo)
		{
			m_pSyncTo->AddObserver(this);

			// fire a change, since the effective value has changed
			FireChange();
		}
	}

	virtual void OnChange(CObservable *pFromObject)
	{
		FireChange();
	}

	void AddFunction(CObject *pFunc)
	{
		m_arrFunctions.Add(pFunc);
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
	CObArray m_arrFunctions;
};

#endif // !defined(AFX_VALUE_H__76008147_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
