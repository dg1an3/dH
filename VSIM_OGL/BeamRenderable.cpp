// BeamRenderable.cpp: implementation of the CBeamRenderable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BeamRenderable.h"

#include <glMatrixVector.h>

// #include <ScalarFunction.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
CMatrix<4> CreateScaleHG(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<3> mScale = CreateScale(vScale);

	CMatrix<4> mScaleHG(mScale);

	return mScaleHG;
}

//////////////////////////////////////////////////////////////////////
// function CreateRotate
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
CMatrix<4> CreateRotateHG(const double& theta, 
							   const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<3> mRotate = CreateRotate(theta, vAxis);

	CMatrix<4> mRotateHG(mRotate);

	return mRotateHG;
}

//////////////////////////////////////////////////////////////////////
// declare function factories for matrix inversion
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// function Invert
//
// stand-alone matrix inversion
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
inline CMatrix<DIM, TYPE> Invert(const CMatrix<DIM, TYPE>& m)
{
	CMatrix<DIM, TYPE> inv(m);
	inv.Invert();
	return inv;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBeamRenderable::CBeamRenderable(CSceneView *pView)
	: CRenderable(pView),
		m_bCentralAxisEnabled(TRUE),
		m_bDivergenceSurfacesEnabled(TRUE),
		m_bGraticuleEnabled(FALSE)
		// privBeamToPatientXform(&m_pBeam, &CBeam::beamToPatientXform),
		// privMachineProjection(&m_pBeam, 
		// 	(CValue< CMatrix<4> > CBeam::*) &CBeam::projection)
{
	// m_pBeam.AddObserver(this, (ChangeFunction) OnChange);

	// set up the modelview matrix for the beam
/*	CValue< CMatrix<4> >& privModelviewMatrix =
		  privBeamToPatientXform 
		* CreateScaleHG(CVector<3>(1.0, 1.0, -1.0))
		* Invert(privMachineProjection);
	modelviewMatrix.SyncTo(&privModelviewMatrix); */
}

CBeamRenderable::~CBeamRenderable()
{

}

void CBeamRenderable::SetREVBeam()
{
/*	modelviewMatrix.SyncTo(&(
		  CreateRotateHG(PI - m_pBeam->gantryAngle,	CVector<3>(0.0, -1.0, 0.0))
		* CreateRotateHG(m_pBeam->collimAngle,		CVector<3>(0.0, 0.0, -1.0))
		* CreateTranslate(m_pBeam->SAD,				CVector<3>(0.0, 0.0, -1.0))	
		* CreateScaleHG(CVector<3>(1.0, 1.0, -1.0))
		* Invert(privMachineProjection)
	)); */
}


CBeam *CBeamRenderable::GetBeam()
{
	return m_pBeam;
}

void CBeamRenderable::SetBeam(CBeam *pBeam)
{
/*	if (m_pBeam != NULL)
		m_pBeam->GetChangeEvent().RemoveObserver(this, (ChangeFunction) OnChange);
*/
	m_pBeam = pBeam;

	if (NULL != m_pBeam)
	{
		// set up the modelview matrix for the beam
		SetModelviewMatrix(
			m_pBeam->GetBeamToPatientXform()
			* CreateScaleHG(CVector<3>(1.0, 1.0, -1.0))
			* Invert(m_pBeam->GetTreatmentMachine()->m_projection));
		// m_pBeam->AddObserver(this, (ChangeFunction) OnChange);

		// CValue< CMatrix<4> > *pProjValue = 
		//	(CValue< CMatrix<4> > *)&m_pBeam->forMachine->projection;
		// privMachineProjection.SyncTo(pProjValue);
	}

	Invalidate();
}

inline void DrawProjectedVertex(const CVector<2>& v)
{
	glVertex3d(v[0], v[1], -1.0);
	glVertex3d(v[0], v[1], 1.0);
}

void CBeamRenderable::DescribeOpaque()
{
	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	if (m_bCentralAxisEnabled)
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
	const CVector<2>& vMin = m_pBeam->GetCollimMin();
	const CVector<2>& vMax = m_pBeam->GetCollimMax();

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

		if (m_bGraticuleEnabled)
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

		for (int nAt = 0; nAt < m_pBeam->GetBlockCount(); nAt++)
		{
			glBegin(GL_LINE_LOOP);

				CPolygon& polygon = *m_pBeam->GetBlockAt(nAt);

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

	if (m_bDivergenceSurfacesEnabled)
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
		
			for (int nAt = 0; nAt < m_pBeam->GetBlockCount(); nAt++)
			{
				CPolygon& polygon = *m_pBeam->GetBlockAt(nAt);

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
