// GenBeamlets.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GenBeamlets.h"

#include <direct.h>

#include <Beam.h>
#include <Plan.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


char g_pszBeamletPath[128];
BOOL g_bCylBeamlets = FALSE;


// TODO: Change scale -> level

///////////////////////////////////////////////////////////////////////////////
// ReadVolumeFile
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void ReadVolumeFile(const char *pszFilename, CVolume<VOXEL_TYPE> *pBeamlet)
{
	cout << pszFilename << endl;

	char pszDir[80];
	getcwd(pszDir, 80);
	cout << pszDir << endl;

	FILE *hFile = fopen(pszFilename, "rt");
	ASSERT(hFile != NULL);

	int nAtCol = 0; 
	int nAtRow = 0;
	char pszLine[40];
	// read first blank line
	fgets(pszLine, 40, hFile);

	do
	{
		if (fgets(pszLine, 40, hFile) != NULL)
		{
			double value;
			if (sscanf(pszLine, "%le", &value) == 1)
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

}	// ReadVolumeFile


///////////////////////////////////////////////////////////////////////////////
// GenBeamlets
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void GenBeamlets(CBeam *pBeam)
{
	BEGIN_LOG_SECTION(GenBeamlets);

	int nMargin = 22;
	LOG_EXPR(nMargin);

	// generate number of shifts for base scale
	int nShifts = pow(2, MAX_SCALES+1) - 1;
	int nDim = nShifts * 2 + 1 + 2 * nMargin;
	LOG_EXPR(nShifts);
	LOG_EXPR(nDim);

	for (int nAt = 0; nAt < pBeam->m_arrBeamlets[0].GetSize(); nAt++)
		delete pBeam->m_arrBeamlets[0][nAt];
	pBeam->m_arrBeamlets[0].RemoveAll();

	// generate beamlets for base scale
	for (int nAtShift = -nShifts; nAtShift <= nShifts; nAtShift++)
	{
		CVolume<REAL> *pBeamlet = new CVolume<REAL>;
		pBeamlet->SetDimensions(nDim, nDim, 1);

		if (g_bCylBeamlets)
		{
			const REAL CYL_SIGMA = 1.0;
			LOG_EXPR(CYL_SIGMA);

			REAL gantry= pBeam->GetGantryAngle();
			REAL ***pppVoxels = pBeamlet->GetVoxels();
			for (int nAtRow = 0; nAtRow < pBeamlet->GetHeight(); nAtRow++)
			{
				for (int nAtCol = 0; nAtCol < pBeamlet->GetWidth(); nAtCol++)
				{
					REAL x = cos(gantry) * (REAL) (nAtCol - nShifts - nMargin)
						- sin(gantry) * (REAL) (nAtRow - nShifts - nMargin)
						- (REAL) nAtShift;

					REAL px = 0.2 * (nAtCol - nShifts - nMargin);
					REAL py = 0.2 * (nAtRow - nShifts - nMargin);

					REAL dx = sin(gantry);
					REAL dy = -cos(gantry);

					REAL b = px * dx + py * dy;
					REAL c = px * px + py * py - 100;

					REAL lambda1 = (-b + sqrt(b*b - 4.0*c)) / 2.0;
					REAL lambda2 = (-b - sqrt(b*b - 4.0*c)) / 2.0;
					REAL depth = __max(lambda1, lambda2);

					pppVoxels[0][nAtRow][nAtCol] = exp(-0.043 * depth) 
						* Gauss<REAL>(x, CYL_SIGMA);
				}
			}    
		}
		else
		{
			static CTypedPtrArray<CPtrArray, CVolume<REAL> *> arrBeamlets;
			if (arrBeamlets.GetSize() == 0)
			{
				arrBeamlets.SetSize(100);
			}

			if (arrBeamlets[nAtShift + 50] == NULL)
			{
				arrBeamlets[nAtShift + 50] = new CVolume<REAL>;
				arrBeamlets[nAtShift + 50]->SetDimensions(127, 127, 1);

				// form file name
				char pszFilename[80];
				sprintf(pszFilename, 
					"%s\\output%i\\format_dose.dat", g_pszBeamletPath, nAtShift + 50);

				ReadVolumeFile(pszFilename, arrBeamlets[nAtShift + 50]);
			}

			CVolume<REAL> beamletOrig = (*arrBeamlets[nAtShift + 50]);

			// rotate to new beamlet position
			::Rotate(&beamletOrig, CVectorD<2>(63.0, 63.0), pBeam->GetGantryAngle(),
				pBeamlet, CVectorD<2>(nDim / 2 + 1, nDim / 2 + 1));

			CMatrixD<4> mBasis;
			mBasis[3][0] = -(pBeamlet->GetWidth() - 1) / 2;
			mBasis[3][1] = -(pBeamlet->GetHeight() - 1) / 2;
			pBeamlet->SetBasis(mBasis);
		}

		LOG_OBJECT((*pBeamlet));

		pBeam->m_arrBeamlets[0].Add(pBeamlet);

	}
	pBeam->m_vBeamletWeights.SetDim(pBeam->m_arrBeamlets[0].GetSize());
	pBeam->m_vBeamletWeights.SetZero();

	// triggers setting dose matrix dimensions
	// pBeam->GetDoseMatrix();

	// generate the kernel for convolution
	CVolume<REAL> kernel;
	kernel.SetDimensions(5, 5, 1);
	CalcBinomialFilter(&kernel);

	for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
	{
		for (int nAt = 0; nAt < pBeam->m_arrBeamlets[nAtScale].GetSize(); nAt++)
			delete pBeam->m_arrBeamlets[nAtScale][nAt];

		pBeam->m_arrBeamlets[nAtScale].RemoveAll();

		nShifts = pow(2, MAX_SCALES+1 - nAtScale) - 1;

		// generate beamlets for base scale
		for (int nAtShift = -nShifts; nAtShift <= nShifts; nAtShift++)
		{
			CVolume<REAL> *pBeamlet = new CVolume<REAL>;
			pBeamlet->SetDimensions(nDim, nDim, 1);
			pBeamlet->SetBasis(pBeam->GetBeamlet(0, nAtScale-1)->GetBasis());
			pBeamlet->ClearVoxels();

			pBeamlet->Accumulate(pBeam->GetBeamlet(nAtShift * 2 - 1, nAtScale-1), 
				2.0 * pBeam->m_vWeightFilter[0]);
			pBeamlet->Accumulate(pBeam->GetBeamlet(nAtShift * 2 + 0, nAtScale-1), 
				2.0 * pBeam->m_vWeightFilter[1]);
			pBeamlet->Accumulate(pBeam->GetBeamlet(nAtShift * 2 + 1, nAtScale-1), 
				2.0 * pBeam->m_vWeightFilter[2]);

			// convolve with gaussian
			CVolume<REAL> *pBeamletConv = new CVolume<REAL>;
			Convolve(pBeamlet, &kernel, pBeamletConv);
			delete pBeamlet;

			CVolume<REAL> *pBeamletConvDec = new CVolume<REAL>;
			Decimate(pBeamletConv, pBeamletConvDec);
			delete pBeamletConv;

			CMatrixD<4> mBasis;
			mBasis[3][0] = -(pBeamletConvDec->GetWidth() - 1) / 2;
			mBasis[3][1] = -(pBeamletConvDec->GetHeight() - 1) / 2;
			pBeamletConvDec->SetBasis(mBasis);

			pBeam->m_arrBeamlets[nAtScale].Add(pBeamletConvDec);
		}

		nDim = pBeam->GetBeamlet(0, nAtScale)->GetWidth();
	}

	END_LOG_SECTION();	// CBeamIMRT::GenBeamlets

}	// CBeamIMRT::GenBeamlets




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

		strcpy(g_pszBeamletPath, argv[1]);

		CPlan *pPlan = new CPlan();
		for (int nAt = 3; nAt < argc; nAt++)
		{
			double gantry;
			sscanf(argv[nAt], "%lf", &gantry);

			CBeam *pBeam = new CBeam();
			pBeam->SetGantryAngle(gantry * PI / 180.0);

			GenBeamlets(pBeam);
			pPlan->AddBeam(pBeam);
		}

		// create archive and store
		CFile planFile(argv[2], CFile::modeCreate 
			| CFile::modeWrite
			| CFile::shareExclusive
			| CFile::typeBinary);
		CArchive ar(&planFile, CArchive::store);

		CObArray arrWorkspace;
		arrWorkspace.Add(pPlan);
		arrWorkspace.Serialize(ar);
		ar.Close();
		planFile.Close();
	}

	return nRetCode;
}


