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
	CBeamDoseCalc(CBeam *pBeam, CEnergyDepKernel *pKernel); 
	virtual ~CBeamDoseCalc();

	// sets up the dose calc region
	void SetDoseCalcRegion(const CVectorD<3>& vMin, const CVectorD<3>& vMax);

	// triggers calculation of beam's pencil beams
	void CalcPencilBeams(CVolume<REAL> *pOrigDensity);

	// vMin, vMax in physical coords at isocentric plane
	void CalcTerma(const CVectorD<2>& vMin_in,
					const CVectorD<2>& vMax_in);

	// convolve
	void CalcSphereConvolve();

	// convolve helper
	void CalcSphereTrace(int nI, int nJ, int nK);

private:
	// my beam
	CBeam *m_pBeam;

	// reference to the source
	CEnergyDepKernel *m_pKernel;

	// how many beamlets?
	int m_nBeamletCount;

	/////////////////////////////////////
	// Mass Density Dist variables

	CPyramid * m_pMassDensityPyr;
	CVolume<REAL> m_densityRep;

	CVolume<REAL> m_kernel;
	CVolume<REAL> m_densityFilt;


public:
	/////////////////////////////////////
	// TERMA calc variables

	// vSource -- source position, in voxel coordinates
	CVectorD<3> m_vSource_vxl;
	CVectorD<3> m_vIsocenter_vxl;

private:
	// returns computed terma
	CVolume<REAL> *m_pTerma;

	// minimum number of rays to use per voxel (on top boundary)
	REAL m_raysPerVoxel;

	//////////////////////////////////////
	// SphereConvolve variables

	// calc region
	CVectorD<3> m_vDoseCalcRegionMin;
	CVectorD<3> m_vDoseCalcRegionMax;

	// computed energy
	CVolume<REAL> *m_pEnergy;

};	// class CBeamDoseCalc


#endif // !defined(AFX_BEAM_H__C6E4B021_B2A3_46C7_B9DE_32949313F009__INCLUDED_)
