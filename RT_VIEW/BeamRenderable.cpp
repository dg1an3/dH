//////////////////////////////////////////////////////////////////////
// BeamRenderable.cpp: implementation of the CBeamRenderable 
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
#include "BeamRenderable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// DrawProjectedVertex
// 
// helper function to render the beam
//////////////////////////////////////////////////////////////////////
inline void DrawProjectedVertex(const CVector<2>& v)
{
	glVertex3d(v[0], v[1], -1.0);
	glVertex3d(v[0], v[1], 1.0);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::CBeamRenderable
// 
// a depiction of the treatment beam
//////////////////////////////////////////////////////////////////////
CBeamRenderable::CBeamRenderable()
	: m_bCentralAxisEnabled(TRUE),
		m_bGraticuleEnabled(FALSE),
		m_bFieldDivergenceSurfacesEnabled(FALSE),
		m_bBlockDivergenceSurfacesEnabled(TRUE)
{
	SetColor(RGB(0, 255, 0));
	SetAlpha(0.25);
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::~CBeamRenderable
// 
// destroys the renderable
//////////////////////////////////////////////////////////////////////
CBeamRenderable::~CBeamRenderable()
{
}

//////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE
// 
// implements the dynamic create behavior for the renderables
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CBeamRenderable, CRenderable);


//////////////////////////////////////////////////////////////////////
// CBeamRenderable::GetBeam
// 
// returns the beam being rendered
//////////////////////////////////////////////////////////////////////
CBeam *CBeamRenderable::GetBeam()
{
	return (CBeam *) GetObject();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::SetBeam
// 
// sets the beam being rendered
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::SetBeam(CBeam *pBeam)
{
	// set the beam object
	SetObject(pBeam);
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::SetBeam
// 
// over-ride to the set the object to a beam
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::SetObject(CObject *pObject)
{
	// ensure it is a beam
	ASSERT(pObject->IsKindOf(RUNTIME_CLASS(CBeam)));

	// remove this as an observer on the old beam
	if (NULL != GetBeam())
	{
		::RemoveObserver<CBeamRenderable>(&GetBeam()->GetChangeEvent(),
			this, OnBeamChanged);
	}

	// set the beam pointer
	CRenderable::SetObject(pObject);

	// add this as an observer on the new beam
	if (NULL != GetBeam())
	{
		::AddObserver<CBeamRenderable>(&GetBeam()->GetChangeEvent(),
			this, OnBeamChanged);

		// fire a change event to update
		OnBeamChanged(&GetBeam()->GetChangeEvent(), NULL);
	}
	else
	{
		// otherwise, just invalidate
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeOpaque
// 
// describes the beam itself
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeOpaque()
{
	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// set up the four corners of the collimator rectangle
	m_vMin = GetBeam()->GetCollimMin();
	m_vMax = GetBeam()->GetCollimMax();
	m_vMinXMaxY[0] = m_vMin[0];
	m_vMinXMaxY[1] = m_vMax[1];
	m_vMaxXMinY[0] = m_vMax[0];
	m_vMaxXMinY[1] = m_vMin[1];

	// set the color for the beam rendering
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	// first describe items on the collimator plane
	glPushMatrix();

		// translate to draw at SCD (-1.0 under the inverse projection)
		glTranslate(CVector<3>(0.0, 0.0, -1.0));

		// describe the graticule
		DescribeGraticule();

		// the field
		DescribeField();

		// and the blocks
		DescribeBlocks();

	glPopMatrix();

	glDisable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// describe the central axis
	DescribeCentralAxis();

	// describe field divergence as lines
	DescribeFieldDivergenceLines();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::OnBeamChanged
// 
// handles changes on the beam parameters
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::OnBeamChanged(CObservableEvent *pEvent, void *pOldValue)
{
	// check to make sure this is the right event
	if (pEvent == &GetBeam()->GetChangeEvent())
	{
		CMatrix<4> mProjInv(GetBeam()->GetTreatmentMachine()->GetProjection());
		mProjInv.Invert();

		// set up the renderable's modelview matrix
 		SetModelviewMatrix(GetBeam()->GetBeamToFixedXform()
 			* CMatrix<4>(CreateScale(CVector<3>(1.0, 1.0, -1.0)))
 			* mProjInv);

		// and re-render
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeGraticule
// 
// describes the graticule at the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeGraticule(double size)
{
	if (m_bGraticuleEnabled)
	{
		// now draw the graticule
		CVector<2> vPos;

		glBegin(GL_LINES);

			vPos = CVector<2>(0.0, 0.0);			
			glVertex(vPos);
			while (vPos[0] > m_vMin[0])
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
			while (vPos[0] < m_vMax[0])
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
			while (vPos[1] > m_vMin[1])
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
			while (vPos[1] < m_vMax[1])
			{
				vPos += CVector<2>(0.0, 1.0);
				glVertex(vPos);
				glVertex(vPos + CVector<2>(0.5, 0.0));
				glVertex(vPos - CVector<2>(0.5, 0.0));
				glVertex(vPos);
			}

		glEnd();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeField
// 
// describes the field on the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeField()
{
	glBegin(GL_LINE_LOOP);

		glVertex(m_vMin     ); 
		glVertex(m_vMinXMaxY); 
		glVertex(m_vMax     ); 
		glVertex(m_vMaxXMinY);

	glEnd();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeBlocks
// 
// describes the blocks on the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeBlocks()
{
	for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++)
	{
		CPolygon *pBlock = GetBeam()->GetBlockAt(nAt);

		glBegin(GL_LINE_LOOP);

			for (int nAtVert = 0; nAtVert < pBlock->GetVertexCount(); 
					nAtVert++)
			{
				// draw divergence lines
				glVertex(pBlock->GetVertex(nAtVert));
			}

		glEnd();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeCentralAxis
// 
// describes the central axis of the beam
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeCentralAxis()
{
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
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeFieldDivergenceLines
// 
// describes the divergence lines at the four corners of the field
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeFieldDivergenceLines()
{
	glBegin(GL_LINES);

		// draw divergence lines
		DrawProjectedVertex(m_vMin     );
		DrawProjectedVertex(m_vMinXMaxY);
		DrawProjectedVertex(m_vMax     );
		DrawProjectedVertex(m_vMaxXMinY);

	glEnd();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeAlpha
// 
// renders the beam surfaces
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeAlpha()
{
	// set up for surface rendering

	// Set the shading model
	glShadeModel(GL_FLAT);

	// glEnable(GL_BLEND);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// describe the field divergence surfaces
	DescribeFieldDivergenceSurfaces();

	// describe the block divergence surfaces
	DescribeBlockDivergenceSurfaces();

	// Set the shading model
	glShadeModel(GL_SMOOTH);
}


//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeFieldDivergenceSurfaces
// 
// describes the divergence surfaces for the field
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeFieldDivergenceSurfaces()
{
	if (m_bFieldDivergenceSurfacesEnabled)
	{
		glBegin(GL_QUAD_STRIP);
		
			// draw 
			DrawProjectedVertex(m_vMin     );
			DrawProjectedVertex(m_vMinXMaxY);
			DrawProjectedVertex(m_vMax     );
			DrawProjectedVertex(m_vMaxXMinY);
			DrawProjectedVertex(m_vMin     );

		glEnd();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DescribeBlockDivergenceSurfaces
// 
// describes the divergence surfaces for the block
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DescribeBlockDivergenceSurfaces()
{
	if (m_bBlockDivergenceSurfacesEnabled)
	{
		// draw the divergence surfaces
		glBegin(GL_QUAD_STRIP);
		
			for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++)
			{
				CPolygon *pPolygon = GetBeam()->GetBlockAt(nAt);

				for (int nAtVert = 0; nAtVert < pPolygon->GetVertexCount(); 
						nAtVert++)
				{
					// draw divergence lines
					DrawProjectedVertex(pPolygon->GetVertex(nAtVert)); 
				}
			}

		glEnd();
	}
}
