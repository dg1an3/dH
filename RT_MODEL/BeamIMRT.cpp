// BeamIMRT.cpp: implementation of the CBeamIMRT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "TestHisto.h"
#include <BeamIMRT.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <MatrixBase.inl>
#include <MatrixD.h>


template<class VOXEL_TYPE>
void ReadVolumeFile(const char *pszFilename, CVolume<VOXEL_TYPE> *pBeamlet)
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
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::CBeamIMRT
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeamIMRT::CBeamIMRT()
{
	m_vWeightFilter.SetDim(3);
	m_vWeightFilter[0] = 0.25;
	m_vWeightFilter[1] = 0.50;
	m_vWeightFilter[2] = 0.25;

	double filtElem[] = { 1.0, 4.0, 6.0, 4.0, 1.0 };

	m_filtGauss5x5.SetDimensions(5, 5, 1);
	double sum = 0.0;
	for (int nAtRow = 0; nAtRow < 5; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < 5; nAtCol++)
		{
			// double value = Gauss2D<double>(((double) nAtRow - 3.0) / 2.0, 
			//		((double) nAtCol - 3.0) / 2.0, 1.0, 1.0);
			double value = filtElem[nAtRow] * filtElem[nAtCol] / 256.0;
			m_filtGauss5x5.GetVoxels()[0][nAtRow][nAtCol] = value;
			sum += value;
		}
	}

	for (nAtRow = 0; nAtRow < m_filtGauss5x5.GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < m_filtGauss5x5.GetWidth(); nAtCol++)
		{
			m_filtGauss5x5.GetVoxels()[0][nAtRow][nAtCol] /= sum;
		}
	}

	GenBeamlets();

	GenFilterMat();

}	// CBeamIMRT::CBeamIMRT

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::~CBeamIMRT
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeamIMRT::~CBeamIMRT()
{
	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		for (int nAtBeamlet = 0; nAtBeamlet < m_arrBeamlets[nAtScale].GetSize(); nAtBeamlet++)
		{
			delete m_arrBeamlets[nAtScale][nAtBeamlet];
		}

		m_arrBeamlets[nAtScale].RemoveAll();
	}

}	// CBeamIMRT::~CBeamIMRT

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::SetGantry
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamIMRT::SetGantryAngle(REAL gantryAngle)
{
	CBeam::SetGantryAngle(gantryAngle);
	// m_gantry = gantry;

	GenBeamlets();

}	// CBeamIMRT::SetGantry


///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GetBeamletCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CBeamIMRT::GetBeamletCount(int nScale)
{
	return m_arrBeamlets[nScale].GetSize();

}	// CBeamIMRT::GetBeamletCount

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GetBeamlet
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> * CBeamIMRT::GetBeamlet(int nShift, int nScale)
{
	return (CVolume<double> *) m_arrBeamlets[nScale]
		[nShift + GetBeamletCount(nScale) / 2];

}	// CBeamIMRT::GetBeamlet

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GetIntensityMap
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CBeamIMRT::GetIntensityMap(int nScale) const
{
/*	if (m_arrRecalcWeights[nScale])
	{
		// get the previous beamlet weights
		CVectorN<> vPrev = GetBeamletWeights(nScale-1);

	} */

	return m_arrBeamletWeights[nScale];


}	// CBeamIMRT::GetIntensityMap


///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::SetIntensityMap
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamIMRT::SetIntensityMap(int nScale, const CVectorN<>& vWeights)
{
	ASSERT(m_arrBeamlets[nScale].GetSize() == vWeights.GetDim());

/*	for (int nAt = 0; nAt < MAX_SCALES; nAt++)
	{
		m_arrRecalcDose[nScale] = TRUE;
		m_arrRecalcWeights[nAt] = TRUE;
	} */

	m_arrBeamletWeights[nScale] = vWeights;
	m_arrRecalcDose[nScale] = TRUE;
	m_arrRecalcWeights[nScale] = FALSE;


}	// CBeamIMRT::SetIntensityMap

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GetDoseMatrix
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> *CBeamIMRT::GetDoseMatrix(int nScale)
{
	CVolume<double> *pDose = &m_arrDose[nScale];

	if (TRUE) // m_arrRecalcDose[nScale])
	{ 
		pDose->ClearVoxels();

		for (int nAt = 0; nAt < m_arrBeamlets[nScale].GetSize(); nAt++)
		{
			CVolume<double> *pBeamlet = (CVolume<double> *) m_arrBeamlets[nScale][nAt];
			pDose->Accumulate(pBeamlet, m_arrBeamletWeights[nScale][nAt]);
		}

		m_arrRecalcDose[nScale] = FALSE;
	}  

	return pDose;

}	// CBeamIMRT::GetDoseMatrix

