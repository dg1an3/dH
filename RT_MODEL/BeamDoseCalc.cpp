// Beam.cpp: implementation of the CBeamDoseCalc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BeamDoseCalc.h"

#include <EnergyDepKernel.h>
#include <Beam.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define CHECK_CONSERVATION_LAW



///////////////////////////////////////////////////////////////////////////////
// nint
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
static int nint(REAL val) 
{
	return (int) floor(val + 0.5);

}	// nint



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::CBeamDoseCalc
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeamDoseCalc::CBeamDoseCalc(CBeam *pBeam, CEnergyDepKernel *pKernel)
:	m_pBeam(pBeam),
		m_pKernel(pKernel),
		m_pMassDensityPyr(NULL),
		m_pTerma(new CVolume<VOXEL_REAL>()),
		m_nBeamletCount(19), // (15),
		m_raysPerVoxel(12),
		m_pEnergy(new CVolume<VOXEL_REAL>())
{
	m_kernel.SetDimensions(9, 9, 1);
	CalcBinomialFilter(&m_kernel);

}	// CBeamDoseCalc::CBeamDoseCalc

///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::~CBeamDoseCalc
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeamDoseCalc::~CBeamDoseCalc()
{
	delete m_pTerma;
	delete m_pEnergy;

}	// CBeamDoseCalc::~CBeamDoseCalc



void CBeamDoseCalc::SetDoseCalcRegion(const CVectorD<3> &vMin, const CVectorD<3> &vMax)
{
	m_vDoseCalcRegionMin = vMin;
	m_vDoseCalcRegionMax = vMax;
}


