// Beam.cpp: implementation of the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DivFluence.h"
#include "Beam.h"


#include <MathUtil.h>

#include "Source.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



///////////////////////////////////////////////////////////////////////////////
// nint
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
static int nint(double val) 
{
	return (int) floor(val + 0.5);

}	// nint




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CBeam::CBeam
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeam::CBeam(CEnergyDepKernel *pSource, double ssd)
:	m_pSource(pSource),
		m_ssd(ssd),
		m_pDensity(NULL),
		m_pFluence(NULL),
		m_pEnergy(NULL)
{

}	// CBeam::CBeam

///////////////////////////////////////////////////////////////////////////////
// CBeam::~CBeam
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeam::~CBeam()
{
	delete m_pEnergy;
	delete m_pFluence;

}	// CBeam::~CBeam

///////////////////////////////////////////////////////////////////////////////
// CBeam::SetDensity
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeam::SetDensity(CVolume<double> *pDensity, double mu)
{
	m_pDensity = pDensity;
	m_mu = mu;

// 	m_pSource->SetBasis(pDensity->GetBasis());

}	// CBeam::SetDensity



void CBeam::SetDoseCalcRegion(const CVectorD<3> &vMin, const CVectorD<3> &vMax)
{
	m_vDoseCalcRegionMin = vMin;
	m_vDoseCalcRegionMax = vMax;
}


///////////////////////////////////////////////////////////////////////////////
// CBeam::GetFluence
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> *CBeam::GetFluence()
{
	// initialize fluence
	if (!m_pFluence)
	{
		m_pFluence = new CVolume<double>();
		m_pFluence->ConformTo(m_pDensity);
	}

	return m_pFluence;

}	// CBeam::GetFluence


