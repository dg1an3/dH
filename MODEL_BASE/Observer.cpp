// Observer.cpp: implementation of the CObserver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Observer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CObservable::AddObserver(CObserver *pObserver) const
{
	// check to ensure the observer is not already in the list
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver)
			return;

	// add to the list of observers
	m_arrObservers.Add(pObserver);

	// notify the new observer of a change to allow it to update 
	//		itself
	pObserver->OnChange(const_cast<CObservable *>(this));
}

void CObservable::RemoveObserver(CObserver *pObserver) const
{
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver)
			m_arrObservers.RemoveAt(nAt);
}

void CObservable::FireChange()
{
	for (int nAt = 0; nAt < m_arrObservers.GetSize(); nAt++)
		m_arrObservers[nAt]->OnChange(this);
}
