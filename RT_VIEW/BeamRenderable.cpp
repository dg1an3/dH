//////////////////////////////////////////////////////////////////////
// BeamRenderable.cpp: implementation of the CBeamRenderable 
//		class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// render context
#include <RenderContext.h>

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
inline void DrawProjectedVertex(CRenderContext *pRC, 
								const CVector<2>& v)
{
	pRC->Vertex(CVector<3>(v[0], v[1], -1.0));
	pRC->Vertex(CVector<3>(v[0], v[1],  1.0));
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
// CBeamRenderable::DrawOpaque
// 
// Draws the beam itself
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawOpaque(CRenderContext *pRC)
{
	// set up for line rendering
	pRC->SetupLines();

	// set up the four corners of the collimator rectangle
	m_vMin = GetBeam()->GetCollimMin();
	m_vMax = GetBeam()->GetCollimMax();
	m_vMinXMaxY[0] = m_vMin[0];
	m_vMinXMaxY[1] = m_vMax[1];
	m_vMaxXMinY[0] = m_vMax[0];
	m_vMaxXMinY[1] = m_vMin[1];

	// first Draw items on the collimator plane
	pRC->PushMatrix();

		// translate to draw at SCD (-1.0 under the inverse projection)
		pRC->Translate(CVector<3>(0.0, 0.0, -1.0));

		// Draw the graticule
		DrawGraticule(pRC);

		// the field
		DrawField(pRC);

		// and the blocks
		DrawBlocks(pRC);

	pRC->PopMatrix();

	// draw items on SID plane
	pRC->PushMatrix();

		// translate to draw at SID (1.0 under the inverse projection)
		pRC->Translate(CVector<3>(0.0, 0.0, 1.0));

		// the field
		DrawField(pRC);

		// and the blocks
		DrawBlocks(pRC);

	pRC->PopMatrix();

	// Draw the central axis
	DrawCentralAxis(pRC);

	// Draw field divergence as lines
	DrawFieldDivergenceLines(pRC);

	// restore opaque surface drawing.  TODO: fix this
	pRC->SetupOpaqueSurface();
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
// CBeamRenderable::DrawGraticule
// 
// Draws the graticule at the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawGraticule(CRenderContext *pRC, double size)
{
	if (m_bGraticuleEnabled)
	{
		// now draw the graticule
		CVector<2> vPos;

		pRC->BeginLines();

			vPos = CVector<2>(0.0, 0.0);			
			pRC->Vertex(vPos);
			while (vPos[0] > m_vMin[0])
			{
				vPos -= CVector<2>(1.0, 0.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVector<2>(0.0, 0.5));
				pRC->Vertex(vPos - CVector<2>(0.0, 0.5));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVector<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[0] < m_vMax[0])
			{
				vPos += CVector<2>(1.0, 0.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVector<2>(0.0, 0.5));
				pRC->Vertex(vPos - CVector<2>(0.0, 0.5));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVector<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[1] > m_vMin[1])
			{
				vPos -= CVector<2>(0.0, 1.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVector<2>(0.5, 0.0));
				pRC->Vertex(vPos - CVector<2>(0.5, 0.0));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVector<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[1] < m_vMax[1])
			{
				vPos += CVector<2>(0.0, 1.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVector<2>(0.5, 0.0));
				pRC->Vertex(vPos - CVector<2>(0.5, 0.0));
				pRC->Vertex(vPos);
			}

		pRC->End();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawField
// 
// Draws the field on the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawField(CRenderContext *pRC)
{
	pRC->BeginLineLoop();

		pRC->Vertex(m_vMin     ); 
		pRC->Vertex(m_vMinXMaxY); 
		pRC->Vertex(m_vMax     ); 
		pRC->Vertex(m_vMaxXMinY);

	pRC->End();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawBlocks
// 
// Draws the blocks on the collimator plane
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawBlocks(CRenderContext *pRC)
{
	for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++)
	{
		CPolygon *pBlock = GetBeam()->GetBlockAt(nAt);

		pRC->BeginLineLoop();

			for (int nAtVert = 0; nAtVert < pBlock->GetVertexCount(); 
					nAtVert++)
			{
				// draw divergence lines
				pRC->Vertex(pBlock->GetVertex(nAtVert));
			}

		pRC->End();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawCentralAxis
// 
// Draws the central axis of the beam
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawCentralAxis(CRenderContext *pRC)
{
	if (m_bCentralAxisEnabled)
	{
		pRC->Color(RGB(0, 255, 0));

		// draw central axis
		pRC->BeginLines();

			// draw central axis
			pRC->Vertex(CVector<3>(0.0, 0.0, -1.0));
			pRC->Vertex(CVector<3>(0.0, 0.0,  1.0));

		pRC->End();

		// draw isocenter
		pRC->BeginLines();

			pRC->Vertex(CVector<3>(-5.0, 0.0, 0.5));
			pRC->Vertex(CVector<3>( 5.0, 0.0, 0.5));

			pRC->Vertex(CVector<3>(0.0, -5.0, 0.5));
			pRC->Vertex(CVector<3>(0.0,  5.0, 0.5));

		pRC->End();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawFieldDivergenceLines
// 
// Draws the divergence lines at the four corners of the field
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawFieldDivergenceLines(CRenderContext *pRC)
{
	pRC->BeginLines();

		// draw divergence lines
		DrawProjectedVertex(pRC, m_vMin     );
		DrawProjectedVertex(pRC, m_vMinXMaxY);
		DrawProjectedVertex(pRC, m_vMax     );
		DrawProjectedVertex(pRC, m_vMaxXMinY);

	pRC->End();
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawTransparent
// 
// renders the beam surfaces
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawTransparent(CRenderContext *pRC)
{
	// set up for beam surface rendering
	pRC->SetSmoothShading(FALSE);
	pRC->SetLighting(FALSE);

	// Draw the field divergence surfaces
	DrawFieldDivergenceSurfaces(pRC);

	// Draw the block divergence surfaces
	DrawBlockDivergenceSurfaces(pRC);

	// restore surface rendering settings
	pRC->SetSmoothShading(TRUE);
	pRC->SetLighting(TRUE);
}


//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawFieldDivergenceSurfaces
// 
// Draws the divergence surfaces for the field
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawFieldDivergenceSurfaces(CRenderContext *pRC)
{
	if (m_bFieldDivergenceSurfacesEnabled)
	{
		pRC->BeginQuadStrip();
		
			// draw 
			DrawProjectedVertex(pRC, m_vMin     );
			DrawProjectedVertex(pRC, m_vMinXMaxY);
			DrawProjectedVertex(pRC, m_vMax     );
			DrawProjectedVertex(pRC, m_vMaxXMinY);
			DrawProjectedVertex(pRC, m_vMin     );

		pRC->End();
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawBlockDivergenceSurfaces
// 
// Draws the divergence surfaces for the block
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawBlockDivergenceSurfaces(CRenderContext *pRC)
{
	if (m_bBlockDivergenceSurfacesEnabled)
	{
		// draw the divergence surfaces
		pRC->BeginQuadStrip();
		
			for (int nAt = 0; nAt < GetBeam()->GetBlockCount(); nAt++)
			{
				CPolygon *pPolygon = GetBeam()->GetBlockAt(nAt);

				for (int nAtVert = 0; nAtVert < pPolygon->GetVertexCount(); 
						nAtVert++)
				{
					// draw divergence lines
					DrawProjectedVertex(pRC, pPolygon->GetVertex(nAtVert)); 
				}
			}

		pRC->End();
	}
}
