// Source.cpp: implementation of the CSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DivFluence.h"
#include "Source.h"

#include <MathUtil.h>


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

CEnergyDepKernel::CEnergyDepKernel()
: m_bSetupRaytrace(true)
{
	memset(m_ang, 0, sizeof(m_ang));
	memset(m_rad_bound, 0, sizeof(m_rad_bound));
	memset(m_inc_energy, 0, sizeof(m_inc_energy));
	memset(m_cum_energy, 0, sizeof(m_cum_energy));
}

CEnergyDepKernel::~CEnergyDepKernel()
{

}



void CEnergyDepKernel::ReadDoseSpread(const char *pszFileName)
{
	// The dose spread arrays produced by SUM_ELEMENT.FOR are read.
	FILE *pFile = fopen(pszFileName, "rt");
	
	char pszLine[100];
	
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%i\n", &m_numphi);	// read (1,*) numphi       !number of angular divisions
	fscanf(pFile, "%i\n", &m_numrad);	// read (1,*) numrad       !number of radial divisions
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	fscanf(pFile, "%[^\n]\n", pszLine);		// read (1,*)
	
	for (int nA = 1; nA <= m_numphi; nA++)
	{
		for (int nR = 1; nR <= m_numrad; nR++)
		{
			fscanf(pFile, "%le", &m_inc_energy[nA-1][nR]);
			// read (1,*) (inc_energy(a,r),r=1,numrad) !read dose spread values 
		}
	}
	
	for (nA = 1; nA <= m_numphi; nA++)
	{
		m_inc_energy[nA-1][0] = 0.0;
		
		for (int nR = 1; nR <= m_numrad; nR++)
		{
			// read dose spread values      
			m_inc_energy[nA-1][nR] += m_inc_energy[nA-1][nR-1];
		}
	}
	
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		// read (1,*)
	
	for (nA = 1; nA <= m_numphi; nA++)
	{        
		fscanf(pFile, "%lf", &m_ang[nA-1]);      // read mean angle of spherical voxels 
	}
	
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		// read (1,*)
	
	m_rad_bound[0] = 0.0;
	
	for (int nR = 1; nR <= m_numrad; nR++)
	{              
		fscanf(pFile, "%lf", &m_rad_bound[nR]);   // read radial boundaries of spherical voxels
	}	

	fclose(pFile);

	EnergyLookup();
}


//////////////////////////////////////////////////////////////////////////////
// energy_lookup
//
// looks up the energies for a phi angle setting 
//////////////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::EnergyLookup()
{
	// these should be read in the table
	for (int phi_in = 1; phi_in <= m_numphi; phi_in++)
	{
		int rad_numb = 1;
		double last_rad = 0.0;
		double rad_dist = 0.0;
		m_cum_energy[phi_in-1][0] = 0.0;
		
		for (int nI = 1; nI <= 599; nI++)
		{
			rad_dist = 0.1 * nI;
			
			for (int r = rad_numb-1; r <= m_numrad; r++)
			{
				if (m_rad_bound[r] > rad_dist) 
				{
					break;
				}
			}                      
			
			rad_numb = r;
			
			// New subroutine interpolates the kernel values to get the 
			// energy absorbed.
			
			double tot_energy = 
				InterpEnergy(
					last_rad,		// inner boundary
					rad_dist,       // outer boundary
					rad_numb,       // radial label of inner voxel
					phi_in			// angular label of voxel
				);
			
			last_rad = rad_dist;
			
			m_cum_energy[phi_in-1][nI] = tot_energy;
		}
	}
	
}	// energy_lookup



//////////////////////////////////////////////////////////////////////////////
// interp_energy
//
// returns energy between boundaries  
//////////////////////////////////////////////////////////////////////////////
double CEnergyDepKernel::InterpEnergy(
					 const double bound_1,		// inner boundary
					 const double bound_2,		// outer boundary
					 const int rad_numb,		// radial label of inner voxel
					 const int phi_numb			// angular label of voxel
					 )        
{
	double inc_energy1 = m_inc_energy[phi_numb-1][rad_numb-1];
	double inc_energy2 = m_inc_energy[phi_numb-1][rad_numb];

	double energy = inc_energy1
		+ (inc_energy2 - inc_energy1) * (bound_2 - m_rad_bound[rad_numb-1]) 
		/ (m_rad_bound[rad_numb] - m_rad_bound[rad_numb-1]);

	return energy;
	
}	// interp_energy


//////////////////////////////////////////////////////////////////////////////
// CBeam::MakeVector
//
// makes a vector from a set of directions
//////////////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::MakeVector(
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

}	// CEnergyDepKernel::MakeVector



//////////////////////////////////////////////////////////////////////////////
// CEnergyDepKernel::SetBasis
//
// sets up the ray trace for conv.
//////////////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::SetBasis(const CMatrixD<4>& mBasis)
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
	for (int nPhi = 1; nPhi <= m_numphi; nPhi++)              
	{
		// trig. for zenith angles
		double sphi = sin(m_ang[nPhi]);
		double cphi = cos(m_ang[nPhi]);
		
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
				sphi*cthet / mBasis[1][1],   // x-dir direction cosine
				sphi*sthet / mBasis[2][2],   // y-dir direction cosine 
				cphi       / mBasis[0][0],   // z-dir direction cosine    
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
				sphi*sthet / mBasis[2][2],   // y-dir direction cosine    
				sphi*cthet / mBasis[1][1],   // x-dir direction cosine    
				cphi       / mBasis[0][0],   // z-dir direction cosine    
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
				cphi       / mBasis[0][0],   // z-dir direction cosine    
				sphi*sthet / mBasis[2][2],   // y-dir direction cosine    
				sphi*cthet / mBasis[1][1],   // x-dir direction cosine    
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

}	// CEnergyDepKernel::SetBasis
