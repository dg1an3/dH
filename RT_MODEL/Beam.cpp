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


// filter for intensity maps
CVectorN<> CBeam::m_vWeightFilter;
CMatrixNxM<> CBeam::m_mFilter[MAX_SCALES - 1];


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CBeam::CBeam
// 
// constructs a new CBeam object
///////////////////////////////////////////////////////////////////////////////
CBeam::CBeam()
	: m_collimAngle(0.0),
		m_gantryAngle(PI),
		m_couchAngle(0.0),

		m_vTableOffset(CVectorD<3>(0.0, 0.0, 0.0)),

		m_vCollimMin(CVectorD<2>(-20.0, -20.0)),
		m_vCollimMax(CVectorD<2>(20.0, 20.0)),

		m_weight(1.0),
		m_bHasShieldingBlocks(FALSE),
		m_bRecalcDose(TRUE)
{
	if (m_vWeightFilter.GetDim() == 0)
	{
		m_vWeightFilter.SetDim(3);
		m_vWeightFilter[0] = 0.25;
		m_vWeightFilter[1] = 0.50;
		m_vWeightFilter[2] = 0.25;
	}

}	// CBeam::CBeam

//////////////////////////////////////////////////////////////////////
// CBeam serialization
// 
// supports serialization of beam and subordinate objects
//////////////////////////////////////////////////////////////////////
#define BEAM_SCHEMA 5
	// Schema 1: geometry description, blocks
	// Schema 2: + dose matrix
	// Schema 3: + call to CModelObject base class serialization
	// Schema 4: + weight
	// Schema 5: + beamlets

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

}	// CBeam::~CBeam


//////////////////////////////////////////////////////////////////////
// CBeam::GetTreatmentMachine
// 
// returns a pointer to the treatment machine
//////////////////////////////////////////////////////////////////////
CTreatmentMachine *CBeam::GetTreatmentMachine()
{
	return &m_Machine;

}	// CBeam::GetTreatmentMachine


//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimAngle
// 
// returns the collimator angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetCollimAngle() const
{
	return m_collimAngle;

}	// CBeam::GetCollimAngle

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimAngle
// 
// sets the collimator angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimAngle(double collimAngle)
{
	m_collimAngle = collimAngle;
	GetChangeEvent().Fire();

}	// CBeam::SetCollimAngle

//////////////////////////////////////////////////////////////////////
// CBeam::GetGantryAngle
// 
// returns the gantry angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetGantryAngle() const
{
	return m_gantryAngle;

}	// CBeam::GetGantryAngle


//////////////////////////////////////////////////////////////////////
// CBeam::SetGantryAngle
// 
// sets the gantry angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetGantryAngle(double gantryAngle)
{
	m_gantryAngle = gantryAngle;
	GetChangeEvent().Fire();

}	// CBeam::SetGantryAngle

//////////////////////////////////////////////////////////////////////
// CBeam::GetCouchAngle
// 
// returns the couch angle value
//////////////////////////////////////////////////////////////////////
double CBeam::GetCouchAngle() const
{
	return m_couchAngle;

}	// CBeam::GetCouchAngle


//////////////////////////////////////////////////////////////////////
// CBeam::SetCouchAngle
// 
// sets the couch angle value
//////////////////////////////////////////////////////////////////////
void CBeam::SetCouchAngle(double couchAngle)
{
	m_couchAngle = couchAngle;
	GetChangeEvent().Fire();

}	// CBeam::SetCouchAngle


//////////////////////////////////////////////////////////////////////
// CBeam::GetTableOffset
// 
// returns the table offset
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CBeam::GetTableOffset() const
{
	return m_vTableOffset;

}	// CBeam::GetTableOffset


//////////////////////////////////////////////////////////////////////
// CBeam::SetTableOffset
// 
// sets the table offset
//////////////////////////////////////////////////////////////////////
void CBeam::SetTableOffset(const CVectorD<3>& vTableOffset)
{
	m_vTableOffset = vTableOffset;
	GetChangeEvent().Fire();

}	// CBeam::SetTableOffset


