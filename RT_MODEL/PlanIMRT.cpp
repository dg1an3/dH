// PlanIMRT.cpp: implementation of the CPlanIMRT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "TestHisto.h"
#include <PlanIMRT.h>

#include <InvFilterEM.h>

#include <PowellOptimizer.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <MatrixBase.inl>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::CPlanIMRT
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPlanIMRT::CPlanIMRT()
{
}	// CPlanIMRT::CPlanIMRT


///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::~CPlanIMRT
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPlanIMRT::~CPlanIMRT()
{
	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		for (int nAt = 0; nAt < m_arrHistograms[nAtScale].GetSize(); nAt++)
		{
			if (nAtScale > 0)
			{
				delete ((CHistogram *)m_arrHistograms[nAtScale][nAt])->GetRegion();
			}

			delete m_arrHistograms[nAtScale][nAt];
		}
	}

}	// CPlanIMRT::~CPlanIMRT


///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::SetBeamCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::SetBeamCount(int nCount)
{
	// TODO: clear current beams
	for (int nAt = 0; nAt < m_arrBeams.GetSize(); nAt++)
		delete m_arrBeams[nAt];
	m_arrBeams.RemoveAll();

	// create new beams
	REAL angleDelta = atan(1.0) * 8.0 / (REAL) nCount;
	REAL angle = 0.0; // 
		// 2.25 * atan(1.0) * 8.0 / 7.0;
	for (int nAngle = 0; nAngle < nCount; nAngle++)
	{
		CBeamIMRT *pBeam = new CBeamIMRT();
		pBeam->SetGantryAngle(angle);
		m_arrBeams.Add(pBeam);

		angle += angleDelta;
	}

}	// CPlanIMRT::SetBeamCount

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetBeam
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeamIMRT * CPlanIMRT::GetBeam(int nAt)
{
	return (CBeamIMRT *) m_arrBeams[nAt];

}	// CPlanIMRT::GetBeam

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::GetStateVector(int nScale, CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights(GetBeamCount(), GetBeam(0)->GetBeamletCount(nScale));
	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		mBeamletWeights[nAtBeam] = GetBeam(nAtBeam)->GetIntensityMap(nScale);
	}
	BeamletWeightsToStateVector(nScale, mBeamletWeights, vState);

}	// CPlanIMRT::GetStateVector

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::SetStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::SetStateVector(int nScale, const CVectorN<>& vState)
{
	CMatrixNxM<> mBeamletWeights;
	StateVectorToBeamletWeights(nScale, vState, mBeamletWeights);

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		GetBeam(nAtBeam)->SetIntensityMap(nScale, mBeamletWeights[nAtBeam]);
	}

}	// CPlanIMRT::SetStateVector


