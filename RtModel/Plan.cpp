// Copyright (C) 2nd Messenger Systems
// $Id: Plan.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include "Plan.h"

#include <EnergyDepKernel.h>

namespace dH 
{

///////////////////////////////////////////////////////////////////////////////
Plan::Plan()
	: m_pSeries(NULL)
	, m_DoseOriginOffset(0.0)
	, m_DoseResolution(4.0) // 
		// 2.0)
{
	m_pKernel = new CEnergyDepKernel(6.0); // 
		// 15.0);

	m_pMassDensity = VolumeReal::New();

	m_pDose = VolumeReal::New();
	m_pBeamDoseRot = VolumeReal::New();
	m_pTempBuffer = VolumeReal::New();

}

///////////////////////////////////////////////////////////////////////////////
Plan::~Plan()
{
	// delete the histograms
	for (auto& entry : m_mapHistograms)
	{
		delete entry.second;
	}

	// delete the kernel
	delete m_pKernel;
}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::SetSeries(dH::Series *pSeries)
{
	// store the series pointer
	m_pSeries = pSeries;

	// trigger calc of dose matrix basis
	SetDoseResolution(GetDoseResolution());
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::GetBeamCount() const
{
	return (int) m_arrBeams.size();
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::GetTotalBeamletCount()
{
	int nBeamlets = 0;

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		nBeamlets += GetBeamAt(nAtBeam)->GetBeamletCount();
	}

	return nBeamlets;
}

///////////////////////////////////////////////////////////////////////////////
CBeam * 
	Plan::GetBeamAt(int nAt)
{
	return m_arrBeams.at(nAt);
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::AddBeam(CBeam *pBeam)
{
	m_arrBeams.push_back(pBeam);
	int nIndex = (int) m_arrBeams.size();
	pBeam->SetPlan(this);

	// a change has occurred, so fire
	// GetChangeEvent().Fire();

	return nIndex;
}

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	Plan::GetDoseMatrix()
{
	// total the dose for all beams
	if (GetBeamCount() > 0)
	{
		// clear the dose matrix
		m_pDose->FillBuffer(0.0);

		for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		{
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pBeamDoseRot);
			m_pBeamDoseRot->FillBuffer(0.0); 

			VolumeReal *pBeamDose = GetBeamAt(nAt)->GetDoseMatrix();
			// Resample(pBeamDose, m_pBeamDoseRot, TRUE);
			// Resample3D(pBeamDose, m_pBeamDoseRot, TRUE);
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
				itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
			resampler->SetInput(pBeamDose);

			typedef itk::AffineTransform<REAL, 3> TransformType;
			TransformType::Pointer transform = TransformType::New();
			transform->SetIdentity();
			resampler->SetTransform(transform);

			typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
			InterpolatorType::Pointer interpolator = InterpolatorType::New();
			resampler->SetInterpolator( interpolator );

			VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(m_pBeamDoseRot);
			resampler->SetOutputParametersFromImage(m_pBeamDoseRot);
			resampler->Update();
			CopyImage<VOXEL_REAL, 3>(resampler->GetOutput(), m_pBeamDoseRot);

			// add this beam's dose matrix to the total
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pTempBuffer);
			// Accumulate<VOXEL_REAL>(m_pBeamDoseRot, 
			//	/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
			Accumulate3D<VOXEL_REAL>(m_pBeamDoseRot, 
				/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
		}
	}

	return m_pDose;

}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::UpdateAllHisto()
{
#ifdef USE_RTOPT
	// first recalc dose matrix
	GetDoseMatrix();

	// now iterate over histo's
	for (auto& entry : m_mapHistograms)
	{
		entry.second->OnVolumeChange(); // NULL, NULL);
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
void
	Plan::SetDoseResolution(const REAL& res)
	// sets shape for dose matrix
{
	m_DoseResolution = res;

	// accessor to planning volume
	const VolumeReal *pVolume = GetSeries()->GetDensity();
	const itk::Vector<REAL> vVolSpacing = pVolume->GetSpacing();

	// compute height / width / depth
	int nHeight = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[1] 
			* vVolSpacing[1] / m_DoseResolution);
	int nWidth = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[0] 
			* vVolSpacing[0] / m_DoseResolution);
	int nDepth =
		Round<int>(pVolume->GetBufferedRegion().GetSize()[2]
			* vVolSpacing[2] / m_DoseResolution);

	// never let a non-empty dimension round to zero: a volume thinner than
	//	half a dose voxel (e.g. the 5-slice x 3 mm micro series at the coarsest
	//	pyramid level's 32 mm resolution) otherwise gets an empty dose matrix,
	//	so that level has no dose, KL = 0, and its optimization is a no-op. One
	//	voxel spanning the whole extent still lets the level contribute a
	//	coarse solution to the next finer level. An empty density volume (no
	//	series loaded yet) must stay empty: the views test the dose size to
	//	decide whether there is anything to draw.
	const VolumeReal::SizeType volSize = pVolume->GetBufferedRegion().GetSize();
	if (volSize[0] > 0) nWidth = __max(nWidth, 1);
	if (volSize[1] > 0) nHeight = __max(nHeight, 1);
	if (volSize[2] > 0) nDepth = __max(nDepth, 1);

	// set dimensions
	m_pDose->SetRegions(MakeSize(nWidth, nHeight, nDepth));
	m_pDose->Allocate();

	// see SetDoseOriginOffset: coarse pyramid levels shift the grid to match
	//	the half-spacing origin shift of their pyramid-filtered beamlets
	VolumeReal::PointType doseOrigin = pVolume->GetOrigin();
	for (int nD = 0; nD < 3; nD++)
		doseOrigin[nD] += m_DoseOriginOffset;
	m_pDose->SetOrigin(doseOrigin);
	m_pDose->SetDirection(pVolume->GetDirection());
	m_pDose->SetSpacing(
		MakeVector<3>(m_DoseResolution, m_DoseResolution, m_DoseResolution));

}


///////////////////////////////////////////////////////////////////////////////
CHistogram *
	Plan::GetHistogram(dH::Structure *pStructure, bool bCreate)
{
	CHistogram *pHisto = NULL;
#ifdef USE_RTOPT
	auto iterFound = m_mapHistograms.find(pStructure->GetName());
	if (iterFound != m_mapHistograms.end())
	{
		pHisto = iterFound->second;
	}
	else
	{
		if (bCreate)
		{
			pHisto = new CHistogram();

			pHisto->SetBinning((REAL) 0.0, (REAL) 0.01 /* 0.02 */, GBINS_BUFFER);
			pHisto->SetVolume(GetDoseMatrix());

			// resample region, if needed
			VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
			pHisto->SetRegion(pResampRegion);

			// calculate slice number for the isocenter
			REAL sliceZ = GetBeamAt(0)->GetIsocenter()[2];
			int nSlice = Round<int>((sliceZ - pResampRegion->GetOrigin()[2]) / pResampRegion->GetSpacing()[2]);
			pHisto->SetSlice(nSlice);

			// add to map
			m_mapHistograms[pStructure->GetName()] = pHisto;
		}
	}

	if (pHisto != NULL)
	{
		// resample region, if needed (this is to always create the exclusion region
		VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
		pHisto->SetRegion(pResampRegion);
	}
#endif

	return pHisto;

}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::RemoveHistogram(dH::Structure *pStructure)
{
#ifdef USE_RTOPT
	auto iterFound = m_mapHistograms.find(pStructure->GetName());
	if (iterFound != m_mapHistograms.end())
	{
		delete iterFound->second;
		m_mapHistograms.erase(iterFound);
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	Plan::GetMassDensity()
	// used to format the mass density array, conformant to dose matrix
{
	// fix mass density
	ConformTo<VOXEL_REAL,3>(GetSeries()->GetDensity(), m_pMassDensity);
	m_pMassDensity->FillBuffer(0.0); 

	// lookup values
	VOXEL_REAL *pCTVoxels = GetSeries()->GetDensity()->GetBufferPointer(); 
	VOXEL_REAL *pMDVoxels = m_pMassDensity->GetBufferPointer(); 
	int nVoxels = m_pMassDensity->GetBufferedRegion().GetNumberOfPixels();
	for (int nAtVoxel = 0; nAtVoxel < nVoxels; nAtVoxel++)
	{
		if (pCTVoxels[nAtVoxel] < 0.0)
		{
			pMDVoxels[nAtVoxel] =
				(VOXEL_REAL)(0.0 + 1.0 * (pCTVoxels[nAtVoxel] - -1024.0) / 1024.0);
		}
		else if (pCTVoxels[nAtVoxel] < 1024.0)
		{
			pMDVoxels[nAtVoxel] = 
				(VOXEL_REAL)(1.0 + 0.0/*0.5*/ * pCTVoxels[nAtVoxel] / 1024.0);
		}
		else
		{
			pMDVoxels[nAtVoxel] = 1.0/*1.5*/;
		}
	}

	return m_pMassDensity;

}

}	// namespace dH