///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::CalcPencilBeams
// 
// Calculates pencil beams + sub-beamlets given the density matrix
///////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::CalcPencilBeams(CVolume<VOXEL_REAL> *pOrigDensity)
{
#define DUMB_FILTER_DENSITY
#ifdef DUMB_FILTER_DENSITY

	// check for valid density
	VOXEL_REAL sum = pOrigDensity->GetSum();
	ASSERT(sum > 1e-6);

	// now filter density
	m_densityFilt.ConformTo(pOrigDensity);
	m_densityFilt.ClearVoxels();
	// ::Convolve(pOrigDensity, &m_kernel, &m_densityFilt);
	VOXEL_REAL filtered_max = m_densityFilt.GetMax();

	// TODO: check that dose matrix is initialized
	ASSERT(m_pBeam->m_dose.GetWidth() > 0);
	m_densityRep.ConformTo(&m_pBeam->m_dose);
	m_densityRep.ClearVoxels();
	::Resample( 
		// &m_densityFilt,
		pOrigDensity,
		&m_densityRep, TRUE); 
	VOXEL_REAL resamp_max = m_densityRep.GetMax();
//	ASSERT(IsApproxEqual(resamp_max, filtered_max));

	// TODO: now, replicate slices
	for (int nZ = 1; nZ < m_densityRep.GetDepth(); nZ++)
	{
		memcpy(&m_densityRep.GetVoxels()[nZ][0][0], 
			&m_densityRep.GetVoxels()[0][0][0], 
			m_densityRep.GetWidth() * m_densityRep.GetHeight() * sizeof(VOXEL_REAL));
	}
	m_densityRep.VoxelsChanged();
	// ASSERT(sum > 1e-6);

	sum = m_densityRep.GetSum();
	ASSERT(sum > 1e-6);

	m_densityRep.LogPlane(m_densityRep.GetDepth()/2, "m_densityRep", THIS_FILE);
	m_densityRep.AssertValid();

#else

	ASSERT(FALSE);

	m_pMassDensityPyr = new CPyramid(pOrigDensity);
	int nLevel = m_pMassDensityPyr->SetLevelBasis(m_pBeam->m_dose.GetBasis());

	CVolume<VOXEL_REAL> *pDensity = m_pMassDensityPyr->GetLevel(nLevel);

	ASSERT(pDensity->GetBasis().IsApproxEqual(m_pBeam->m_dose.GetBasis())); */

//	m_densityRep.ConformTo(pDensity);
//	m_densityRep.SetDimensions(pDensity->GetWidth(), pDensity->GetHeight(), 10);

	// TODO: now, replicate slices
	for (int nZ = 1; nZ < m_densityRep.GetDepth(); nZ++)
	{
		memcpy(&m_densityRep.GetVoxels()[nZ][0][0], 
			&m_densityRep.GetVoxels()[0][0][0], 
			m_densityRep.GetWidth() * m_densityRep.GetHeight() * sizeof(VOXEL_REAL));
	}
#endif

	// determine beamlet spacing
	// TODO: fix this
	REAL beamletSpacing = 4.0;   /* m_densityRep.GetPixelSpacing()[1]; */

	// iterate for level 0 beamlets
	for (int nAt = -m_nBeamletCount; nAt <= m_nBeamletCount; nAt++)
	{
		TRACE("\tGenerating Beamlet %i\n", nAt);

		// set beamlet size
		CVectorD<2> vMin(((REAL) nAt - 0.5) * beamletSpacing, -5.0);
		CVectorD<2> vMax(((REAL) nAt + 0.5) * beamletSpacing,  5.0);

		// calculate terma for pencil beam
		CalcTerma(vMin, vMax);

		m_pTerma->LogPlane(m_pTerma->GetDepth()/2, "m_pTerma", THIS_FILE);
		m_pTerma->AssertValid();

		// convolve terma with energy deposition kernel to form dose
		CalcSphereConvolve();
		m_pEnergy->LogPlane(m_pEnergy->GetDepth()/2, "m_pEnergy", THIS_FILE);
		m_pEnergy->AssertValid();

		CVolume<VOXEL_REAL> *pEnergy2D = new CVolume<VOXEL_REAL>();
		pEnergy2D->ConformTo(m_pEnergy);
		pEnergy2D->SetDimensions(m_pEnergy->GetWidth(), m_pEnergy->GetHeight(), 1);

		// now move single plane to center
		int nCenterPlane = m_pEnergy->GetDepth() / 2;

		// copy voxels from 3D energy to 2D array
		memcpy(&pEnergy2D->GetVoxels()[0][0][0], 
			&m_pEnergy->GetVoxels()[nCenterPlane][0][0],
			m_pEnergy->GetWidth() * m_pEnergy->GetHeight() * sizeof(VOXEL_REAL));

		pEnergy2D->VoxelsChanged();
		TRACE("pEnergy2D->GetMax() == %f\n", pEnergy2D->GetMax());
		TRACE("m_pEnergy->GetMax() == %f\n", m_pEnergy->GetMax());

		// set pencil beam
		m_pBeam->m_arrBeamlets.Add(pEnergy2D);
	}

	// generate the kernel for convolution
	CVolume<VOXEL_REAL> kernel;
	kernel.SetDimensions(5, 5, 1);
	CalcBinomialFilter(&kernel);

	// stores beamlet count for level N
	int nBeamletCount = m_nBeamletCount;

	// generate level 0 beamlets

	// clear existing beamlets
	for (int nAt = 0; nAt < m_pBeam->m_arrBeamletsSub[0].GetSize(); nAt++)
	{
		delete m_pBeam->m_arrBeamletsSub[0][nAt];
	}
	m_pBeam->m_arrBeamletsSub[0].RemoveAll();

	CVolume<VOXEL_REAL> *pBeamlet = m_pBeam->GetBeamlet(0);
	if (beamletSpacing > pBeamlet->GetPixelSpacing()[0] + DEFAULT_EPSILON)
	{
		// generate beamlets for base scale
		for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
		{
			CVolume<VOXEL_REAL> *pBeamlet = m_pBeam->GetBeamlet(nAtShift);

			// convolve with gaussian
			CVolume<VOXEL_REAL> beamletConv;
			beamletConv.ConformTo(pBeamlet);
			Convolve(pBeamlet, &kernel, &beamletConv);

			CVolume<VOXEL_REAL> *pBeamletConvDec = new CVolume<VOXEL_REAL>;
			Decimate(&beamletConv, pBeamletConvDec);

			m_pBeam->m_arrBeamletsSub[0].Add(pBeamletConvDec);

			LOG("Scale %i beamlet %i\n", 0, nAtShift);
			LOG_OBJECT((*pBeamletConvDec));
		}
	}
	else
	{
		// generate beamlets for base scale
		for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
		{
			CVolume<VOXEL_REAL> *pBeamlet = m_pBeam->GetBeamlet(nAtShift);
			m_pBeam->m_arrBeamletsSub[0].Add(pBeamlet);
		}
	}

	// now calc subbeamlets
	CalcPencilSubBeamlets();

}	// CBeamDoseCalc::CalcPencilBeams

