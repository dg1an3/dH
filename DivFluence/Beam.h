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

class CEnergyDepKernel;

class CBeam  
{
public:
	// constructor / destructor
	CBeam(CEnergyDepKernel *pSource, double ssd);
	virtual ~CBeam();

	// accessor for density
	void SetDensity(CVolume<double> *pDensity, double mu);

	// collimated field size
	double xmin, xmax;
	double ymin, ymax;

	void SetDoseCalcRegion(const CVectorD<3>& vMin, const CVectorD<3>& vMax);

	// accessor for fluence 
	CVolume<double> *GetFluence();

	// fluence calc
	void DivFluenceCalc(
		const double rays_per_voxel_in,
		const double thickness_in
		);

	// aceessor for energy
	CVolume<double> *GetEnergy();

	// convolve
	void SphereConvolve(const double thickness_in);
	void SphereTrace(const double thickness_in, int nI, int nJ, int nK);

private:

	// reference to the source
	CEnergyDepKernel *m_pSource;
	double m_ssd;

	// reference / interpretation of density
	CVolume<double> * m_pDensity;
	double m_mu;

	// calc region
	CVectorD<3> m_vDoseCalcRegionMin;
	CVectorD<3> m_vDoseCalcRegionMax;

	// reference to primary fluence matrix
	CVolume<double> *m_pFluence;

	// reference to energy matrix
	CVolume<double> *m_pEnergy;
};

#endif // !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
