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
CBeam::CBeam(CSource *pSource, double ssd)
:	m_pSource(pSource),
		m_ssd(ssd),
		m_pDensity(NULL),
		m_pFluence(NULL),
		m_pEnergy(NULL),
		m_bSetupRaytrace(true)
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
void CBeam::SetDensity(CVolume<double> *pDensity, CVectorD<3> vBasis, double mu)
{
	m_pDensity = pDensity;
	m_vBasis = vBasis;
	m_mu = mu;

}	// CBeam::SetDensity



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
	if (!m_pFluence)
	{
		m_pFluence = new CVolume<double>();
		m_pFluence->ConformTo(m_pDensity);
	}
	m_pFluence->ClearVoxels();

	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();

	int nOX = m_pDensity->GetWidth() / 2;
	int nOY = m_pDensity->GetHeight() / 2;

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
	double xinc = m_vBasis[0]
		/ rays_per_voxel_in;          
	double yinc = m_vBasis[1]
		/ rays_per_voxel_in;
	
	// integers defining the rays
	int nSmallFieldI = nint(xmin / xinc);    
	int nLargeFieldI = nint(xmax / xinc);    
	int nSmallFieldJ = nint(ymin / yinc);
	int nLargeFieldJ = nint(ymax / yinc);
	
	// the depth of the surface voxels.
	double mindepth = m_ssd - offset + 0.5 * m_vBasis[2];   	
	int nDepthNum = nint(thickness_in / m_vBasis[2]);

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

			// distance increment; recall that leninc/len0 = m_vBasis[2]/mindepth
			double leninc = len0 * m_vBasis[2] / mindepth; 
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
				double latscale = 1.0 + double(nK-1) * m_vBasis[2] / mindepth;        
				
				// these values are the distance from the central axis to the rays.
				double fieldx = fieldx0*latscale;              
				double fieldy = fieldy0*latscale;              
				
				// the integers describing the nearest voxel
				int nNearI = nint(fieldx / m_vBasis[0]);   
				int nNearJ = nint(fieldy / m_vBasis[1]);   
				
				// Calculate the radiological path increment travelled in the phantom.
				//                  the factor 0.5 in pathinc is introduced so to have 
				//                  smaller steps and account better for inhomogeneities
				
				// radiological pathlength increment
				double pathinc = 0.5 * leninc * pppDensity[nNearJ+nOY][nNearI+nOX][nK-1];
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
					(m_ssd + (double(nK) - 0.5) * m_vBasis[2]) 
						/ m_ssd;

				double minfieldx = xmin * div_scale;
				double maxfieldx = xmax * div_scale;
				double minfieldy = ymin * div_scale;
				double maxfieldy = ymax * div_scale;
				
				if (nNearI < nint(minfieldx / m_vBasis[0])) 
				{
					weightx = 0.0;
				}
				else if (nNearI == nint(minfieldx / m_vBasis[0]))
				{
					weightx = double(nNearI) 
						- minfieldx / m_vBasis[0] + 0.5;
				}
				else if (nNearI == nint(maxfieldx / m_vBasis[0]))
				{
					weightx = maxfieldx / m_vBasis[0] 
						- 0.5 - double(nNearI-1);
				}
				else if (nNearI > nint(maxfieldx/m_vBasis[0])) 
				{
					weightx = 0.0;
				}
				
				if (nNearJ < nint(minfieldy / m_vBasis[1])) 
				{	
					weighty = 0.0;
				}
				else if (nNearJ == nint(minfieldy / m_vBasis[1]))
				{
					weighty = double(nNearJ) 
						- minfieldy / m_vBasis[1] + 0.5;
				}
				else if (nNearJ == nint(maxfieldy / m_vBasis[1]))
				{
					weighty = maxfieldy / m_vBasis[1] 
						- 0.5 - double(nNearJ-1);
				}
				else if (nNearJ > nint(maxfieldy / m_vBasis[1])) 
				{	
					weighty = 0.0;
				}
				
				// Calculate the relative amount of fluence that interacts in each voxel
				// and convert from photons/cm**2 to photons by multiplying by the
				// cross-sectional area of the voxel. Also include weighting factors
				// that determine the relative number of photons interacting near
				// the boundary of the field.
				
				if (pppDensity[nNearJ+nOY][nNearI+nOX][nK-1] != 0.0)
				{
					// nmu = -alog(f_factor(nint(path)))/path
					atten *= exp(-m_mu * delta_path);
					
					// fluence increment    
					fluinc = incfluence * atten * m_mu
						* weightx * weighty;
					
					//  the divergence correction above was removed from here to be put
					//  in new_sphere_convolve
					
					pppFluence[nNearJ+nOY][nNearI+nOX][nK-1] += fluinc;  // fluence
					
					// normalization
					pppNorm[nNearJ+nOY][nNearI+nOX][nK-1]++;    
				}
				else
				{
					pppFluence[nNearJ+nOY][nNearI+nOX][nK-1] = 0.0;
					
					// avoid divide by zero
					pppNorm[nNearJ+nOY][nNearI+nOX][nK-1] = 1;	
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
	int nMinI = nint(x_bottom_min / m_vBasis[0]);    
	int nMaxI = nint(x_bottom_max / m_vBasis[0]);    
	int nMinJ = nint(y_bottom_min / m_vBasis[1]);      
	int nMaxJ = nint(y_bottom_max / m_vBasis[1]);    
	
	for (nI = nMinI; nI <= nMaxI; nI++)
	{
		for (int nJ = nMinJ; nJ <= nMaxJ; nJ++)
		{
			for (int nK = 1; nK <= nDepthNum; nK++)
			{
				if (pppNorm[nJ+nOY][nI+nOX][nK-1] != 0)
				{
					// do normalization
					pppFluence[nJ+nOY][nI+nOX][nK-1] 
						/= (double) pppNorm[nJ+nOY][nI+nOX][nK-1];
				}				
			}
		}
	}

}	// CBeam::DivFluenceCalc


