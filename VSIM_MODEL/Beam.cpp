// Beam.cpp: implementation of the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "cube.h"
#include "Beam.h"

#include "math.h"

#include <Function.h>
#include <Vector.h>
#include <Matrix.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBeam::CBeam()
	: forMachine(&m_Machine),
		SAD(&forMachine, &CTreatmentMachine::SAD),
		projection(&forMachine, &CTreatmentMachine::projection),
		isDoseValid(FALSE)
{
	forMachine.SetAutoObserver(this, (ChangeFunction) OnChange);

	::AddObserver<CBeam>(&collimAngle, this, OnChange);
	::AddObserver<CBeam>(&gantryAngle, this, OnChange);
	::AddObserver<CBeam>(&couchAngle, this, OnChange);
	::AddObserver<CBeam>(&tableOffset, this, OnChange);
	::AddObserver<CBeam>(&collimMin, this, OnChange);
	::AddObserver<CBeam>(&collimMax, this, OnChange);
	::AddObserver<CBeam>(&blocks, this, OnChange);
	::AddObserver<CBeam>(&beamToPatientXform, this, OnBeamToPatientXformChanged);

	// set up the beam-to-patient transform computation
	CValue< CMatrix<4> >& privBeamToPatientXform =
		  CreateTranslate(tableOffset)
		* CreateRotate(couchAngle,		CVector<3>(0.0, 0.0, -1.0))
		* CreateRotate(PI - gantryAngle,	CVector<3>(0.0, -1.0, 0.0))
		* CreateRotate(collimAngle,		CVector<3>(0.0, 0.0, -1.0))
		* CreateTranslate(SAD, CVector<3>(0.0, 0.0, -1.0));
	beamToPatientXform.SyncTo(&privBeamToPatientXform);
	::AddObserver<CBeam>(&beamToPatientXform, this, OnChange);

	collimAngle.Set(0.0);
	gantryAngle.Set(PI);
	couchAngle.Set(0.0);
	tableOffset.Set(CVector<3>(0.0, 0.0, 0.0));
	collimMin.Set(CVector<2>(-20.0, -20.0));
	collimMax.Set(CVector<2>(20.0, 20.0));
}

#define BEAM_SCHEMA 2
IMPLEMENT_SERIAL(CBeam, CModelObject, VERSIONABLE_SCHEMA | BEAM_SCHEMA)
	// Schema 1: geometry description, blocks
	// Schema 2: + dose matrix

CBeam::~CBeam()
{
}

void CBeam::Serialize(CArchive &ar)
{
	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : BEAM_SCHEMA;

	// serialize the beam name
	name.Serialize(ar);

	// TODO: get this right
	forMachine->Serialize(ar);

	// serialize the beam geometry
	collimAngle.Serialize(ar);
	gantryAngle.Serialize(ar);
	couchAngle.Serialize(ar);
	tableOffset.Serialize(ar);

	if (ar.IsLoading())
	{
		collimAngle.Set(0);
		gantryAngle.Set(0);
		couchAngle.Set(0);
	}

	// serialize the collimator jaw settings
	collimMin.Serialize(ar);
	collimMax.Serialize(ar);

	// if we are reading in a new beam, clear out any existing 
	//		blocks before serializing
	if (!ar.IsStoring())
		blocks.RemoveAll();	

	// serialize the block(s)
	blocks.Serialize(ar);

	// check the beam object's schema; only serialize the dose if 
	//		we are storing or if we are loading with schema > 1
	if (nSchema > 1)
	{
		// serialize the dose matrix valid flag
		isDoseValid.Serialize(ar);

		// empty the dose matrix if it is not valid
		if (ar.IsStoring() && !isDoseValid.Get())
		{
			dose.width.Set(0);
			dose.height.Set(0);
			dose.depth.Set(0);
		}

		// serialize the dose matrix
		dose.Serialize(ar);
	}

	// fire a change if we have just read in a new beam
	if (!ar.IsStoring())
		FireChange();		// TODO: is this necessary?
}

