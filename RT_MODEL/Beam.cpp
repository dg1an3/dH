//////////////////////////////////////////////////////////////////////
// Beam.cpp: implementation of the CBeam class.
//
// Copyright (C) 1999-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// math include
#include <math.h>

// utility macros
#include <UtilMacros.h>

// class declaration
#include "Beam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CBeam::CBeam
// 
// constructs a new CBeam object
//////////////////////////////////////////////////////////////////////
CBeam::CBeam()
	: m_collimAngle(0.0),
		m_gantryAngle(PI),
		m_couchAngle(0.0),

		m_vTableOffset(CVector<3>(0.0, 0.0, 0.0)),

		m_vCollimMin(CVector<2>(-20.0, -20.0)),
		m_vCollimMax(CVector<2>(20.0, 20.0)),
		m_bDoseValid(FALSE)
{
}

//////////////////////////////////////////////////////////////////////
// CBeam serialization
// 
// supports serialization of beam and subordinate objects
//////////////////////////////////////////////////////////////////////
#define BEAM_SCHEMA 3
	// Schema 1: geometry description, blocks
	// Schema 2: + dose matrix
	// Schema 3: + call to CModelObject base class serialization

IMPLEMENT_SERIAL(CBeam, CModelObject, VERSIONABLE_SCHEMA | BEAM_SCHEMA)

