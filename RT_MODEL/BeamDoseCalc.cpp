// Beam.cpp: implementation of the CBeamDoseCalc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BeamDoseCalc.h"


#include <MathUtil.h>

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
CBeamDoseCalc::CBeamDoseCalc(CBeam *pBeam, CEnergyDepKernel *pKernel) // , REAL ssd)
:	m_pBeam(pBeam),
		m_pKernel(pKernel),
		// m_ssd(ssd),
//		m_pDensity(NULL),
//		m_pFluence(NULL),
//		m_pEnergy(NULL),
		m_pMassDensityPyr(NULL),
		m_pTerma(new CVolume<REAL>())
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
//	delete m_pEnergy;
//	delete m_pFluence;

}	// CBeamDoseCalc::~CBeamDoseCalc

/*
///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::SetDensity
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::SetDensity(CVolume<REAL> *pDensity, REAL mu)
{
	m_pDensity = pDensity;
	m_mu = mu;

// 	m_pKernel->SetBasis(pDensity->GetBasis());

}	// CBeamDoseCalc::SetDensity
*/


void CBeamDoseCalc::SetDoseCalcRegion(const CVectorD<3> &vMin, const CVectorD<3> &vMax)
{
	m_vDoseCalcRegionMin = vMin;
	m_vDoseCalcRegionMax = vMax;
}