//////////////////////////////////////////////////////////////////////////////
// CBeam::DivFluenceCalc
//
// divergent fluence calculation
//////////////////////////////////////////////////////////////////////////////
void CBeam::DivFluenceCalc(
						   const double rays_per_voxel_in,
						   const double thickness_in
						   )
{	
	// initialize fluence
	GetFluence()->ClearVoxels();

	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();

	const CMatrixD<4> &mBasis = m_pDensity->GetBasis();

	int nOX = nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = nint(mBasis[3][2] / mBasis[2][2]); 

	// array for normalization
	static CVolume<double> *pNorm = NULL;
	if (!pNorm)
	{
		pNorm = new CVolume<double>();
	}
	pNorm->ConformTo(m_pDensity);
	pNorm->ClearVoxels();
	double ***pppNorm = pNorm->GetVoxels();

	// fluinc initialization
	double fluinc = 0.0;
	
	// added to waive the CF correction for mononergetic beams	
	double offset = 0.0;
	
	// the distance between the rays at the phantom surface
	double xinc = mBasis[1][1] 
		/ rays_per_voxel_in;          
	double yinc = mBasis[2][2]
		/ rays_per_voxel_in;
	
	// integers defining the rays
	int nSmallFieldI = nint(xmin / xinc);    
	int nLargeFieldI = nint(xmax / xinc);    
	int nSmallFieldJ = nint(ymin / yinc);
	int nLargeFieldJ = nint(ymax / yinc);
	
	// the depth of the surface voxels.
	double mindepth = m_ssd - offset + 0.5 * mBasis[0][0]; 
	int nDepthNum = nint(thickness_in / mBasis[0][0]); 

	// do for the x-direction.
	for (int nI = nSmallFieldI; nI <= nLargeFieldI; nI++)
	{	
		// do for the y-direction.
		for (int nJ = nSmallFieldJ; nJ <= nLargeFieldJ; nJ++)
		{
			
			double incfluence = 1.0; // accessory[nI+450][nJ+450];
			
			// these values are the distance from
			// the central axis to the ray for
			// rays at the surface voxels.
			double fieldx0 = ((double)nI) * xinc * mindepth / m_ssd;							
			double fieldy0 = ((double)nJ) * yinc * mindepth / m_ssd;	
			
			// Calculate the distance from the focal spot to the ray crossing
			
			// initial distance
			double lensquare = fieldx0*fieldx0 
				+ fieldy0*fieldy0 
				+ mindepth*mindepth;				
			
			// squared
			double len0 = sqrt(lensquare);           // initial distance

			// distance increment; recall that leninc/len0 = mBasis[0][0]/mindepth
			double leninc = len0 * mBasis[0][0] 
				/ mindepth; 
			double distance = len0;             
			
			// radiological pathlength in the last voxel
			double last_pathinc = 0.0;      

			// radiological pathlength in the phantom
			double path = 0.0;              

			// amount of exponential attenuation in the phantom
			double atten = 1.0;             
			
			// Start raytracing along the ray.
			
			// the number of depth voxels
			for (int nK = 1; nK <= nDepthNum; nK++)
			{
				// the relative increase in the ray divergence
				double latscale = 1.0 + double(nK-1) * mBasis[0][0] / mindepth;        
				
				// these values are the distance from the central axis to the rays.
				double fieldx = fieldx0*latscale;              
				double fieldy = fieldy0*latscale;              
				
				// the integers describing the nearest voxel
				int nNearI = nint(fieldx / mBasis[1][1]);   
				int nNearJ = nint(fieldy / mBasis[2][2]);   
				
				// Calculate the radiological path increment travelled in the phantom.
				//                  the factor 0.5 in pathinc is introduced so to have 
				//                  smaller steps and account better for inhomogeneities
				
				// radiological pathlength increment
				double pathinc = 0.5 * leninc * pppDensity[nNearJ-nOY][nNearI-nOX][nK-1];
				double delta_path = pathinc + last_pathinc;   
				path += delta_path;
				last_pathinc = pathinc;
				
				// these are the weights if the voxel is completely inside the field.
				double weightx = 1.0;                  
				double weighty = 1.0;                  
				
				// The following if statements tests to see if the voxel is sitting on
				// the field boundary. If it is the proportion of the voxel inside the
				// field is calculated. Dave Convery's second reported bug was fixed 
				// by properly scaling the field size for the divergence of the beam.
				
				double div_scale = 
					(m_ssd + (double(nK) - 0.5) * mBasis[0][0]) 
						/ m_ssd;

				double minfieldx = xmin * div_scale;
				double maxfieldx = xmax * div_scale;
				double minfieldy = ymin * div_scale;
				double maxfieldy = ymax * div_scale;
				
				if (nNearI < nint(minfieldx / mBasis[1][1])) 
				{
					weightx = 0.0;
				}
				else if (nNearI == nint(minfieldx / mBasis[1][1]))
				{
					weightx = double(nNearI) 
						- minfieldx / mBasis[1][1] + 0.5;
				}
				else if (nNearI == nint(maxfieldx / mBasis[1][1]))
				{
					weightx = maxfieldx / mBasis[1][1]  
						- 0.5 - double(nNearI-1);
				}
				else if (nNearI > nint(maxfieldx/mBasis[1][1])) 
				{
					weightx = 0.0;
				}
				
				if (nNearJ < nint(minfieldy / mBasis[2][2])) 
				{	
					weighty = 0.0;
				}
				else if (nNearJ == nint(minfieldy / mBasis[2][2]))
				{
					weighty = double(nNearJ) 
						- minfieldy / mBasis[2][2] + 0.5;
				}
				else if (nNearJ == nint(maxfieldy / mBasis[2][2]))
				{
					weighty = maxfieldy / mBasis[2][2] 
						- 0.5 - double(nNearJ-1);
				}
				else if (nNearJ > nint(maxfieldy / mBasis[2][2])) 
				{	
					weighty = 0.0;
				}
				
				// Calculate the relative amount of fluence that interacts in each voxel
				// and convert from photons/cm**2 to photons by multiplying by the
				// cross-sectional area of the voxel. Also include weighting factors
				// that determine the relative number of photons interacting near
				// the boundary of the field.
				
				if (pppDensity[nNearJ-nOY][nNearI-nOX][nK-1] != 0.0)
				{
					// nmu = -alog(f_factor(nint(path)))/path
					atten *= exp(-m_mu * delta_path);
					
					// fluence increment    
					fluinc = incfluence * atten * m_mu
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
	double bottom_scale = (m_ssd + thickness_in) / m_ssd;        
	
	//       x_bottom_min = xmin*bottom_scale          // lateral dimensions of bottom
	//       x_bottom_max = xmax*bottom_scale          // voxels
	//       y_bottom_min = ymin*bottom_scale          // 
	//       y_bottom_max = ymax*bottom_scale          // 
	
	
	double x_bottom_min = (xmin > 0.0) ? xmin 
		: xmin * bottom_scale;
	
	double x_bottom_max = (xmax < 0.0) ? xmax
		: xmax * bottom_scale; 	
	
	double y_bottom_min = (ymin > 0.0) ? ymin 
		: ymin * bottom_scale; 
	
	double y_bottom_max = (ymax < 0.0) ? ymax
		: ymax * bottom_scale; 
	
	// location of bottom voxels saves time in normalization below
	int nMinI = nint(x_bottom_min / mBasis[1][1]);    
	int nMaxI = nint(x_bottom_max / mBasis[1][1]);    
	int nMinJ = nint(y_bottom_min / mBasis[2][2]);      
	int nMaxJ = nint(y_bottom_max / mBasis[2][2]);    
	
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
						/= (double) pppNorm[nJ-nOY][nI-nOX][nK-1];
				}				
			}
		}
	}

}	// CBeam::DivFluenceCalc


