// GenDens.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GenDens.h"

#include <direct.h>

#include <Dib.h>
#include <Volumep.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CVolume<double> *GenBeamlet(CVolume<double> *pDensity, 
							double gantry, 
							double minCollim, double maxCollim,
							double densityScale)
{
	// rotate to gantry angle
	CVolume<double> densityRotate;
	densityRotate.SetDimensions(pDensity->GetWidth(), pDensity->GetHeight(), 1);

	CVectorD<2> vCenter(pDensity->GetWidth(), pDensity->GetHeight());
	vCenter *= 0.5;

	::Rotate(pDensity, vCenter, -gantry, &densityRotate, vCenter);

	// now write out pixels
	FILE *hFile = fopen("coni\\format_density.dat", "wt");
	if (hFile != NULL)
	{
		fprintf(hFile, "\n");
		for (int nRow = 0; nRow < densityRotate.GetHeight(); nRow++)
		{
			for (int nCol = 0; nCol < densityRotate.GetWidth(); nCol++)
			{
				fprintf(hFile, "\t%8.4lf\n", 
					densityScale * densityRotate.GetVoxels()[0][nRow][nCol]);
			}
			fprintf(hFile, "\n");
		}
		fclose(hFile);
	}

	// now prepare inputs
	CString strInputs;
	strInputs.Format(IDS_PENBEAM_INPUTS, minCollim, maxCollim);

	FILE *hInputFile = fopen("penbeam_input.txt", "wt");
	fprintf(hInputFile, "%s", strInputs.GetBuffer(256));
	fclose(hInputFile);

	system("do_convolve_main.bat");

	// now read the dose distribution for the pencil beam
	CVolume<double> dose;
	dose.SetDimensions(densityRotate.GetWidth(), densityRotate.GetHeight(), 1);
	FILE *hFileDose = fopen("cono\\format_dose.dat", "rt");
	if (hFileDose != NULL)
	{
		for (int nRow = 0; nRow < dose.GetHeight(); nRow++)
		{
			for (int nCol = 0; nCol < dose.GetWidth(); nCol++)
			{
				fscanf(hFileDose, "%lf", &dose.GetVoxels()[0][nRow][nCol]);
			}
		}

		fclose(hFileDose);
	}

	// now rotate the dose distribution for the pencil beam
	CVolume<double> *pDoseRotate = new CVolume<double>();
	pDoseRotate->SetDimensions(dose.GetWidth(), dose.GetHeight(), 1);

	::Rotate(&dose, vCenter, gantry, pDoseRotate, vCenter);

	return pDoseRotate;
}

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

		CDib dib;
		BOOL bRes = dib.Load("CT0044_full_offset_B0.bmp");

		BITMAP bm;
		dib.GetBitmap(&bm);
		bm.bmWidthBytes = 16 * ((bm.bmWidth * bm.bmBitsPixel) / 16 + 1) / bm.bmBitsPixel;

		CArray<BYTE, BYTE&> arrPixels;
		arrPixels.SetSize(bm.bmHeight * bm.bmWidthBytes);
		int nCount = dib.GetBitmapBits(arrPixels.GetSize(), arrPixels.GetData());
		ASSERT(nCount == arrPixels.GetSize());

		CVolume<double> density;
		density.SetDimensions(bm.bmWidth, bm.bmHeight, 1);
		for (int nRow = 0; nRow < bm.bmHeight; nRow++)
		{
			for (int nCol = 0; nCol < bm.bmWidth; nCol++)
			{
				density.GetVoxels()[0][nRow][nCol] = 
					arrPixels[nRow * bm.bmWidthBytes + nCol];
			}
		}

		_chdir("..\\..\\PenBeam1\\code\\");

		double gantry = 360.0 / 7.0 * PI / 180.0;
		double densityScale = 1.0 / 125.0;
		CVolume<double> *pBeamlet = 
			GenBeamlet(&density, gantry, -0.1, 0.1, densityScale);

		// now write out pixels
		FILE *hFileRot = fopen("cono\\dose_rotate.dat", "wt");
		if (hFileRot != NULL)
		{
			fprintf(hFileRot, "\n");
			for (int nRow = 0; nRow < bm.bmHeight; nRow++)
			{
				for (int nCol = 0; nCol < bm.bmWidth; nCol++)
				{
					fprintf(hFileRot, "\t%8.4lf\n", 
						pBeamlet->GetVoxels()[0][nRow][nCol]);
				}
				fprintf(hFileRot, "\n");
			}
			fclose(hFileRot);
		}
	}

	return nRetCode;
}