void CBeamDoseCalc::CalcPencilBeams(CVolume<REAL> *pOrigDensity)
{
	// initialize density
/*	if (!m_pDensity)
	{
		m_pDensity = new CVolume<REAL>();
	} */

#define DUMB_FILTER_DENSITY
#ifdef DUMB_FILTER_DENSITY
	// TODO: check that dose matrix is initialized
	ASSERT(m_pBeam->m_dose.GetWidth() > 0);
	m_densityRep.ConformTo(&m_pBeam->m_dose);

	m_densityFilt.ConformTo(pOrigDensity);
	::Convolve(pOrigDensity, &m_kernel, &m_densityFilt);

	// TODO: do we need to filter?
	// TODO: check this
	::Resample(&m_densityFilt, &m_densityRep, TRUE); 

	// TODO: now, replicate slices
	for (int nZ = 1; nZ < m_densityRep.GetDepth(); nZ++)
	{
		memcpy(&m_densityRep.GetVoxels()[nZ][0][0], 
			&m_densityRep.GetVoxels()[0][0][0], 
			m_densityRep.GetWidth() * m_densityRep.GetHeight() * sizeof(REAL));
	}
//	LOG_OBJECT((*m_pDensity));

#else
	m_pMassDensityPyr = new CPyramid(pOrigDensity);
	int nLevel = m_pMassDensityPyr->SetLevelBasis(m_pBeam->m_dose.GetBasis());

	CVolume<REAL> *pDensity = m_pMassDensityPyr->GetLevel(nLevel);

	ASSERT(pDensity->GetBasis().IsApproxEqual(m_pBeam->m_dose.GetBasis())); */

//	m_densityRep.ConformTo(pDensity);
//	m_densityRep.SetDimensions(pDensity->GetWidth(), pDensity->GetHeight(), 10);

	// TODO: now, replicate slices
	for (int nZ = 1; nZ < m_densityRep.GetDepth(); nZ++)
	{
		memcpy(&m_densityRep.GetVoxels()[nZ][0][0], 
			&m_densityRep.GetVoxels()[0][0][0], 
			m_densityRep.GetWidth() * m_densityRep.GetHeight() * sizeof(REAL));
	}
//	LOG_OBJECT((*m_pDensity));
#endif

	CXMLElement *pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "BeamDoseCalc");
	pElem->Attribute("type", "CPlane");
	pElem->Attribute("name", "pDensity");
	m_densityRep.LogPlane(0, pElem);
	CXMLLogFile::GetLogFile()->CloseElement(); 

	// initialize SSD
	CVectorD<4> vTop = m_densityRep.GetBasis() * 
		ToHG(CVectorD<3, REAL>(m_densityRep.GetWidth() / 2, 
			0, m_densityRep.GetDepth() / 2)); 

	// compute ssd : TODO: fix this
	m_ssd = 1000.0 - vTop.GetLength(); // (REAL) m_densityRep.GetHeight() / 2.0 * 4.0; // vTop[0];

	// determine pencil beam spacing & count
	CVectorD<4> vBeamletMin = m_densityRep.GetBasis() * ToHG(CVectorD<3, REAL>(0, // TODO: fix to isocentric plane
		-0.5, 0));
	CVectorD<4> vBeamletMax = m_densityRep.GetBasis() * ToHG(CVectorD<3, REAL>(0, // TODO: fix to isocentric plane
		+0.5, 0));
	REAL beamletSpacing = // 0.2; //
		(FromHG<3, REAL>(vBeamletMax) - FromHG<3, REAL>(vBeamletMin)).GetLength();

	int nBeamletCount = 15; // 40;

	// initialize fluence (reused for all pencil beams
	CVolume<REAL> *pFluence = m_pTerma; // new CVolume<REAL>();

	REAL rays_per_voxel = 6;
	REAL thickness = vTop.GetLength() // fabs(vTop[0])
		// * 4.0		// pixel spacing in depth (X) direction
		* 2.0;
	for (int nAt = -nBeamletCount; nAt <= nBeamletCount; nAt++)
	{
		TRACE("\tGenerating Beamlet %i\n", nAt);

		// set beamlet size
		CVectorD<2> vMin(((REAL) nAt - 0.5) * beamletSpacing /* 1000.0 * m_ssd */, -5.0);
		CVectorD<2> vMax(((REAL) nAt + 0.5) * beamletSpacing /* 1000.0 * m_ssd */,  5.0);
//		xmin = ((REAL) nBeamletCount - 0.5) * beamletSpacing / 100.0 * m_ssd;
//		xmax = ((REAL) nBeamletCount + 0.5) * beamletSpacing / 100.0 * m_ssd;
//		ymin = -5.0;
//		ymax = 5.0;

		// calculate fluence for pencil beam
		m_raysPerVoxel = rays_per_voxel;
		CalcTerma(vMin, vMax);
		// DivFluenceCalc(vMin, vMax, rays_per_voxel, thickness, pFluence);
		LOG_OBJECT((*pFluence));

/*		CXMLElement *pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "BeamDoseCalc");
		pElem->Attribute("type", "CPlane");
		pElem->Attribute("name", "pFluence");
		pFluence->LogPlane(0, pElem);
		CXMLLogFile::GetLogFile()->CloseElement(); */

		// intialize energy
		CVolume<REAL> *pEnergy = new CVolume<REAL>();

		// convolve
		SphereConvolve(pFluence, thickness, pEnergy);

		CVolume<REAL> *pEnergy2D = new CVolume<REAL>();
		pEnergy2D->ConformTo(// pFluence); // 
			pEnergy);
		pEnergy2D->SetDimensions(// pFluence->GetWidth(), pFluence->GetHeight(), 1); // 
			pEnergy->GetWidth(), pEnergy->GetHeight(), 1);

		// now move single plane to center
		int nCenterPlane = // pEnergy->GetDepth() / 2;
			pEnergy->GetDepth() / 2;

		memcpy(&pEnergy2D->GetVoxels()[0][0][0], 
			// &pFluence->GetVoxels()[nCenterPlane][0][0],
			// pFluence->GetWidth() * pFluence->GetHeight() * sizeof(REAL));
			&pEnergy->GetVoxels()[nCenterPlane][0][0],
			pEnergy->GetWidth() * pEnergy->GetHeight() * sizeof(REAL));
		delete pEnergy;

		// set pencil beam
		m_pBeam->m_arrBeamlets[0].Add(pEnergy2D);
		LOG_OBJECT((*pEnergy2D));

/*		pElem = CXMLLogFile::GetLogFile()->NewElement("lo", "BeamDoseCalc");
		pElem->Attribute("type", "CPlane");
		pElem->Attribute("name", "pEnergy");
		pEnergy->LogPlane(0, pElem);
		CXMLLogFile::GetLogFile()->CloseElement();  */
	}

	// generate the kernel for convolution
	CVolume<REAL> kernel;
	kernel.SetDimensions(5, 5, 1);
	CalcBinomialFilter(&kernel);

	REAL pixelScale = 4.0;

	// now generate level 1..n beamlets
	for (int nAtScale = 1; nAtScale < MAX_SCALES; nAtScale++)
	{
		for (int nAt = 0; nAt < m_pBeam->m_arrBeamlets[nAtScale].GetSize(); nAt++)
			delete m_pBeam->m_arrBeamlets[nAtScale][nAt];

		m_pBeam->m_arrBeamlets[nAtScale].RemoveAll();

		nBeamletCount = nBeamletCount / 2;
		// int nShifts = pow(2, MAX_SCALES+1 - nAtScale) - 1;
		// int nDim = pBeam->GetBeamlet(0, nAtScale-1)->GetWidth();
		pixelScale *= 2.0;

		// generate beamlets for base scale
		for (int nAtShift = -nBeamletCount; nAtShift <= nBeamletCount; nAtShift++)
		{
			CVolume<REAL> *pBeamlet = new CVolume<REAL>;
			// pBeamlet->SetDimensions(nDim, nDim, 1);
			pBeamlet->ConformTo(m_pBeam->GetBeamlet(0, nAtScale-1));
			pBeamlet->ClearVoxels();

			pBeamlet->Accumulate(m_pBeam->GetBeamlet(nAtShift * 2 - 1, nAtScale-1), 
				2.0 * m_pBeam->m_vWeightFilter[0]);
			pBeamlet->Accumulate(m_pBeam->GetBeamlet(nAtShift * 2 + 0, nAtScale-1), 
				2.0 * m_pBeam->m_vWeightFilter[1]);
			pBeamlet->Accumulate(m_pBeam->GetBeamlet(nAtShift * 2 + 1, nAtScale-1), 
				2.0 * m_pBeam->m_vWeightFilter[2]);

			// convolve with gaussian
			CVolume<REAL> *pBeamletConv = new CVolume<REAL>;
			pBeamletConv->ConformTo(pBeamlet);
			Convolve(pBeamlet, &kernel, pBeamletConv);
			delete pBeamlet;

			CVolume<REAL> *pBeamletConvDec = new CVolume<REAL>;
			Decimate(pBeamletConv, pBeamletConvDec);
			delete pBeamletConv;

/*			CMatrixD<2> mRot 
				= ::CreateRotate(m_pBeam->GetGantryAngle()); // * PI / 180.0); // + PI / 2);

			CMatrixD<4> mBasisRot;
			mBasisRot[0][0] = pixelScale * mRot[0][0];
			mBasisRot[0][1] = pixelScale * mRot[0][1];
			mBasisRot[1][0] = pixelScale * mRot[1][0];
			mBasisRot[1][1] = pixelScale * mRot[1][1];

			CMatrixD<4> mBasis;
			mBasis[3][0] = -(pBeamletConvDec->GetWidth() - 1) / 2;
			mBasis[3][1] = -(pBeamletConvDec->GetHeight() - 1) / 2;
			pBeamletConvDec->SetBasis(mBasisRot * mBasis); */

			m_pBeam->m_arrBeamlets[nAtScale].Add(pBeamletConvDec);

			TRACE("Scale %i beamlet %i\n", nAtScale, nAtShift);

			LOG("Scale %i beamlet %i\n", nAtScale, nAtShift);
			LOG_OBJECT((*pBeamletConvDec));
		}
	}
}


