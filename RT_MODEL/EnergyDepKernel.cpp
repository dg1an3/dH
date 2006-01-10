// Source.cpp: implementation of the CSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnergyDepKernel.h"

#include <MathUtil.h>

#include <Volumep.h>

#include <direct.h>

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

CEnergyDepKernel::CEnergyDepKernel(REAL energy)
	: m_energy(energy)
{
	Init();
}

CEnergyDepKernel::~CEnergyDepKernel()
{

}


void CEnergyDepKernel::Init()
{
	CString strFilename;

	if (IsApproxEqual(m_energy, 15.0))
	{
		strFilename = "C:\\15MV_kernel.dat";
		m_mu = 1.941E-02;
	}
	else if (IsApproxEqual(m_energy, 6.0))
	{
		strFilename = "C:\\6MV_kernel.dat";
		m_mu = 2.770E-02;
	}
	else if (IsApproxEqual(m_energy, 2.0))
	{
		strFilename = "C:\\2MV_kernel.dat";
		m_mu = 4.942E-02;

	}
	else
	{
		ASSERT(FALSE);
	}

	// The dose spread arrays produced by SUM_ELEMENT.FOR are read.
	FILE *pFile = fopen(strFilename, "rt");
	
	char pszLine[100];
	

	fscanf(pFile, "%[^\n]\n", pszLine);	
	fscanf(pFile, "%[^\n]\n", pszLine);	

	int nNumPhiIn;
	int nNumRadIn;

	fscanf(pFile, "%i\n", &nNumPhiIn);	
	fscanf(pFile, "%i\n", &nNumRadIn);	

	fscanf(pFile, "%[^\n]\n", pszLine);	
	fscanf(pFile, "%[^\n]\n", pszLine);	
	
	
	// set up increment energy array
	CMatrixNxM<REAL> mIncEnergyIn;
	mIncEnergyIn.Reshape(nNumPhiIn+1, nNumRadIn+1);
	for (int nA = 1; nA <= nNumPhiIn; nA++)
	{
		mIncEnergyIn[nA-1][0] = 0.0;
		for (int nR = 1; nR <= nNumRadIn; nR++)
		{
			// read dose spread values      
			fscanf(pFile, "%lf", &mIncEnergyIn[nA-1][nR]);
			mIncEnergyIn[nA-1][nR] += mIncEnergyIn[nA-1][nR-1];
		}
	}  

	// read (1,*)
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		
	
	// set up angle vector
	m_vAnglesIn.SetDim(nNumPhiIn+1);
	for (nA = 1; nA <= nNumPhiIn; nA++)
	{        
		// read mean angle of spherical voxels 
		fscanf(pFile, "%lf", &m_vAnglesIn[nA-1]);      
	} 
	
	// set up radial bounds vector
	// read (1,*)
	fscanf(pFile, "%*s%[^\n]\n", pszLine);		
	
	CVectorN<REAL> vRadialBoundsIn;
	vRadialBoundsIn.SetDim(nNumRadIn+1);
	vRadialBoundsIn[0] = 0.0;
	for (int nR = 1; nR <= nNumRadIn; nR++)
	{              
		// read radial boundaries of spherical voxels
		fscanf(pFile, "%lf", &vRadialBoundsIn[nR]);   
	} 

	fclose(pFile);

	// now interpolate values to mm resolution
	InterpCumEnergy(mIncEnergyIn, vRadialBoundsIn);
}


