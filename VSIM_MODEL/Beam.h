// Beam.h: interface for the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>

#include <Vector.h>
#include <Matrix.h>

#include <Polygon.h>
#include <Volumep.h>

#include "TreatmentMachine.h"

class CBeam : public CModelObject
{
public:
	// constructur/destructor
	CBeam();
	virtual ~CBeam();

	// serialization support
	DECLARE_SERIAL(CBeam)

	// returns a pointer to the treatment machine
	CTreatmentMachine *GetTreatmentMachine();

	// angle values
	double GetCollimAngle() const;
	void SetCollimAngle(double collimAngle);
	double GetGantryAngle() const;
	void SetGantryAngle(double gantryAngle);
	double GetCouchAngle() const;
	void SetCouchAngle(double couchAngle);

	// table offset
	const CVector<3>& GetTableOffset() const;
	void SetTableOffset(const CVector<3>& vTableOffset);

	// collimator jaw settings
	const CVector<2>& GetCollimMin() const;
	void SetCollimMin(const CVector<2>& vCollimMin);

	const CVector<2>& GetCollimMax() const;
	void SetCollimMax(const CVector<2>& vCollimMin);

	// computed transform from patient to beam coordinates
	const CMatrix<4>& GetBeamToPatientXform() const;
	void SetBeamToPatientXform(const CMatrix<4>& m);

	// boolean value to indicate that shielding blocks are used
	//		by this beam
	BOOL GetHasShieldingBlocks() const;

	// collection of blocks
	int GetBlockCount() const;
	CPolygon *GetBlockAt(int nAt);
	int AddBlock(CPolygon *pBlock);

	// the weight for this beam (from 0.0 to 1.0)
	double GetWeight() const;
	void SetWeight(double weight);

	// flag to indicate whether the plan's dose is valid
	BOOL IsDoseValid() const;
	void SetDoseValid(BOOL bValid = TRUE);

	// the computed dose for this beam (NULL if no dose exists)
	CVolume<double> *GetDoseMatrix();

	// beam serialization
	void Serialize(CArchive &ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// private copy of a treatment machine; default machine for this beam
	CTreatmentMachine m_Machine;

	// angles
	double m_collimAngle;
	double m_gantryAngle;
	double m_couchAngle;

	// table parameters
	CVector<3> m_vTableOffset;

	// collimator jaw settings
	CVector<2> m_vCollimMin;
	CVector<2> m_vCollimMax;

	// stores the current xform matrix; mutable because it is recomputed
	//		in the Get accessor
	mutable CMatrix<4> m_beamToPatientXform;

	// boolean value to indicate that shielding blocks are used
	//		by this beam
	BOOL m_bHasShieldingBlocks;

	// collection of blocks
	CObArray m_arrBlocks;	// CPolygon

	// the weight for this beam (from 0.0 to 1.0)
	double m_weight;

	// flag to indicate whether the plan's dose is valid
	BOOL m_bDoseValid;

	// the computed dose for this beam (NULL if no dose exists)
	CVolume<double> m_dose;
};

#endif // !defined(AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