///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GenBeamlets
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamIMRT::GenBeamlets()
{
	BEGIN_LOG_SECTION(CBeamIMRT::GenBeamlets);

	int nMargin = 22;
	LOG_EXPR(nMargin);

	// generate number of shifts for base scale
	int nShifts = pow(2, MAX_SCALES+1) - 1;
	int nDim = nShifts * 2 + 1 + 2 * nMargin;
	LOG_EXPR(nShifts);
	LOG_EXPR(nDim);

	const REAL CYL_SIGMA = 1.0;
	LOG_EXPR(CYL_SIGMA);

	for (int nAt = 0; nAt < m_arrBeamlets[0].GetSize(); nAt++)
		delete m_arrBeamlets[0][nAt];
	m_arrBeamlets[0].RemoveAll();

	// generate beamlets for base scale
	for (int nAtShift = -nShifts; nAtShift <= nShifts; nAtShift++)
	{
		CVolume<double> *pBeamlet = new CVolume<double>;
		pBeamlet->SetDimensions(nDim, nDim, 1);

#define READ_BEAMLETS
#ifdef READ_BEAMLETS
		CVolume<double> beamletOrig;
		beamletOrig.SetDimensions(127, 127, 1); // 101, 101, 1);

		// form file name
		char pszFilename[80];
		sprintf(pszFilename, "..\\..\\..\\cono\\output%i\\format_dose.dat", nAtShift + 50);

		ReadVolumeFile(pszFilename, &beamletOrig);

		// rotate to new beamlet position
		::Rotate(&beamletOrig, CVectorD<2>(63.0, 63.0), GetGantryAngle(),
			pBeamlet, CVectorD<2>(nDim / 2 + 1, nDim / 2 + 1));
#else		
		double ***pppVoxels = pBeamlet->GetVoxels();
		for (int nAtRow = 0; nAtRow < pBeamlet->GetHeight(); nAtRow++)
		{
			for (int nAtCol = 0; nAtCol < pBeamlet->GetWidth(); nAtCol++)
			{
				double x = cos(m_gantry) * (double) (nAtCol - nShifts - nMargin)
					- sin(m_gantry) * (double) (nAtRow - nShifts - nMargin)
					- (double) nAtShift;

				double px = 0.2 * (nAtCol - nShifts - nMargin);
				double py = 0.2 * (nAtRow - nShifts - nMargin);

				double dx = sin(m_gantry);
				double dy = -cos(m_gantry);

				double b = px * dx + py * dy;
				double c = px * px + py * py - 100;

				double lambda1 = (-b + sqrt(b*b - 4.0*c)) / 2.0;
				double lambda2 = (-b - sqrt(b*b - 4.0*c)) / 2.0;
				double depth = __max(lambda1, lambda2);

				pppVoxels[0][nAtRow][nAtCol] = exp(-0.043 * depth) 
					* Gauss<double>(x, CYL_SIGMA);
			}
		}    
#endif

		LOG_OBJECT((*pBeamlet));

		m_arrBeamlets[0].Add(pBeamlet);

	}
	m_arrBeamletWeights[0].SetDim(m_arrBeamlets[0].GetSize());
	m_arrBeamletWeights[0].SetZero();

	m_arrDose[0].SetDimensions(nDim, nDim, 1);

	for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
	{
		for (int nAt = 0; nAt < m_arrBeamlets[nAtScale].GetSize(); nAt++)
			delete m_arrBeamlets[nAtScale][nAt];

		m_arrBeamlets[nAtScale].RemoveAll();

		nShifts = pow(2, MAX_SCALES+1 - nAtScale) - 1;

		// generate beamlets for base scale
		for (int nAtShift = -nShifts; nAtShift <= nShifts; nAtShift++)
		{
			CVolume<double> *pBeamlet = new CVolume<double>;
			pBeamlet->SetDimensions(nDim, nDim, 1);
			pBeamlet->ClearVoxels();

			pBeamlet->Accumulate(GetBeamlet(nAtShift * 2 - 1, nAtScale-1), 2.0 * m_vWeightFilter[0]);
			pBeamlet->Accumulate(GetBeamlet(nAtShift * 2 + 0, nAtScale-1), 2.0 * m_vWeightFilter[1]);
			pBeamlet->Accumulate(GetBeamlet(nAtShift * 2 + 1, nAtScale-1), 2.0 * m_vWeightFilter[2]);

			// convolve with gaussian
			CVolume<double> *pBeamletConv = new CVolume<double>;
			Convolve(pBeamlet, &m_filtGauss5x5, pBeamletConv);
			delete pBeamlet;

			CVolume<double> *pBeamletConvDec = new CVolume<double>;
			Decimate(pBeamletConv, pBeamletConvDec);
			delete pBeamletConv;

			m_arrBeamlets[nAtScale].Add(pBeamletConvDec);
		}
		m_arrBeamletWeights[nAtScale].SetDim(m_arrBeamlets[nAtScale].GetSize());
		m_arrBeamletWeights[nAtScale].SetZero();

		nDim = GetBeamlet(0, nAtScale)->GetWidth();
		m_arrDose[nAtScale].SetDimensions(nDim, nDim, 1);
	}

	END_LOG_SECTION();	// CBeamIMRT::GenBeamlets

}	// CBeamIMRT::GenBeamlets


///////////////////////////////////////////////////////////////////////////////
// CBeamIMRT::GenFilterMat
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamIMRT::GenFilterMat()
{
	for (int nAtScale = 0; nAtScale < MAX_SCALES-1; nAtScale++)
	{
		// generate the filter matrix
		m_mFilter[nAtScale].Reshape(GetBeamletCount(nAtScale), GetBeamletCount(nAtScale+1));
		ZeroMemory(m_mFilter[nAtScale], 
			m_mFilter[nAtScale].GetRows() * m_mFilter[nAtScale].GetCols() * sizeof(REAL));

		// iterate over rows
		for (int nAtRow = 0; nAtRow < GetBeamletCount(nAtScale+1); nAtRow++)
		{
			int nColStart = nAtRow * 2 + 1 - m_vWeightFilter.GetDim() / 2;
			for (int nColOffset = 0; nColOffset < m_vWeightFilter.GetDim(); nColOffset++)
			{
				m_mFilter[nAtScale][nColStart + nColOffset][nAtRow] = m_vWeightFilter[nColOffset];
			}
		}

//		TRACE_MATRIX("m_mFilter", m_mFilter[nAtScale]);

//		m_mFilter_pinv[nAtScale].Reshape(GetBeamletCount(nAtScale), GetBeamletCount(nAtScale+1));
//		m_mFilter_pinv[nAtScale] = m_mFilter[nAtScale];
//		m_mFilter_pinv[nAtScale].Pseudoinvert();
	}

}	// CBeamIMRT::GenFilterMat


