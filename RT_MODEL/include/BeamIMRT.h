// BeamIMRT.h: interface for the CBeamIMRT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAMIMRT_H__0C6C936D_780E_4D0D_ACFD_87AE7AAEA4FF__INCLUDED_)
#define AFX_BEAMIMRT_H__0C6C936D_780E_4D0D_ACFD_87AE7AAEA4FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorN.h>
#include <MatrixNxM.h>

#include <ModelObject.h>
#include <Volumep.h>

#include <Beam.h>

class CBeamIMRT : public CBeam
{
public:
	CBeamIMRT();
	virtual ~CBeamIMRT();

	// over-ride to gen beamlets
	void SetGantryAngle(double gantryAngle);

	int GetBeamletCount(int nScale = 0);
	CVolume<double> *GetBeamlet(int nShift, int nScale = 0);

	const CVectorN<>& GetIntensityMap(int nScale = 0) const;
	void SetIntensityMap(int nScale, const CVectorN<>& vWeights);

	void InvFiltIntensityMap(int nScale, const CVectorN<>& vWeights, 
		CVectorBase<>& vFiltWeights);

	// over-ride to accumulate beamlets
	CVolume<double> *GetDoseMatrix(int nScale = 0);

protected:
	void GenBeamlets();
	void GenFilterMat();

	friend class CPlan;

	mutable CVectorN<> m_arrBeamletWeights[MAX_SCALES];
	mutable BOOL m_arrRecalcWeights[MAX_SCALES];

	CMatrixNxM<> m_mFilter[MAX_SCALES - 1];

private:
	CTypedPtrArray<CPtrArray, CVolume<double>* > m_arrBeamlets[MAX_SCALES];

	CVolume<double> m_filtGauss5x5;

	CVectorN<> m_vWeightFilter;
};

#endif // !defined(AFX_BEAMIMRT_H__0C6C936D_780E_4D0D_ACFD_87AE7AAEA4FF__INCLUDED_)