void CBeamDoseCalc::CalcPencilSubBeamlets()
{
	// stores beamlet count for level N
	int nBeamletCount = m_nBeamletCount;

	CVolume<VOXEL_REAL> kernel;
	kernel.SetDimensions(5, 5, 1);
	CalcBinomialFilter(&kernel);

	// now generate level 1..n beamlets
	for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
	{
		// clear existing beamlets
		for (int nAt = 0; nAt < m_pBeam->m_arrBeamletsSub[nAtScale].GetSize(); nAt++)
		{
			delete m_pBeam->m_arrBeamletsSub[nAtScale][nAt];
		}
		m_pBeam->m_arrBeamletsSub[nAtScale].RemoveAll();

		// each level halves the number of beamlets
		nBeamletCount /= 2;

		// generate beamlets for base scale
		for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
		{
			CVolume<VOXEL_REAL> beamlet;
			beamlet.ConformTo(m_pBeam->GetBeamletSub(0, nAtScale-1));
			beamlet.ClearVoxels();

			CVolume<VOXEL_REAL> * pPrevBeamletLow = 
				m_pBeam->GetBeamletSub(nAtShift * 2 - 1, nAtScale-1);
			if (pPrevBeamletLow != NULL)
				beamlet.Accumulate(pPrevBeamletLow, 
					2.0 * m_pBeam->m_vWeightFilter[0]);
			beamlet.Accumulate(m_pBeam->GetBeamletSub(nAtShift * 2 + 0, nAtScale-1), 
				2.0 * m_pBeam->m_vWeightFilter[1]);
			CVolume<VOXEL_REAL> * pPrevBeamletHigh = 
				m_pBeam->GetBeamletSub(nAtShift * 2 + 1, nAtScale-1);
			if (pPrevBeamletHigh != NULL)
				beamlet.Accumulate(pPrevBeamletHigh, 
					2.0 * m_pBeam->m_vWeightFilter[2]);

			// convolve with gaussian
			CVolume<VOXEL_REAL> beamletConv;
			beamletConv.ConformTo(&beamlet);
			Convolve(&beamlet, &kernel, &beamletConv);

			CVolume<VOXEL_REAL> *pBeamletConvDec = new CVolume<VOXEL_REAL>;
			Decimate(&beamletConv, pBeamletConvDec);

			m_pBeam->m_arrBeamletsSub[nAtScale].Add(pBeamletConvDec);

			LOG("Scale %i beamlet %i\n", nAtScale, nAtShift);
			LOG_OBJECT((*pBeamletConvDec));
		}
	}
}




///////////////////////////////////////////////////////////////////////////////
// DistToIntersectPlane
// 
// Helper function to compute distance of a vector component to interset the
//	next plane at a 0.5-boundary
///////////////////////////////////////////////////////////////////////////////
inline REAL DistToIntersectPlane(REAL pos, REAL dir, int& nCurrIndex)
{
	const REAL EPS = (REAL) 1e-6;
	
	if (dir > 0)
	{
		nCurrIndex = (int) floor(pos + 0.5 + EPS);
		return (((REAL) nCurrIndex + 0.5) - pos) / dir;
	}
	else
	{
		nCurrIndex = (int) ceil(pos - 0.5 - EPS);
		return (((REAL) nCurrIndex - 0.5) - pos) / dir;
	}

}	// DistToIntersectPlane



///////////////////////////////////////////////////////////////////////////////
// GetLinearInterpWeights
// 
// Helper function to calculate linear interpolation weights for -1, 0, 1 
//		neighborhood.  Three of these sets of interpolations are needed
//		for tri-linear interp
///////////////////////////////////////////////////////////////////////////////
inline void GetLinearInterpWeights(REAL pos, int nIndex, REAL (&weights)[3])
{
	weights[-1 + 1] = pos < (REAL) nIndex ? fabs((REAL) nIndex - pos) : 0.0;
	weights[ 0 + 1] = 1.0 - fabs((REAL) nIndex - pos);
	weights[ 1 + 1] = pos > (REAL) nIndex ? fabs((REAL) nIndex - pos) : 0.0;

}	// GetLinearInterpWeights


