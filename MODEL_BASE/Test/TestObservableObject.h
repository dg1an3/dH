// TestObservableObject.h: interface for the CTestObservableObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTOBSERVABLEOBJECT_H__D9B3845C_60E2_11D5_AB84_00B0D0AB90D6__INCLUDED_)
#define AFX_TESTOBSERVABLEOBJECT_H__D9B3845C_60E2_11D5_AB84_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Observer.h>

class CTestObservableObject : public CObject
{
public:
	CTestObservableObject();

	// test change listener
	void OnChange(CObservableObject *pSource, void *pValue);

	// resets flags from an OnChange event
	void Reset();

	// run the test
	static void RunTest();

protected:
	// flag to indicate that the OnChange event was fired
	BOOL m_bReceivedChange;

	// stores the pointer passed to OnChange
	void *m_pOldValue;
};

#endif // !defined(AFX_TESTOBSERVABLEOBJECT_H__D9B3845C_60E2_11D5_AB84_00B0D0AB90D6__INCLUDED_)