/*
///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::GetFluence
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<REAL> *CBeamDoseCalc::GetFluence()
{
	// initialize fluence
	if (!m_pFluence)
	{
		m_pFluence = new CVolume<REAL>();
		m_pFluence->ConformTo(m_densityRep);
	}

	return m_pFluence;

}	// CBeamDoseCalc::GetFluence
*/




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
		nCurrIndex = floor(pos + 0.5 + EPS);
		return (((REAL) nCurrIndex + 0.5) - pos) / dir;
	}
	else
	{
		nCurrIndex = ceil(pos - 0.5 - EPS);
		return (((REAL) nCurrIndex - 0.5) - pos) / dir;
	}

}	// DistToIntersectPlane


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
	const X = 1;
	const Y = 2;
	const Z = 0;

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
	const REAL mu = m_pKernel->m_mu;

	// set up density voxel accessor
	REAL ***pppDensity = m_densityRep.GetVoxels();

	// set up terma and voxel accessor
	m_pTerma->ConformTo(&m_densityRep);
	m_pTerma->ClearVoxels();
	REAL ***pppTerma = m_pTerma->GetVoxels();

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
			while (nNdx[X] >= 0 && nNdx[X] < m_pTerma->GetHeight()
				&& nNdx[Y] >= 0 && nNdx[Y] < m_pTerma->GetDepth()
				&& nNdx[Z] < m_pTerma->GetWidth())
			{
				// delta path
				//		initial radiological path (based on mass density) through voxel
				REAL deltaPath = pppDensity[nNdx[Y]][nNdx[X]][nNdx[Z]];

				// find closest intersections
				for (int nDim = 0; nDim <= 2; nDim++)
				{
					// is this plane intersection closest?
					if (dist[nDim] <= dist[(nDim + 1) % 3] 
						&& dist[nDim] <= dist[(nDim + 2) % 3])
					{
						// update current ray position to move to the given plane intersection
						vRay += dist[nDim] * vDir;

						// update radiological path for length of partial volume traversal
						deltaPath *= dist[nDim] * dirLength;

						// done, since we found the minimum
						break;
					}
				}

				// update cumulative path
				path += deltaPath;

				// add to terma = -(derivative of fluence along ray)
				pppTerma[nNdx[Y]][nNdx[X]][nNdx[Z]] += 
					fluence0 * exp(-mu * path) * mu * deltaPath;

				// find next intersections
				for (nDim = 0; nDim <= 2; nDim++)
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
	ASSERT(IsApproxEqual(fluenceSurfIntegral, m_pTerma->GetSum(), 
		fluenceSurfIntegral * (REAL) 1e-2));

#endif

}	// CBeamDoseCalc::CalcTerma



