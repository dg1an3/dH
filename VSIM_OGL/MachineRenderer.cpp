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
	: COpenGLRenderer(pView)
{
}

CMachineRenderer::~CMachineRenderer()
{
}

void CMachineRenderer::OnRenderScene()
{
	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// set the color for the beam rendering
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	// render the gantry
	// glBegin(GL_QUAD_STRIP);

	// glEnd();

	// render the gantry sides
	glBegin(GL_LINE_STRIP );

		glVertex(CVector<3>(-250.0,  -250.0,  250.0));
		glVertex(CVector<3>(-250.0,  -250.0,  500.0));
		glVertex(CVector<3>(-250.0,   500.0,  500.0));
		glVertex(CVector<3>(-250.0,   500.0,  250.0));
		glVertex(CVector<3>(-250.0,  -250.0,  250.0));

	glEnd();

	glBegin(GL_LINE_STRIP );

		glVertex(CVector<3>(250.0,  -250.0,  250.0));
		glVertex(CVector<3>(250.0,  -250.0,  500.0));
		glVertex(CVector<3>(250.0,   500.0,  500.0));
		glVertex(CVector<3>(250.0,   500.0,  250.0));
		glVertex(CVector<3>(250.0,  -250.0,  250.0));

	glEnd();

	// render the gantry sides
	glBegin(GL_POLYGON);
	
	glEnd();

	glBegin(GL_POLYGON);

	glEnd();
	
	glEnable(GL_LIGHTING);
}