///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::CalcTerma
// 
// Calculates TERMA for the given source geometry, and mass density field
// vMin, vMax in physical coords at isocentric plane
///////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::CalcTerma(const CVectorD<2>& vMin_in,
							  const CVectorD<2>& vMax_in)
{
	// consts for index positions
	const int X = 1;
	const int Y = 2;
	const int Z = 0;

	// vPixSpacing is spacing factor for three coordinates
	//		= conversion from voxel to physical coordinates
	const CVectorD<3> vPixSpacing = m_densityRep.GetPixelSpacing();

	// calculate conversion from isocenter to top boundary
	const REAL convIso2Top = 
		(-0.5 - m_vSource_vxl[Z])		// z-distance from upper boundary to source
			/ (m_vIsocenter_vxl[Z] - m_vSource_vxl[Z]);

	// set up starting min vector position in voxel coords
	CVectorD<3> vMin = m_vIsocenter_vxl;
	vMin[X] += convIso2Top * vMin_in[0] / vPixSpacing[X];
	vMin[Y] += convIso2Top * vMin_in[1] / vPixSpacing[Y];

	// set up max vector position in voxel coords
	CVectorD<3> vMax = m_vIsocenter_vxl;
	vMax[X] += convIso2Top * vMax_in[0] / vPixSpacing[X];
	vMax[Y] += convIso2Top * vMax_in[1] / vPixSpacing[Y];

	// now set starting z-coord (at upper boundary of voxels)
	vMin[Z] = vMax[Z] = -0.5;

	// based on rays per voxel -- in voxel coordinates
	const REAL deltaRay = 1.0 / m_raysPerVoxel;

	// initial fluence per ray -- set so incident fluence is constant
	const REAL fluence0 = vPixSpacing[X] * vPixSpacing[Y] * deltaRay * deltaRay;

	// stores mu
	const REAL mu = m_pKernel->Get_mu();

	// set up density voxel accessor
	VOXEL_REAL ***pppDensity = m_densityRep.GetVoxels();

	// set up terma and voxel accessor
	m_pTerma->ConformTo(&m_densityRep);
	m_pTerma->ClearVoxels();
	VOXEL_REAL ***pppTerma = m_pTerma->GetVoxels();

#ifdef CHECK_CONSERVATION_LAW

	// initialize surface integral of fluence 
	REAL fluenceSurfIntegral = 0.0;

#endif

	// iterate over X & Y voxel positions
	for (CVectorD<3> vX = vMin; vX[X] < vMax[X]; vX[X] += deltaRay)
	{
		for (CVectorD<3> vY = vX; vY[Y] < vMax[Y]; vY[Y] += deltaRay)
		{
			// initial ray position (voxel coordinates)
			CVectorD<3> vRay = vY;

			// unit ray direction vector (voxel coordinates)
			CVectorD<3> vDir = vRay - m_vSource_vxl;
			vDir.Normalize();

			// calc physical length of vDir, in cm!!! (not mm)
			CVectorD<3> vDirPhysical = vDir;
			vDirPhysical[X] *= vPixSpacing[X];
			vDirPhysical[Y] *= vPixSpacing[Y];
			vDirPhysical[Z] *= vPixSpacing[Z];
			const REAL dirLength = 0.1 * vDirPhysical.GetLength();

			// initialize radiological path length for raytrace
			REAL path = 0.0;

#ifdef CHECK_CONSERVATION_LAW

			// increment by incident fluence
			fluenceSurfIntegral += fluence0;
#endif
			// stores current voxel indices
			int nNdx[3];

			// stores distances for each dimension
			REAL dist[3];

			// find next intersections
			for (int nDim = 0; nDim <= 2; nDim++)
			{
				dist[nDim] = DistToIntersectPlane(vRay[nDim], vDir[nDim], nNdx[nDim]);
			}

			// iterate ray trace until volume boundary is reached 
			while (nNdx[X] >= 1 
				&& nNdx[X] < m_pTerma->GetHeight()-1
				&& nNdx[Y] >= 1 
				&& nNdx[Y] < m_pTerma->GetDepth()-1
				&& nNdx[Z] < m_pTerma->GetWidth()-1)
			{
				// delta path = radiological path (based on mass density) through voxel
				REAL deltaPath;

				// store trilinear interpolation weights
				REAL weights[3][3];

				// find closest intersections
				for (int nDim = 0; nDim <= 2; nDim++)
				{
					// is this plane intersection closest?
					if (dist[nDim] <= dist[(nDim + 1) % 3] 
						&& dist[nDim] <= dist[(nDim + 2) % 3])
					{
						// compute avg position of ray within voxel
						//		use this position for trilinear weights
						CVectorD<3> vPos = vRay + (REAL) (0.5 * dist[nDim]) * vDir;

						// compute tri-linear interpolation weights
						GetLinearInterpWeights(vPos[X], nNdx[X], weights[X]);
						GetLinearInterpWeights(vPos[Y], nNdx[Y], weights[Y]);
						GetLinearInterpWeights(vPos[Z], nNdx[Z], weights[Z]);

						// update delta path
						deltaPath = 0.0;
						for (int nDZ = -1; nDZ <= 1; nDZ++)
						{
							for (int nDY = -1; nDY <= 1; nDY++)
							{
								for (int nDX = -1; nDX <= 1; nDX++)
								{
									deltaPath += 
										weights[X][nDX+1] 
										* weights[Y][nDY+1] 
										* weights[Z][nDZ+1] 
										* pppDensity[nNdx[Y]+nDY][nNdx[X]+nDX][nNdx[Z]+nDZ];
								}
							}
						}

						// update radiological path for length of partial volume traversal
						deltaPath *= dist[nDim] * dirLength;

						// update current ray position to move to the given plane intersection
						vRay += dist[nDim] * vDir;

						// done, since we found the minimum
						break;
					}
				}

				// update cumulative path
				path += deltaPath;

				// add to terma = -(derivative of fluence along ray)	
				REAL fluenceInc = fluence0 * exp(-mu * path) * mu * deltaPath;

				// iterate over neighborhood
				for (int nDZ = -1; nDZ <= 1; nDZ++)
				{
					for (int nDY = -1; nDY <= 1; nDY++)
					{
						for (int nDX = -1; nDX <= 1; nDX++)
						{
							//	use trilinear interpolation weights to update all neighboring 
							//		terma voxels
							pppTerma[nNdx[Y]+nDY][nNdx[X]+nDX][nNdx[Z]+nDZ] += (VOXEL_REAL)
								( weights[X][nDX+1]
								* weights[Y][nDY+1] 
								* weights[Z][nDZ+1] 
								* fluenceInc );
						}
					}
				}

				// find next intersections
				for (int nDim = 0; nDim <= 2; nDim++)
				{
					dist[nDim] = DistToIntersectPlane(vRay[nDim], vDir[nDim], nNdx[nDim]);
				}

			}	// while


#ifdef CHECK_CONSERVATION_LAW

			// reduce by amount of remaining (un-attenuated) fluence exiting volume
			fluenceSurfIntegral -= fluence0 * exp(-mu * path) ;
#endif
		}	// for vY
	}	// for vX

	// flag change to terma voxels
	m_pTerma->VoxelsChanged();

#ifdef CHECK_CONSERVATION_LAW

	TRACE("Fluence Surface Integral = %lf\n", fluenceSurfIntegral);
	TRACE("TERMA Integral = %lf\n", m_pTerma->GetSum());

	// fluence surface integral should be equal to integral of TERMA
	ASSERT(IsApproxEqual(fluenceSurfIntegral, R(m_pTerma->GetSum()), 
		fluenceSurfIntegral * R(1e-2) + R(1e-6)));

#endif

}	// CBeamDoseCalc::CalcTerma