///////////////////////////////////////////////////////////////////////////////
// CBeam::GetFluence
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> *CBeam::GetFluence()
{
	return m_pFluence;

}	// CBeam::GetFluence


///////////////////////////////////////////////////////////////////////////////
// CBeam::SphereTrace
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeam::SphereTrace(const double thickness_in, int nI, int nJ, int nK)
{
	RayTraceSetup();

	const int numstep_in = 64;
	
	bool b_ray_trace = true;
	
	// accessors for voxels
	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();
	double ***pppEnergy = m_pEnergy->GetVoxels();

	int nOX = m_pDensity->GetWidth() / 2;
	int nOY = m_pDensity->GetHeight() / 2;

	// do for all azimuthal angles
	for (int thet = 1; thet <= NUM_THETA; thet++)            
	{
		// do for zenith angles 
		for (int phi = 1; phi <= m_pSource->m_numphi; phi++)             
		{
			double last_rad = 0.0;
			double rad_dist = 0.0;
			double last_energy = 0.0;
			
			// loop over radial increments
			for (int rad_inc = 1; rad_inc <= numstep_in; rad_inc++)       
			{
				// integer distances between the interaction and the dose depostion voxels
				int deli = m_delta_i[rad_inc-1][phi-1][thet-1];    
				int delj = m_delta_j[rad_inc-1][phi-1][thet-1];    
				int delk = m_delta_k[rad_inc-1][phi-1][thet-1];    
				
				int nDepthNum = nint(thickness_in / m_vBasis[2]);
				
				// test to see if inside field and phantom
				if (nK-delk < 1         
					|| nK-delk > nDepthNum 
					|| nI-deli > nOX 
					|| nI-deli < -nOX 
					|| nJ-delj > nOY 
					|| nJ-delj < -nOY) 
				{
					break;	
				}
				
				// Increment rad_dist, the radiological path from the dose deposition site. 
				
				double pathinc = m_radius[rad_inc][phi-1][thet-1];	// (rad_inc,phi,thet);
				
				const double avdens = 1.0; 
				const double kerndens = 1.0;
																		//	(phi,nint(rad_dist*10.0));

				double delta_rad = b_ray_trace 
					? pathinc * pppDensity[nJ-delj+nOY][nI-deli+nOX][nK-delk-1] 
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
				
				double tot_energy = m_pSource->m_cum_energy[phi-1][nint(rad_dist*10.0)];
				
				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				
				double energy = tot_energy - last_energy;
				last_energy = tot_energy;             
				
				GetFluence()->GetVoxelAt(nK-delk-1, nJ-delj+nOY, nI-deli+nOX);

				// The energy is accumulated - superposition
				pppEnergy[nJ+nOY][nI+nOX][nK-1] +=  
					energy * pppFluence[nJ-delj+nOY][nI-deli+nOX][nK-delk-1];
				
			}	// end of radial path loop                

		}	// end end of zenith angle loop

	}	// end of azimuth angle loop 
}


