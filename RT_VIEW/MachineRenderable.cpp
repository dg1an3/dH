//////////////////////////////////////////////////////////////////////
// MachineRenderable.cpp: implementation of the CMachineRenderable 
//		class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// OpenGL includes
#include <glMatrixVector.h>

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
	return m_pBeam;
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::SetBeam
// 
// sets the beam from which treatment parameters are taken
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::SetBeam(CBeam *pBeam)
{
	// remove this as an observer on the old beam
	if (NULL != m_pBeam)
	{
		::RemoveObserver<CMachineRenderable>(&pBeam->GetChangeEvent(),
			this, Invalidate);
	}

	// set the beam pointer
	m_pBeam = pBeam;

	// add this as an observer on the new beam
	if (NULL != m_pBeam)
	{
		::AddObserver<CMachineRenderable>(&pBeam->GetChangeEvent(),
			this, Invalidate);
	}

	// re-render
	Invalidate();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DescribeOpaque
// 
// describes the treatment machine
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DescribeOpaque()
{
	// set up the rendering parameters
	if (m_bWireFrame)
	{
		glColor(RGB(255, 255, 255));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDisable(GL_LIGHTING);

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
	}
	else
	{
		// set the color for the machine rendering
		glColor(GetColor());

		// set the material properties
		GLfloat specular [] = { 0.0, 0.0, 0.0, 1.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		GLfloat shininess [] = { 0.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);

		glEnable(GL_LIGHTING);
	}

	// describe the table
	DescribeTable();

	// compute the axis-to-collimator distance
	double SAD = m_pBeam->GetTreatmentMachine()->m_SAD;
	double SCD = m_pBeam->GetTreatmentMachine()->m_SCD;
	double axisToCollim = SAD - SCD;

	// rotate for the gantry
	glRotated(m_pBeam->GetGantryAngle() * 180.0 / PI, 0.0, 1.0, 0.0);

	// describe the gantry itself
	DescribeGantry(axisToCollim);

	// describe the collimator
	DescribeCollimator(axisToCollim);

	// turn off wire frame mode
	if (m_bWireFrame)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_LIGHTING);
	}

}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DescribeTable
// 
// describes the treatment machine table
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DescribeTable()
{
	glPushMatrix();

	glTranslate(-1.0 * m_pBeam->GetTableOffset());
	glRotated(m_pBeam->GetCouchAngle() * 180.0 / PI, 
		0.0, 0.0, 1.0);

	glBegin(GL_QUADS);

		glNormal(CVector<3>(0.0, 0.0, 1.0));

		glVertex(CVector<3>(-250.0, -1200.0,  -100.0));
		glVertex(CVector<3>( 250.0, -1200.0,  -100.0));
		glVertex(CVector<3>( 250.0,   250.0,  -100.0));
		glVertex(CVector<3>(-250.0,   250.0,  -100.0));

		glNormal(CVector<3>(0.0, 0.0, -1.0));

		glVertex(CVector<3>(-250.0, -1200.0,  -150.0));
		glVertex(CVector<3>( 250.0, -1200.0,  -150.0));
		glVertex(CVector<3>( 250.0,   250.0,  -150.0));
		glVertex(CVector<3>(-250.0,   250.0,  -150.0));

		glNormal(CVector<3>(-1.0, 0.0, 0.0));

		glVertex(CVector<3>(-250.0, -1200.0,  -100.0));
		glVertex(CVector<3>(-250.0, -1200.0,  -150.0));
		glVertex(CVector<3>(-250.0,   250.0,  -150.0));
		glVertex(CVector<3>(-250.0,   250.0,  -100.0));

		glNormal(CVector<3>( 1.0, 0.0, 0.0));

		glVertex(CVector<3>( 250.0, -1200.0,  -100.0));
		glVertex(CVector<3>( 250.0, -1200.0,  -150.0));
		glVertex(CVector<3>( 250.0,   250.0,  -150.0));
		glVertex(CVector<3>( 250.0,   250.0,  -100.0));

		glNormal(CVector<3>( 0.0, 1.0, 0.0));

		glVertex(CVector<3>(-250.0,  250.0,  -100.0));
		glVertex(CVector<3>(-250.0,  250.0,  -150.0));
		glVertex(CVector<3>( 250.0,  250.0,  -150.0));
		glVertex(CVector<3>( 250.0,  250.0,  -100.0));

		glNormal(CVector<3>( 0.0, -1.0, 0.0));

		glVertex(CVector<3>(-250.0,-1200.0,  -100.0));
		glVertex(CVector<3>(-250.0,-1200.0,  -150.0));
		glVertex(CVector<3>( 250.0,-1200.0,  -150.0));
		glVertex(CVector<3>( 250.0,-1200.0,  -100.0));


	glEnd();

	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DescribeGantry
// 
// describes the treatment machine gantry; assumes the rotation has
//		already been set
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DescribeGantry(double axisToCollim)
{
	// render the gantry sides
	glBegin(GL_POLYGON);

		glNormal(CVector<3>(-1.0, 0.0, 0.0));

		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0,  750.0,  axisToCollim + 100.0) );

	glEnd();

	glBegin(GL_POLYGON);

		glNormal(CVector<3>(-1.0, 0.0, 0.0));

		glVertex(CVector<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0,  500.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0,  500.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>(-300.0,  750.0,-(axisToCollim + 300.0)));

	glEnd();

	// render the gantry sides
	glBegin(GL_POLYGON);

		glNormal(CVector<3>( 1.0, 0.0, 0.0));

		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  750.0,  axisToCollim + 100.0) );

	glEnd();

	glBegin(GL_POLYGON);

		glNormal(CVector<3>( 1.0, 0.0, 0.0));

		glVertex(CVector<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  500.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>( 300.0,  750.0,-(axisToCollim + 300.0)));

	glEnd();

	// render the gantry faces
	glBegin(GL_QUADS);

		glNormal(CVector<3>(0.0, -1.0, 0.0));

		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 300.0) );

		glNormal(CVector<3>(0.0, 0.0, 1.0));

		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>(-300.0,  750.0,  axisToCollim + 300.0) );

		glNormal(CVector<3>(0.0, 1.0, 0.0));

		glVertex(CVector<3>(-300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  750.0,  axisToCollim + 300.0) );
		glVertex(CVector<3>( 300.0,  750.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>(-300.0,  750.0,-(axisToCollim + 300.0)));

		glNormal(CVector<3>(0.0, 0.0, -1.0));

		glVertex(CVector<3>(-300.0,  750.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>( 300.0,  750.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>(-300.0,  500.0,-(axisToCollim + 300.0)));

		glNormal(CVector<3>(0.0, -1.0, 0.0));

		glVertex(CVector<3>(-300.0,  500.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>( 300.0,  500.0,-(axisToCollim + 300.0)));
		glVertex(CVector<3>( 300.0,  500.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>(-300.0,  500.0,  axisToCollim + 100.0) );

		glNormal(CVector<3>(0.0, 0.0, -1.0));

		glVertex(CVector<3>(-300.0,  500.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>( 300.0,  500.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>( 300.0, -250.0,  axisToCollim + 100.0) );
		glVertex(CVector<3>(-300.0, -250.0,  axisToCollim + 100.0) );

	glEnd();
}

//////////////////////////////////////////////////////////////////////
// CMachineRenderable::DescribeCollimator
// 
// describes the treatment machine collimator; assumes the rotation 
//		has already been set
//////////////////////////////////////////////////////////////////////
void CMachineRenderable::DescribeCollimator(double axisToCollim)
{
	// render the collimator
	glBegin(GL_QUAD_STRIP);

		// compute the upper and lower edges
		CVector<4> vCollimEdgeLower(0.0, 100.0, axisToCollim, 1.0);
		CVector<4> vCollimEdgeUpper(0.0, 100.0, axisToCollim + 100.0, 1.0);

		// number of steps around the circle
		const double STEPS = 32.0;
		for (double angle = 0.0; angle < 2 * PI; angle += (2 * PI) / STEPS)
		{
			// rotate about the central axis,
			CMatrix<4> mRot = 
				CMatrix<4>(CreateRotate(angle, CVector<3>(0.0, 0.0, 1.0)));

			// draw a quad for each step
			glVertex(mRot * vCollimEdgeLower);
			glVertex(mRot * vCollimEdgeUpper);

			// normal points outwards
			glNormal(mRot * CVector<4>(0.0, 1.0, 0.0, 1.0));
		}

		// finish off the collimator
		glVertex(vCollimEdgeLower);
		glVertex(vCollimEdgeUpper);

		// and set the normal
		glNormal(CVector<4>(0.0, 1.0, 0.0, 1.0));

	glEnd();
}
