//////////////////////////////////////////////////////////////////////
// MachineRenderable.cpp: implementation of the CMachineRenderable 
//		class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// rendering context
#include <RenderContext.h>

// class declaration
#include "MachineRenderable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::CMachineRenderable
// 
// a depiction of the treatment machine
//////////////////////////////////////////////////////////////////////
CMachineRenderable::CMachineRenderable()
	: m_bWireFrame(FALSE)
{
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::~CMachineRenderable
// 
// destroys the renderable
//////////////////////////////////////////////////////////////////////
CMachineRenderable::~CMachineRenderable()
{
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::GetBeam
// 
// returns the beam from which treatment parameters are taken
//////////////////////////////////////////////////////////////////////
CBeam *CMachineRenderable::GetBeam()
{
	return (CBeam *) GetObject();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::SetBeam
// 
// sets the beam from which treatment parameters are taken
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::SetBeam(CBeam *pBeam)
{
	SetObject(pBeam);
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::SetObject
// 
// over-ride to register change listeners, etc.
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::SetObject(CObject *pObject)
{
	// make sure the object is of the right class
	ASSERT(pObject->IsKindOf(RUNTIME_CLASS(CBeam)));

	// remove this as an observer on the old beam
	if (NULL != GetBeam())
	{
		::RemoveObserver<CMachineRenderable>(&GetBeam()->GetChangeEvent(),
			this, Invalidate);
	}

	// set the beam pointer
	CRenderable::SetObject(pObject);

	// add this as an observer on the new beam
	if (NULL != GetBeam())
	{
		::AddObserver<CMachineRenderable>(&GetBeam()->GetChangeEvent(),
			this, Invalidate);
	}

	// re-render
	Invalidate();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DrawOpaque
// 
// Draws the treatment machine
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DrawOpaque(CRenderContext *pRC)
{
	// set up the rendering parameters
	if (m_bWireFrame)
	{
		// setup for line drawing
		pRC->SetupLines();

		// Draw the table
		DrawTable(pRC);

		// compute the axis-to-collimator distance
		double SAD = GetBeam()->GetTreatmentMachine()->GetSAD();
		double SCD = GetBeam()->GetTreatmentMachine()->GetSCD();
		double axisToCollim = SAD - SCD;

		// rotate for the gantry
		pRC->Rotate(GetBeam()->GetGantryAngle() * 180.0 / PI, 
			CVectorD<3>(0.0, 1.0, 0.0));

		// Draw the gantry itself
		DrawGantry(pRC, axisToCollim);

		// Draw the collimator
		DrawCollimator(pRC, axisToCollim);
	}
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DrawTransparent
// 
// Draws the treatment machine
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DrawTransparent(CRenderContext *pRC)
{
	// Draw the table
	DrawTable(pRC);

	// compute the axis-to-collimator distance
	double SAD = GetBeam()->GetTreatmentMachine()->GetSAD();
	double SCD = GetBeam()->GetTreatmentMachine()->GetSCD();
	double axisToCollim = SAD - SCD;

	// rotate for the gantry
	pRC->Rotate(GetBeam()->GetGantryAngle() * 180.0 / PI, 
		CVectorD<3>(0.0, 1.0, 0.0));

	// Draw the gantry itself
	DrawGantry(pRC, axisToCollim);

	// Draw the collimator
	DrawCollimator(pRC, axisToCollim);
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DrawTable
// 
// Draws the treatment machine table
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DrawTable(CRenderContext *pRC)
{
	pRC->PushMatrix();

	pRC->Translate(-1.0 * GetBeam()->GetTableOffset());
	pRC->Rotate(GetBeam()->GetCouchAngle() * 180.0 / PI, 
		CVectorD<3>(.0, 0.0, 1.0));

	pRC->BeginQuads();

		pRC->Normal(CVectorD<3>(0.0, 0.0, 1.0));

		pRC->Vertex(CVectorD<3>(-250.0, -1200.0,  -100.0));
		pRC->Vertex(CVectorD<3>( 250.0, -1200.0,  -100.0));
		pRC->Vertex(CVectorD<3>( 250.0,   250.0,  -100.0));
		pRC->Vertex(CVectorD<3>(-250.0,   250.0,  -100.0));

		pRC->Normal(CVectorD<3>(0.0, 0.0, -1.0));

		pRC->Vertex(CVectorD<3>(-250.0, -1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0, -1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,   250.0,  -150.0));
		pRC->Vertex(CVectorD<3>(-250.0,   250.0,  -150.0));

		pRC->Normal(CVectorD<3>(-1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>(-250.0, -1200.0,  -100.0));
		pRC->Vertex(CVectorD<3>(-250.0, -1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>(-250.0,   250.0,  -150.0));
		pRC->Vertex(CVectorD<3>(-250.0,   250.0,  -100.0));

		pRC->Normal(CVectorD<3>( 1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>( 250.0, -1200.0,  -100.0));
		pRC->Vertex(CVectorD<3>( 250.0, -1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,   250.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,   250.0,  -100.0));

		pRC->Normal(CVectorD<3>( 0.0, 1.0, 0.0));

		pRC->Vertex(CVectorD<3>(-250.0,  250.0,  -100.0));
		pRC->Vertex(CVectorD<3>(-250.0,  250.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,  250.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,  250.0,  -100.0));

		pRC->Normal(CVectorD<3>( 0.0, -1.0, 0.0));

		pRC->Vertex(CVectorD<3>(-250.0,-1200.0,  -100.0));
		pRC->Vertex(CVectorD<3>(-250.0,-1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,-1200.0,  -150.0));
		pRC->Vertex(CVectorD<3>( 250.0,-1200.0,  -100.0));


	pRC->End();

	pRC->PopMatrix();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DrawGantry
// 
// Draws the treatment machine gantry; assumes the rotation has
//		already been set
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DrawGantry(CRenderContext *pRC, 
										double axisToCollim)
{
	// render the gantry sides
	pRC->BeginPolygon();

		pRC->Normal(CVectorD<3>(-1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  750.0,  axisToCollim + 100.0) );

	pRC->End();

	pRC->BeginPolygon();

		pRC->Normal(CVectorD<3>(-1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  500.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  500.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>(-300.0,  750.0,-(axisToCollim + 300.0)));

	pRC->End();

	// render the gantry sides
	pRC->BeginPolygon();

		pRC->Normal(CVectorD<3>( 1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,  axisToCollim + 100.0) );

	pRC->End();

	pRC->BeginPolygon();

		pRC->Normal(CVectorD<3>( 1.0, 0.0, 0.0));

		pRC->Vertex(CVectorD<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,-(axisToCollim + 300.0)));

	pRC->End();

	// render the gantry faces
	pRC->BeginQuads();

		pRC->Normal(CVectorD<3>(0.0, -1.0, 0.0));

		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 300.0) );

		pRC->Normal(CVectorD<3>(0.0, 0.0, 1.0));

		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  750.0,  axisToCollim + 300.0) );

		pRC->Normal(CVectorD<3>(0.0, 1.0, 0.0));

		pRC->Vertex(CVectorD<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>(-300.0,  750.0,-(axisToCollim + 300.0)));

		pRC->Normal(CVectorD<3>(0.0, 0.0, -1.0));

		pRC->Vertex(CVectorD<3>(-300.0,  750.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>( 300.0,  750.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>(-300.0,  500.0,-(axisToCollim + 300.0)));

		pRC->Normal(CVectorD<3>(0.0, -1.0, 0.0));

		pRC->Vertex(CVectorD<3>(-300.0,  500.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>(-300.0,  500.0,  axisToCollim + 100.0) );

		pRC->Normal(CVectorD<3>(0.0, 0.0, -1.0));

		pRC->Vertex(CVectorD<3>(-300.0,  500.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>( 300.0,  500.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		pRC->Vertex(CVectorD<3>(-300.0, -250.0,  axisToCollim + 100.0) );

	pRC->End();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DrawCollimator
// 
// Draws the treatment machine collimator; assumes the rotation 
//		has already been set
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DrawCollimator(CRenderContext *pRC,
											double axisToCollim)
{
	// render the collimator
	pRC->BeginQuadStrip();

		// compute the upper and lower edges
		CVectorD<4> vCollimEdgeLower(0.0, 100.0, axisToCollim, 1.0);
		CVectorD<4> vCollimEdgeUpper(0.0, 100.0, axisToCollim + 100.0, 1.0);

		// number of steps around the circle
		const double STEPS = 32.0;
		for (double angle = 0.0; angle < 2 * PI; angle += (2 * PI) / STEPS)
		{
			// rotate about the central axis,
			CMatrixD<4> mRot = 
				CMatrixD<4>(CreateRotate(angle, CVectorD<3>(0.0, 0.0, 1.0)));

			// draw a quad for each step
			pRC->Vertex(mRot * vCollimEdgeLower);
			pRC->Vertex(mRot * vCollimEdgeUpper);

			// normal points outwards
			pRC->Normal(mRot * CVectorD<4>(0.0, 1.0, 0.0, 1.0));
		}

		// finish off the collimator
		pRC->Vertex(vCollimEdgeLower);
		pRC->Vertex(vCollimEdgeUpper);

		// and set the normal
		pRC->Normal(CVectorD<4>(0.0, 1.0, 0.0, 1.0));

	pRC->End();
}