//////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::DivFluenceCalc
//
// divergent fluence calculation
//////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::DivFluenceCalc(
							const CVectorD<2>& vMin,
							const CVectorD<2>& vMax,
							const REAL rays_per_voxel_in,
							const REAL thickness_in,
							CVolume<REAL> *pFluence_out
						   )
{	
	// initialize fluence
	pFluence_out->ConformTo(&m_densityRep);
	pFluence_out->ClearVoxels();

	REAL ***pppDensity = m_densityRep.GetVoxels();
	REAL ***pppFluence = pFluence_out->GetVoxels();

	// const CMatrixD<4> &mBasis = m_densityRep.GetBasis();

	int nOX = -m_densityRep.GetHeight() / 2; // nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = -m_densityRep.GetDepth() / 2; // nint(mBasis[3][2] / mBasis[2][2]); 

	// array for normalization
	static CVolume<REAL> *pNorm = NULL;
	if (!pNorm)
	{
		pNorm = new CVolume<REAL>();
	}
	pNorm->ConformTo(&m_densityRep);
	pNorm->ClearVoxels();
	REAL ***pppNorm = pNorm->GetVoxels();

	// fluinc initialization
	REAL fluinc = 0.0;
	
	// the distance between the rays at the phantom surface
	REAL xinc = 4.0 // mBasis[1][1] 
		/ rays_per_voxel_in;          
	REAL yinc = 4.0 // mBasis[2][2]
		/ rays_per_voxel_in;
	
	// integers defining the rays
	int nSmallFieldI = nint(vMin[0] /* xmin */ / xinc);    
	int nLargeFieldI = nint(vMax[0] /* xmax */ / xinc);    
	int nSmallFieldJ = nint(vMin[1] / yinc);
	int nLargeFieldJ = nint(vMax[1] / yinc);
	
	// the depth of the surface voxels.
	REAL mindepth = m_ssd - 0.5 * 4.0; // mBasis[0][0]; 
	int nDepthNum = nint(thickness_in / 4.0); // mBasis[0][0]); 

	// do for the x-direction.
	for (int nI = nSmallFieldI; nI <= nLargeFieldI; nI++)
	{	
		// do for the y-direction.
		for (int nJ = nSmallFieldJ; nJ <= nLargeFieldJ; nJ++)
		{
			
			REAL incfluence = 1.0; // accessory[nI+450][nJ+450];
			
			// these values are the distance from
			// the central axis to the ray for
			// rays at the surface voxels.
			REAL fieldx0 = ((REAL)nI) * xinc * mindepth / m_ssd;							
			REAL fieldy0 = ((REAL)nJ) * yinc * mindepth / m_ssd;	
			
			// Calculate the distance from the focal spot to the ray crossing
			
			// initial distance
			REAL length0 = sqrt(
				fieldx0*fieldx0 
				+ fieldy0*fieldy0 
				+ mindepth*mindepth);

			// distance increment; recall that leninc/len0 = mBasis[0][0]/mindepth
			REAL leninc = length0 * 4.0 /* mBasis[0][0] */ / mindepth; 
			REAL distance = length0;             
			
			// radiological pathlength in the last voxel
			REAL last_pathinc = 0.0;      

			// radiological pathlength in the phantom
//			REAL path = 0.0;              

			// amount of exponential attenuation in the phantom
			REAL atten = 1.0;             
			
			// Start raytracing along the ray.
			
			// the number of depth voxels
			for (int nK = 1; nK <= nDepthNum; nK++)
			{
				// the relative increase in the ray divergence
				REAL latscale = 1.0 + REAL(nK-1) * 4.0 /* mBasis[0][0] */ / mindepth;        
				
				// these values are the distance from the central axis to the rays.
				REAL fieldx = fieldx0*latscale;              
				REAL fieldy = fieldy0*latscale;              
				
				// the integers describing the nearest voxel
				int nNearI = nint(fieldx / 4.0 /* mBasis[1][1] */);   
				int nNearJ = nint(fieldy / 4.0 /* mBasis[2][2] */);   
				
				// Calculate the radiological path increment travelled in the phantom.
				//                  the factor 0.5 in pathinc is introduced so to have 
				//                  smaller steps and account better for inhomogeneities
				
				// radiological pathlength increment
/*				if (pppDensity[nNearJ-nOY][nNearI-nOX][nK-1] > 0.5)
				{
					TRACE("> 0.5 at %i, %i, %i = %f\n", nNearJ, nNearI, nK, pppDensity[nNearJ-nOY][nNearI-nOX][nK-1]);
				} */
				REAL pathinc = 0.0; 
				pathinc = 0.5 * leninc * pppDensity[nNearJ-nOY][nNearI-nOX][nK-1];

				REAL delta_path = pathinc + last_pathinc;   
//				path += delta_path;
				last_pathinc = pathinc;
				
				// these are the weights if the voxel is completely inside the field.
				REAL weightx = 1.0;                  
				REAL weighty = 1.0;                  
				
				// The following if statements tests to see if the voxel is sitting on
				// the field boundary. If it is the proportion of the voxel inside the
				// field is calculated. Dave Convery's second reported bug was fixed 
				// by properly scaling the field size for the divergence of the beam.
				
				REAL div_scale = 
					(m_ssd + (REAL(nK) - 0.5) * 4.0 /* mBasis[0][0] */) 
						/ m_ssd;

				REAL minfieldx = vMin[0] /* xmin */ * div_scale;
				REAL maxfieldx = vMax[0] * div_scale;
				REAL minfieldy = vMin[1] * div_scale;
				REAL maxfieldy = vMax[1] * div_scale;
				
				if (nNearI < nint(minfieldx / 4.0 /* mBasis[1][1] */)) 
				{
					weightx = 0.0;
				}
				else if (nNearI == nint(minfieldx / 4.0 /* mBasis[1][1] */))
				{
					weightx = REAL(nNearI) 
						- minfieldx / 4.0 /* mBasis[1][1] */ + 0.5;
				}
				else if (nNearI == nint(maxfieldx / 4.0 /* mBasis[1][1] */))
				{
					weightx = maxfieldx / 4.0 /* mBasis[1][1]  */ 
						- 0.5 - REAL(nNearI-1);
				}
				else if (nNearI > nint(maxfieldx / 4.0 /* mBasis[1][1] */)) 
				{
					weightx = 0.0;
				}
				
				if (nNearJ < nint(minfieldy / 4.0 /* mBasis[2][2] */)) 
				{	
					weighty = 0.0;
				}
				else if (nNearJ == nint(minfieldy / 4.0 /* mBasis[2][2] */))
				{
					weighty = REAL(nNearJ) 
						- minfieldy / 4.0 /* mBasis[2][2] */ + 0.5;
				}
				else if (nNearJ == nint(maxfieldy / 4.0 /* mBasis[2][2] */))
				{
					weighty = maxfieldy / 4.0 /* mBasis[2][2]  */
						- 0.5 - REAL(nNearJ-1);
				}
				else if (nNearJ > nint(maxfieldy / 4.0 /* mBasis[2][2] */)) 
				{	
					weighty = 0.0;
				}
				
				// Calculate the relative amount of fluence that interacts in each voxel
				// and convert from photons/cm**2 to photons by multiplying by the
				// cross-sectional area of the voxel. Also include weighting factors
				// that determine the relative number of photons interacting near
				// the boundary of the field.

				REAL mu = m_pKernel->m_mu;
				if (pppDensity[nNearJ-nOY][nNearI-nOX][nK-1] != 0.0)
				{
					// nmu = -alog(f_factor(nint(path)))/path
					atten *= exp(-mu * 0.1 * delta_path);
					
					// fluence increment    
					fluinc = incfluence * atten * mu
						* weightx * weighty;
					
					//  the divergence correction above was removed from here to be put
					//  in new_sphere_convolve
					
					pppFluence[nNearJ-nOY][nNearI-nOX][nK-1] += fluinc;  // fluence
					
					// normalization
					pppNorm[nNearJ-nOY][nNearI-nOX][nK-1]++;    
				}
				else
				{
					pppFluence[nNearJ-nOY][nNearI-nOX][nK-1] = 0.0;
					
					// avoid divide by zero
					pppNorm[nNearJ-nOY][nNearI-nOX][nK-1] = 1;	
				}
				
				// distance from the source
				distance += leninc;      
                
		   }	// for		   
		}	// for
	}	// for			// close the loops for all the (nI,nJ) voxel pairs
	
	// Normalize the fluence because the voxels had a different number of rays
	// passing through them. First have to find out the range through which
	// to do the normalization.
	
	// increase in lateral dimension compared to the surface
	REAL bottom_scale = (m_ssd + thickness_in) / m_ssd;        
	
	//       x_bottom_min = xmin*bottom_scale          // lateral dimensions of bottom
	//       x_bottom_max = xmax*bottom_scale          // voxels
	//       y_bottom_min = ymin*bottom_scale          // 
	//       y_bottom_max = ymax*bottom_scale          // 
	
	
	REAL x_bottom_min = (vMin[0] > 0.0) ? vMin[0] 
		: vMin[0] * bottom_scale;
	
	REAL x_bottom_max = (vMax[0] < 0.0) ? vMax[0]
		: vMax[0] * bottom_scale; 	
	
	REAL y_bottom_min = (vMin[1] > 0.0) ? vMin[1] 
		: vMin[1] * bottom_scale; 
	
	REAL y_bottom_max = (vMax[1] < 0.0) ? vMax[1]
		: vMax[1] * bottom_scale; 
	
	// location of bottom voxels saves time in normalization below
	int nMinI = nint(x_bottom_min / 4.0 /* mBasis[1][1] */);    
	int nMaxI = nint(x_bottom_max / 4.0 /* mBasis[1][1] */);    
	int nMinJ = nint(y_bottom_min / 4.0 /* mBasis[2][2] */);      
	int nMaxJ = nint(y_bottom_max / 4.0 /* mBasis[2][2] */);    
	
	for (nI = nMinI; nI <= nMaxI; nI++)
	{
		for (int nJ = nMinJ; nJ <= nMaxJ; nJ++)
		{
			for (int nK = 1; nK <= nDepthNum; nK++)
			{
				if (pppNorm[nJ-nOY][nI-nOX][nK-1] != 0)
				{
					// do normalization
					pppFluence[nJ-nOY][nI-nOX][nK-1] 
						/= (REAL) pppNorm[nJ-nOY][nI-nOX][nK-1];
				}				
			}
		}
	}

}	// CBeamDoseCalc::DivFluenceCalc