//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimMin
// 
// returns the collimator minimum position as a 2D vector
//////////////////////////////////////////////////////////////////////
const CVectorD<2>& CBeam::GetCollimMin() const
{
	return m_vCollimMin;

}	// CBeam::GetCollimMin

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimMin
// 
// sets the collimator minimum position as a 2D vector
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimMin(const CVectorD<2>& vCollimMin)
{
	m_vCollimMin = vCollimMin;
	GetChangeEvent().Fire();

}	// CBeam::SetCollimMin

//////////////////////////////////////////////////////////////////////
// CBeam::GetCollimMax
// 
// returns the collimator maximum position as a 2D vector
//////////////////////////////////////////////////////////////////////
const CVectorD<2>& CBeam::GetCollimMax() const
{
	return m_vCollimMax;

}	// CBeam::GetCollimMax

//////////////////////////////////////////////////////////////////////
// CBeam::SetCollimMax
// 
// sets the collimator maximum position as a 2D vector
//////////////////////////////////////////////////////////////////////
void CBeam::SetCollimMax(const CVectorD<2>& vCollimMax)
{
	m_vCollimMax = vCollimMax;
	GetChangeEvent().Fire();

}	// CBeam::SetCollimMax

//////////////////////////////////////////////////////////////////////
// CBeam::GetBeamToFixedXform
// 
// returns the beam to IEC fixed coordinate system transform
//////////////////////////////////////////////////////////////////////
const CMatrixD<4>& CBeam::GetBeamToFixedXform() const
{
	// set up the beam-to-patient transform computation
	m_beamToFixedXform = 

		// gantry rotation
		CMatrixD<4>( CreateRotate( ((REAL) (PI - m_gantryAngle)),	
			CVectorD<3>(0.0, -1.0, 0.0)) )

		// collimator rotation
		* CMatrixD<4>( CreateRotate( ((REAL) m_collimAngle),		
			CVectorD<3>(0.0, 0.0, -1.0)) )

		// SAD translation
		* CreateTranslate(m_Machine.GetSAD(),		
			CVectorD<3>(0.0, 0.0, -1.0));

	return m_beamToFixedXform;

}	// CBeam::GetBeamToFixedXform

//////////////////////////////////////////////////////////////////////
// CBeam::GetBeamToPatientXform
// 
// returns the beam to IEC patient coordinate system transform
//////////////////////////////////////////////////////////////////////
const CMatrixD<4>& CBeam::GetBeamToPatientXform() const
{
	// set up the beam-to-patient transform computation
	m_beamToPatientXform = 
		
		// table offset translation
		CreateTranslate(m_vTableOffset)

		// couch rotation
		* CMatrixD<4>(CreateRotate( ((REAL) m_couchAngle),	
			CVectorD<3>(0.0, 0.0, -1.0)))

		// beam to IEC fixed xform
		* GetBeamToFixedXform();

	return m_beamToPatientXform;

}	// CBeam::GetBeamToPatientXform

//////////////////////////////////////////////////////////////////////
// CBeam::SetBeamToPatientXform
// 
// sets the beam to IEC patient coordinate system transform
//////////////////////////////////////////////////////////////////////
void CBeam::SetBeamToPatientXform(const CMatrixD<4>& m)
{
	// factor the B2P matrix into the rotation components
	double gantry = acos(m[2][2]);	// gantry in [0..PI]

	double couch = 0.0;
	double coll = 0.0;

	if (gantry != 0.0)
	{
		double cos_couch = m[0][2] / sin(gantry);

		// make sure the couch angle will be in [-PI..PI]
		if (cos_couch < 0.0)
		{
			gantry = 2 * PI - gantry;
			cos_couch = m[0][2] / sin(gantry);
		}

		double sin_couch = m[1][2] / sin(gantry);
		couch = AngleFromSinCos(sin_couch, cos_couch);

		double cos_coll =  -m[2][0] / sin(gantry);
		double sin_coll =  m[2][1] / sin(gantry);
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

}	// CBeam::SetBeamToPatientXform


//////////////////////////////////////////////////////////////////////
// CBeam::GetHasShieldingBlocks
// 
// boolean value to indicate that shielding blocks are used by this 
//		beam
//////////////////////////////////////////////////////////////////////
BOOL CBeam::GetHasShieldingBlocks() const
{
	return m_bHasShieldingBlocks;

}	// CBeam::GetHasShieldingBlocks

//////////////////////////////////////////////////////////////////////
// CBeam::GetBlockCount
// 
// returns the number of blocks on this beam
//////////////////////////////////////////////////////////////////////
int CBeam::GetBlockCount() const
{
	return m_arrBlocks.GetSize();

}	// CBeam::GetBlockCount

//////////////////////////////////////////////////////////////////////
// CBeam::GetBlock
// 
// returns a particular block on this beam
//////////////////////////////////////////////////////////////////////
CPolygon *CBeam::GetBlock(int nAt)
{
	return m_arrBlocks[nAt];

}	// CBeam::GetBlock


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

}	// CBeam::AddBlock