//////////////////////////////////////////////////////////////////////////////
// CEnergyDepKernel::InterpCumEnergy
//
// looks up the energies for a phi angle setting 
//////////////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::InterpCumEnergy(const CMatrixNxM<>& mIncEnergy, 
									   const CVectorN<>& vRadialBounds)
{
	// TODO: fix hard-coding for 60.0 cm
	m_mCumEnergy.Reshape(mIncEnergy.GetCols(), 600);

	// these should be read in the table
	for (int nPhi = 1; nPhi <= m_mCumEnergy.GetCols()-1; nPhi++)
	{
		m_mCumEnergy[nPhi-1][0] = 0.0;
		
		int nRadial = 1;
		for (int nI = 1; nI < m_mCumEnergy.GetRows(); nI++)
		{
			// distance in cm
			double radialDist = 0.1 * nI;
			
			while (nRadial < vRadialBounds.GetDim()
				&& vRadialBounds[nRadial] < radialDist)
			{
				nRadial++;
			}

			if (nRadial < vRadialBounds.GetDim())
			{
				// energy at lower boundary
				double incEnergy = mIncEnergy[nPhi-1][nRadial-1];

				// increase in energy between lower and upper
				double incEnergyDelta = mIncEnergy[nPhi-1][nRadial] - incEnergy;

				// linear interpolate using lookup table values
				incEnergy += incEnergyDelta 
					* (radialDist - vRadialBounds[nRadial-1]) 
						/ (vRadialBounds[nRadial] - vRadialBounds[nRadial-1]);

				m_mCumEnergy[nPhi-1][nI] = incEnergy;
			}
			else
			{
				// if over end of radial bounds, just fill out with same value
				m_mCumEnergy[nPhi-1][nI] = m_mCumEnergy[nPhi-1][nI-1];
			}
		}
	}
	
}	// CEnergyDepKernel::InterpCumEnergy





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
		
		if (fabs(factor1_in) >= 1e-04) 
		{	
			// radius to intersection point
			r_out[nI-1] = fabs(d / factor1_in);
		}      
		else
		{
			// effectively infinity
			r_out[nI-1] = 100000.0;                 
		}
		
		// Calculate a distance along a coordinate direction and find the nearest
		// integer to specify the voxel direction.
        
		delta1_out[nI-1] = nint(0.99 * r_out[nI-1] * factor1_in);  // 0.99 prevents being exactly on the voxel boundary
		delta2_out[nI-1] = nint(r_out[nI-1] * factor2_in);       
		delta3_out[nI-1] = nint(r_out[nI-1] * factor3_in);       
	}

}	// CEnergyDepKernel::MakeVector



