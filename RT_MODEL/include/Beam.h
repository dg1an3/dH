//////////////////////////////////////////////////////////////////////
// Beam.h: declaration of the CBeam class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(BEAM_H)
#define BEAM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
using namespace std;

// vector/matrix includes
#include <VectorD.h>
#include <MatrixD.h>

// model object base class
#include <ModelObject.h>

// geom model classes
#include <Polygon.h>

#include <Volumep.h>

// treatment machine upon which beam is based
#include "TreatmentMachine.h"


//////////////////////////////////////////////////////////////////////
// class CBeam
//
// represents a single treatment beam
//////////////////////////////////////////////////////////////////////
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
	const CVectorD<3>& GetTableOffset() const;
	void SetTableOffset(const CVectorD<3>& vTableOffset);

	// collimator jaw settings
	const CVectorD<2>& GetCollimMin() const;
	void SetCollimMin(const CVectorD<2>& vCollimMin);

	const CVectorD<2>& GetCollimMax() const;
	void SetCollimMax(const CVectorD<2>& vCollimMin);

	// returns the beam-to-IEC fixed (room) xform
	const CMatrixD<4>& GetBeamToFixedXform() const;

	// computed transform from patient to beam coordinates
	const CMatrixD<4>& GetBeamToPatientXform() const;
	void SetBeamToPatientXform(const CMatrixD<4>& m);

	// boolean value to indicate that shielding blocks are used
	//		by this beam
	BOOL GetHasShieldingBlocks() const;

	// collection of blocks
	int GetBlockCount() const;
	CPolygon *GetBlock(int nAt);
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
	CVectorD<3> m_vTableOffset;

	// collimator jaw settings
	CVectorD<2> m_vCollimMin;
	CVectorD<2> m_vCollimMax;

	// stores the current IEC fixed xform matrix; mutable because it is 
	//		recomputed in the Get accessor
	mutable CMatrixD<4> m_beamToFixedXform;

	// stores the current xform matrix; mutable because it is recomputed
	//		in the Get accessor
	mutable CMatrixD<4> m_beamToPatientXform;

	// boolean value to indicate that shielding blocks are used
	//		by this beam
	BOOL m_bHasShieldingBlocks;

	// collection of blocks
	CObArray m_arrBlocks;

	// the weight for this beam (from 0.0 to 1.0)
	double m_weight;

	// flag to indicate whether the plan's dose is valid
	BOOL m_bDoseValid;

	// the computed dose for this beam (NULL if no dose exists)
	CVolume<double> m_dose;
};

#endif // !defined(AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
