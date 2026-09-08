// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PlanPyramid.cpp 647 2009-11-05 21:52:59Z dglane001 $
#include "StdAfx.h"
#include "PlanPyramid.h"

#include "itkBinomialBlurImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkNearestNeighborExtrapolateImageFunction.h"


namespace dH
{

///////////////////////////////////////////////////////////////////////////////
PlanPyramid::PlanPyramid(CPlan *pPlan)
{
	SetPlan(pPlan);

	if (m_vWeightFilter.GetDim() == 0)
	{
		m_vWeightFilter.SetDim(3);
		m_vWeightFilter[0] = 0.25;
		m_vWeightFilter[1] = 0.50;
		m_vWeightFilter[2] = 0.25;
	}
}

///////////////////////////////////////////////////////////////////////////////
PlanPyramid::~PlanPyramid(void)
{
	for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
		delete m_arrPlans[nLevel];
}

///////////////////////////////////////////////////////////////////////////////
void 
	PlanPyramid::SetPlan(CPlan *pPlan)
{
	m_pPlan = pPlan;


	// generate sub-plans
	CPlan *pPrevPlan = GetPlan();
	if (m_arrPlans.size() == 0)
		m_arrPlans.push_back(pPrevPlan);

	REAL doseResolution = pPlan->GetDoseResolution();
	for (int nLevel = 1; nLevel < MAX_SCALES; nLevel++)
	{
		doseResolution *= 2.0;
		CPlan *pNextPlan = NULL; 
		if (m_arrPlans.size() <= nLevel)
		{
			pNextPlan = new CPlan();
			m_arrPlans.push_back(pNextPlan);
		}
		else
		{
			pNextPlan = m_arrPlans[nLevel];
		}

		pNextPlan->SetSeries(m_pPlan->GetSeries());
		// coarse beamlets come from ITK's pyramid filter, whose output origin is
		//	inputOrigin + (outputSpacing - inputSpacing) / 2 per level; cumulatively
		//	(R_level - R_0) / 2 from the finest grid. Shift this level's dose grid
		//	by the same amount so beamlets and grid coincide (Plan::SetDoseOriginOffset).
		pNextPlan->SetDoseOriginOffset((doseResolution - pPlan->GetDoseResolution()) / 2.0);
		pNextPlan->SetDoseResolution(doseResolution);
		for (int nAt = 0; nAt < GetPlan()->GetBeamCount(); nAt++)
		{
			CBeam *pPrevBeam = pPrevPlan->GetBeamAt(nAt);
			CBeam::Pointer pNextBeam;
			if (pNextPlan->GetBeamCount() <= nAt)
			{
				pNextBeam = dH::Beam::New(); // CBeam(/*pPrevBeam*/);
				// need to add the beam first, because the plan is needed to set gantry angle
				pNextPlan->AddBeam(pNextBeam);
			}
			else
			{
				pNextBeam = pNextPlan->GetBeamAt(nAt);
			}

			pNextBeam->SetGantryAngle(pPrevBeam->GetGantryAngle());
			pNextBeam->SetIsocenter(pPrevBeam->GetIsocenter());

		}
		ASSERT(pPrevPlan->GetBeamCount() == pNextPlan->GetBeamCount());

		pPrevPlan = pNextPlan;
	}

}	// PlanPyramid::SetPlan

///////////////////////////////////////////////////////////////////////////////
CPlan *
	PlanPyramid::GetPlan(int nLevel)
{
	return m_arrPlans[nLevel];

}	// PlanPyramid::SetPlan

///////////////////////////////////////////////////////////////////////////////
void 
	PlanPyramid::CalcPencilSubBeamlets(int nBeam)
{
	// make sure pencil subbeamlets are properly generated
	for (int nAt = ((nBeam == -1) ? (GetPlan()->GetBeamCount()-1) : nBeam); nAt >= ((nBeam == -1) ? 0 : nBeam); nAt--)
	{
		CBeam *pBeam = GetPlan()->GetBeamAt(nAt);

		// only recalc if they need to be
		if (!pBeam->m_bRecalcBeamlets)
			return;

			// stores beamlet count for level N
		int nBeamletCount = 19;
		// TODO: reconcile this with nBeamletCount used in PlanPyramid

		REAL beamletSpacing = 4.0; // 2.0;
		// TODO: reconcile this with beamletSpacing used in BeamDoseCalc

		// now generate level 1..n beamlets
		for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
		{
			CBeam *pBeamSub = m_arrPlans[nAtScale]->GetBeamAt(nAt);
			CBeam *pBeamSubPrev = m_arrPlans[nAtScale-1]->GetBeamAt(nAt);

			// each level halves the number of beamlets
			nBeamletCount /= 2;
			beamletSpacing *= 2.0;

			// set up the beams beamlets; do this by defining the intensity map parameters
			CBeam::IntensityMap *pIM = pBeamSub->GetIntensityMap();

			// set up the intensity map indexing
			CBeam::IntensityMap::RegionType region;
			itk::Index<1> index = {{-nBeamletCount}};
			region.SetIndex(index);
			region.SetSize(MakeSize(nBeamletCount*2 + 1));
			pIM->SetRegions(region);	/// TODO: make this index from -n/2..n/2
			pIM->Allocate();

			// set up the intensity map spacing
			pIM->SetSpacing(beamletSpacing);
			/// TODO: figure out the origin
			REAL origin[] = {0}; 
				// {-beamletSpacing * nBeamletCount};
			pIM->SetOrigin(origin);
			pIM->FillBuffer(0);

			// this will allocate the necessary beamlets
			pBeamSub->OnIntensityMapChanged();

			// Coarse beamlets: Gaussian-smooth the accumulated finer beamlet, then
			//	resample it onto a grid with twice the spacing. These are the two
			//	steps itk::MultiResolutionPyramidImageFilter performs for a 2-level
			//	pyramid (variance (factor/2)^2 in pixel units, linear interpolation,
			//	output origin shifted by half the spacing increase), done here
			//	explicitly so the resampler can be given an extrapolator: on an axis
			//	where the finer beamlet is a single voxel, the coarse sample point
			//	lands exactly on the interpolator's half-voxel boundary, which ITK
			//	excludes, and the pyramid filter returned an all-zero beamlet (the
			//	5-slice x 3 mm series at the 32 mm level). Nearest-neighbour
			//	extrapolation returns the edge value there; interior samples are
			//	unchanged.
			typedef itk::DiscreteGaussianImageFilter<VolumeReal, VolumeReal> SmootherType;
			SmootherType::Pointer pSmoother = SmootherType::New();
			pSmoother->SetUseImageSpacing(false);
			pSmoother->SetVariance(1.0);			// (0.5 * shrink factor 2)^2
			pSmoother->SetMaximumError(0.1);		// pyramid filter default

			typedef itk::ResampleImageFilter<VolumeReal, VolumeReal> ShrinkerType;
			ShrinkerType::Pointer pShrinker = ShrinkerType::New();
			pShrinker->SetInput(pSmoother->GetOutput());
			pShrinker->SetInterpolator(
				itk::LinearInterpolateImageFunction<VolumeReal, double>::New());
			pShrinker->SetExtrapolator(
				itk::NearestNeighborExtrapolateImageFunction<VolumeReal, double>::New());

			VolumeReal::Pointer beamlet = VolumeReal::New();
			pSmoother->SetInput(beamlet);
			VolumeReal::Pointer beamletAccum = VolumeReal::New();

			ConformTo<VOXEL_REAL,3>(pBeamSubPrev->GetBeamlet(0), beamlet);
			ConformTo<VOXEL_REAL,3>(pBeamSubPrev->GetBeamlet(0), beamletAccum);

			// the coarse grid, as the pyramid filter computes it: half the size
			//	(at least one voxel), twice the spacing, origin shifted by half the
			//	spacing increase along the image direction
			{
				const VolumeReal::SizeType inSize = beamlet->GetBufferedRegion().GetSize();
				const VolumeReal::SpacingType inSpacing = beamlet->GetSpacing();
				VolumeReal::SizeType outSize;
				VolumeReal::SpacingType outSpacing;
				itk::Vector<REAL, 3> vOffset;
				for (int nD = 0; nD < 3; nD++)
				{
					outSize[nD] = __max((int) (inSize[nD] / 2), 1);
					outSpacing[nD] = inSpacing[nD] * 2.0;
					vOffset[nD] = (outSpacing[nD] - inSpacing[nD]) * 0.5;
				}
				VolumeReal::PointType outOrigin = beamlet->GetOrigin() + beamlet->GetDirection() * vOffset;
				pShrinker->SetSize(outSize);
				pShrinker->SetOutputSpacing(outSpacing);
				pShrinker->SetOutputOrigin(outOrigin);
				pShrinker->SetOutputDirection(beamlet->GetDirection());
			}

			// generate beamlets for base scale
			for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
			{
				// helpers for calculating sub beamlets
				beamlet->FillBuffer(0.0);

				VolumeReal * pPrevBeamletLow = pBeamSubPrev->GetBeamlet(nAtShift * 2 - 1);
				VolumeReal * pPrevBeamletHigh = pBeamSubPrev->GetBeamlet(nAtShift * 2 + 1);
				if (pPrevBeamletLow != NULL)
				{
					// NOTE: these are all * 2.0 because there are only half as many sub-beamlets 
					//		contributing; this means that the intensity map interpolation needs 
					//		no scaling
					Accumulate3D<VOXEL_REAL>(pPrevBeamletLow, 
						(pPrevBeamletHigh != NULL) 
						? 2.0 * m_vWeightFilter[0] 
						: 2.0 * m_vWeightFilter[0] /*/ 0.75*/, 
						beamlet, beamletAccum);
				}

				Accumulate3D<VOXEL_REAL>(pBeamSubPrev->GetBeamlet(nAtShift * 2 + 0), 
					(pPrevBeamletHigh != NULL && pPrevBeamletLow != NULL) 
						? 2.0 * m_vWeightFilter[1] 
						: 2.0 * m_vWeightFilter[1] /*/ 0.75*/, 
					beamlet, beamletAccum); 


				if (pPrevBeamletHigh != NULL)
				{
					Accumulate3D<VOXEL_REAL>(pPrevBeamletHigh, 
						(pPrevBeamletLow != NULL) 
						? 2.0 * m_vWeightFilter[2] 
						: 2.0 * m_vWeightFilter[2] /*/ 0.75*/,
						beamlet, beamletAccum);
				}
				// the accumulation wrote the input buffer in place: flag it so the
				//	smooth + resample pipeline re-executes
				beamlet->Modified();
				pShrinker->UpdateLargestPossibleRegion();
				// TODO: investigate whether resulting filtered beamlet is scaled properly

				CopyImage<VOXEL_REAL,3>(pShrinker->GetOutput(), pBeamSub->GetBeamlet(nAtShift));

				// check that resolution is correct
				ASSERT(pBeamSub->GetBeamlet(nAtShift)->GetSpacing()[0] == pBeamSub->GetPlan()->GetDoseResolution());
			}
		}

		/// TODO: move this flag to PlanPyramid::m_bRecalcBeamlets
		pBeam->m_bRecalcBeamlets = false;
	}

}	// PlanPyramid::CalcPencilSubBeamlets

///////////////////////////////////////////////////////////////////////////////
void
PlanPyramid::InvFiltIntensityMap(int nLevel, const CBeam::IntensityMap * vWeights,
								CBeam::IntensityMap * vFiltWeights)
{
#define MANUAL_INTERP
#ifdef MANUAL_INTERP
	{
	const int nWeightsSize = (int) vWeights->GetBufferedRegion().GetSize()[0];
	const int nFiltWeightsSize = (int) vFiltWeights->GetBufferedRegion().GetSize()[0];

	ASSERT(nWeightsSize == GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount());
	ASSERT(nFiltWeightsSize == GetPlan(nLevel-1)->GetBeamAt(0)->GetBeamletCount());

	// the ASSERTs above are compiled out in release, where a size mismatch
	//	silently overruns the buffers indexed below (the write extent comes
	//	from GetBeamletCount, not from the buffer itself), corrupting the
	//	heap. Bail out rather than write out of bounds.
	if (nWeightsSize != GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount()
		|| nFiltWeightsSize != GetPlan(nLevel-1)->GetBeamAt(0)->GetBeamletCount())
		return;

	int nBeamletCountPrev = GetPlan(nLevel)->GetBeamAt(0)->GetBeamletCount() / 2;
	int nBeamletCountNext = GetPlan(nLevel-1)->GetBeamAt(0)->GetBeamletCount() / 2;

	// generate beamlets for base scale
	for (int nAtShift = -nBeamletCountNext; nAtShift <= nBeamletCountNext; nAtShift++)
	{
		if (abs(nAtShift) % 2 == 0)
		{
			vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] =
				vWeights->GetBufferPointer()[nAtShift/2 + nBeamletCountPrev];
		}
		else
		{
			int nLower = floor((double) nAtShift / 2.0);
			int nHigher = ceil((double) nAtShift / 2.0);
			if (nLower + nBeamletCountPrev < 0)
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] = 
					0.5 * vWeights->GetBufferPointer()[nHigher + nBeamletCountPrev];
			}
			else if (nHigher + nBeamletCountPrev >= vWeights->GetBufferedRegion().GetSize()[0])
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] =  
					0.5 * vWeights->GetBufferPointer()[nLower + nBeamletCountPrev];
			}
			else
			{
				vFiltWeights->GetBufferPointer()[nAtShift + nBeamletCountNext] = 
					0.5 * vWeights->GetBufferPointer()[nLower + nBeamletCountPrev]
						+ 0.5 * vWeights->GetBufferPointer()[nHigher + nBeamletCountPrev];
			}
		}
	}

	}
#endif


}	// PlanPyramid::InvFiltIntensityMap

}	// namespace dH