//////////////////////////////////////////////////////////////////////////////
// CEnergyDepKernel::SetBasis
//
// sets up the ray trace for conv.
//////////////////////////////////////////////////////////////////////////////
void CEnergyDepKernel::SetBasis(const CVectorD<3>& vPixSpacing)
{
	// check if we are already set up
	if (m_vPixSpacing.IsApproxEqual(vPixSpacing))
	{
		return;
	}
	
	// the azimuthal angle increment is calculated
	double ang_inc = 2.0 * PI 
		/ double(NUM_THETA); 
	
	// loop thru all zenith angles
	for (int nPhi = 1; nPhi <= m_vAnglesIn.GetDim()-1; nPhi++)              
	{
		// trig. for zenith angles
		double sphi = sin(m_vAnglesIn[nPhi]);
		double cphi = cos(m_vAnglesIn[nPhi]);
		
//		CMatrixD<3> mPhiRot = CreateRotate(CreateRotate(m_angles[nPhi], 
//			CVectorD<3>(0.0, 1.0, 0.0);

		// loop thru all azimuthal angles
		for (int nTheta = 1; nTheta <= NUM_THETA; nTheta++)                  
		{
//			CMatrixD<3> mThetaRot = CreateRotate(CreateRotate(m_angles[nPhi], 
//				CVectorD<3>(1.0, 0.0, 0.0);
//
//			CVectorD<3> vDir = mThetaRot * mPhiRot 
//				* CVectorD<3>(1.0, 0.0, 0.0):

//			vDir[0] /= vPixSpacing[0];
//			vDir[1] /= vPixSpacing[1];
//			vDir[2] /= vPixSpacing[2];

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
			
			MakeVector(NUM_RADIAL_STEPS,              
				// vDir[1], vDir[2], vDir[0],
				sphi*cthet / vPixSpacing[1],   // x-dir direction cosine : units voxels / cm
				sphi*sthet / vPixSpacing[2],   // y-dir direction cosine 
				cphi       / vPixSpacing[0],   // z-dir direction cosine    
				rx,                   // list of dist. to x-bound  
				delta_i_x,            // x-dir voxel location      
				delta_j_x,            // y-dir voxel location      
				delta_k_x);           // z-dir voxel location      
			
			// Call for the x-z plane crossing list. Plane defined by the y-coordinate.
			
			double ry[64];
			int delta_i_y[64];	// (64)
			int delta_j_y[64];	// (64)
			int delta_k_y[64];	// (64)
			
			MakeVector(NUM_RADIAL_STEPS,     
				// vDir[2], vDir[1], vDir[0],
				sphi*sthet / vPixSpacing[2],   // y-dir direction cosine    
				sphi*cthet / vPixSpacing[1],   // x-dir direction cosine    
				cphi       / vPixSpacing[0],   // z-dir direction cosine    
				ry,                   // list of dist. to y-bound  
				delta_j_y,            // y-dir voxel location      
				delta_i_y,            // x-dir voxel location      
				delta_k_y);           // z-dir voxel location      
			
			// Call for the x-y plane crossing list. Plane defined by the z-coordinate.
			
			double rz[64];
			int delta_i_z[64];	// (64)      
			int delta_j_z[64];	// (64)
			int delta_k_z[64];	// (64)
			MakeVector(NUM_RADIAL_STEPS,     
				// vDir[0], vDir[2], vDir[1],
				cphi       / vPixSpacing[0],   // z-dir direction cosine    
				sphi*sthet / vPixSpacing[2],   // y-dir direction cosine    
				sphi*cthet / vPixSpacing[1],   // x-dir direction cosine    
				rz,                   // list of dist. to z-bound  
				delta_k_z,            // z-dir voxel location      
				delta_j_z,            // y-dir voxel location      
				delta_i_z);           // x-dir voxel location      
			
			int nI = 1;
			int nJ = 1;
			int nK = 1;
			
			m_radius[nTheta-1][nPhi-1][0] = 0.0;             // radius at origin is 0
			double last_radius = 0.0;
			
			// The following sorts through the distance vectors, rx,ry,rz
			// to find the next smallest value. This will be the next plane crossed.
			// A merged vector is created that lists the location of the voxel crossed
			// and the length thru it in the order of crossings.
			
			for (int nN = 1; nN <= NUM_RADIAL_STEPS; nN++)
			{
				// done if plane defined by the x-coord crossed
				if (rx[nI-1] <= ry[nJ-1] && rx[nI-1] <= rz[nK-1])
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = rx[nI-1] - last_radius;	

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nTheta-1][nPhi-1][nN-1] = delta_i_x[nI-1];	
					m_delta_j[nTheta-1][nPhi-1][nN-1] = delta_j_x[nI-1];
					m_delta_k[nTheta-1][nPhi-1][nN-1] = delta_k_x[nI-1];

					// radius gone at this point
					last_radius = rx[nI-1];	
					
					// decrement x-dir counter
					nI = nI+1;											
				}
				// done if plane defined by the y-coord crossed
				else if (ry[nJ-1] <= rx[nI-1] && ry[nJ-1] <= rz[nK-1])    
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = ry[nJ-1]-last_radius;		

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nTheta-1][nPhi-1][nN-1] = delta_i_y[nJ-1];	
					m_delta_j[nTheta-1][nPhi-1][nN-1] = delta_j_y[nJ-1];	
					m_delta_k[nTheta-1][nPhi-1][nN-1] = delta_k_y[nJ-1];	

					// radius gone at this point
					last_radius = ry[nJ-1];								

					// decrement y-dir counter					
					nJ = nJ+1;											
				}
				// done if plane defined by the z-coord crossed
				else if (rz[nK-1] <= rx[nI-1] && rz[nK-1] <= ry[nJ-1])     
				{
					// length thru voxel
					m_radius[nTheta-1][nPhi-1][nN] = rz[nK-1]-last_radius;		

					// location of voxel in x-dir / y-dir / z-dir
					m_delta_i[nTheta-1][nPhi-1][nN-1] = delta_i_z[nK-1];	
					m_delta_j[nTheta-1][nPhi-1][nN-1] = delta_j_z[nK-1];	
					m_delta_k[nTheta-1][nPhi-1][nN-1] = delta_k_z[nK-1];	

					// radius gone at this point
					last_radius = rz[nK-1];								

					// decrement z-dir counter
					nK = nK+1;											
				}
			}
		}
	}

	// store the pixel spacing used
	m_vPixSpacing = vPixSpacing;

}	// CEnergyDepKernel::SetBasis
