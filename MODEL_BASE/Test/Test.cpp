// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Test.h"

#include "TestObservableObject.h"
#include "TestValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;


void Pause()
{
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		CTestObservableObject::RunTest();
		Pause();
		cout << endl << endl;

		CTestValue<int>::RunTest();
		Pause();
		cout << endl << endl;

		CTestValue<double>::RunTest();
		Pause();
		cout << endl << endl;

		CTestValue<BOOL>::RunTest();
		Pause();
		cout << endl << endl;

		CTestValue<CString>::RunTest();
		Pause();
		cout << endl << endl;
	}

	return nRetCode;
}