//////////////////////////////////////////////////////////////////////////////
// CBeam::SphereConvolve
//
// spherical convolution
//////////////////////////////////////////////////////////////////////////////
void CBeam::SphereConvolve(const double thickness_in)
{
	// initialize energy
	if (!m_pEnergy)
	{
		m_pEnergy = new CVolume<double>();
		m_pEnergy->ConformTo(m_pDensity);
	}
	m_pEnergy->ClearVoxels();

	// accessors for voxels
	double ***pppDensity = m_pDensity->GetVoxels();
	double ***pppFluence = m_pFluence->GetVoxels();
	double ***pppEnergy = m_pEnergy->GetVoxels();

	int nOX = m_pDensity->GetWidth() / 2;
	int nOY = m_pDensity->GetHeight() / 2;

	// parameters
	const int mini_in = 
		nint( // 8.0 * xmin 
			-10.0 / m_vBasis[0]);
	const int maxi_in =  
		nint( // 8.0 * xmax 
			10.0 / m_vBasis[0]);
	const int minj_in = ymin / m_vBasis[1];
	const int maxj_in = ymax / m_vBasis[1];
	const int mink_in = 1;
	const int maxk_in = 100;
						 
	// Routine sets up the ray-trace calculation.	
	m_pSource->EnergyLookup();

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
				if (pppDensity[nJ+nOY][nI+nOX][nK-1] != 0.0) 
				{
					SphereTrace(thickness_in, nI, nJ, nK);
				}
				
			}	// end of z-direction loop  
			
		}	// end of y-direction loop
		
	}	// end of x-direction loop 
	
	// Normalize the energy deposited to take into account the
	// summation over the azimuthal angles. Convert the energy to  
	// dose by dividing by mass and find the position of dmax
	// and the dose at dmax.                              
	
	for (nK = mink_in; nK <= maxk_in; nK++)
	{
		for (int nJ = minj_in; nJ <= maxj_in; nJ++)
		{
			for (int nI = mini_in; nI <= maxi_in; nI++)
			{				
				// The divergence correction is done also here (instead in div_fluence_calc)
				// Added by Nikos
				
				double new_distance = sqrt(
					(m_ssd + float(nK) * m_vBasis[2] - 0.5) 
						* (m_ssd + float(nK) * m_vBasis[2] - 0.5)
					+ (float(nJ) * m_vBasis[1]) 
						* (float(nJ) * m_vBasis[1])
					+ (float(nI) * m_vBasis[0]) 
						* (float(nI) * m_vBasis[0]));
				
				// depth_distance added for the CF kernel depth hardening effect
				double depth_distance = sqrt(
					(float(nK) * m_vBasis[2] - 0.5)
						* (float(nK) * m_vBasis[2] - 0.5) 
					+ (float(nJ) * m_vBasis[1])
						* (float(nJ) * m_vBasis[1]) 
					+ (float(nI) * m_vBasis[0])
						* (float(nJ) * m_vBasis[1]));
				
				// added to waive the CF correction for mononergetic beams	
				const double mvalue = 0; 
				const double bvalue = 1;

				double c_factor = mvalue * depth_distance + bvalue;
				
				// convert to Gy cm**2 and take into account the
				// azimuthal sum
				m_pEnergy->GetVoxelAt(nK-1, nJ+nOY, nI+nOX);
				pppEnergy[nJ+nOY][nI+nOX][nK-1] *= 1.602e-10 / (double) NUM_THETA;
				pppEnergy[nJ+nOY][nI+nOX][nK-1] *= (m_ssd / new_distance) * (m_ssd / new_distance);
				pppEnergy[nJ+nOY][nI+nOX][nK-1] *= c_factor;
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
				ASSERT(pppEnergy[nJ+nOY][nI+nOX][nK-1] <= maxx);
				pppEnergy[nJ+nOY][nI+nOX][nK-1] = pppEnergy[nJ+nOY][nI+nOX][nK-1] / maxx;
			}
		}
	} 

}	// CBeam::SphereConvolve



