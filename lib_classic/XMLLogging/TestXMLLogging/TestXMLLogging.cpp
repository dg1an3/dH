// TestStructLogXML.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "resource.h"

#include <vector>

#define XMLLOGGING_ON
#include <XMLLogging.h>
USES_FMT;

#define USE_XMLLOGGING
#include <VectorD.h>
#include <MatrixD.h>
#include <MatrixBase.inl>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

class CTestXMLLoggingApp : public CWinApp
{
public:
	CTestXMLLoggingApp() 
	{ 
	}

	virtual BOOL InitApplication()
	{
		SetRegistryKey(_T("Local AppWizard-Generated Applications"));
		
		return CWinApp::InitApplication();
	}
};

CTestXMLLoggingApp theApp;


class LogTest : public CXMLLoggableObject
{
public:
	LogTest() 
		: CONSTRUCT_LOGGABLE_OBJECT(),
			m_nInt(24), 
			m_double(245.0) 
	{ 
	}

	DECLARE_DYNAMIC(LogTest);

	virtual void Log(CXMLElement *pElem) const
	{
		LOG_EXPR(m_nInt);
		LOG_EXPR(m_double);
	}

private:
	int m_nInt;
	double m_double;
};

IMPLEMENT_DYNAMIC(LogTest, CObject);

///////////////////////////////////////////////////////////////////////////////
// NextPerm
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
BOOL NextPerm(int *pInd, int nCount, int nMax)
{
	BOOL bRes = FALSE;
	
	BEGIN_LOG_SECTION(NextPerm);
	LOG_EXPR(*pInd);
	LOG_EXPR(nCount);
	LOG_EXPR(nMax);
	
	if (nCount == 0)
	{
		LOG("nCount == 0");
	}
	else if (!NextPerm(pInd+1, nCount-1, nMax))
	{
		if (*pInd < nMax - (nCount - 1))
		{ 
			LOG(FMT("Incrementing *pInd, = %i", *pInd));
			(*pInd)++;
			for (int nAt = 1; nAt < nCount; nAt++)
			{
				pInd[nAt] = pInd[nAt-1] + 1;
			}
			bRes = TRUE;
		}
	}
	
	END_LOG_SECTION();
	
	return bRes;

}	// NextPerm

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// _tmain
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		ASSERT(::AfxGetApp() == &theApp);
		BOOL bRes = ::AfxGetApp()->InitApplication();

		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;

		CXMLLogFile::GetLogFile()->LogExpr(2.123, "name", "file");

		BEGIN_LOG_SECTION(LogExpr::Hello);

		int nTest = 2;
		LOG_EXPR(nTest+1);

		double value = 1.2e-6;
		LOG_EXPR(value);

		LOG("Hello, World!");

		LOG(FMT("\n[ 1 0 0;\n  0 1 0;\n  0 0 %i ]\n", 1));

		END_LOG_SECTION();

		BEGIN_LOG_SECTION(LogObj);

		LogTest test;

		LOG_OBJECT(test);

		CVectorD<3> vD(1.0, 0.5, -0.4);
		CVectorD<3> vE(0.1, -1.5, -0.4);
		LOG_EXPR_EXT(vD);

		CMatrixD<> mMat;
		LOG_EXPR_EXT(mMat);

		int arrInd[] = { 1, 2, 3 };
		NextPerm(arrInd, 3, 5);

		LOG_EXPR(arrInd[0]);
		LOG_EXPR(arrInd[1]);
		LOG_EXPR(arrInd[2]);

		END_LOG_SECTION();
	}
	else
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}

	return nRetCode;

}	// _tmain


