// Source.h: interface for the CSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
#define AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixD.h>
#include <VectorN.h>
#include <MatrixNxM.h>


const int NUM_THETA = 8;
const int NUM_RADIAL_STEPS = 64;


class CEnergyDepKernel  
{
public:
	CEnergyDepKernel(REAL energy);
	virtual ~CEnergyDepKernel();

	double Get_mu() { return m_mu; }

	// sets the basis for the corresponding dose matrix -- pix spacing in cm
	void SetBasis(const CVectorD<3>& vPixSpacing);

	int GetNumPhi();
	int GetNumTheta() { return NUM_THETA; }
	int GetNumRadii() { return NUM_RADIAL_STEPS; }

	void GetDelta(int nTheta, int nPhi, int nRadial, int& nX, int& nY, int& nZ);
	double GetRadius(int nTheta, int nPhi, int nRadInc);
	double GetCumEnergy(int nPhi, double rad_dist);

 protected:
	// reads the appropriate EDK
	void Init();

	// sets up cumulative energy LUT
	void InterpCumEnergy(const CMatrixNxM<>& mIncEnergy, 
					   const CVectorN<>& vRadialBounds);

	// constructs spherical LUT for a single dimension
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
	// energy for this kernel
	double m_energy;

	// attenuation coeffecient (H2O) for this energy
	double m_mu;

	// mean angle values from kernel
	CVectorN<REAL> m_vAnglesIn;

	// interpolated energy lookup table
	CMatrixNxM<REAL> m_mCumEnergy;

	// spherical xform lookup table
	CVectorD<3> m_vPixSpacing;

	double m_radius[NUM_THETA][48][65];	// (0:64,48,48); 
	
	int m_delta_i[NUM_THETA][48][64];	// (64,48,48);
	int m_delta_j[NUM_THETA][48][64];	// (64,48,48);
	int m_delta_k[NUM_THETA][48][64];	// (64,48,48);

};	// class CEnergyDepKernel



inline int CEnergyDepKernel::GetNumPhi()
{
	return m_mCumEnergy.GetCols();
}

inline void CEnergyDepKernel::GetDelta(int nTheta, int nPhi, int nRadial, int& nX, int& nY, int& nZ)
{
	nX -= m_delta_k[nTheta-1][nPhi-1][nRadial-1];
	nY -= m_delta_i[nTheta-1][nPhi-1][nRadial-1]; 
	nZ -= m_delta_j[nTheta-1][nPhi-1][nRadial-1];   
}

// returns radius in cm
inline double CEnergyDepKernel::GetRadius(int nTheta, int nPhi, int nRadial)
{
	return m_radius[nTheta-1][nPhi-1][nRadial];
}

inline double CEnergyDepKernel::GetCumEnergy(int nPhi, double rad_dist /* cm */)
{
	// The resolution of the array in the radial direction is every mm
	//	hence the multiply by 10.0 in the arguement.
	int nRadial = (int) floor(rad_dist * 10.0 + 0.5);

	// check for overflow
	nRadial = __min(nRadial, m_mCumEnergy.GetRows()-1);

	return m_mCumEnergy[nPhi-1][nRadial];
}

#endif // !defined(AFX_SOURCE_H__A104E4D2_7288_4520_9F88_3EFAFFAD46CB__INCLUDED_)