/////////////////////////////////////////////////////////////////////////////
// CBeam diagnostics

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

	DC_TAB(dc) << "Collimator Angle = " << collimAngle.Get() << "\n";
	DC_TAB(dc) << "Gantry Angle     = " << gantryAngle.Get() << "\n";
	DC_TAB(dc) << "Couch Angle      = " << couchAngle.Get()  << "\n";

	DC_TAB(dc) << "Table Offset     = <" << tableOffset.Get()[0] 
		<< ", " << tableOffset.Get()[1] 
		<< ", " << tableOffset.Get()[2] << ">\n";

	DC_TAB(dc) << "Collimator Min X = " << collimMin.Get()[0] << "\n";
	DC_TAB(dc) << "Collimator Max X = " << collimMax.Get()[0] << "\n";
	DC_TAB(dc) << "Collimator Min Y = " << collimMin.Get()[1] << "\n";
	DC_TAB(dc) << "Collimator Max Y = " << collimMax.Get()[1] << "\n";

	PUSH_DUMP_DEPTH(dc);

	// dump the plan's beams
	// m_arrBlocks.Dump(dc);

	POP_DUMP_DEPTH(dc);
}
#endif //_DEBUG

void CBeam::SetBeamToPatientXform(const CMatrix<4>& mXform)
{
	beamToPatientXform.Set(mXform);
	return;

	// now factor the B2P matrix into translation and rotation components
	double gantry = acos(mXform[2][2]);	// gantry in [0..PI]

	double couch = 0.0;
	double coll = 0.0;

	if (gantry != 0.0)
	{
		double cos_couch = mXform[2][0] / sin(gantry);
		// make sure the couch angle will be in [-PI..PI]
		if (cos_couch < 0.0)
		{
			gantry = 2 * PI - gantry;
			cos_couch = mXform[2][0] / sin(gantry);
		}

		double sin_couch = mXform[2][1] / sin(gantry);
		couch = AngleFromSinCos(sin_couch, cos_couch);

		double cos_coll =  -mXform[0][2] / sin(gantry);
		double sin_coll =  mXform[1][2] / sin(gantry);
		coll = AngleFromSinCos(sin_coll, cos_coll);
	}

	couchAngle.Set(couch);

	double actGantry = PI - gantry;
	actGantry = (actGantry < 0.0) ? (2 * PI + actGantry) : actGantry;
	gantryAngle.Set(actGantry);

	collimAngle.Set(coll);

	CMatrix<4> mOldXform = beamToPatientXform.Get();
	mOldXform[0][3] = 0.0;
	mOldXform[1][3] = 0.0;
	mOldXform[2][3] = 0.0;

	CMatrix<4> mNewXform = mXform;
	mNewXform[0][3] = 0.0;
	mNewXform[1][3] = 0.0;
	mNewXform[2][3] = 0.0;

	// inverse-compute the expected matrix
	CMatrix<4> mBeamToPatientExpected = 
		CreateRotate(couch,			CVector<3>(0.0, 0.0, -1.0))
		* CreateRotate(gantry,		CVector<3>(0.0, -1.0, 0.0))
		* CreateRotate(coll,		CVector<3>(0.0, 0.0, -1.0));

	CMatrix<4> mIdent = mNewXform * Transpose(mOldXform);
	TRACE_MATRIX4("Should be identity", mIdent);
	// ASSERT(mIdent.IsApproxEqual(CMatrix<4>()));
}

BOOL m_bChangingXform = FALSE;

void CBeam::OnBeamToPatientXformChanged(CObservableObject *pFromObject, 
	void *pOldValue)
{
	if (m_bChangingXform)
		return;

	// get a reference to the transform
	const CMatrix<4>& mXform = beamToPatientXform.Get();

	// now factor the B2P matrix into translation and rotation components
	double gantry = acos(mXform[2][2]);	// gantry in [0..PI]

	double couch = 0.0;
	double coll = 0.0;

	if (gantry != 0.0)
	{
		double cos_couch = mXform[2][0] / sin(gantry);
		// make sure the couch angle will be in [-PI..PI]
		if (cos_couch < 0.0)
		{
			gantry = 2 * PI - gantry;
			cos_couch = mXform[2][0] / sin(gantry);
		}

		double sin_couch = mXform[2][1] / sin(gantry);
		couch = AngleFromSinCos(sin_couch, cos_couch);

		double cos_coll =  -mXform[0][2] / sin(gantry);
		double sin_coll =  mXform[1][2] / sin(gantry);
		coll = AngleFromSinCos(sin_coll, cos_coll);
	}

	m_bChangingXform = TRUE;

	couchAngle.Set(couch);

	double actGantry = PI - gantry;
	actGantry = (actGantry < 0.0) ? (2 * PI + actGantry) : actGantry;

	gantryAngle.Set(actGantry);
	collimAngle.Set(coll);

	m_bChangingXform = FALSE;
}
