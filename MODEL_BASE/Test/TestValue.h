// TestValue.h: interface for the CTestValue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTVALUE_H__09C4CC8B_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_)
#define AFX_TESTVALUE_H__09C4CC8B_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>

#include "TestObservableObject.h"

using namespace std;

ostream& operator<<(ostream& s, CString strOut) 
{ 
	s << (const char *) strOut.GetBuffer(256);
	strOut.ReleaseBuffer();

	return s;
}

template<class VALUE_TYPE>
VALUE_TYPE CreateRandom()
{
	return (VALUE_TYPE) rand();
}

CString CreateRandom<CString>()
{
	unsigned char nStrLength = CreateRandom<unsigned char>();
	nStrLength /= 2;

	CString strRandom;

	while (nStrLength > 0)
	{
		char chRandom = CreateRandom<char>();
		if (chRandom >= ' ' && chRandom <= '\177')
			strRandom += chRandom;
		nStrLength--;
	}

	return strRandom;
}

template<class VALUE_TYPE>
class CTestValue : public CTestObservableObject
{
public:
	void OnChange(CObservableObject *pSource, void *pValue)
	{
		cout << "\n\t\tReceived change event, old value = ";

		if (pValue)
		{
			VALUE_TYPE *pOldValue = (VALUE_TYPE *)pValue;
			cout << (*pOldValue) << endl;
			m_oldValue = (*pOldValue);
		}
		else
		{
			cout << "NULL" << endl;
		}

		m_bReceivedChange = TRUE;
	}

	// run the test
	static void RunTest()
	{
		const char *pszTypeName = typeid(VALUE_TYPE).name();
		BEGIN_TEST_BANNER("CValue<" << pszTypeName << ">");

		///////////////////////////////////////////////////////////////////////
		//	Test (1) 
		//		- Initialize a new CValue with a random initial value
		//		* The stored value should equal the random initial value

			VALUE_TYPE initialValue = CreateRandom<VALUE_TYPE>();
			PERFORM_STEP("Creating a CValue<" << pszTypeName << ">",
				CValue<VALUE_TYPE> testValue(initialValue));

		// The stored value should equal the initial value
		ASSERT_RESULT(1, testValue.Get() == initialValue); ////////////////////

		///////////////////////////////////////////////////////////////////////
		//	Test (2) 
		//		- Create an observer
		//		- Add the observer to the observable
		//		- Change the observable's value to a new value
		//		* Observer should receive the change with the initial value
		//			as the old value

			PERFORM_STEP("Creating observer...",
				CTestValue testValueObserver);

			PERFORM_STEP("Adding observer...",
				testValue.AddObserver(&testValueObserver, (ChangeFunction) OnChange));

			VALUE_TYPE newValue = CreateRandom<VALUE_TYPE>();
			PERFORM_STEP("Changing the value to " << (VALUE_TYPE) newValue,
				testValue.Set(newValue));

		ASSERT_RESULT(2, testValueObserver.m_bReceivedChange &&	///////////////
			testValueObserver.m_oldValue == initialValue);

		// reset the change flag
		testValueObserver.Reset();

		///////////////////////////////////////////////////////////////////////
		//	Test (3) 
		//		* The stored value should now equal the newly generated value

		ASSERT_RESULT(3, testValue.Get() == newValue); ////////////////////////

		///////////////////////////////////////////////////////////////////////
		//	Test (4) 
		//		- Remove the observer
		//		- Change the value
		//		* The listener should not have received a change event

			PERFORM_STEP("Removing the observer...",
				testValue.RemoveObserver(&testValueObserver, (ChangeFunction) OnChange));

			VALUE_TYPE newValue2 = CreateRandom<VALUE_TYPE>();
			PERFORM_STEP("Changing the value to " << (VALUE_TYPE) newValue2,
				testValue.Set(newValue2));

		ASSERT_RESULT(4, !testValueObserver.m_bReceivedChange);	///////////////

		///////////////////////////////////////////////////////////////////////
		//	Test (5) 
		//		- Create an observer
		//		- Create an observable object
		//		- Add an observer
		//		- Fire a change with a dummy pointer
		//		* Observer should receive the change, receiving the dummy pointer

		// The new value should equal the second randomly generated value
		ASSERT_RESULT(5, testValue.Get() == newValue2); ///////////////////////


		///////////////////////////////////////////////////////////////////////
		//	Test (6)
		//		Synchronization - values
		//		- Create a second value
		//		- Initialize the second value to a random value
		//		- Sync the second value to the first value
		//		* The value of the second should equal the value of the first

			VALUE_TYPE initialValue2 = CreateRandom<VALUE_TYPE>();
			PERFORM_STEP("Creating a CValue<" << pszTypeName << ">",
				CValue<VALUE_TYPE> testValue2(initialValue2));

			PERFORM_STEP("Syncing to the first test value",
				testValue2.SyncTo(&testValue));

		ASSERT_RESULT(6, testValue2.Get() == testValue.Get());

		///////////////////////////////////////////////////////////////////////
		//	Test (7)
		//		Synchronization - change events
		//		- Change the second value
		//		* the first value should equal the second value

			// generate a unique new value
			VALUE_TYPE newValue3;
			do {
				newValue3 = CreateRandom<VALUE_TYPE>();
			} while (newValue3 == newValue);
			PERFORM_STEP("Changing the synched-to object",
				testValue.Set(newValue3));

		ASSERT_RESULT(7, testValue.Get() == testValue2.Get());

		///////////////////////////////////////////////////////////////////////
		//	Test (8)
		//		De-Synchronization - values
		//		- De-synchronize the synchronized object
		//		- Change the value of the first
		//		* The value of the first should be different than the second,
		//			and the second should

			PERFORM_STEP("De-synchronizing the synchronized object",
				testValue2.SyncTo(NULL));

			PERFORM_STEP("Change the value of the first",
				testValue.Set(newValue));
			
		ASSERT_RESULT(8, testValue.Get() != testValue2.Get()
			&& testValue2.Get() == newValue3);

		///////////////////////////////////////////////////////////////////////
		//	Test (9)
		//		De-Synchronization - change events

		//		Test for synchronization to NULL, synchronization to self

		///////////////////////////////////////////////////////////////////////
		//	Test (10) 
		//		De-Synchronization - after synchronized object is deleted
	

		END_TEST_BANNER("CValue<" << pszTypeName << ">");
	}

protected:
	VALUE_TYPE m_oldValue;
};


#endif // !defined(AFX_TESTVALUE_H__09C4CC8B_60E4_11D5_AB84_00B0D0AB90D6__INCLUDED_)
