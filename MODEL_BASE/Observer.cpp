// Observer.cpp: implementation of the CObservableObject class
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Observer.h"

IMPLEMENT_DYNAMIC(CObservableObject, CObject)

void CObservableObject::AddObserver(CObserver *pObserver) const
{
	// check to ensure the observer is not already in the list
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver)
			return;

	// add to the list of observers
	m_arrObservers.Add(pObserver);
}

void CObservableObject::RemoveObserver(CObserver *pObserver) const
{
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver)
			m_arrObservers.RemoveAt(nAt);
}

void CObservableObject::FireChange(void *pOldValue)
{
	for (int nAt = 0; nAt < m_arrObservers.GetSize(); nAt++)
		m_arrObservers[nAt]->OnChange(this, pOldValue);
}