//////////////////////////////////////////////////////////////////////
// CBeam::GetWeight
// 
// returns the relative weight for this beam (from 0.0 to 1.0)
//////////////////////////////////////////////////////////////////////
double CBeam::GetWeight() const
{
	return m_weight;

}	// CBeam::GetWeight

//////////////////////////////////////////////////////////////////////
// CBeam::SetWeight
// 
// sets the relative weight for this beam (from 0.0 to 1.0)
//////////////////////////////////////////////////////////////////////
void CBeam::SetWeight(double weight)
{
	m_weight = weight;

	GetChangeEvent().Fire();

}	// CBeam::SetWeight

///////////////////////////////////////////////////////////////////////////////
// CBeam::GetBeamletCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CBeam::GetBeamletCount(int nLevel)
{
	return m_arrBeamlets[nLevel].GetSize();

}	// CBeam::GetBeamletCount

///////////////////////////////////////////////////////////////////////////////
// CBeam::GetBeamlet
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<REAL> * CBeam::GetBeamlet(int nShift, int nLevel)
{
	return (CVolume<REAL> *) m_arrBeamlets[nLevel]
		[nShift + GetBeamletCount(nLevel) / 2];

}	// CBeam::GetBeamlet


///////////////////////////////////////////////////////////////////////////////
// CBeam::GetIntensityMap
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CBeam::GetIntensityMap() const
{
	return m_vBeamletWeights;

}	// CBeam::GetIntensityMap


///////////////////////////////////////////////////////////////////////////////
// CBeam::SetIntensityMap
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeam::SetIntensityMap(const CVectorN<>& vWeights)
{
	ASSERT(m_arrBeamlets[0].GetSize() == vWeights.GetDim());

	m_vBeamletWeights = vWeights;
	m_bRecalcDose = TRUE;

}	// CBeam::SetIntensityMap


///////////////////////////////////////////////////////////////////////////////
// CBeam::InvFiltIntensityMap
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CBeam::InvFiltIntensityMap(int nLevel, const CVectorBase<>& vWeights,
								CVectorBase<>& vFiltWeights)
{
	BEGIN_LOG_SECTION(CBeam::InvFiltIntensityMap);
	LOG_EXPR(nLevel);
	LOG_EXPR_EXT(vWeights);

	CVectorN<> vFilterOut;
	vFilterOut.SetDim(vWeights.GetDim() * 2 + 1);
	vFilterOut.SetZero();

	for (int nAt = 0; nAt < vWeights.GetDim(); nAt++)
	{
		vFilterOut[nAt * 2 + 1] = vWeights[nAt];
	}

	vFiltWeights = GetFilterMat(nLevel-1) * vFilterOut;
	vFiltWeights *= 2.0;

	END_LOG_SECTION();

}	// CBeam::InvFiltIntensityMap