//////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::CalcSphereConvolve
//
// spherical convolution
//////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::CalcSphereConvolve() 
{
	// initialize energy
	m_pEnergy->ConformTo(m_pTerma);
	m_pEnergy->ClearVoxels();

	// accessors for voxels
	VOXEL_REAL ***pppDensity = m_densityRep.GetVoxels();
	VOXEL_REAL ***pppEnergy = m_pEnergy->GetVoxels();

	// TODO: get rid of this
	VOXEL_REAL ***pppFluence = m_pTerma->GetVoxels();

	// set up pixel spacing
	CVectorD<3> vPixSpacing = m_densityRep.GetPixelSpacing();

	// convert to cm
	vPixSpacing *= (REAL) 0.1;

	// and set up lookup table for sphere convolve
	m_pKernel->SetBasis(vPixSpacing);

	//////////////////////////////////////////////////////////////
	// Now do the convolution.
	
	for (int nZ = 0; nZ < m_densityRep.GetDepth(); nZ++)         
	{
		for (int nY = 0; nY < m_densityRep.GetHeight(); nY++)        
		{
			for (int nX = 0; nX < m_densityRep.GetWidth(); nX++)          
			{
				// dose at zero density?
				if (pppDensity[nZ][nY][nX] > 0.05) 
				{
// #define TERMA_ONLY
#if !defined(TERMA_ONLY)
					// spherical convolution at this point
					CalcSphereTrace(nX, nY, nZ);
#else
					pppEnergy[nZ][nY][nX] = pppFluence[nZ][nY][nX];
#endif

					// Convert the energy to dose by dividing by mass                            

					// convert to Gy cm**2 and take into account the azimuthal sum
					pppEnergy[nZ][nY][nX] *= (VOXEL_REAL) (1.602e-10 / (REAL) NUM_THETA);

				}
			}
		}
	}

	// flag that voxels have changed
	m_pEnergy->VoxelsChanged();

	// get new maximum for energy dist
	REAL dmax = m_pEnergy->GetMax();

	// now normalize to dmax
	if (dmax > 0.0)
	{
		for (int nZ = 0; nZ < m_densityRep.GetDepth(); nZ++)          
		{
			for (int nY = 0; nY < m_densityRep.GetHeight(); nY++)         
			{
				for (int nX = 0; nX < m_densityRep.GetWidth(); nX++)        
				{
					pppEnergy[nZ][nY][nX] /= (VOXEL_REAL) dmax;
				}
			}
		} 
	}

}	// CBeamDoseCalc::CalcSphereConvolve



