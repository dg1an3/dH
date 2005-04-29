// TestGEOM_MODEL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestGEOM_MODEL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

#include <Volumep.h>

template<class VOXEL_TYPE>
void ReadBeamlet(const char *pszFilename, CVolume<VOXEL_TYPE> *pBeamlet)
{
	FILE *hFile = fopen(pszFilename, "rt");

	int nAtCol = 0; 
	int nAtRow = 0;
	char pszLine[40];
	// read first blank line
	fgets(pszLine, 40, hFile);

	do
	{
		if (fgets(pszLine, 40, hFile) != NULL)
		{
			VOXEL_TYPE value;
			if (sscanf(pszLine, "%e", &value) == 1)
			{
				pBeamlet->GetVoxels()[0][nAtRow][nAtCol] = value;
				nAtCol++;
			}
			else
			{
				nAtRow++;
				nAtCol = 0;
			}
		}

	} while (!feof(hFile));

	fclose(hFile);
}

/*
template<class VOXEL_TYPE>
void WriteCSV(const char *pszFilename, CVolume<VOXEL_TYPE> *pVol, int nAtDepth)
{
	FILE *hFile = fopen(pszFilename, "wt");

	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth(); nAtCol++)
		{
			fprintf(hFile, "%f", pVol->GetVoxels()[nAtDepth][nAtRow][nAtCol]);
			if (nAtCol < pVol->GetWidth()-1)
				fprintf(hFile, ",");
		}

		fprintf(hFile, "\n");
	}

	fclose(hFile);
}
*/

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
		BEGIN_LOG_SECTION(TestRotate::main);

		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;

		CVolume<REAL> volBeamlet;
		volBeamlet.SetDimensions(101, 101, 1);

		CMatrixD<4> mBasis;
		mBasis[3][0] = -50.0;
		mBasis[3][1] = -50.0;
		volBeamlet.SetBasis(mBasis);

		ReadBeamlet("dose.dat", &volBeamlet);

		CVolume<REAL> volRotateBeamlet;
		volRotateBeamlet.ConformTo(&volBeamlet);
		volRotateBeamlet.ClearVoxels();

		CMatrixD<2> mRot2D = ::CreateRotate(0.1);

		CMatrixD<4> mRotate;
		mRotate[0][0] = mRot2D[0][0];
		mRotate[0][1] = mRot2D[0][1];
		mRotate[1][0] = mRot2D[1][0];
		mRotate[1][1] = mRot2D[1][1];
		LOG_EXPR(mRotate);

		volRotateBeamlet.SetBasis(mRotate *volRotateBeamlet.GetBasis());

		::Resample(&volBeamlet, &volRotateBeamlet, TRUE);

/*		CVolume<REAL> *pCropBeamlet = new CVolume<REAL>;
		pCropBeamlet->SetDimensions(31, 31, 1);

		::Rotate(&volBeamlet, CVectorD<2>(50.0, 50.0), 0.1,
			pCropBeamlet, CVectorD<2>(15.0, 15.0));
*/
		WriteCSV("orig.csv", &volBeamlet, 0);
		WriteCSV("rot.csv", // pCropBeamlet, 0);
			&volRotateBeamlet, 0);

		END_LOG_SECTION(); // TestRotate::main);
	}

	return nRetCode;
}


