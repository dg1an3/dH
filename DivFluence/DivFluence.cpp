// DivFluence.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DivFluence.h"

#include <math.h>

#include "Source.h"

#include "Beam.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////
// input data

/////////////////////////////////////////////
// input parameters

double ssd = 80.0; 

double lengthx = 0.2; 
double lengthy = 1.0; 
double lengthz = 0.2; 

double ray = 6.0;

double xmin = -0.1; 
double xmax = 0.1; 
double ymin = -5.0; 
double ymax = 5.0;

double thickness = 20.0;

double mu = 0.0494;


/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

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
		cout << (LPCTSTR) strHello << endl;

		CVolume<double> *pDensity = new CVolume<double>();
		pDensity->SetDimensions(101, 101, 101);

		// populate density
		for (int nAtZ = 1; nAtZ <= 101; nAtZ++)
		{
			for (int nAtY = -50; nAtY <= 50; nAtY++)
			{
				for (int nAtX = -50; nAtX <= 50; nAtX++)
				{
					if ((nAtX * nAtX + nAtY * nAtY) < 45 * 45)
					{
						pDensity->GetVoxels()[nAtZ-1][nAtY+50][nAtX+50] = 1.0;
					}
				}
			}
		}
		CMatrixD<4> mBasis;
		mBasis[0][0] = lengthz;
		mBasis[1][1] = lengthx;
		mBasis[2][2] = lengthy;
		mBasis[3][1] = -(REAL) (pDensity->GetWidth() / 2) * lengthx;
		mBasis[3][2] = -(REAL) (pDensity->GetHeight() / 2) * lengthy;
		pDensity->SetBasis(mBasis);

		// now check fluence
		CXMLElement *pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "_tmain");
		pElem->Attribute("type", "CMatrix");
		pElem->Attribute("name", "pDensity mPlane 50");
		pDensity->LogPlane(50, pElem);
		CXMLLogFile::GetLogFile()->CloseElement();

		// LOG_OBJECT((*pDensity));

		CEnergyDepKernel *pSource = new CEnergyDepKernel();
		pSource->ReadDoseSpread("lang48rad48.dat");

		CBeam *pBeam = new CBeam(pSource, ssd);
		pBeam->xmin = xmin; 
		pBeam->xmax = xmax; 
		pBeam->ymin = ymin; 
		pBeam->ymax = ymax;

		pBeam->SetDoseCalcRegion(CVectorD<3>(0.2, -10.0, -5.0),
			CVectorD<3>(20.0, 10.0, 5.0));

		pBeam->SetDensity(pDensity,	mu);

		pBeam->DivFluenceCalc(ray, thickness);

		// now check fluence
		pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "_tmain");
		pElem->Attribute("type", "CMatrix");
		pElem->Attribute("name", "GetFluence mPlane 50");
		pBeam->GetFluence()->LogPlane(50, pElem);
		CXMLLogFile::GetLogFile()->CloseElement();
//		LOG_OBJECT((*pBeam->GetFluence()));

		pBeam->SphereConvolve(thickness);

		// now check energy
		pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "_tmain");
		pElem->Attribute("type", "CMatrix");
		pElem->Attribute("name", "GetEnergy mPlane 50");
		pBeam->GetEnergy()->LogPlane(50, pElem);
		CXMLLogFile::GetLogFile()->CloseElement();
//		LOG_OBJECT((*pBeam->GetEnergy()));

		delete pBeam;
		delete pSource;
		delete pDensity;
	}
	
	return nRetCode;

}	// _tmain


