//////////////////////////////////////////////////////////////////////
// Observer.cpp: implementation of the CObservableObject class
//
// $Id$
// Copyright (C) 1999-2001
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the main include for the class
#include "Observer.h"

// debug new statement
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// declares CObservableObject as a dynamically creatable class
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CObservableObject, CObject)

//////////////////////////////////////////////////////////////////////
// CObservableObject::AddObserver
// 
// member function to an observer to the CObservableObject
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// CObservableObject::RemoveObserver
// 
// member function to an observer to the CObservableObject
//////////////////////////////////////////////////////////////////////
void CObservableObject::RemoveObserver(CObject *pObserver, ChangeFunction func) const
{
	for (int nAt = m_arrObservers.GetSize()-1; nAt >= 0; nAt--)
		if (m_arrObservers[nAt] == pObserver && m_arrFunctions[nAt] == func)
		{
			m_arrObservers.RemoveAt(nAt);
			m_arrFunctions.RemoveAt(nAt);
		}
}

//////////////////////////////////////////////////////////////////////
// CObservableObject::FireChange
// 
// fires a change, notifying all observers that the object has 
// changed.
//////////////////////////////////////////////////////////////////////
void CObservableObject::FireChange(void *pOldValue)
{
	for (int nAt = 0; nAt < m_arrObservers.GetSize(); nAt++)
		(m_arrObservers[nAt]->*m_arrFunctions[nAt])(this, pOldValue);
}
