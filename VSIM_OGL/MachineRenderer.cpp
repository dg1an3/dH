// MachineRenderer.cpp: implementation of the CMachineRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vsim_ogl.h"
#include "MachineRenderer.h"

#include <glMatrixVector.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMachineRenderer::CMachineRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView),
		isWireFrame(FALSE)
{
	forBeam.AddObserver(this, (ChangeFunction) OnChange);
	isWireFrame.AddObserver(this, (ChangeFunction) OnChange);

	// set up the modelview matrix for the beam
	// CValue< CMatrix<4> >& privModelviewMatrix =
	//	  CreateRotate(privGantryAngle, CVector<3>(0.0, 1.0, 0.0));
	// modelviewMatrix.SyncTo(&privModelviewMatrix);
}

CMachineRenderer::~CMachineRenderer()
{
}

void CMachineRenderer::OnRenderScene()
{
	if (isWireFrame.Get())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDisable(GL_LIGHTING);

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
	}
	else
	{
		GLfloat specular [] = { 0.0, 0.0, 0.0, 1.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		GLfloat shininess [] = { 0.0 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);
	}

	// set the color for the machine rendering
	glColor(RGB(128, 128, 255));

	// render the table

	glPushMatrix();

	glTranslate(-1.0 * forBeam->tableOffset.Get());
	glRotated(forBeam->couchAngle.Get() * 180.0 / PI, 
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

	// compute the axis-to-collimator distance
	double SAD = forBeam->forMachine->SAD.Get();
	double SCD = forBeam->forMachine->SCD.Get();
	double axisToCollim = SAD - SCD;

	// rotate for the gantry
	glRotated(forBeam->gantryAngle.Get() * 180.0 / PI, 0.0, 1.0, 0.0);

	// render the collimator
	glBegin(GL_QUAD_STRIP);

		CVector<4> vCollimEdgeLower(0.0, 100.0, axisToCollim, 1.0);
		CVector<4> vCollimEdgeUpper(0.0, 100.0, axisToCollim + 100.0, 1.0);
		double STEPS = 32.0;
		for (double angle = 0.0; angle < 2 * PI; angle += (2 * PI) / STEPS)
		{

			CMatrix<4> mRot = CreateRotate(angle, CVector<3>(0.0, 0.0, 1.0));
			glVertex(mRot * vCollimEdgeLower);
			glVertex(mRot * vCollimEdgeUpper);
			glNormal(mRot * CVector<4>(0.0, 1.0, 0.0, 1.0));
		}

		glVertex(vCollimEdgeLower);
		glVertex(vCollimEdgeUpper);
		glNormal(CVector<4>(0.0, 1.0, 0.0, 1.0));

	glEnd();

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

void CMachineRenderer::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if (pSource == &forBeam)
	{
		privGantryAngle.SyncTo(&forBeam->gantryAngle);
		forBeam->gantryAngle.AddObserver(this, (ChangeFunction) OnChange);
		forBeam->couchAngle.AddObserver(this, (ChangeFunction) OnChange);
		forBeam->tableOffset.AddObserver(this, (ChangeFunction) OnChange);
	}

	COpenGLRenderer::OnChange(pSource, pOldValue);
}

