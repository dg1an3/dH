// Test_GEOM_MODEL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Test_GEOM_MODEL.h"

#include <Polygon.h>
#include <Volumep.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

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
		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;

		CPolygon contour;
		contour.AddVertex(CVectorD<2>(10.0, 20.0));
		contour.AddVertex(CVectorD<2>(15.0, 25.0));
		contour.AddVertex(CVectorD<2>(20.0, 20.0));
		contour.AddVertex(CVectorD<2>(15.0, 15.0));
		contour.AddVertex(CVectorD<2>(10.0, 20.0));


		CVolume<double> region;

		CreateRegion(&contour, &region);
	}

	return nRetCode;
}


