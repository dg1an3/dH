// Beam.h: interface for the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>
#include <Value.h>
#include <Association.h>
// #include <AutoSyncValue.h>
#include <Collection.h>

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

private:
	// private copy of a treatment machine; default machine for this beam
	CTreatmentMachine m_Machine;

public:
	// association to the treatment machine for this beam
	CAssociation< CTreatmentMachine > forMachine;

	// synced copies of machine parameters
	CAutoSyncValue< CTreatmentMachine, double > SAD;
	CAutoSyncValue< CTreatmentMachine, CMatrix<4> > projection;

	// angle values
	CValue< double > collimAngle;
	CValue< double > gantryAngle;
	CValue< double > couchAngle;

	// table offset
	CValue< CVector<3> > tableOffset;

	// collimator jaw settings
	CValue< CVector<2> > collimMin;
	CValue< CVector<2> > collimMax;

	// computed transform from patient to beam coordinates
	CValue< CMatrix<4> > beamToPatientXform;

	// boolean value to indicate that shielding blocks are used
	//		by this beam
	CValue< BOOL > hasShieldingBlocks;

	// collection of blocks
	CCollection< CPolygon > blocks;

	// the weight for this beam (from 0.0 to 1.0)
	CValue< double > weight;

	// flag to indicate whether the plan's dose is valid
	CValue<BOOL> isDoseValid;

	// the computed dose for this beam (NULL if no dose exists)
	CVolume<double> dose;

	// beam serialization
	void Serialize(CArchive &ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif // !defined(AFX_BEAM_H__C7A6AA30_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