///////////////////////////////////////////////////////////////////////////////
// CBeam::GetEnergy
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<double> *CBeam::GetEnergy()
{
	return m_pEnergy;

}	// CBeam::GetEnergy


//////////////////////////////////////////////////////////////////////////////
// CBeam::MakeVector
//
// makes a vector from a set of directions
//////////////////////////////////////////////////////////////////////////////
void CBeam::MakeVector(
				  const int numstep_in,         // number of voxels passed thru
				  const double factor1_in,      // principle direction cosine
				  const double factor2_in,      // secondary
				  const double factor3_in,      //    direction cosines
				  
				  double (&r_out)[64],    // list of lengths thru a voxel
				  int (&delta1_out)[64],  // location in principle direction
				  int (&delta2_out)[64],  // locations in
				  int (&delta3_out)[64]   //    secondary directions
				  )
{                 
	// At each ray-trace step determine the radius and the voxel number 
	// along each coordinate direction.                                 
	
	for (int nI = 1; nI <= numstep_in; nI++)
	{
        // distance to the end of a voxel
		double d = double(nI) - 0.5;                 
		
		// want absolute distance
		if (factor1_in < 0.0) 
		{
			d = -d;     
		}
		
		if (fabs(factor1_in) >= 1e-04) 
		{	
			// radius to interection point
			r_out[nI-1] = d / factor1_in;                
		}      
		else
		{
			// effectively infinity
			r_out[nI-1] = 100000.0;                 
		}
		
		// Calculate a distance along a coordinate direction and find the nearest
		// integer to specify the voxel direction.
        
		delta1_out[nI-1] = nint(0.99 * r_out[nI-1] * factor1_in);  
		// 0.99 prevents being exactly on the voxel boundary
		delta2_out[nI-1] = nint(r_out[nI-1] * factor2_in);       
		delta3_out[nI-1] = nint(r_out[nI-1] * factor3_in);       
	}

}	// CBeam::MakeVector



