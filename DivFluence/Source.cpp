// Source.cpp: implementation of the CSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DivFluence.h"
#include "Source.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSource::CSource()
{
	memset(m_ang, 0, sizeof(m_ang));
	memset(m_rad_bound, 0, sizeof(m_rad_bound));
	memset(m_inc_energy, 0, sizeof(m_inc_energy));
	memset(m_cum_energy, 0, sizeof(m_cum_energy));
}

CSource::~CSource()
{

}



void CSource::ReadDoseSpread(const char *pszFileName)
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
}


//////////////////////////////////////////////////////////////////////////////
// energy_lookup
//
// looks up the energies for a phi angle setting 
//////////////////////////////////////////////////////////////////////////////
void CSource::EnergyLookup()
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
double CSource::InterpEnergy(
					 const double bound_1,		// inner boundary
					 const double bound_2,		// outer boundary
					 const int rad_numb,		// radial label of inner voxel
					 const int phi_numb			// angular label of voxel
					 )        
{
	const double (&radius)[49] = m_rad_bound;	// radius of voxel boundaries	(0:48)

	double inc_energy1 = m_inc_energy[phi_numb-1][rad_numb-1];
	double inc_energy2 = m_inc_energy[phi_numb-1][rad_numb];

	double energy = inc_energy1
		+ (inc_energy2 - inc_energy1) * (bound_2 - radius[rad_numb-1]) 
		/ (radius[rad_numb] - radius[rad_numb-1]);

	return energy;
	
}	// interp_energy



