// Beam.h: interface for the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
#define AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

#include <Volumep.h>

class CSource;

class CBeam  
{
public:
	// constructor / destructor
	CBeam(CSource *pSource, double ssd);
	virtual ~CBeam();

	// accessor for density
	void SetDensity(CVolume<double> *pDensity, CVectorD<3> vBasis, double mu);

	// collimated field size
	double xmin, xmax;
	double ymin, ymax;

	// fluence calc
	void DivFluenceCalc(
		const double rays_per_voxel_in,
		const double thickness_in
		);

	// accessor for fluence 
	CVolume<double> *GetFluence();

	// convolve
	void SphereConvolve(const double thickness_in);

	// aceessor for energy
	CVolume<double> *GetEnergy();

protected:
	void RayTraceSetup(
		double (&r_out)[65][48][48],		// list of lengths thru voxels	// (0:64,48,48)
		int (&delta_i_out)[64][48][48],	// list of voxel # in x-dir
		int (&delta_j_out)[64][48][48],	// list of voxel # in y-dir		// (64,48,48)
		int (&delta_k_out)[64][48][48]	// list of voxel # in z-dir		// (64,48,48)				  
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

	// reference to the source
	CSource *m_pSource;
	double m_ssd;

	// reference / interpretation of density
	CVolume<double> * m_pDensity;
	double m_mu;
	CVectorD<3> m_vBasis;

	// reference to primary fluence matrix
	CVolume<double> *m_pFluence;

	// reference to energy matrix
	CVolume<double> *m_pEnergy;
};

#endif // !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