/*
///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::GetEnergy
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<REAL> *CBeamDoseCalc::GetEnergy()
{
	// initialize energy
	if (!m_pEnergy)
	{
		m_pEnergy = new CVolume<REAL>();
		m_pEnergy->ConformTo(m_densityRep);
	}
	return m_pEnergy;

}	// CBeamDoseCalc::GetEnergy
*/

//////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::SphereConvolve
//
// spherical convolution
//////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::SphereConvolve(CVolume<REAL> *pFluence_in, 
								   const REAL thickness_in,
									CVolume<REAL> *pEnergy_out) 
{
	// initialize energy
	pEnergy_out->ConformTo(pFluence_in);
	pEnergy_out->ClearVoxels();

	// accessors for voxels
	REAL ***pppDensity = m_densityRep.GetVoxels();
	REAL ***pppFluence = pFluence_in->GetVoxels();
	REAL ***pppEnergy = pEnergy_out->GetVoxels();

	// const CMatrixD<4> &mBasis = m_densityRep.GetBasis();
	// CMatrixD<4> mBasisInv = mBasis;
	// mBasisInv.Invert();

	// TODO: fix this
	// CVectorD<4> vOrigIndex = mBasisInv * ToHG(CVectorD<3, REAL>(0.0, 0.0, 0.0));

//	int nOX = -vOrigIndex[0]; // nint(mBasis[3][1] / mBasis[1][1]); 
//	int nOY = -vOrigIndex[1]; // nint(mBasis[3][2] / mBasis[2][2]); 
	int nOX = -m_densityRep.GetHeight() / 2; // nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = -m_densityRep.GetDepth() / 2; // nint(mBasis[3][2] / mBasis[2][2]); 

	// parameters
	// TODO: fix this
	const int mini_in = (nOX + 1);
		// nint(m_vDoseCalcRegionMin[1] / mBasis[1][1]);
	const int maxi_in =  -(nOX + 1);
		// nint(m_vDoseCalcRegionMax[1] / mBasis[1][1]);
	const int minj_in = // -63; // 
		(nOY + 1);
		// nint(m_vDoseCalcRegionMin[2] / mBasis[2][2]);
	const int maxj_in =  // -63; // 
		-(nOY + 1);
		// nint(m_vDoseCalcRegionMax[2] / mBasis[2][2]);
	const int mink_in = 1;
	const int maxk_in = 127;

	// CMatrixD<4> mBasisInv = mBasis;
	// mBasisInv.Invert();

	// CVectorD<3> vIndexMin = FromHG<3, REAL>(mBasisInv * ToHG(m_vDoseCalcRegionMin));
	// CVectorD<3> vIndexMax = FromHG<3, REAL>(mBasisInv * ToHG(m_vDoseCalcRegionMax));

	// set up
	CVectorD<3> &vPixSpacing = m_densityRep.GetPixelSpacing();

	// leave as mm (because radial energy lookup in mm)
	vPixSpacing *= 0.1;
	m_pKernel->SetBasis(vPixSpacing);

	// Do the convolution.
	
	// do for region of interest in z-dir
	for (int nK = mink_in; nK <= maxk_in; nK++)          
	{
		// do for region of interest in y-dir
		for (int nJ = minj_in; nJ <= maxj_in; nJ++)         
		{
			// do for region of interest in x-dir
			for (int nI = mini_in; nI <= maxi_in; nI++)        
			{
				// dose at zero density?
				if (pppDensity[nJ-nOY][nI-nOX][nK-1] > 0.05) 
				{
					SphereTrace(pFluence_in, thickness_in, nI, nJ, nK, pEnergy_out);
				}
				
			}	// end of z-direction loop  
			
		}	// end of y-direction loop
		
	}	// end of x-direction loop 
	
	// Normalize the energy deposited to take into account the
	// summation over the azimuthal angles. Convert the energy to  
	// dose by dividing by mass                            
	
	for (nK = mink_in; nK <= maxk_in; nK++)
	{
		for (int nJ = minj_in; nJ <= maxj_in; nJ++)
		{
			for (int nI = mini_in; nI <= maxi_in; nI++)
			{				
				// The divergence correction is done also here (instead in div_fluence_calc)
				// Added by Nikos

#ifdef CORRECT_DIVERGENCE
				REAL new_distance = sqrt(
					(m_ssd + float(nK) * 4.0 /* mBasis[0][0] */ - 0.5) 
						* (m_ssd + float(nK) * 4.0 /* mBasis[0][0] */ - 0.5)
					+ (float(nJ) * 4.0 /* mBasis[2][2] */) 
						* (float(nJ) * 4.0 /* mBasis[2][2] */)
					+ (float(nI) * 4.0 /* mBasis[1][1] */) 
						* (float(nI) * 4.0 /* mBasis[1][1] */));
#endif

#ifdef CORRECT_DEPTH_DISTANCE
				// depth_distance added for the CF kernel depth hardening effect
				REAL depth_distance = sqrt(
					(float(nK) * 4.0 /* mBasis[0][0] */ - 0.5)
						* (float(nK) * 4.0 /* mBasis[0][0] */ - 0.5) 
					+ (float(nJ) * 4.0 /* mBasis[2][2] */)
						* (float(nJ) * 4.0 /* mBasis[2][2] */) 
					+ (float(nI) * 4.0 /* mBasis[1][1] */)
						* (float(nJ) * 4.0 /* mBasis[2][2] */));
				
				// added to waive the CF correction for mononergetic beams	
				const REAL mvalue = 0; 
				const REAL bvalue = 1;

				REAL c_factor = mvalue * depth_distance + bvalue;
#endif

				// convert to Gy cm**2 and take into account the
				// azimuthal sum
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= 1.602e-10 / (REAL) NUM_THETA;

#ifdef CORRECT_DIVERGENCE
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= (m_ssd / new_distance) * (m_ssd / new_distance);
#endif

#ifdef CORRECT_DEPTH_DISTANCE
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= c_factor;
#endif
			} 
		}
	}
	pEnergy_out->VoxelsChanged();

	REAL maxx = pEnergy_out->GetMax();

	// now normalize to dmax
	for (nK = mink_in; nK <= maxk_in; nK++)
	{
		for (int nJ = minj_in; nJ <= maxj_in; nJ++)
		{
			for (int nI = mini_in; nI <= maxi_in; nI++)
			{								
				pppEnergy[nJ-nOY][nI-nOX][nK-1] = pppEnergy[nJ-nOY][nI-nOX][nK-1] / maxx;
			}
		}
	} 

}	// CBeamDoseCalc::SphereConvolve



