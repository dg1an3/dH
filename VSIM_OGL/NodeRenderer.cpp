// NodeRenderer.cpp: implementation of the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeRenderer.h"

#include <gl/gl.h>
#include <gl/glu.h>

#include <glMatrixVector.h>


const int NUM_STEPS = 100;
const double RADIUS = 5.0;

class CMolding 
{
public:
	CMolding();
	CMolding(double theta, CVector<3> vOffset);

	static CMolding g_masterMolding;
	CVector<3> m_arrVertices[NUM_STEPS + 1];
	CVector<3> m_arrNormals[NUM_STEPS + 1];
};

CMolding CMolding::g_masterMolding;

CMolding::CMolding()
{
	int nAt = 0;
	const double step = (PI / 2) / (3 * NUM_STEPS / 4);
	for (double angle = 0; angle <= PI / 2; angle += step)
	{
		m_arrVertices[nAt][0] = cos(angle);
		m_arrVertices[nAt][2] = sin(angle);

		m_arrNormals[nAt][0] = cos(angle);
		m_arrNormals[nAt][2] = sin(angle);

		nAt++;
	}

	double x = 0.0;
	for (; nAt < NUM_STEPS + 1; nAt++)
	{
		m_arrVertices[nAt][0] = x;
		m_arrVertices[nAt][2] = 1.0;

		m_arrNormals[nAt][2] = 1.0;

		x -= 1.0 / (NUM_STEPS / 4);
	}
}

CMolding::CMolding(double theta, CVector<3> vOffset)
{
	CMatrix<4> mRotate = CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0));
	for (int nAt = 0; nAt < NUM_STEPS+1; nAt++)
	{
		m_arrNormals[nAt] = 
			FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrNormals[nAt]));

		m_arrVertices[nAt] = 
			RADIUS * FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrVertices[nAt]));

		m_arrVertices[nAt] += vOffset;
	}
}

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

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeRenderer::GetOuterRect()
{
	// outer rectangle = client rectangle
	// CRect rectOuter;
	// GetClientRect(&rectOuter);

	return m_rectOuter;
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeRenderer::GetInnerRect()
{
	CRect rectOuter = GetOuterRect();

	// compute the r, which represents the amount of "elliptical-ness"
	// float r = 1.0f - (1.0f - 1.0f / (float) sqrt(2.0f))
	// 	* (float) exp(-8.0f * privActivation.Get());

	CRect rectInner = rectOuter;
	rectInner.top = (long) ((float) (rectOuter.top + rectOuter.Height() / 2) 
		- m_r * (float) rectOuter.Height() / 2.0f);
	rectInner.bottom = (long) ((float) (rectOuter.top + rectOuter.Height() / 2) 
		+ m_r * (float) rectOuter.Height() / 2.0f);
	rectInner.left = (long) ((float) (rectOuter.left + rectOuter.Width() / 2) 
		- m_r * (float) rectOuter.Width() / 2.0f);
	rectInner.right = (long) ((float) (rectOuter.left + rectOuter.Width() / 2) 
		+ m_r * (float) rectOuter.Width() / 2.0f);

	return rectInner;
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetLeftRightEllipseRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeRenderer::GetLeftRightEllipseRect()
{
	CRect rectOuter = GetOuterRect();
	CRect rectInner = GetInnerRect();

	// compute the ellipse's b
	float rx = ((float) rectInner.Width())
			/ ((float) rectOuter.Width()); 
	float bx = (float) sqrt((float) (rectInner.Height() 
		* rectInner.Height()) * 0.25f
			/ (1.0f - rx * rx));

	// now compute the ellipse's rectangle
	CRect rectLeftRightEllipse = rectOuter;
	rectLeftRightEllipse.top = (rectOuter.top + rectOuter.Height() / 2) - (long) bx;
	rectLeftRightEllipse.bottom = (rectOuter.top + rectOuter.Height() / 2) + (long) bx;

	return rectLeftRightEllipse;
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetTopBottomEllipseRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeRenderer::GetTopBottomEllipseRect()
{
	CRect rectOuter = GetOuterRect();
	CRect rectInner = GetInnerRect();

	// compute the ellipse's b
	float ry = ((float) rectInner.Height())
			/ ((float) rectOuter.Height()); 
	float by = (float) sqrt((float) (rectInner.Width() * rectInner.Width()) * 0.25f
			/ (1.0f - ry * ry));

	// now compute the ellipse's rectangle
	CRect rectTopBottomEllipse = rectOuter;
	rectTopBottomEllipse.left = (rectOuter.left + rectOuter.Width() / 2) - (long) by;
	rectTopBottomEllipse.right = (rectOuter.left + rectOuter.Width() / 2) + (long) by;

	return rectTopBottomEllipse;
}


CVector<3> EvalShape2D(double theta)
{
	double a = 15.0;
	double b = 30.0;

	if (theta > PI / 4 && theta < 3 * PI / 4)
	{
		a = 30.0;
		b = 15.0;
	}

	if (theta > 5 * PI / 4 && theta < 7 * PI / 4)
	{
		a = 30.0;
		b = 15.0;
	}

	return CVector<3>(a * cos(theta), b * sin(theta), 0.0);
}

void CNodeRenderer::OnRenderScene()
{

	glColor(color.Get());

	// glTranslated(150.0, 100.0, 0.0);

	// draw an elliptangle
/*	glBegin(GL_QUADS);
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
*/
	double step = 0.05;
	CMolding *pOldMolding = new CMolding(0.0, EvalShape2D(0.0));
	for (double theta = 0.0; theta <= (2 * PI + step); theta += step)
	{
		// create the molding from the template
		CVector<3> vShapePt = EvalShape2D(theta);
		CMolding *pNewMolding = new CMolding(atan2(vShapePt[1], vShapePt[0]), vShapePt);

		for (int nAt = 0; nAt < NUM_STEPS; nAt++)
		{
			glBegin(GL_QUADS);

				glNormal(pOldMolding->m_arrNormals[nAt]);
				glVertex(pOldMolding->m_arrVertices[nAt]);

				glNormal(pOldMolding->m_arrNormals[nAt+1]);
				glVertex(pOldMolding->m_arrVertices[nAt+1]);

				glNormal(pNewMolding->m_arrNormals[nAt+1]);
				glVertex(pNewMolding->m_arrVertices[nAt+1]);

				glNormal(pNewMolding->m_arrNormals[nAt]);
				glVertex(pNewMolding->m_arrVertices[nAt]);

			glEnd();
		}

		delete pOldMolding;
		pOldMolding = pNewMolding;
	}

#ifdef NONE
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
#endif

}