///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::CalcSphereTrace
// 
// helper function to convolve at a single point in the energy volume
///////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::CalcSphereTrace(int nX, int nY, int nZ)
{
	// TODO: move this to EnergyDepKernel
	const REAL kernelDensity = 1.0;

	// accessors for voxels
	VOXEL_REAL ***pppDensity = m_densityRep.GetVoxels();
	VOXEL_REAL ***pppTerma = m_pTerma->GetVoxels();
	VOXEL_REAL ***pppEnergy = m_pEnergy->GetVoxels();

	// do for all azimuthal angles
	for (int nTheta = 1; nTheta <= m_pKernel->GetNumTheta(); nTheta++)            
	{
		// do for zenith angles 
		for (int nPhi = 1; nPhi <= m_pKernel->GetNumPhi(); nPhi++)
		{
			// stores total radiological distance traversed
			REAL radDist = 0.0;

			// stores previous cumulative energy value
			REAL prevEnergy = 0.0;
			
			// loop over radial increments
			for (int nRadial = 1; nRadial <= m_pKernel->GetNumRadii(); nRadial++)       
			{
				// integer distances between the interaction and the dose depostion voxels
				int nKernelX = nX; 
				int nKernelY = nY; 
				int nKernelZ = nZ; 
				m_pKernel->GetDelta(nTheta, nPhi, nRadial, nKernelX, nKernelY, nKernelZ);

				if (nKernelX >= 0 
					&& nKernelX < m_densityRep.GetWidth()
					&& nKernelY >= 0 
					&& nKernelY < m_densityRep.GetHeight()
					&& nKernelZ >= 0 
					&& nKernelZ < m_densityRep.GetDepth())
				{					
					// compute physical path length increment
					REAL deltaPhysDist = m_pKernel->GetRadius(nTheta, nPhi, nRadial);

					// compute radiological path length increment
					REAL deltaRadDist = deltaPhysDist * pppDensity[nKernelZ][nKernelY][nKernelX]
							/ kernelDensity;
					
					// update radiological path
					radDist += deltaRadDist;

					// quit after 60cm rad dist
					if (radDist > 60.0)
					{
						break;
					}

					// Use lookup table to find the value of the cumulative energy
					// deposited up to this radius. No interpolation is done. 
					REAL totalEnergy = m_pKernel->GetCumEnergy(nPhi, nRadial);
				
					// Subtract the last cumulative energy from the new cumulative energy
					// to get the amount of energy deposited in this interval and set the
					// last cumulative energy for the next time the lookup table is used.
					REAL energy = totalEnergy - prevEnergy;
					prevEnergy = totalEnergy;             
					
					// The energy is accumulated - superposition
					pppEnergy[nZ][nY][nX] +=  
						(VOXEL_REAL) (energy * pppTerma[nKernelZ][nKernelY][nKernelX]);

				}	// if 
				
			}	// end of radial path loop                
		}	// end end of zenith angle loop
	}	// end of azimuth angle loop 

}	// CBeamDoseCalc::CalcSphereTrace