//////////////////////////////////////////////////////////////////////
// CBeam::GetDoseMatrix
// 
// the computed dose for this beam (NULL if no dose exists)
//////////////////////////////////////////////////////////////////////
CVolume<REAL> *CBeam::GetDoseMatrix()
{
	if (m_bRecalcDose)
	{ 
		// check dose matrix size
		CVolume<REAL> *pBeamlet = m_arrBeamlets[0][0];
		if (m_dose.GetWidth() != pBeamlet->GetWidth())
		{
			// TODO: take this from plan dose, not beamlets
			m_dose.SetDimensions(pBeamlet->GetWidth(), 
				pBeamlet->GetHeight(), pBeamlet->GetDepth());

			CMatrixD<4> mBasis;
			mBasis[3][0] = -(m_dose.GetWidth() - 1) / 2;
			mBasis[3][1] = -(m_dose.GetHeight() - 1) / 2;
			m_dose.SetBasis(mBasis);

		}

		// clear voxels for accumulation
		m_dose.ClearVoxels();

		for (int nAt = 0; nAt < m_arrBeamlets[0].GetSize(); nAt++)
		{
			CVolume<REAL> *pBeamlet = m_arrBeamlets[0][nAt];
			m_dose.Accumulate(pBeamlet, m_vBeamletWeights[nAt]);
		}

		m_bRecalcDose = FALSE;
	}  

	return &m_dose;

}	// CBeam::GetDoseMatrix

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

	// serialize the block(s) -- first prepare the array
	if (ar.IsLoading())
	{
		// delete any existing structures
		for (int nAt = 0; nAt < GetBlockCount(); nAt++)
		{
			delete GetBlock(nAt);
		}
		m_arrBlocks.SetSize(0);

		DWORD nCount = ar.ReadCount();
		for (nAt = 0; nAt < nCount; nAt++)
		{
			// and add it to the array
			AddBlock(new CPolygon());
		}
	}
	else
	{
		ar.WriteCount(GetBlockCount());
	}

	// now serialize the blocks
	for (int nAt = 0; nAt < GetBlockCount(); nAt++)
	{
		GetBlock(nAt)->Serialize(ar);
	}

	// check the beam object's schema; only serialize the dose if 
	//		we are storing or if we are loading with schema >= 2
	if (nSchema >= 2)
	{
		// WAS m_bDoseValid flag (deprecated)
		BOOL bDoseValid = TRUE;
		SERIALIZE_VALUE(ar, bDoseValid);

		// serialize the dose matrix
		m_dose.Serialize(ar);
	}

	// serialize the beam weight
	if (nSchema >= 4)
	{
		SERIALIZE_VALUE(ar, m_weight);
	}

	// serialize the beamlets
	if (nSchema >= 5)
	{
		int nLevels = MAX_SCALES;
		SERIALIZE_VALUE(ar, nLevels);
		ASSERT(nLevels <= MAX_SCALES);

		for (int nAtLevel = 0; nAtLevel < nLevels; nAtLevel++)
		{
			int nBeamlets = m_arrBeamlets[nAtLevel].GetSize();
			SERIALIZE_VALUE(ar, nBeamlets);

			// TODO: delete old beamlets
			if (ar.IsLoading())
			{
				m_arrBeamlets[nAtLevel].RemoveAll();
			}

			for (int nAtBeamlet = 0; nAtBeamlet < nBeamlets; nAtBeamlet++)
			{
				if (ar.IsLoading())
				{
					m_arrBeamlets[nAtLevel].Add(new CVolume<REAL>());
				}
				m_arrBeamlets[nAtLevel][nAtBeamlet]->Serialize(ar);
			}
		}
		
		// serialize the weights as well
		SERIALIZE_VALUE(ar, m_vBeamletWeights);
	}

}	// CBeam::Serialize

///////////////////////////////////////////////////////////////////////////////
// CBeam::GetFilterMat
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& CBeam::GetFilterMat(int nLevel)
{
	if (m_mFilter[nLevel].GetCols() != GetBeamletCount(nLevel))
	{
		// set up the filter matrix
		m_mFilter[nLevel].Reshape(GetBeamletCount(nLevel), 
			GetBeamletCount(nLevel));
		ZeroMemory(m_mFilter[nLevel], 
			m_mFilter[nLevel].GetRows() * m_mFilter[nLevel].GetCols() * sizeof(REAL));

		for (int nAt = 0; nAt < m_mFilter[nLevel].GetRows(); nAt++)
		{
			if (nAt > 0)
			{
				m_mFilter[nLevel][nAt - 1][nAt] = m_vWeightFilter[0];
			}

			m_mFilter[nLevel][nAt][nAt] = m_vWeightFilter[1];

			if (nAt < m_mFilter[nLevel].GetRows()-1)
			{
				m_mFilter[nLevel][nAt + 1][nAt] = m_vWeightFilter[2];
			}
		}
		LOG_EXPR_EXT(m_mFilter[nLevel]);
	}

	return m_mFilter[nLevel];

}	// CBeam::GetFilterMat


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