//////////////////////////////////////////////////////////////////////////////
// CBeam::RayTraceSetup
//
// sets up the ray trace for conv.
//////////////////////////////////////////////////////////////////////////////
void CBeam::RayTraceSetup()
{
	if (!m_bSetupRaytrace)
	{
		return;
	}
	
	const int numstep_in = 64;

	// the azimuthal angle increment is calculated
	double ang_inc = 2.0 * PI 
		/ double(NUM_THETA); 
	
	// loop thru all zenith angles
	for (int nPhi = 1; nPhi <= m_pSource->m_numphi; nPhi++)              
	{
		// trig. for zenith angles
		double sphi = sin(m_pSource->m_ang[nPhi]);
		double cphi = cos(m_pSource->m_ang[nPhi]);
		
		// loop thru all azimuthal angles
		for (int nTheta = 1; nTheta <= NUM_THETA; nTheta++)                  
		{
			// trig. for azimuthal angles
			double sthet = sin(double(nTheta) * ang_inc);    
			double cthet = cos(double(nTheta) * ang_inc);
			
			// MAKE_VECTOR is called for each direction. It returns the distance from
			// the intersection of a plane defined by a coordinate value and the ray along
			// each direction.                                
			
			// Call for the y-z plane crossing list. Plane defined by the x-coordinate.
			
			double rx[64];
			int delta_i_x[64];	// (64)
			int delta_j_x[64];	// (64)
			int delta_k_x[64];	// (64)
			
			MakeVector(numstep_in,              
				sphi*cthet / m_vBasis[0],   // x-dir direction cosine
				sphi*sthet / m_vBasis[1],   // y-dir direction cosine 
				cphi       / m_vBasis[2],   // z-dir direction cosine    
				rx,                   // list of dist. to x-bound  
				delta_i_x,            // x-dir voxel location      
				delta_j_x,            // y-dir voxel location      
				delta_k_x);           // z-dir voxel location      
			
			// Call for the x-z plane crossing list. Plane defined by the y-coordinate.
			
			double ry[64];
			int delta_i_y[64];	// (64)
			int delta_j_y[64];	// (64)
			int delta_k_y[64];	// (64)
			
			MakeVector(numstep_in,              
				sphi*sthet / m_vBasis[1],   // y-dir direction cosine    
				sphi*cthet / m_vBasis[0],   // x-dir direction cosine    
				cphi       / m_vBasis[2],   // z-dir direction cosine    
				ry,                   // list of dist. to y-bound  
				delta_j_y,            // y-dir voxel location      
				delta_i_y,            // x-dir voxel location      
				delta_k_y);           // z-dir voxel location      
			
			// Call for the x-y plane crossing list. Plane defined by the z-coordinate.
			
			double rz[64];
			int delta_i_z[64];	// (64)      
			int delta_j_z[64];	// (64)
			int delta_k_z[64];	// (64)
			MakeVector(numstep_in,              
				cphi       / m_vBasis[2],   // z-dir direction cosine    
				sphi*sthet / m_vBasis[1],   // y-dir direction cosine    
				sphi*cthet / m_vBasis[0],   // x-dir direction cosine    
				rz,                   // list of dist. to z-bound  
				delta_k_z,            // z-dir voxel location      
				delta_j_z,            // y-dir voxel location      
				delta_i_z);           // x-dir voxel location      
			
			int nI = 1;
			int nJ = 1;
			int nK = 1;
			
			m_radius[0][nPhi-1][nTheta-1] = 0.0;             // radius at origin is 0
			double last_radius = 0.0;
			
			// The following sorts through the distance vectors, rx,ry,rz
			// to find the next smallest value. This will be the next plane crossed.
			// A merged vector is created that lists the location of the voxel crossed
			// and the length thru it in the order of crossings.
			
			for (int nN = 1; nN <= numstep_in; nN++)
			{
				// done if plane defined by the x-coord crossed
				if (rx[nI-1] <= ry[nJ-1] && rx[nI-1] <= rz[nK-1])
				{
					// length thru voxel
					m_radius[nN][nPhi-1][nTheta-1] = rx[nI-1] - last_radius;	

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nN-1][nPhi-1][nTheta-1] = delta_i_x[nI-1];	
					m_delta_j[nN-1][nPhi-1][nTheta-1] = delta_j_x[nI-1];
					m_delta_k[nN-1][nPhi-1][nTheta-1] = delta_k_x[nI-1];

					// radius gone at this point
					last_radius = rx[nI-1];	
					
					// decrement x-dir counter
					nI = nI+1;											
				}
				// done if plane defined by the y-coord crossed
				else if (ry[nJ-1] <= rx[nI-1] && ry[nJ-1] <= rz[nK-1])    
				{
					// length thru voxel
					m_radius[nN][nPhi-1][nTheta-1] = ry[nJ-1]-last_radius;		

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nN-1][nPhi-1][nTheta-1] = delta_i_y[nJ-1];	
					m_delta_j[nN-1][nPhi-1][nTheta-1] = delta_j_y[nJ-1];	
					m_delta_k[nN-1][nPhi-1][nTheta-1] = delta_k_y[nJ-1];	

					// radius gone at this point
					last_radius = ry[nJ-1];								

					// decrement y-dir counter					
					nJ = nJ+1;											
				}
				// done if plane defined by the z-coord crossed
				else if (rz[nK-1] <= rx[nI-1] && rz[nK-1] <= ry[nJ-1])     
				{
					// length thru voxel
					m_radius[nN][nPhi-1][nTheta-1] = rz[nK-1]-last_radius;		

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nN-1][nPhi-1][nTheta-1] = delta_i_z[nK-1];	
					m_delta_j[nN-1][nPhi-1][nTheta-1] = delta_j_z[nK-1];	
					m_delta_k[nN-1][nPhi-1][nTheta-1] = delta_k_z[nK-1];	

					// radius gone at this point
					last_radius = rz[nK-1];								

					// decrement z-dir counter
					nK = nK+1;											
				}
			}
		}
	}

	m_bSetupRaytrace = false;

}	// CBeam::RayTraceSetup


