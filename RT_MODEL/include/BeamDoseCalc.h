// Beam.h: interface for the CBeamDoseCalc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
#define AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

#include <Pyramid.h>

#include <Volumep.h>

class CEnergyDepKernel;
class CBeam;

class CBeamDoseCalc  
{
public:
	// constructor / destructor
	CBeamDoseCalc(CBeam *pBeam, CEnergyDepKernel *pKernel); // , double ssd);
	virtual ~CBeamDoseCalc();

	// accessor for density
	// void SetDensity(CVolume<double> *pDensity, double mu);

	// collimated field size
//	REAL xmin, xmax;
//	REAL ymin, ymax;

	void SetDoseCalcRegion(const CVectorD<3>& vMin, const CVectorD<3>& vMax);

	CPyramid * m_pMassDensityPyr;
	CVolume<REAL> m_densityRep;

	CVolume<REAL> m_kernel;
	CVolume<REAL> m_densityFilt;

	// triggers calculation of beam's pencil beams
	void CalcPencilBeams(CVolume<REAL> *pOrigDensity);

	// accessor for fluence 
	// CVolume<REAL> *GetFluence();

	// fluence calc
	void DivFluenceCalc(
		const CVectorD<2>& vMin,
		const CVectorD<2>& vMax,
		const REAL rays_per_voxel_in,
		const REAL thickness_in,
		CVolume<REAL> *pFluence_out
		);

	// aceessor for energy
	// CVolume<REAL> *GetEnergy();

	// convolve
	void SphereConvolve(CVolume<REAL> *pFluence_in, 
		const REAL thickness_in,
		CVolume<REAL> *pEnergy_out);

	void SphereTrace(CVolume<REAL> *pFluence_in, 
		const REAL thickness_in, int nI, int nJ, int nK,
		CVolume<REAL> *pEnergy_out);

private:
	// my beam
	CBeam *m_pBeam;

	// reference to the source
	CEnergyDepKernel *m_pKernel;
	REAL m_ssd;

	// reference / interpretation of density
	// CVolume<REAL> * m_pDensity;

	// calc region
	CVectorD<3> m_vDoseCalcRegionMin;
	CVectorD<3> m_vDoseCalcRegionMax;

	// reference to primary fluence matrix
	// CVolume<REAL> *m_pFluence;

	// reference to energy matrix
	// CVolume<REAL> *m_pEnergy;
};

#endif // !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
