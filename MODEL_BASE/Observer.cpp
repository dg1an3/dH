// Observer.cpp: implementation of the CObservableObject class
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Observer.h"

IMPLEMENT_DYNAMIC(CObservableObject, CObject)

void CObservableObject::AddObserver(CObject *pObserver, ChangeFunction func) const
{
	// check to ensure the observer is not already in the list
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver && m_arrFunctions[nAt] == func)
			return;

	// add to the list of observers
	m_arrObservers.Add(pObserver);
	m_arrFunctions.Add(func);
}

void CObservableObject::RemoveObserver(CObject *pObserver, ChangeFunction func) const
{
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver && m_arrFunctions[nAt] == func)
		{
			m_arrObservers.RemoveAt(nAt);
			m_arrFunctions.RemoveAt(nAt);
		}
}

void CObservableObject::FireChange(void *pOldValue)
{
	for (int nAt = 0; nAt < m_arrObservers.GetSize(); nAt++)
		(m_arrObservers[nAt]->*m_arrFunctions[nAt])(this, pOldValue);
}
