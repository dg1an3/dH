// BeamRenderer.cpp: implementation of the CBeamRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BeamRenderer.h"

#include "gl/gl.h"

#include "glMatrixVector.h"

#include "MatrixFunction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBeamRenderer::CBeamRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView),
		isCentralAxisEnabled(TRUE),
		isDivergenceSurfacesEnabled(TRUE),
		isGraticuleEnabled(FALSE),
		privBeamToPatientXform(&forBeam, &CBeam::beamToPatientXform),
		privMachineProjection(&forBeam, 
			(CValue< CMatrix<4> > CBeam::*) &CBeam::projection)
{
	forBeam.AddObserver(this, (ChangeFunction) OnChange);

	// set up the modelview matrix for the beam
	CValue< CMatrix<4> >& privModelviewMatrix =
		  privBeamToPatientXform 
		* CreateScale(CVector<3>(1.0, 1.0, -1.0))
		* Invert(privMachineProjection);
	modelviewMatrix.SyncTo(&privModelviewMatrix);
}

CBeamRenderer::~CBeamRenderer()
{

}

CBeam *CBeamRenderer::GetBeam()
{
	return forBeam.Get();
}

void CBeamRenderer::SetBeam(CBeam *pBeam)
{
	if (forBeam.Get() != NULL)
		forBeam->RemoveObserver(this, (ChangeFunction) OnChange);

	forBeam.Set(pBeam);

	if (forBeam.Get() != NULL)
	{
		forBeam->AddObserver(this, (ChangeFunction) OnChange);
		CValue< CMatrix<4> > *pProjValue = 
			(CValue< CMatrix<4> > *)&forBeam->forMachine->projection;
		privMachineProjection.SyncTo(pProjValue);
	}

	Invalidate();
}

inline void DrawProjectedVertex(const CVector<2>& v)
{
	glVertex3d(v[0], v[1], -1.0);
	glVertex3d(v[0], v[1], 1.0);
}

void CBeamRenderer::OnRenderScene()
{
	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	if (isCentralAxisEnabled.Get())
	{
		glColor3f(0.0f, 1.0f, 0.0f);

		// draw central axis
		glBegin(GL_LINES);

			// draw central axis
			glVertex3d(0.0, 0.0, -1.0);
			glVertex3d(0.0, 0.0,  1.0);

		glEnd();
	}

	// set up the four corners of the collimator rectangle
	const CVector<2> vMin = forBeam->collimMin.Get();
	const CVector<2> vMax = forBeam->collimMax.Get();

	const CVector<2> vMinXMaxY(vMin[0], vMax[1]);
	const CVector<2> vMaxXMinY(vMax[0], vMin[1]);

	// set the color for the beam rendering
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	glPushMatrix();

		// translate to draw at SCD (-1.0 under the inverse projection)
		glTranslate(CVector<3>(0.0, 0.0, -1.0));

		glBegin(GL_LINE_LOOP);

			glVertex(vMin     ); 
			glVertex(vMinXMaxY); 
			glVertex(vMax     ); 
			glVertex(vMaxXMinY);

		glEnd();

		if (isGraticuleEnabled.Get())
		{
			// now draw the graticule
			CVector<2> vPos;

			glBegin(GL_LINES);

				vPos = CVector<2>(0.0, 0.0);			
				glVertex(vPos);
				while (vPos[0] > vMin[0])
				{
					vPos -= CVector<2>(1.0, 0.0);
					glVertex(vPos);
					glVertex(vPos + CVector<2>(0.0, 0.5));
					glVertex(vPos - CVector<2>(0.0, 0.5));
					glVertex(vPos);
				}

			glEnd();

			glBegin(GL_LINES);

				vPos = CVector<2>(0.0, 0.0);
				glVertex(vPos);
				while (vPos[0] < vMax[0])
				{
					vPos += CVector<2>(1.0, 0.0);
					glVertex(vPos);
					glVertex(vPos + CVector<2>(0.0, 0.5));
					glVertex(vPos - CVector<2>(0.0, 0.5));
					glVertex(vPos);
				}

			glEnd();

			glBegin(GL_LINES);

				vPos = CVector<2>(0.0, 0.0);
				glVertex(vPos);
				while (vPos[1] > vMin[1])
				{
					vPos -= CVector<2>(0.0, 1.0);
					glVertex(vPos);
					glVertex(vPos + CVector<2>(0.5, 0.0));
					glVertex(vPos - CVector<2>(0.5, 0.0));
					glVertex(vPos);
				}

			glEnd();

			glBegin(GL_LINES);

				vPos = CVector<2>(0.0, 0.0);
				glVertex(vPos);
				while (vPos[1] < vMax[1])
				{
					vPos += CVector<2>(0.0, 1.0);
					glVertex(vPos);
					glVertex(vPos + CVector<2>(0.5, 0.0));
					glVertex(vPos - CVector<2>(0.5, 0.0));
					glVertex(vPos);
				}

			glEnd();
		}

		for (int nAt = 0; nAt < forBeam->blocks.GetSize(); nAt++)
		{
			glBegin(GL_LINE_LOOP);

				CPolygon& polygon = *forBeam->blocks.Get(nAt);

				for (int nAtVert = 0; nAtVert < polygon.GetVertexCount(); 
						nAtVert++)
				{
					// draw divergence lines
					glVertex(polygon.GetVertex(nAtVert));
				}

			glEnd();
		}

	glPopMatrix();

	glBegin(GL_LINES);

		// draw divergence lines
		DrawProjectedVertex(vMin     );
		DrawProjectedVertex(vMinXMaxY);
		DrawProjectedVertex(vMax     );
		DrawProjectedVertex(vMaxXMinY);

	glEnd();

	if (isDivergenceSurfacesEnabled.Get())
	{
		// Create a Directional Light Source
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

		// make the depth mask read-only
		glDepthMask(GL_FALSE);

		// glEnable(GL_BLEND);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glShadeModel(GL_FLAT);

		glColor4f(0.0f, 1.0f, 0.0f, 0.25f);

		// draw the divergence surfaces
		glBegin(GL_QUAD_STRIP);
		
			for (int nAt = 0; nAt < forBeam->blocks.GetSize(); nAt++)
			{
				CPolygon& polygon = *forBeam->blocks.Get(nAt);

				for (int nAtVert = 0; nAtVert < polygon.GetVertexCount(); 
						nAtVert++)
				{
					// draw divergence lines
					DrawProjectedVertex(polygon.GetVertex(nAtVert)); 
				}
			}

		glEnd();

#ifdef DRAW_COLLIMATOR_DIV
		glBegin(GL_QUAD_STRIP);
		
			// draw 
			DrawProjectedVertex(vMin     );
			DrawProjectedVertex(vMinXMaxY);
			DrawProjectedVertex(vMax     );
			DrawProjectedVertex(vMaxXMinY);
			DrawProjectedVertex(vMin     );

		glEnd();
#endif

		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_DEPTH_TEST);

		// Set the shading model
		glShadeModel(GL_SMOOTH);

		// Set the polygon mode to fill

		// Create a Directional Light Source
		// glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		// glEnable(GL_BLEND);
		glDisable(GL_BLEND);

		// make the depth mask read-only
		glDepthMask(GL_TRUE);
	}

	glEnable(GL_LIGHTING);
}