///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetBeamletFromSVElem
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::GetBeamletFromSVElem(int nElem, int nScale, int *pnBeam, int *pnBeamlet)
{
	(*pnBeam) = nElem % GetBeamCount();

	int nShift = (nElem / GetBeamCount() + 1) / 2;
	int nDir = pow(-1, nElem / GetBeamCount());
	(*pnBeamlet) = nShift * nDir;

}	// CPlanIMRT::GetBeamletFromSVElem

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetTotalBeamletCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlanIMRT::GetTotalBeamletCount(int nScale)
{
	int nBeamlets = 0;

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		nBeamlets += GetBeam(nAtBeam)->GetBeamletCount(nScale);
	}

	return nBeamlets;

}	// CPlanIMRT::GetTotalBeamletCount

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::AddRegion
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlanIMRT::AddRegion(CVolume<double> *pRegion)
{
	BEGIN_LOG_SECTION(CPlanIMRT::AddRegion);

	ASSERT(pRegion->GetWidth() == GetDoseMatrix(0)->GetWidth());

	double filtElem[] = { 1.0, 4.0, 6.0, 4.0, 1.0 };

	CVolume<double> filtGauss5x5;
	filtGauss5x5.SetDimensions(5, 5, 1);
	double sum = 0.0;
	for (int nAtRow = 0; nAtRow < 5; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < 5; nAtCol++)
		{
			// double value = Gauss2D<double>(((double) nAtRow - 3.0) / 2.0, 
			//		((double) nAtCol - 3.0) / 2.0, 1.0, 1.0);
			double value = filtElem[nAtRow] * filtElem[nAtCol] / 256.0;
			filtGauss5x5.GetVoxels()[0][nAtRow][nAtCol] = value;
			sum += value;
		}
	}

	for (int nAtScale = 0; nAtScale < MAX_SCALES; nAtScale++)
	{
		LOG_OBJECT((*pRegion));

		CHistogram *pHisto = new CHistogram();
		pHisto->SetVolume(GetDoseMatrix(nAtScale));
		pHisto->SetRegion(pRegion);

		for (int nAtElem = 0; nAtElem < GetTotalBeamletCount(nAtScale); nAtElem++)
		{
			int nBeam;
			int nBeamlet;
			GetBeamletFromSVElem(nAtElem, nAtScale, &nBeam, &nBeamlet);

			pHisto->Add_dVolume(GetBeam(nBeam)->GetBeamlet(nBeamlet, nAtScale));
		}

		m_arrHistograms[nAtScale].Add(pHisto);

		if (nAtScale < MAX_SCALES-1)
		{
			CVolume<double> convRegion;
			Convolve(pRegion, &filtGauss5x5, &convRegion);

			CVolume<double> *pDecRegion = new CVolume<double>();
			Decimate(&convRegion, pDecRegion);

			pRegion = pDecRegion;
		}
	}

	END_LOG_SECTION();

	return m_arrHistograms[0].GetSize();

}	// CPlanIMRT::AddRegion

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetRegionCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlanIMRT::GetRegionCount()
{
	return m_arrHistograms[0].GetSize();

}	// CPlanIMRT::GetRegionCount

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetRegion
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> * CPlanIMRT::GetRegion(int nAt, int nScale)
{
	return GetRegionHisto(nAt, nScale)->GetRegion();

}	// CPlanIMRT::GetRegion

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::GetRegionHisto
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CHistogram * CPlanIMRT::GetRegionHisto(int nAt, int nScale)
{
	return (CHistogram *) m_arrHistograms[nScale][nAt];

}	// CPlanIMRT::GetRegionHisto

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::InvFilterStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::InvFilterStateVector(const CVectorN<>& vIn, int nScale, CVectorN<>& vOut, BOOL bFixNeg)
{
	BEGIN_LOG_SECTION(CPlanIMRT::InvFilterStateVector);

	CMatrixNxM<> mBeamletWeights;
	StateVectorToBeamletWeights(nScale, vIn, mBeamletWeights);
	LOG_EXPR_EXT(mBeamletWeights);

	CInvFilterEM em_obj;
	CMatrixNxM<> mFiltBeamletWeights(mBeamletWeights.GetCols(),
		GetBeam(0)->GetBeamletCount(nScale-1));

	CVectorN<> vInit;
	CVectorN<> vRes;
	CVectorN<> vSubOutput;
	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		em_obj.SetFilterOutput(mBeamletWeights[nAtBeam]);
		mFiltBeamletWeights[nAtBeam] = em_obj.m_mFilter * em_obj.m_vFilterOut;
		mFiltBeamletWeights[nAtBeam] *= 2.0;

	} 
	LOG_EXPR_EXT(mFiltBeamletWeights);

	BeamletWeightsToStateVector(nScale-1, mFiltBeamletWeights, vOut); 


	END_LOG_SECTION();	// CPlanIMRT::InvFilterStateVector

}	// CPlanIMRT::InvFilterStateVector

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::StateVectorToBeamletWeights
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::StateVectorToBeamletWeights(int nScale, const CVectorN<>& vState, CMatrixNxM<>& mBeamletWeights)
{
	int nBeamletCount = GetBeam(0)->GetBeamletCount(nScale);
	int nBeamletOffset = nBeamletCount / 2;
	mBeamletWeights.Reshape(GetBeamCount(), nBeamletCount);
	for (int nAtElem = 0; nAtElem < GetTotalBeamletCount(nScale); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, nScale, &nBeam, &nBeamlet);

		mBeamletWeights[nBeam][nBeamlet + nBeamletOffset] = vState[nAtElem];
	}

}	// CPlanIMRT::StateVectorToBeamletWeights

///////////////////////////////////////////////////////////////////////////////
// CPlanIMRT::BeamletWeightsToStateVector
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlanIMRT::BeamletWeightsToStateVector(int nScale, const CMatrixNxM<>& mBeamletWeights, CVectorN<>& vState)
{
	int nBeamletCount = GetBeam(0)->GetBeamletCount(nScale);
	vState.SetDim(GetTotalBeamletCount(nScale));
	int nBeamletOffset = nBeamletCount / 2;
	for (int nAtElem = 0; nAtElem < GetTotalBeamletCount(nScale); nAtElem++)
	{
		int nBeam;
		int nBeamlet;
		GetBeamletFromSVElem(nAtElem, nScale, &nBeam, &nBeamlet);

		vState[nAtElem] = mBeamletWeights[nBeam][nBeamlet + nBeamletOffset];
	}

}	// CPlanIMRT::BeamletWeightsToStateVector
