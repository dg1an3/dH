// NodeRenderer.cpp: implementation of the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeRenderer.h"

#include <gl/gl.h>
#include <gl/glu.h>

#include <glMatrixVector.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeRenderer::CNodeRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView)
{
	color.Set(RGB(240, 240, 240));
}

CNodeRenderer::~CNodeRenderer()
{

}

void CNodeRenderer::OnRenderScene()
{

	glColor(color.Get());

	// glTranslated(150.0, 100.0, 0.0);

	// draw an elliptangle
	glBegin(GL_QUADS);
		glNormal3d(0.0, 1.0, 0.0);
		glVertex3d(-50.0, 5.0, -50.0);
		glVertex3d(-50.0, 5.0,  50.0);
		glVertex3d( 50.0, 5.0,  50.0);
		glVertex3d( 50.0, 5.0, -50.0);
	glEnd();

	glBegin(GL_QUADS);
		glNormal3d(0.0, -1.0, 0.0);
		glVertex3d(-50.0, -5.0, -50.0);
		glVertex3d(-50.0, -5.0,  50.0);
		glVertex3d( 50.0, -5.0,  50.0);
		glVertex3d( 50.0, -5.0, -50.0);
	glEnd();

	double a_lat = 15.0;
	double b_lat = 60.0;

	for (double xDir = -1.0; xDir <= 1.0; xDir += 2.0)
	{
		for (double z = -50.0; z < 50.0; z += 2.0)
		{
			double x = 50.0 + a_lat * sqrt(1.0 - z * z / (b_lat * b_lat));

			glBegin(GL_QUAD_STRIP);

				glNormal(CVector<3>(0.0, -1.0, 0.0));

				glVertex3d(xDir * (x - 0.1), -5.0, z);
				glVertex3d(xDir * (x - 0.1), -5.0, z + 2.0);

				for (double y = -5.0; y <= 5.0; y += 0.5)
				{
					double xOffset = sqrt(5.0 * 5.0 - y * y);

					CVector<3> vNormal(xDir * xOffset, y, 0.0);
					vNormal.Normalize();
					glNormal(vNormal);

					glVertex3d(xDir * (x + xOffset), y, z);
					glVertex3d(xDir * (x + xOffset), y, z + 2.0);
				}

				glNormal(CVector<3>(0.0, 1.0, 0.0));

				glVertex3d(xDir * (x - 0.1), 5.0, z);
				glVertex3d(xDir * (x - 0.1), 5.0, z + 2.0);

			glEnd();
		}
	}

	for (double zDir = -1.0; zDir <= 1.0; zDir += 2.0)
	{
		for (double x = -50.0; x < 50.0; x += 2.0)
		{
			double z = 50.0; // + b * sqrt(1.0 - x * x / (a * a));;

			glBegin(GL_QUAD_STRIP);

				glNormal(CVector<3>(0.0, -1.0, 0.0));

				glVertex3d(x,		-5.0, zDir * (z - 0.1));
				glVertex3d(x + 2.0, -5.0, zDir * (z - 0.1));

				for (double y = -5.0; y <= 5.0; y += 0.5)
				{
					double zOffset = sqrt(5.0 * 5.0 - y * y);

					CVector<3> vNormal(0.0, y, zDir * zOffset);
					vNormal.Normalize();
					glNormal(vNormal);

					glVertex3d(x,       y, zDir * (z + zOffset));
					glVertex3d(x + 2.0, y, zDir * (z + zOffset));
				}

				glNormal(CVector<3>(0.0, 1.0, 0.0));

				glVertex3d(x,		5.0, zDir * (z - 0.1));
				glVertex3d(x + 2.0, 5.0, zDir * (z - 0.1));

			glEnd();
		}
	}

	for (double x = -50.0; x <= 50.0; x += 100.0)
		for (double z = -50.0; z <= 50.0; z += 100.0)
		{
			glPushMatrix();

			glTranslated(x, 0, z);

			// draw a sphere
			GLUquadricObj *pQuad = gluNewQuadric();
			gluQuadricNormals(pQuad, GLU_SMOOTH);
			gluSphere(pQuad, 5.0, 20, 20);
			gluDeleteQuadric(pQuad);

			glPopMatrix();
		}

}
