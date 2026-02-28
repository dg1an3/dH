// TestObservableObject.cpp: implementation of the CTestObservableObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestObservableObject.h"

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTestObservableObject::CTestObservableObject()
	: m_bReceivedChange(FALSE),
		m_pOldValue(NULL)
{
}

// test change listener
void CTestObservableObject::OnChange(CObservableObject *pSource, void *pValue)
{
	// message to indicate the change event had occurred
	cout << "\n\t\tReceived change event, old value = ";

	// store the pointer
	m_pOldValue = pValue;

	// print out the pointer
	cout << pValue << endl;

	// set the flag
	m_bReceivedChange = TRUE;
}

void CTestObservableObject::Reset()
{
	// reset the flag
	m_bReceivedChange = FALSE;

	// reset the pointer
	m_pOldValue = NULL;
}

// run the test
void CTestObservableObject::RunTest()
{
	BEGIN_TEST_BANNER("CObservableObject");

	//	Test (1) 
	//		- Create an observer
	//		- Create an observable object
	//		- Add an observer
	//		- Fire a change with a dummy pointer
	//		* Observer should receive the change, receiving the dummy pointer

		PERFORM_STEP("Creating observer", 
			CTestObservableObject testObserver);

		PERFORM_STEP("Creating observable object...",
			CObservableObject testObservable);

		PERFORM_STEP("Adding observer...",
			testObservable.AddObserver(&testObserver, (ChangeFunction) OnChange));

		int temp1;
		PERFORM_STEP("Firing a change with void * = " << &temp1,
			testObservable.FireChange(&temp1));

	// the observer should have received the change
	ASSERT_RESULT(1, testObserver.m_bReceivedChange
		&& testObserver.m_pOldValue == (void *)&temp1);

	// reset the observer for the next test
	testObserver.Reset();

	//	Test (2) 
	//		- Remove the observer
	//		- Fire a change
	//		* Observer should not receive the change

		PERFORM_STEP("Removing the observer...",
			testObservable.RemoveObserver(&testObserver, (ChangeFunction) OnChange));

		int temp2;
		PERFORM_STEP("Firing a change with void * = " << &temp2,
			testObservable.FireChange(&temp2));

	// the observer should NOT have received the change
	ASSERT_RESULT(2, !testObserver.m_bReceivedChange);

	// notify of end of test
	END_TEST_BANNER("CObservableObject");
}