///////////////////////////////////////////////////////////////////////////////
// CBeam::GetEnergy
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> *CBeam::GetEnergy()
{
	// initialize energy
	if (!m_pEnergy)
	{
		m_pEnergy = new CVolume<double>();
		m_pEnergy->ConformTo(m_pDensity);
	}
	return m_pEnergy;

}	// CBeam::GetEnergy


//////////////////////////////////////////////////////////////////////////////
// CBeam::SphereConvolve
//
// spherical convolution
//////////////////////////////////////////////////////////////////////////////
void CBeam::SphereConvolve(const double thickness_in)
{
	// initialize energy
	GetEnergy()->ClearVoxels();

	// accessors for voxels
	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();
	double ***pppEnergy = m_pEnergy->GetVoxels();

	const CMatrixD<4> &mBasis = m_pDensity->GetBasis();

	int nOX = nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = nint(mBasis[3][2] / mBasis[2][2]); 

	// parameters
	const int mini_in = 
		nint(m_vDoseCalcRegionMin[1] / mBasis[1][1]);
	const int maxi_in =  
		nint(m_vDoseCalcRegionMax[1] / mBasis[1][1]);
	const int minj_in = 
		nint(m_vDoseCalcRegionMin[2] / mBasis[2][2]);
	const int maxj_in = 
		nint(m_vDoseCalcRegionMax[2] / mBasis[2][2]);
	const int mink_in = 1;
	const int maxk_in = 100;

	// CMatrixD<4> mBasisInv = mBasis;
	// mBasisInv.Invert();

	// CVectorD<3> vIndexMin = FromHG<3, REAL>(mBasisInv * ToHG(m_vDoseCalcRegionMin));
	// CVectorD<3> vIndexMax = FromHG<3, REAL>(mBasisInv * ToHG(m_vDoseCalcRegionMax));

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
				if (pppDensity[nJ-nOY][nI-nOX][nK-1] != 0.0) 
				{
					SphereTrace(thickness_in, nI, nJ, nK);
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
				
				double new_distance = sqrt(
					(m_ssd + float(nK) * mBasis[0][0] - 0.5) 
						* (m_ssd + float(nK) * mBasis[0][0] - 0.5)
					+ (float(nJ) * mBasis[2][2]) 
						* (float(nJ) * mBasis[2][2])
					+ (float(nI) * mBasis[1][1]) 
						* (float(nI) * mBasis[1][1]));
				
				// depth_distance added for the CF kernel depth hardening effect
				double depth_distance = sqrt(
					(float(nK) * mBasis[0][0] - 0.5)
						* (float(nK) * mBasis[0][0] - 0.5) 
					+ (float(nJ) * mBasis[2][2])
						* (float(nJ) * mBasis[2][2]) 
					+ (float(nI) * mBasis[1][1])
						* (float(nJ) * mBasis[2][2]));
				
				// added to waive the CF correction for mononergetic beams	
				const double mvalue = 0; 
				const double bvalue = 1;

				double c_factor = mvalue * depth_distance + bvalue;
				
				// convert to Gy cm**2 and take into account the
				// azimuthal sum
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= 1.602e-10 / (double) NUM_THETA;
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= (m_ssd / new_distance) * (m_ssd / new_distance);
				pppEnergy[nJ-nOY][nI-nOX][nK-1] *= c_factor;
			} 
		}
	}
	m_pEnergy->VoxelsChanged();

	double maxx = m_pEnergy->GetMax();

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

}	// CBeam::SphereConvolve



