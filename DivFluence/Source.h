// Source.h: interface for the CSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
#define AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixD.h>

const int NUM_THETA = 8;


class CEnergyDepKernel  
{
public:
	CEnergyDepKernel();
	virtual ~CEnergyDepKernel();

	void ReadDoseSpread(const char *pszFileName);

	void SetBasis(const CMatrixD<4>& mBasis);

	int GetNumPhi();

	void GetDelta(int theta, int phi, int rad_inc, CVectorD<3, int>& vDelta_out);
	double GetRadius(int nTheta, int nPhi, int nRadInc);
	double GetCumEnergy(int nPhi, double rad_dist);

// protected:
	void EnergyLookup();

	double InterpEnergy(
		const double bound_1,				// inner boundary
		const double bound_2,				// outer boundary
		const int rad_numb,					// radial label of inner voxel
		const int phi_numb					// angular label of voxel
		);

	void MakeVector(
		const int numstep_in,         // number of voxels passed thru
		const double factor1_in,      // principle direction cosine
		const double factor2_in,      // secondary
		const double factor3_in,      //    direction cosines
		
		double (&r_out)[64],    // list of lengths thru a voxel
		int (&delta1_out)[64],  // location in principle direction
		int (&delta2_out)[64],  // locations in
		int (&delta3_out)[64]   //    secondary directions
		);

private:
	int m_numphi;
	int m_numrad;

	double m_ang[48];				// zenith angle list           		// (48)
	double m_rad_bound[49];		// (0:48)
	double m_inc_energy[48][49];	// (48,0:48)

	double m_cum_energy[48][601];	// (48,0:600)

	bool m_bSetupRaytrace;

	double m_radius[65][48][NUM_THETA];	// (0:64,48,48); 
	
public:
	int m_delta_i[64][48][NUM_THETA];	// (64,48,48);
	int m_delta_j[64][48][NUM_THETA];	// (64,48,48);
	int m_delta_k[64][48][NUM_THETA];	// (64,48,48);

};	// class CEnergyDepKernel



inline int CEnergyDepKernel::GetNumPhi()
{
	return m_numphi;
}

inline void CEnergyDepKernel::GetDelta(int nTheta, int nPhi, int nRadInc, CVectorD<3, int> &vDelta_out)
{
	vDelta_out[0] =	m_delta_i[nRadInc-1][nPhi-1][nTheta-1];    
	vDelta_out[1] = m_delta_j[nRadInc-1][nPhi-1][nTheta-1]; 
	vDelta_out[2] = m_delta_k[nRadInc-1][nPhi-1][nTheta-1];    
}

inline double CEnergyDepKernel::GetRadius(int nTheta, int nPhi, int nRadInc)
{
	return m_radius[nRadInc][nPhi-1][nTheta-1];
}

inline double CEnergyDepKernel::GetCumEnergy(int nPhi, double rad_dist)
{
	return m_cum_energy[nPhi-1][(int) floor(rad_dist*10.0 + 0.5)];
}

#endif // !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