///////////////////////////////////////////////////////////////////////////////
// CBeamDoseCalc::SphereTrace
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeamDoseCalc::SphereTrace(CVolume<REAL> *pFluence_in, 
								const REAL thickness_in, int nI, int nJ, int nK,
								CVolume<REAL> *pEnergy_out)
{
	const int numstep_in = 16; // 64;
	
	bool b_ray_trace = true;
	
	// accessors for voxels
	REAL ***pppDensity = m_densityRep.GetVoxels();
	REAL ***pppFluence = pFluence_in->GetVoxels();
	REAL ***pppEnergy = pEnergy_out->GetVoxels();

	const CMatrixD<4> &mBasis = m_densityRep.GetBasis();

	// int nOX = nint(mBasis[3][1] / 4.0 /* mBasis[1][1] */); 
	// int nOY = nint(mBasis[3][2] / 4.0 /* mBasis[2][2] */); 
	int nOX = -m_densityRep.GetHeight() / 2; // nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = -m_densityRep.GetDepth() / 2; // nint(mBasis[3][2] / mBasis[2][2]); 

	// set up (moved to higher level)
	// m_pKernel->SetBasis(mBasis);

	// do for all azimuthal angles
	for (int thet = 1; thet <= NUM_THETA; thet++)            
	{
		// do for zenith angles 
		for (int phi = 1; phi <= m_pKernel->GetNumPhi(); phi++)
		{
//			REAL last_rad = 0.0;
			REAL rad_dist = 0.0;
			REAL last_energy = 0.0;
			
			// loop over radial increments
			for (int rad_inc = 1; rad_inc <= numstep_in; rad_inc++)       
			{
				// integer distances between the interaction and the dose depostion voxels
				int deli = m_pKernel->m_delta_i[thet-1][phi-1][rad_inc-1];    
				int delj = m_pKernel->m_delta_j[thet-1][phi-1][rad_inc-1];    
				int delk = m_pKernel->m_delta_k[thet-1][phi-1][rad_inc-1];    

				// CVectorD<3, int> vDelta;
				// m_pKernel->GetDelta(theta, phi, rad_inc, vDelta);

//				int nDepthNum = nint(thickness_in / 4.0 /* mBasis[0][0] */);
				
				// test to see if inside field and phantom
				if (nK-delk-1 < 0         
					|| nK-delk-1 >= m_densityRep.GetWidth() 
					|| nI-deli-nOX < 0 
					|| nI-deli-nOX >= m_densityRep.GetHeight() 
					|| nJ-delj-nOY < 0
					|| nJ-delj-nOY >= m_densityRep.GetDepth()) 
				{
					break;	
				}
				
				// Increment rad_dist, the radiological path from the dose deposition site. 
				
				REAL pathinc = m_pKernel->GetRadius(thet, phi, rad_inc);

				
				const REAL avdens = 1.0; 
				const REAL kerndens = 1.0;
																		//	(phi,nint(rad_dist*10.0));

				REAL delta_rad = b_ray_trace 
					? pathinc * pppDensity[nJ-delj-nOY][nI-deli-nOX][nK-delk-1] 
						/ kerndens
					:pathinc * avdens 
						/ kerndens;
				
				rad_dist += delta_rad;
				
				// the radiological path exceeds the kernel size 
				//		which is 60cm by definition for high density medium
				if (rad_dist >= 60) 
				{
					break;
				}
				
				// Use lookup table to find the value of the cumulative energy
				// deposited up to this radius. No interpolation is done. The
				// resolution of the array in the radial direction is every mm
				// hence the multiply by 10.0 in the arguement.
				
				REAL tot_energy = m_pKernel->GetCumEnergy(phi, rad_dist);
				
				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				
				REAL energy = tot_energy - last_energy;
				last_energy = tot_energy;             
				
				// The energy is accumulated - superposition
				pppEnergy[nJ-nOY][nI-nOX][nK-1] +=  
					energy * pppFluence[nJ-delj-nOY][nI-deli-nOX][nK-delk-1];
				
			}	// end of radial path loop                

		}	// end end of zenith angle loop

	}	// end of azimuth angle loop 
}




