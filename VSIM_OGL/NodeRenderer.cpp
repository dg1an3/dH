// NodeRenderer.cpp: implementation of the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeRenderer.h"

#include <gl/gl.h>
#include <gl/glu.h>

#include <glMatrixVector.h>


const int NUM_STEPS = 4;

class CMolding 
{
public:
	CMolding();
	CMolding(double theta, CVector<3> vOffset, double radius);

	CVector<3> m_arrVertices[NUM_STEPS + 1];
	CVector<3> m_arrNormals[NUM_STEPS + 1];

	static CMolding g_masterMolding;
};

CMolding CMolding::g_masterMolding;

CMolding::CMolding()
{
	int nAt = 0;
	const double step = (PI / 2) / (NUM_STEPS - 1);
	for (double angle = 0; angle <= PI / 2; angle += step)
	{
		m_arrVertices[nAt][0] = cos(angle);
		m_arrVertices[nAt][2] = sin(angle);

		m_arrNormals[nAt][0] = cos(angle);
		m_arrNormals[nAt][2] = sin(angle);

		nAt++;
	}

	m_arrVertices[nAt][0] = -4.5;
	m_arrVertices[nAt][2] = 1.0;

	m_arrNormals[nAt][2] = 1.0;
}

CMolding::CMolding(double theta, CVector<3> vOffset, double radius)
{
	CMatrix<4> mRotate = CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0));
	for (int nAt = 0; nAt < NUM_STEPS+1; nAt++)
	{
		m_arrNormals[nAt] = 
			FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrNormals[nAt]));

		m_arrVertices[nAt] = 
			radius * FromHomogeneous<3, double>(mRotate * ToHomogeneous(
				g_masterMolding.m_arrVertices[nAt]));

		m_arrVertices[nAt] += vOffset;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeRenderer::CNodeRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView),
		borderRadius(10.0),
		rectangularity(1.0 / sqrt(2.0)),
		width(25.0),
		aspectRatio(0.625)
		// color(RGB(192, 192, 192))
{
	color.Set(RGB(192, 192, 192));

	activation.AddObserver(this, (ChangeFunction) OnActivationChanged);
	rectangularity.AddObserver(this, (ChangeFunction) OnChange);
	width.AddObserver(this, (ChangeFunction) OnChange);
}

CNodeRenderer::~CNodeRenderer()
{

}


CVector<3> CNodeRenderer::EvalEllipseMolding(double theta, double a, double b)
{
	// compute theta_prime for the ellipse
	double sign = cos(theta) > 0 ? 1.0 : -1.0;
	double theta_prime = atan2(sign * a * tan(theta), sign * b);

	return CVector<3>(a * cos(theta_prime), b * sin(theta_prime), 0.0);

}

CMolding *CNodeRenderer::RenderMoldingSection(double angle_start, double angle_stop,
										 double a, double b, CMolding *pOldMolding)
{
	if (pOldMolding == NULL)
		pOldMolding = 
			new CMolding(angle_start, EvalEllipseMolding(angle_start, a, b), borderRadius.Get());

	double step = (angle_stop - angle_start) / 8.0;
	for (double theta = angle_start; theta <= angle_stop; theta += step)
	{
		// create the molding from the template
		CVector<3> vShapePt = EvalEllipseMolding(theta, a, b);

		// create a new molding object
		CMolding *pNewMolding = new CMolding(theta, vShapePt, borderRadius.Get());

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

	return pOldMolding;
}

void CNodeRenderer::OnRenderScene()
{
	// set the color for the node
	glColor(color.Get());

	// compute the base ellipse a and b
	double a = width.Get();
	double b = a * aspectRatio.Get();

	// draw an elliptangle
	glBegin(GL_QUADS);
		glNormal3d(  0.0, 0.0,   1.0);
		glVertex3d(-a * rectangularity.Get(), -b * rectangularity.Get(), borderRadius.Get());
		glVertex3d( a * rectangularity.Get(), -b * rectangularity.Get(), borderRadius.Get());
		glVertex3d( a * rectangularity.Get(),  b * rectangularity.Get(), borderRadius.Get());
		glVertex3d(-a * rectangularity.Get(),  b * rectangularity.Get(), borderRadius.Get());
	glEnd();

	// compute the angle dividing the sections of the elliptangle
	double sectionAngle = atan2(b, a);

	// computing the a and b adjustments for the sections of the elliptangle
	double r_factor = rectangularity.Get()
		/ sqrt(1.0 - rectangularity.Get() * rectangularity.Get());

	// now render the four sections of the molding
	CMolding* pOld;
	pOld = RenderMoldingSection(2*PI,                2*PI + sectionAngle, 
		a, b * r_factor);
	pOld = RenderMoldingSection(       sectionAngle,   PI - sectionAngle, 
		a * r_factor, b, pOld);
	pOld = RenderMoldingSection(  PI - sectionAngle,   PI + sectionAngle, 
		a, b * r_factor, pOld);
	pOld = RenderMoldingSection(  PI + sectionAngle, 2*PI - sectionAngle, 
		a * r_factor, b, pOld);
	pOld = RenderMoldingSection(2*PI - sectionAngle, 2*PI + 0.1, 
		a, b * r_factor, pOld);

	// delete the connecting molding
	delete pOld;
}

void CNodeRenderer::OnActivationChanged(CObservableObject *pSource, void *pOldValue)
{
	// set the new width for the node renderer
	width.Set(50.0 + 200.0 * sqrt(activation.Get() / aspectRatio.Get()));

	// set the new rectangularity for the shape
	const double c = 0.88 - 1.0 / sqrt(2.0);
	rectangularity.Set((activation.Get() * c) + 1.0 / sqrt(2.0));

	// set the aspect ratio for the renderer
	aspectRatio.Set(0.75 - 0.375 * exp(-8.0 * activation.Get()));

	// set the new border radius
	borderRadius.Set(8.0 + sqrt(activation.Get()) * 10.0);
}