///////////////////////////////////////////////////////////////////////////////
// CBeam::SphereTrace
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeam::SphereTrace(const double thickness_in, int nI, int nJ, int nK)
{
	const int numstep_in = 64;
	
	bool b_ray_trace = true;
	
	// accessors for voxels
	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();
	double ***pppEnergy = m_pEnergy->GetVoxels();

	const CMatrixD<4> &mBasis = m_pDensity->GetBasis();

	int nOX = nint(mBasis[3][1] / mBasis[1][1]); 
	int nOY = nint(mBasis[3][2] / mBasis[2][2]); 

	m_pSource->SetBasis(mBasis);

	// do for all azimuthal angles
	for (int thet = 1; thet <= NUM_THETA; thet++)            
	{
		// do for zenith angles 
		for (int phi = 1; phi <= m_pSource->GetNumPhi(); phi++)
		{
			double last_rad = 0.0;
			double rad_dist = 0.0;
			double last_energy = 0.0;
			
			// loop over radial increments
			for (int rad_inc = 1; rad_inc <= numstep_in; rad_inc++)       
			{
				// integer distances between the interaction and the dose depostion voxels
				int deli = m_pSource->m_delta_i[rad_inc-1][phi-1][thet-1];    
				int delj = m_pSource->m_delta_j[rad_inc-1][phi-1][thet-1];    
				int delk = m_pSource->m_delta_k[rad_inc-1][phi-1][thet-1];    

				// CVectorD<3, int> vDelta;
				// m_pSource->GetDelta(theta, phi, rad_inc, vDelta);

				int nDepthNum = nint(thickness_in / mBasis[0][0]);
				
				// test to see if inside field and phantom
				if (nK-delk-1 < 0         
					|| nK-delk-1 >= m_pDensity->GetWidth() 
					|| nI-deli-nOX < 0 
					|| nI-deli-nOX >= m_pDensity->GetHeight() 
					|| nJ-delj-nOY < 0
					|| nJ-delj-nOY >= m_pDensity->GetDepth()) 
				{
					break;	
				}
				
				// Increment rad_dist, the radiological path from the dose deposition site. 
				
				double pathinc = m_pSource->GetRadius(thet, phi, rad_inc);

				
				const double avdens = 1.0; 
				const double kerndens = 1.0;
																		//	(phi,nint(rad_dist*10.0));

				double delta_rad = b_ray_trace 
					? pathinc * pppDensity[nJ-delj-nOY][nI-deli-nOX][nK-delk-1] 
						/ kerndens
					:pathinc * avdens 
						/ kerndens;
				
				rad_dist = rad_dist+delta_rad;
				
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
				
				double tot_energy = m_pSource->GetCumEnergy(phi, rad_dist);
				
				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				
				double energy = tot_energy - last_energy;
				last_energy = tot_energy;             
				
				// The energy is accumulated - superposition
				pppEnergy[nJ-nOY][nI-nOX][nK-1] +=  
					energy * pppFluence[nJ-delj-nOY][nI-deli-nOX][nK-delk-1];
				
			}	// end of radial path loop                

		}	// end end of zenith angle loop

	}	// end of azimuth angle loop 
}




