// Beam.cpp: implementation of the CBeam class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "cube.h"
#include "Beam.h"

#include "math.h"

#include "Function.h"

#include "MatrixFunction.h"

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
		projection(&forMachine, &CTreatmentMachine::projection)
{
	forMachine.SetAutoObserver(this);

	collimAngle.AddObserver(this);
	gantryAngle.AddObserver(this);
	couchAngle.AddObserver(this);
	tableOffset.AddObserver(this);
	collimMin.AddObserver(this);
	collimMax.AddObserver(this);
	blocks.AddObserver(this);

	// set up the beam-to-patient transform computation
	CValue< CMatrix<4> >& privBeamToPatientXform =
		  CreateTranslate(tableOffset)
		* CreateRotate(couchAngle,		CVector<3>(0.0, 0.0, -1.0))
		* CreateRotate(PI - gantryAngle,	CVector<3>(0.0, -1.0, 0.0))
		* CreateRotate(collimAngle,		CVector<3>(0.0, 0.0, -1.0))
		* CreateTranslate(SAD, CVector<3>(0.0, 0.0, -1.0));
	beamToPatientXform.SyncTo(&privBeamToPatientXform);
	beamToPatientXform.AddObserver(this);

	collimAngle.Set(0.0);
	gantryAngle.Set(PI);
	couchAngle.Set(0.0);
	tableOffset.Set(CVector<3>(0.0, 0.0, 0.0));
	collimMin.Set(CVector<2>(-20.0, -20.0));
	collimMax.Set(CVector<2>(20.0, 20.0));
}

IMPLEMENT_SERIAL(CBeam, CModelObject, 1)

CBeam::~CBeam()
{
}

void CBeam::OnChange(CObservableObject *pSource)
{
	// propagate the change to any listeners on the beam
	FireChange();
}

void CBeam::Serialize(CArchive &ar)
{
	name.Serialize(ar);

	// TODO: get this right
	forMachine->Serialize(ar);

	collimAngle.Serialize(ar);
	gantryAngle.Serialize(ar);
	couchAngle.Serialize(ar);
	tableOffset.Serialize(ar);
	collimMin.Serialize(ar);
	collimMax.Serialize(ar);

	// if we are reading in a new beam, clear out any existing 
	//		blocks before serializing
	if (!ar.IsStoring())
		blocks.RemoveAll();	

	// serialize the block(s)
	blocks.Serialize(ar);

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