//////////////////////////////////////////////////////////////////////
// CBeam::~CBeam
// 
// destroys the CBeam object
//////////////////////////////////////////////////////////////////////
CBeam::~CBeam()
{
	// delete the blocks
	for (int nAt = 0; nAt < m_arrBlocks.GetSize(); nAt++)
	{
		delete m_arrBlocks[nAt];
	}
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetTreatmentMachine
// 
// returns a pointer to the treatment machine
//////////////////////////////////////////////////////////////////////
CTreatmentMachine *CBeam::GetTreatmentMachine()
{
	return &m_Machine;
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimAngle
// 
// returns the collimator angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetCollimAngle() const
{
	return m_collimAngle;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimAngle
// 
// sets the collimator angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimAngle(double collimAngle)
{
	m_collimAngle = collimAngle;
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetGantryAngle
// 
// returns the gantry angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetGantryAngle() const
{
	return m_gantryAngle;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetGantryAngle
// 
// sets the gantry angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetGantryAngle(double gantryAngle)
{
	m_gantryAngle = gantryAngle;
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetCouchAngle
// 
// returns the couch angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetCouchAngle() const
{
	return m_couchAngle;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetCouchAngle
// 
// sets the couch angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetCouchAngle(double couchAngle)
{
	m_couchAngle = couchAngle;
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetTableOffset
// 
// returns the table offset
//////////////////////////////////////////////////////////////////////
const CVector<3>& CBeam::GetTableOffset() const
{
	return m_vTableOffset;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetTableOffset
// 
// sets the table offset
//////////////////////////////////////////////////////////////////////
void CBeam::SetTableOffset(const CVector<3>& vTableOffset)
{
	m_vTableOffset = vTableOffset;
	GetChangeEvent().Fire();
}


//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimMin
// 
// returns the collimator minimum position as a 2D vector
//////////////////////////////////////////////////////////////////////
const CVector<2>& CBeam::GetCollimMin() const
{
	return m_vCollimMin;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimMin
// 
// sets the collimator minimum position as a 2D vector
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimMin(const CVector<2>& vCollimMin)
{
	m_vCollimMin = vCollimMin;
	GetChangeEvent().Fire();
}


//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimMax
// 
// returns the collimator maximum position as a 2D vector
//////////////////////////////////////////////////////////////////////
const CVector<2>& CBeam::GetCollimMax() const
{
	return m_vCollimMax;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimMax
// 
// sets the collimator maximum position as a 2D vector
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimMax(const CVector<2>& vCollimMax)
{
	m_vCollimMax = vCollimMax;
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetBeamToFixedXform
// 
// returns the beam to IEC fixed coordinate system transform
//////////////////////////////////////////////////////////////////////
const CMatrix<4>& CBeam::GetBeamToFixedXform() const
{
	// set up the beam-to-patient transform computation
	m_beamToFixedXform = 

		// gantry rotation
		CMatrix<4>(CreateRotate(PI - m_gantryAngle,	
			CVector<3>(0.0, -1.0, 0.0)))

		// collimator rotation
		* CMatrix<4>(CreateRotate(m_collimAngle,		
			CVector<3>(0.0, 0.0, -1.0)))

		// SAD translation
		* CreateTranslate(m_Machine.GetSAD(),		
			CVector<3>(0.0, 0.0, -1.0));

	return m_beamToFixedXform;
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetBeamToPatientXform
// 
// returns the beam to IEC patient coordinate system transform
//////////////////////////////////////////////////////////////////////
const CMatrix<4>& CBeam::GetBeamToPatientXform() const
{
	// set up the beam-to-patient transform computation
	m_beamToPatientXform = 
		
		// table offset translation
		CreateTranslate(m_vTableOffset)

		// couch rotation
		* CMatrix<4>(CreateRotate(m_couchAngle,	
			CVector<3>(0.0, 0.0, -1.0)))

		// beam to IEC fixed xform
		* GetBeamToFixedXform();

	return m_beamToPatientXform;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetBeamToPatientXform
// 
// sets the beam to IEC patient coordinate system transform
//////////////////////////////////////////////////////////////////////
void CBeam::SetBeamToPatientXform(const CMatrix<4>& m)
{
	// factor the B2P matrix into the rotation components
	double gantry = acos(m[2][2]);	// gantry in [0..PI]

	double couch = 0.0;
	double coll = 0.0;

	if (gantry != 0.0)
	{
		double cos_couch = m[2][0] / sin(gantry);

		// make sure the couch angle will be in [-PI..PI]
		if (cos_couch < 0.0)
		{
			gantry = 2 * PI - gantry;
			cos_couch = m[2][0] / sin(gantry);
		}

		double sin_couch = m[2][1] / sin(gantry);
		couch = AngleFromSinCos(sin_couch, cos_couch);

		double cos_coll =  -m[0][2] / sin(gantry);
		double sin_coll =  m[1][2] / sin(gantry);
		coll = AngleFromSinCos(sin_coll, cos_coll);
	}

	// set the couch angle
	m_couchAngle = couch;

	// ensure the gantry angle is in the correct range
	double actGantry = PI - gantry;
	actGantry = (actGantry < 0.0) ? (2 * PI + actGantry) : actGantry;
	m_gantryAngle = actGantry;

	// set the collimator angle
	m_collimAngle = coll;

	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetHasShieldingBlocks
// 
// boolean value to indicate that shielding blocks are used by this 
//		beam
//////////////////////////////////////////////////////////////////////
BOOL CBeam::GetHasShieldingBlocks() const
{
	return m_bHasShieldingBlocks;
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetBlockCount
// 
// returns the number of blocks on this beam
//////////////////////////////////////////////////////////////////////
int CBeam::GetBlockCount() const
{
	return m_arrBlocks.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetBlockAt
// 
// returns a particular block on this beam
//////////////////////////////////////////////////////////////////////
CPolygon *CBeam::GetBlockAt(int nAt)
{
	return (CPolygon *) m_arrBlocks[nAt];
}

//////////////////////////////////////////////////////////////////////
// CBeam::AddBlock
// 
// adds a new block to this machine
//////////////////////////////////////////////////////////////////////
int CBeam::AddBlock(CPolygon *pBlock)
{
	// add the block and return the index of the added block
	int nAt = m_arrBlocks.Add(pBlock);

	// fire a change event
	GetChangeEvent().Fire();

	// return the new block's index
	return nAt;
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetWeight
// 
// returns the relative weight for this beam (from 0.0 to 1.0)
//////////////////////////////////////////////////////////////////////
double CBeam::GetWeight() const
{
	return m_weight;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetWeight
// 
// sets the relative weight for this beam (from 0.0 to 1.0)
//////////////////////////////////////////////////////////////////////
void CBeam::SetWeight(double weight)
{
	m_weight = weight;

	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CBeam::IsDoseValid
// 
// flag to indicate whether the plan's dose is valid
//////////////////////////////////////////////////////////////////////
BOOL CBeam::IsDoseValid() const
{
	return m_bDoseValid;
}

//////////////////////////////////////////////////////////////////////
// CBeam::SetDoseValid
// 
// flag to indicate whether the plan's dose is valid
//////////////////////////////////////////////////////////////////////
void CBeam::SetDoseValid(BOOL bValid)
{
	m_bDoseValid = bValid;
}

//////////////////////////////////////////////////////////////////////
// CBeam::GetDoseMatrix
// 
// the computed dose for this beam (NULL if no dose exists)
//////////////////////////////////////////////////////////////////////
CVolume<double> *CBeam::GetDoseMatrix()
{
	return &m_dose;
}


//////////////////////////////////////////////////////////////////////
// CBeam::Serialize
// 
// loads/saves beam to archive
//////////////////////////////////////////////////////////////////////
void CBeam::Serialize(CArchive &ar)
{
	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : BEAM_SCHEMA;

	// call base class for schema >= 3
	if (nSchema >= 3)
	{
		CModelObject::Serialize(ar);
	}
	else
	{
		SERIALIZE_VALUE(ar, m_strName);
	}

	// serialize the machine description
	m_Machine.Serialize(ar);

	// serialize angles
	SERIALIZE_VALUE(ar, m_collimAngle);
	SERIALIZE_VALUE(ar, m_gantryAngle);
	SERIALIZE_VALUE(ar, m_couchAngle);

	// TODO: is this really necessary?
	if (ar.IsLoading())
	{
		m_collimAngle = 0.0;
		m_gantryAngle = 0.0;
		m_couchAngle = 0.0;
	}

	// serialize table parameters
	SERIALIZE_VALUE(ar, m_vTableOffset);

	// serialize the collimator jaw settings
	SERIALIZE_VALUE(ar, m_vCollimMin);
	SERIALIZE_VALUE(ar, m_vCollimMax);

	// serialize the block(s)
	SERIALIZE_ARRAY(ar, m_arrBlocks);

	// check the beam object's schema; only serialize the dose if 
	//		we are storing or if we are loading with schema >= 2
	if (nSchema >= 2)
	{
		SERIALIZE_VALUE(ar, m_bDoseValid);

		// empty the dose matrix if it is not valid
		if (ar.IsStoring() && !m_bDoseValid)
		{
			m_dose.SetDimensions(0, 0, 0);
		}

		// serialize the dose matrix
		m_dose.Serialize(ar);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBeam diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CBeam::AssertValid() const
{
	CObject::AssertValid();

	m_Machine.AssertValid();

	// make sure the plan's beams are valid
	// m_arrBlocks.AssertValid();
}

void CBeam::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	// dump the machine description
	// m_Machine.Dump(dc);

	CREATE_TAB(dc.GetDepth());

	DC_TAB(dc) << "Collimator Angle = " << GetCollimAngle() << "\n";
	DC_TAB(dc) << "Gantry Angle     = " << GetGantryAngle() << "\n";
	DC_TAB(dc) << "Couch Angle      = " << GetCouchAngle()  << "\n";

	DC_TAB(dc) << "Table Offset     = <" << GetTableOffset()[0] 
		<< ", " << GetTableOffset()[1] 
		<< ", " << GetTableOffset()[2] << ">\n";

	DC_TAB(dc) << "Collimator Min X = " << GetCollimMin()[0] << "\n";
	DC_TAB(dc) << "Collimator Max X = " << GetCollimMax()[0] << "\n";
	DC_TAB(dc) << "Collimator Min Y = " << GetCollimMin()[1] << "\n";
	DC_TAB(dc) << "Collimator Max Y = " << GetCollimMax()[1] << "\n";

	PUSH_DUMP_DEPTH(dc);

	// dump the plan's beams
	// m_arrBlocks.Dump(dc);

	POP_DUMP_DEPTH(dc);
}
#endif //_DEBUG
