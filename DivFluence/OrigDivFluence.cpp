// DivFluence.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DivFluence.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const double pi = 3.141593;


int nint(double val) 
{
	return (int) floor(val + 0.5);
}

//////////////////////////////////////////////////////////////////////////////
// div_fluence_calc
//
// divergent fluence calculation
//////////////////////////////////////////////////////////////////////////////
void div_fluence_calc(
					  const double ssd_in,
					  
					  const double lengthx_in,
					  const double lengthy_in,
					  const double lengthz_in,
					  
					  const double ray_in,
					  
					  const double xmin_in,
					  const double xmax_in,
					  const double ymin_in,
					  const double ymax_in,
					  
					  const double thickness_in,
					  
					  const double mu_in,
					  // const 
					  double (&density_in)[101][101][101],	// (-50:50,-50:50,1:101)
					  
					  double (&fluence_out)[101][101][101]
					  )
{
	// fluinc initialization
	double fluinc = 0.0;
	
	// added to waive the CF correction for mononergetic beams	
	double offset = 0.0;
	
	int depthnum = nint(thickness_in / lengthz_in);
	
	// the depth of the surface voxels.
	double mindepth = ssd_in - offset + 0.5 * lengthz_in;   
	
	// the distance between the rays at the phantom surface
	double xinc = lengthx_in/ray_in;                
	double yinc = lengthy_in/ray_in;  
	
	// integers defining the rays
	int smallfieldi = nint(xmin_in/xinc);    
	int largefieldi = nint(xmax_in/xinc);    
	int smallfieldj = nint(ymin_in/yinc);
	int largefieldj = nint(ymax_in/yinc);
	
	static int norm[101][101][101];		// (-50:50,-50:50,1:101)
	memset(norm, 0, sizeof(norm));
	
	// do nI = smallfieldi,largefieldi            // do for the x-direction.
	for (int nI = smallfieldi; nI <= largefieldi; nI++)
	{	
		// do nJ = smallfieldj,largefieldj           // do for the y-direction.
		for (int nJ = smallfieldj; nJ <= largefieldj; nJ++)
		{
			
			double incfluence = 1.0; // accessory[nI+450][nJ+450];
			
			// these values are the distance from
			// the central axis to the ray for
			// rays at the surface voxels.
			double fieldx0 = ((double)nI) * xinc*mindepth / ssd_in;							
			double fieldy0 = ((double)nJ) * yinc*mindepth / ssd_in;	
			
			// Calculate the distance from the focal spot to the ray crossing
			
			double lensquare = fieldx0*fieldx0 + fieldy0*fieldy0 
				+ mindepth*mindepth;				// initial distance
			
			// squared
			double len0 = sqrt(lensquare);           // initial distance
			double leninc = len0 * lengthz_in/mindepth; // distance increment; recall that
			double distance = len0;                  //		leninc/len0 = lengthz/mindepth
			
			double last_pathinc = 0.0;      // radiological pathlength in the last voxel
			double path = 0.0;              // radiological pathlength in the phantom
			double atten = 1.0;             // amount of exponential attenuation in the
			// phantom
			
			// Start raytracing along the ray.
			
			// do nK = 1,depthnum      // the number of depth voxels
			for (int nK = 1; nK <= depthnum; nK++)
			{
				double latscale = 1.0 + double(nK-1) * lengthz_in / mindepth;        // the relative
				
				// increase in the
				// ray divergence
				double fieldx = fieldx0*latscale;              // these values are the distance
				double fieldy = fieldy0*latscale;              // from the central axis to
				
				// the rays.
				int inear = nint(fieldx/lengthx_in);   // the integers describing the
				int jnear = nint(fieldy/lengthy_in);   // nearest voxel
				
				// Calculate the radiological path increment travelled in the phantom.
				//                  the factor 0.5 in pathinc is introduced so to have 
				//                  smaller steps and account better for inhomogeneities
				
				double pathinc = 0.5 * leninc * density_in[inear+50][jnear+50][nK-1];
				double delta_path = pathinc + last_pathinc;   // radiological pathlength increment
				path += delta_path;
				last_pathinc = pathinc;
				
				double weightx = 1.0;                  // these are the weights if the voxel is
				double weighty = 1.0;                  // completely inside the field.
				
				// The following if statements tests to see if the voxel is sitting on
				// the field boundary. If it is the proportion of the voxel inside the
				// field is calculated. Dave Convery's second reported bug was fixed 
				// by properly scaling the field size for the divergence of the beam.
				
				double div_scale = (ssd_in + (double(nK)-0.5)*lengthz_in) / ssd_in;
				double minfieldx = xmin_in*div_scale;
				double maxfieldx = xmax_in*div_scale;
				double minfieldy = ymin_in*div_scale;
				double maxfieldy = ymax_in*div_scale;
				
				if (inear < nint(minfieldx/lengthx_in)) 
				{
					weightx = 0.0;
				}
				else if (inear == nint(minfieldx/lengthx_in))
				{
					weightx = double(inear) - minfieldx / lengthx_in + 0.5;
				}
				else if (inear == nint(maxfieldx/lengthx_in))
				{
					weightx = maxfieldx / lengthx_in - 0.5 - double(inear-1);
				}
				else if (inear > nint(maxfieldx/lengthx_in)) 
				{
					weightx = 0.0;
				}
				
				if (jnear < nint(minfieldy/lengthy_in)) 
				{	
					weighty = 0.0;
				}
				else if (jnear == nint(minfieldy/lengthy_in))
				{
					weighty = double(jnear) - minfieldy / lengthy_in + 0.5;
				}
				else if (jnear == nint(maxfieldy/lengthy_in))
				{
					weighty = maxfieldy / lengthy_in - 0.5 - double(jnear-1);
				}
				else if (jnear > nint(maxfieldy/lengthy_in)) 
				{	
					weighty = 0.0;
				}
				
				// Calculate the relative amount of fluence that interacts in each voxel
				// and convert from photons/cm**2 to photons by multiplying by the
				// cross-sectional area of the voxel. Also include weighting factors
				// that determine the relative number of photons interacting near
				// the boundary of the field.
				
				if (density_in[inear+50][jnear+50][nK-1] != 0.0)
				{
					// nmu = -alog(f_factor(nint(path)))/path
					atten *= exp(-mu_in * delta_path);
					
					// fluence increment    
					fluinc = incfluence * atten * mu_in      
						* weightx * weighty;
					
					//  the divergence correction above was removed from here to be put
					//  in new_sphere_convolve
					
					fluence_out[inear+50][jnear+50][nK-1] += fluinc;  // fluence
					
					// if (fluinc > oldfluinc && fluinc != 0)
					// {
					// ???
					// continue;
					// }
					
					// normalization
					norm[inear+50][jnear+50][nK-1]++;    
				}
				else
				{
					fluence_out[inear+50][jnear+50][nK-1] = 0.0;
					
					// avoid divide by zero
					norm[inear+50][jnear+50][nK-1] = 1;	
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
	double bottom_scale = (ssd_in + thickness_in) / ssd_in;        
	
	//       x_bottom_min = xmin*bottom_scale          // lateral dimensions of bottom
	//       x_bottom_max = xmax*bottom_scale          // voxels
	//       y_bottom_min = ymin*bottom_scale          // 
	//       y_bottom_max = ymax*bottom_scale          // 
	
	
	double x_bottom_min = (xmin_in > 0.0) ? xmin_in 
		: xmin_in * bottom_scale;
	
	double x_bottom_max = (xmax_in < 0.0) ? xmax_in
		: xmax_in * bottom_scale; 	
	
	double y_bottom_min = (ymin_in > 0.0) ? ymin_in 
		: ymin_in * bottom_scale; 
	
	double y_bottom_max = (ymax_in < 0.0) ? ymax_in
		: ymax_in * bottom_scale; 
	
	// location of bottom voxels saves time in normalization below
	int imin = nint(x_bottom_min/lengthx_in);    
	int imax = nint(x_bottom_max/lengthx_in);    
	int jmin = nint(y_bottom_min/lengthy_in);      
	int jmax = nint(y_bottom_max/lengthy_in);    
	
	// do nI = imin,imax
	for (nI = imin; nI <= imax; nI++)
	{
		// do nJ = jmin,jmax
		for (int nJ = jmin; nJ <= jmax; nJ++)
		{
			// do nK = 1,depthnum
			for (int nK = 1; nK <= depthnum; nK++)
			{
				if (norm[nI+50][nJ+50][nK-1] != 0)
				{
					// do normalization
					fluence_out[nI+50][nJ+50][nK-1] /= (double) norm[nI+50][nJ+50][nK-1];
				}				
			}
		}
	}

}	// div_fluence_calc



//////////////////////////////////////////////////////////////////////////////
// interp_energy
//
// returns energy between boundaries  
//////////////////////////////////////////////////////////////////////////////
double interp_energy(
					 const double bound_1,					// inner boundary
					 const double bound_2,					// outer boundary
					 const int rad_numb,					// radial label of inner voxel
					 const int phi_numb,					// angular label of voxel
					 const double radius[49],			// radius of voxel boundaries	(0:48)
					 const double inc_energy[48][49]		// array of incremental energy (48,0:48)
					 )        
{
	double inc_energy1 = inc_energy[phi_numb-1][rad_numb-1];
    double inc_energy2 = inc_energy[phi_numb-1][rad_numb];
	
	double energy = inc_energy1
		+ (inc_energy2 - inc_energy1) * (bound_2 - radius[rad_numb-1]) 
		/ (radius[rad_numb] - radius[rad_numb-1]);
	
	return energy;
	
}	// interp_energy



//////////////////////////////////////////////////////////////////////////////
// energy_lookup
//
// looks up the energies for a phi angle setting 
//////////////////////////////////////////////////////////////////////////////
void energy_lookup(
				   const double rad_bound_in[49],	// (0:48)
				   const int numrad_in,
				   const int phi_in,
				   const double inc_energy_in[48][49],	// (48,0:48)
				   
				   double (&cum_energy_out)[48][601]	// (48,0:600)
				   )
{
	int rad_numb = 1;
	double last_rad = 0.0;
	double rad_dist = 0.0;
	cum_energy_out[phi_in-1][0] = 0.0;
	
	for (int nI = 1; nI <= 599; nI++)
	{
		rad_dist = nI*0.1;
		
		for (int r = rad_numb-1; r <= numrad_in; r++)
		{
			if (rad_bound_in[r] > rad_dist) 
			{
				break;
			}
		}                      
		
		rad_numb = r;
		
		// New subroutine interpolates the kernel values to get the 
		// energy absorbed.
		
		double tot_energy = 
			interp_energy(
			last_rad,		 // inner boundary
			rad_dist,        // outer boundary
			rad_numb,        // radial label of inner voxel
			phi_in,          // angular label of voxel
			rad_bound_in,    // radius of voxel boundaries
			inc_energy_in    // array of incremental energy
			);
		
		last_rad = rad_dist;
		
		cum_energy_out[phi_in-1][nI] = tot_energy;
	}
	
}	// energy_lookup



//////////////////////////////////////////////////////////////////////////////
// make_vector
//
// makes a vector from a set of direction cosines
//////////////////////////////////////////////////////////////////////////////
void make_vector(
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
	
}	// make_vector



//////////////////////////////////////////////////////////////////////////////
// ray_trace_set_up
//
// sets up lookup tables for the ray trace
//////////////////////////////////////////////////////////////////////////////
void ray_trace_set_up(
					  const int numphi_in,		// number of zenith angles
					  const int numthet_in,		// number of azimuthal angles
					  const int numstep_in,		// number of voxels passed thru
					  const double ang_in[48],	// zenith angle list           		// (48)
						  //		read from dose spread array

					  double lengthx_in,		// dim. of voxel in x-dir
					  double lengthy_in,		// dim. of voxel in y-dir
					  double lengthz_in,		// dim. of voxel in z-dir
					  
					  double (&r_out)[65][48][48],		// list of lengths thru voxels	// (0:64,48,48)
					  int (&delta_i_out)[64][48][48],	// list of voxel # in x-dir
					  int (&delta_j_out)[64][48][48],	// list of voxel # in y-dir		// (64,48,48)
					  int (&delta_k_out)[64][48][48]	// list of voxel # in z-dir		// (64,48,48)
					  )	
{
	// the azimuthal angle increment is calculated
	double ang_inc = 2.0 * pi 
		/ double(numthet_in); 
	
	// loop thru all zenith angles
	for (int phi = 1; phi <= numphi_in; phi++)              
	{
		// trig. for zenith angles
		double sphi = sin(ang_in[phi]);
		double cphi = cos(ang_in[phi]);
		
		// loop thru all azimuthal angles
		for (int thet = 1; thet <= numthet_in; thet++)                  
		{
			// trig. for azimuthal angles
			double sthet = sin(double(thet) * ang_inc);    
			double cthet = cos(double(thet) * ang_inc);
			
			// MAKE_VECTOR is called for each direction. It returns the distance from
			// the intersection of a plane defined by a coordinate value and the ray along
			// each direction.                                
			
			// Call for the y-z plane crossing list. Plane defined by the x-coordinate.
			
			double rx[64];
			int delta_i_x[64];	// (64)
			int delta_j_x[64];	// (64)
			int delta_k_x[64];	// (64)
			
			make_vector(numstep_in,              
				sphi*cthet / lengthx_in,   // x-dir direction cosine
				sphi*sthet / lengthy_in,   // y-dir direction cosine 
				cphi       / lengthz_in,   // z-dir direction cosine    
				rx,                   // list of dist. to x-bound  
				delta_i_x,            // x-dir voxel location      
				delta_j_x,            // y-dir voxel location      
				delta_k_x);           // z-dir voxel location      
			
			// Call for the x-z plane crossing list. Plane defined by the y-coordinate.
			
			double ry[64];
			int delta_i_y[64];	// (64)
			int delta_j_y[64];	// (64)
			int delta_k_y[64];	// (64)
			
			make_vector(numstep_in,              
				sphi*sthet / lengthy_in,   // y-dir direction cosine    
				sphi*cthet / lengthx_in,   // x-dir direction cosine    
				cphi       / lengthz_in,   // z-dir direction cosine    
				ry,                   // list of dist. to y-bound  
				delta_j_y,            // y-dir voxel location      
				delta_i_y,            // x-dir voxel location      
				delta_k_y);           // z-dir voxel location      
			
			// Call for the x-y plane crossing list. Plane defined by the z-coordinate.
			
			double rz[64];
			int delta_i_z[64];	// (64)      
			int delta_j_z[64];	// (64)
			int delta_k_z[64];	// (64)
			make_vector(numstep_in,              
				cphi       / lengthz_in,   // z-dir direction cosine    
				sphi*sthet / lengthy_in,   // y-dir direction cosine    
				sphi*cthet / lengthx_in,   // x-dir direction cosine    
				rz,                   // list of dist. to z-bound  
				delta_k_z,            // z-dir voxel location      
				delta_j_z,            // y-dir voxel location      
				delta_i_z);           // x-dir voxel location      
			
			int nI = 1;
			int nJ = 1;
			int nK = 1;
			
			r_out[0][phi-1][thet-1] = 0.0;             // radius at origin is 0
			double last_radius = 0.0;
			
			// The following sorts through the distance vectors, rx,ry,rz
			// to find the next smallest value. This will be the next plane crossed.
			// A merged vector is created that lists the location of the voxel crossed
			// and the length thru it in the order of crossings.
			
			for (int n = 1; n <= numstep_in; n++)
			{
				// done if plane defined by the x-coord crossed
				if (rx[nI-1] <= ry[nJ-1] && rx[nI-1] <= rz[nK-1])
				{
					r_out[n][phi-1][thet-1] = rx[nI-1] - last_radius;	// length thru voxel
					delta_i_out[n-1][phi-1][thet-1] = delta_i_x[nI-1];	// location of voxel in x-dir.
					delta_j_out[n-1][phi-1][thet-1] = delta_j_x[nI-1];	// location of voxel in y-dir.
					delta_k_out[n-1][phi-1][thet-1] = delta_k_x[nI-1];	// location of voxel in z-dir.
					last_radius = rx[nI-1];								// radius gone at this point
					nI = nI+1;											// decrement x-dir counter
				}
				// done if plane defined by the y-coord crossed
				else if (ry[nJ-1] <= rx[nI-1] && ry[nJ-1] <= rz[nK-1])    
				{
					r_out[n][phi-1][thet-1] = ry[nJ-1]-last_radius;		// length thru voxel
					delta_i_out[n-1][phi-1][thet-1] = delta_i_y[nJ-1];	// location of voxel in x-dir.
					delta_j_out[n-1][phi-1][thet-1] = delta_j_y[nJ-1];	// location of voxel in y-dir.
					delta_k_out[n-1][phi-1][thet-1] = delta_k_y[nJ-1];	// location of voxel in z-dir.
					last_radius = ry[nJ-1];								// radius gone at this point
					nJ = nJ+1;											// decrement y-dir counter					
				}
				// done if plane defined by the z-coord crossed
				else if (rz[nK-1] <= rx[nI-1] && rz[nK-1] <= ry[nJ-1])     
				{
					r_out[n][phi-1][thet-1] = rz[nK-1]-last_radius;		// length thru voxel
					delta_i_out[n-1][phi-1][thet-1] = delta_i_z[nK-1];	// location of voxel in x-dir.
					delta_j_out[n-1][phi-1][thet-1] = delta_j_z[nK-1];	// location of voxel in y-dir.
					delta_k_out[n-1][phi-1][thet-1] = delta_k_z[nK-1];	// location of voxel in z-dir.
					last_radius = rz[nK-1];								// radius gone at this point
					nK = nK+1;											// decrement y-dir counter
				}
			}
		}
	}
	
}	// ray_trace_set_up


// added to waive the CF correction for mononergetic beams	
const double mvalue = 0; 
const double bvalue = 1;


//////////////////////////////////////////////////////////////////////////////
// new_sphere_convolve
//
// spherical convolution
//////////////////////////////////////////////////////////////////////////////
void new_sphere_convolve(
						 const int numphi_in,
						 const int numthet_in,	// = 8;
						 const int numstep_in,	// = 64;
						 const int numrad_in,	// = 48
						 
						 const int mini_in,
						 const int maxi_in,
						 const int minj_in,
						 const int maxj_in,
						 const int mink_in,
						 const int maxk_in,
						 
						 const double ssd_in,
						 
						 const double lengthx_in,
						 const double lengthy_in,
						 const double lengthz_in,
						 
						 const double thickness_in,
						 
						 const double ang_in[48],	// zenith angle list           		// (48)
						 const double rad_bound_in[49],	// (0:48)
						 const double inc_energy_in[48][49],	// (48,0:48)
						 
						 double (&fluence_in)[101][101][101],		// (-50:50,-50:50,1:101);
						 double (&density_in)[101][101][101],		// (-50:50,-50:50,1:101);
						 
						 double (&energy_out)[101][101][101],	// (-50:50,-50:50,1:101);
						 double (&energy_out2)[101][101][101]	// (-50:50,-50:50,1:101);
						 )
{
	
	// Routine sets up the ray-trace calculation.
	
	bool b_ray_trace = true;
	
	static double radius[65][48][48];	// (0:64,48,48); 
	
	static int delta_i[64][48][48];	// (64,48,48);
	static int delta_j[64][48][48];	// (64,48,48);
	static int delta_k[64][48][48];	// (64,48,48);
	
	ray_trace_set_up(
		numphi_in, 
		numthet_in, 
		numstep_in, 
		ang_in, 
		lengthx_in, 
		lengthy_in, 
		lengthz_in, 
		radius, 
		delta_i, 
		delta_j, 
		delta_k
		);

	
	static double cum_energy[48][601];	// (48,0:600);

	for (int nA = 1; nA <= numphi_in; nA++)
	{
		energy_lookup(rad_bound_in, numrad_in, nA, inc_energy_in, cum_energy);
	}
	
	
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
				if (density_in[nI+50][nJ+50][nK-1] == 0.0) 
				{
					continue;
				}
				
				// do for all azimuthal angles
				for (int thet = 1; thet <= numthet_in; thet++)            
				{
					// do for zenith angles 
					for (int phi = 1; phi <= numphi_in; phi++)             
					{
						double last_rad = 0.0;
						double rad_dist = 0.0;
						double last_energy = 0.0;
						
						// loop over radial increments
						for (int rad_inc = 1; rad_inc <= numstep_in; rad_inc++)       
						{
							// integer distances between the interaction and the dose depostion voxels
							int deli = delta_i[rad_inc-1][phi-1][thet-1];    
							int delj = delta_j[rad_inc-1][phi-1][thet-1];    
							int delk = delta_k[rad_inc-1][phi-1][thet-1];    
							
							int depthnum = nint(thickness_in / lengthz_in);
							
							// test to see if inside field and phantom
							if (nK-delk < 1         
								|| nK-delk > depthnum 
								|| nI-deli > 50 
								|| nI-deli < -50 
								|| nJ-delj > 50 
								|| nJ-delj < -50) 
							{
								break;	
							}
							
							// Increment rad_dist, the radiological path from the dose deposition site. 
							
							double pathinc = radius[rad_inc][phi-1][thet-1];	// (rad_inc,phi,thet);
							
							const double avdens = 1.0; 
							const double kerndens = 1.0;
																					//	(phi,nint(rad_dist*10.0));
							double delta_rad = b_ray_trace 
								? pathinc*density_in[nI-deli+50][nJ-delj+50][nK-delk-1] / kerndens
								:pathinc * avdens / kerndens;
							
							rad_dist = rad_dist+delta_rad;
							
							// the radiological path exceeds the kernel size 
							//		which is 60cm by definition
							if (rad_dist >= 60) 
							{
								break;
								// goto label1000;    // for high density medium
							}
							
							// Use lookup table to find the value of the cumulative energy
							// deposited up to this radius. No interpolation is done. The
							// resolution of the array in the radial direction is every mm
							// hence the multiply by 10.0 in the arguement.
							
							double tot_energy = cum_energy[phi-1][nint(rad_dist*10.0)];	
							
							// Subtract the last cumulative energy from the new cumulative energy
							// to get the amount of energy deposited in this interval and set the
							// last cumulative energy for the next time the lookup table is used.
							
							double energy = tot_energy - last_energy;
							last_energy = tot_energy;             
							
							// The energy is accumulated.
							
							energy_out[nI+50][nJ+50][nK-1] +=  
								energy * fluence_in[nI-deli+50][nJ-delj+50][nK-delk-1];  // superposition
							
						}	// end of radial path loop                

					}	// end of azimuth angle loop 

				}	// end end of zenith angle loop
				
			}	// end of z-direction loop  
			
		}	// end of y-direction loop
		
	}	// end of x-direction loop 
	
	// Normalize the energy deposited to take into account the
	// summation over the azimuthal angles. Convert the energy to  
	// dose by dividing by mass and find the position of dmax
	// and the dose at dmax.                              
	
	double max = 0.0;
	int dmaxi; 
	int dmaxj; 
	int dmaxk;
	
	for (nK = mink_in; nK <= maxk_in; nK++)
	{
		for (int nJ = minj_in; nJ <= maxj_in; nJ++)
		{
			for (int nI = mini_in; nI <= maxi_in; nI++)
			{				
				// The divergence correction is done also here (instead in div_fluence_calc)
				// Added by Nikos
				
				double new_distance = sqrt(
					(ssd_in + float(nK)*lengthz_in - 0.5)
					* (ssd_in + float(nK)*lengthz_in - 0.5)+
					(float(nJ) * lengthy_in) * (float(nJ) * lengthy_in)+
					(float(nI) * lengthx_in) * (float(nI) * lengthx_in));
				
				// depth_distance added for the CF kernel depth hardening effect
				double depth_distance = sqrt((float(nK)*lengthz_in-0.5)*(float(nK)*lengthz_in-0.5) +
					(float(nJ)*lengthy_in)*(float(nJ)*lengthy_in) +
					(float(nI)*lengthx_in)*(float(nJ)*lengthy_in));
				
				double c_factor = mvalue*depth_distance+bvalue;
				
				// convert to Gy cm**2 and take into account the
				// azimuthal sum
				energy_out2[nI+50][nJ+50][nK-1] = 
					(energy_out[nI+50][nJ+50][nK-1]*1.602e-10/numthet_in)
					* (ssd_in/new_distance) * (ssd_in/new_distance) 
					* c_factor;
				
				if (energy_out2[nI+50][nJ+50][nK-1] > max)
				{		
					// store position and value of dmax
					max = energy_out2[nI+50][nJ+50][nK-1];
					dmaxi = nI;                     
					dmaxj = nJ;               
					dmaxk = nK;                                                             
				}					
			}
		}
	}
}

/////////////////////////////////////////////
// input data


//////////////////////////////////////////////////////////////////////////////
// read_dose_spread
//
// looks up the energies for a phi angle setting 
//////////////////////////////////////////////////////////////////////////////
void read_dose_spread(FILE *pFile,

					  int *pNumPhi_out,
					  int *pNumRad_out,
					  double (&inc_energy_out)[48][49],	// (48,0:48)
					  double (&ang_out)[48],
					  double (&rad_bound_out)[49]			// (0:48)
					  )
{
	// The dose spread arrays produced by SUM_ELEMENT.FOR are read.
	
	char pszLine[100];
	sscanf("Hello!\n", "%[^\n]", pszLine);
	
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%i\n", pNumPhi_out);	// read (1,*) numphi       !number of angular divisions
	fscanf(pFile, "%i\n", pNumRad_out);	// read (1,*) numrad       !number of radial divisions
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	
	for (int nA = 1; nA <= (*pNumPhi_out); nA++)
	{
		for (int nR = 1; nR <= (*pNumRad_out); nR++)
		{
			fscanf(pFile, "%le", &inc_energy_out[nA-1][nR]);
			// read (1,*) (inc_energy(a,r),r=1,numrad) !read dose spread values 
		}
	}
	
	for (nA = 1; nA <= (*pNumPhi_out); nA++)
	{
		inc_energy_out[nA-1][0] = 0.0;
		
		for (int nR = 1; nR <= (*pNumRad_out); nR++)
		{
			// read dose spread values      
			inc_energy_out[nA-1][nR] += inc_energy_out[nA-1][nR-1];
		}
	}
	
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		// read (1,*)
	
	for (nA = 1; nA <= (*pNumPhi_out); nA++)
	{        
		fscanf(pFile, "%lf", &ang_out[nA-1]);      // read mean angle of spherical voxels 
	}
	
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		// read (1,*)
	
	rad_bound_out[0] = 0.0;
	
	for (int nR = 1; nR <= (*pNumRad_out); nR++)
	{              
		fscanf(pFile, "%lf", &rad_bound_out[nR]);   // read radial boundaries of spherical voxels
	}
	
}	// read_dose_spread


/////////////////////////////////////////////
// input parameters

double ssd = 90.0; 

double lengthx = 0.2; 
double lengthy = 1.0; 
double lengthz = 0.2; 

double ray = 6.0;

double xmin = -0.1; 
double xmax = 0.1; 
double ymin = -5.0; 
double ymax = 5.0;

double thickness = 20.0;

double mu = 0.0494;


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

		static double density[101][101][101];	// (-50:50,-50:50,1:101)

		// double accessory[901][901];		// (-450:450,-450:450)


		// output data
		static double fluence[101][101][101];	// (-50:50,-50:50,1:101)

		div_fluence_calc(
			ssd,	
			
			lengthx,	
			lengthy,	
			lengthz,	
			
			ray,
			
			xmin,
			xmax,
			ymin,
			ymax,
			
			thickness,	
			
			mu,

			density,
			
			fluence
			);
		
		int nNumPhi;
		int nNumRad;
		static double inc_energy[48][49];
		static double ang[48];
		static double rad_bound[49];

		FILE *pFile = fopen("lang48rad48.dat", "rt");
		read_dose_spread(pFile, &nNumPhi, &nNumRad, inc_energy, ang, rad_bound);
		fclose(pFile);

		static double energy_out[101][101][101];	// (-50:50,-50:50,1:101);
		static double energy_out2[101][101][101];	// (-50:50,-50:50,1:101);

		new_sphere_convolve(
			nNumPhi,
			8,		// const int numthet_in
			64,		// const int numstep_in
			nNumRad,
			
			xmin / lengthx,	// const int mini_in,
			xmax / lengthx,	// const int maxi_in,
			
			ymin / lengthy,	// const int minj_in,
			ymax / lengthy,	// const int maxj_in,
			
			1, 100,	// const int mink_in,
			// const int maxk_in,
			
			ssd,	
			
			lengthx,	
			lengthy,	
			lengthz,	
			
			thickness,	
			
			ang,
			rad_bound,
			inc_energy,
			
			fluence,
			density,
			
			energy_out,
			energy_out2
			);
	}
	
	return nRetCode;
}


