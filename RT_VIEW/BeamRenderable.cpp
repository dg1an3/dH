//////////////////////////////////////////////////////////////////////
// BeamRenderable.cpp: implementation of the CBeamRenderable 
//		class.
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// extra inline functions for matrix
#include <MatrixBase.inl>

// render context
#include <RenderContext.h>

#include <SceneView.h>

// class declaration
#include "BeamRenderable.h"

#include "Results.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

typedef WORD SimplexIndex[3];

//////////////////////////////////////////////////////////////////////
// DrawProjectedVertex
// 
// helper function to render the beam
//////////////////////////////////////////////////////////////////////
inline void DrawProjectedVertex(CRenderContext *pRC, 
								const CVectorD<2>& v)
{
	pRC->Vertex(CVectorD<3>(v[0], v[1], -1.0));
	pRC->Vertex(CVectorD<3>(v[0], v[1],  1.0));
}

void GenerateTriangles(double *pVert2D, 
			   const CMatrixD<4>& mBeamXform, 
			   CUSTOMVERTEX_POS_NORM *pVertices, int nAtVert,
			   SimplexIndex *pIndices)
{
	CVectorD<3> vVert3D;
	vVert3D[0] = pVert2D[0];
	vVert3D[1] = pVert2D[1];
	vVert3D[2] = -1.0;
	CVectorD<3> vVertXform = 
		FromHG<3, REAL>(mBeamXform * ToHG<3, REAL>(vVert3D));

	pVertices[nAtVert+0].position.x = vVertXform[0];
	pVertices[nAtVert+0].position.y = vVertXform[1];
	pVertices[nAtVert+0].position.z = vVertXform[2];
	pVertices[nAtVert+0].normal.x = 0.0;
	pVertices[nAtVert+0].normal.y = 0.0;
	pVertices[nAtVert+0].normal.z = 0.0;

	vVert3D[2] = 1.0;
	vVertXform = 
		FromHG<3, REAL>(mBeamXform * ToHG<3, REAL>(vVert3D));

	pVertices[nAtVert+1].position.x = vVertXform[0];
	pVertices[nAtVert+1].position.y = vVertXform[1];
	pVertices[nAtVert+1].position.z = vVertXform[2];
	pVertices[nAtVert+1].normal.x = 0.0;
	pVertices[nAtVert+1].normal.y = 0.0;
	pVertices[nAtVert+1].normal.z = 0.0;

	if (NULL != pIndices)
	{
		pIndices[0][0] = nAtVert-2;
		pIndices[0][1] = nAtVert-1;
		pIndices[0][2] = nAtVert+0;

		pIndices[1][0] = nAtVert+0;
		pIndices[1][1] = nAtVert-1;
		pIndices[1][2] = nAtVert+1;
	}
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
		m_bBlockDivergenceSurfacesEnabled(TRUE),
		m_pBlockDivSurfMesh(NULL)
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
	if (m_pBlockDivSurfMesh)
	{
		m_pBlockDivSurfMesh->Release();
	}
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
void CBeamRenderable::DrawOpaque(LPDIRECT3DDEVICE8 pd3dDev)
{
/*	// set up for line rendering
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
		pRC->Translate(CVectorD<3>(0.0, 0.0, -1.0));

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
		pRC->Translate(CVectorD<3>(0.0, 0.0, 1.0));

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
	pRC->SetupOpaqueSurface(); */
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
		CMatrixD<4> mProjInv(GetBeam()->GetTreatmentMachine()->GetProjection());
		mProjInv.Invert();

		// set up the renderable's modelview matrix
 		SetModelviewMatrix(GetBeam()->GetBeamToFixedXform()
 			* CMatrixD<4>(CreateScale(CVectorD<3>(1.0, 1.0, -1.0)))
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
		CVectorD<2> vPos;

		pRC->BeginLines();

			vPos = CVectorD<2>(0.0, 0.0);			
			pRC->Vertex(vPos);
			while (vPos[0] > m_vMin[0])
			{
				vPos -= CVectorD<2>(1.0, 0.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVectorD<2>(0.0, 0.5));
				pRC->Vertex(vPos - CVectorD<2>(0.0, 0.5));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVectorD<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[0] < m_vMax[0])
			{
				vPos += CVectorD<2>(1.0, 0.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVectorD<2>(0.0, 0.5));
				pRC->Vertex(vPos - CVectorD<2>(0.0, 0.5));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVectorD<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[1] > m_vMin[1])
			{
				vPos -= CVectorD<2>(0.0, 1.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVectorD<2>(0.5, 0.0));
				pRC->Vertex(vPos - CVectorD<2>(0.5, 0.0));
				pRC->Vertex(vPos);
			}

		pRC->End();

		pRC->BeginLines();

			vPos = CVectorD<2>(0.0, 0.0);
			pRC->Vertex(vPos);
			while (vPos[1] < m_vMax[1])
			{
				vPos += CVectorD<2>(0.0, 1.0);
				pRC->Vertex(vPos);
				pRC->Vertex(vPos + CVectorD<2>(0.5, 0.0));
				pRC->Vertex(vPos - CVectorD<2>(0.5, 0.0));
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
		CPolygon *pBlock = GetBeam()->GetBlock(nAt);

		pRC->BeginLineLoop();

			for (int nAtVert = 0; nAtVert < pBlock->GetVertexCount(); 
					nAtVert++)
			{
				// draw divergence lines
				pRC->Vertex(pBlock->GetVertexAt(nAtVert));
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
			pRC->Vertex(CVectorD<3>(0.0, 0.0, -1.0));
			pRC->Vertex(CVectorD<3>(0.0, 0.0,  1.0));

		pRC->End();

		// draw isocenter
		pRC->BeginLines();

			pRC->Vertex(CVectorD<3>(-5.0, 0.0, 0.5));
			pRC->Vertex(CVectorD<3>( 5.0, 0.0, 0.5));

			pRC->Vertex(CVectorD<3>(0.0, -5.0, 0.5));
			pRC->Vertex(CVectorD<3>(0.0,  5.0, 0.5));

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
void CBeamRenderable::DrawTransparent(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC);
{
	// set up for beam surface rendering
//	pRC->SetSmoothShading(FALSE);
//	pRC->SetLighting(FALSE);

	// Draw the field divergence surfaces
//	DrawFieldDivergenceSurfaces(pRC);

	ASSERT_HRESULT(pd3dDev->SetRenderState(D3DRS_SHADEMODE , D3DSHADE_FLAT) );

	// Draw the block divergence surfaces
	DrawBlockDivergenceSurfaces(pd3dDev); // pRC);

	ASSERT_HRESULT(pd3dDev->SetRenderState(D3DRS_SHADEMODE , D3DSHADE_GOURAUD ) );

	// restore surface rendering settings
//	pRC->SetSmoothShading(TRUE);
//	pRC->SetLighting(TRUE);
}


//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawFieldDivergenceSurfaces
// 
// Draws the divergence surfaces for the field
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawFieldDivergenceSurfaces(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC);
{
	if (m_bFieldDivergenceSurfacesEnabled)
	{
/*		pRC->BeginQuadStrip();
		
			// draw 
			DrawProjectedVertex(pRC, m_vMin     );
			DrawProjectedVertex(pRC, m_vMinXMaxY);
			DrawProjectedVertex(pRC, m_vMax     );
			DrawProjectedVertex(pRC, m_vMaxXMinY);
			DrawProjectedVertex(pRC, m_vMin     );

		pRC->End(); */
	}
}

//////////////////////////////////////////////////////////////////////
// CBeamRenderable::DrawBlockDivergenceSurfaces
// 
// Draws the divergence surfaces for the block
//////////////////////////////////////////////////////////////////////
void CBeamRenderable::DrawBlockDivergenceSurfaces(LPDIRECT3DDEVICE8 pd3dDev) 
{
	if (m_bBlockDivergenceSurfacesEnabled)
	{
		if (NULL == m_pBlockDivSurfMesh)
		{
			CPolygon *pBlock = GetBeam()->GetBlock(0);
			// IPolygon3D *pIBlock = NULL;
			// ASSERT_HRESULT(GetBeam()->get_Block(0)->QueryInterface(&pIBlock));

			// CComVariant varVert;
			// pIBlock->get_Vertices(&varVert);
			// double *pBlockVert = NULL;
			//ASSERT_HRESULT(
			//	::SafeArrayAccessData(*varVert.pparray, (void **) &pBlockVert));

			long nVertCount = pBlock->GetVertexCount();
			// ASSERT_HRESULT(pIBlock->get_VertexCount(&nVertCount));

			m_pBlockDivSurfMesh = m_pView->CreateMesh(nVertCount * 2, 
				nVertCount * 2, D3DFVF_CUSTOMVERTEX_POS_NORM);

			SimplexIndex *pIndices = NULL;
			ASSERT_HRESULT( m_pBlockDivSurfMesh->LockIndexBuffer(0, (BYTE **) &pIndices) ); 
	
			CUSTOMVERTEX_POS_NORM *pVertices = NULL;
			ASSERT_HRESULT( m_pBlockDivSurfMesh->LockVertexBuffer(0, (BYTE **) &pVertices) );

			CMatrixD<4> mProjInv(GetBeam()->GetTreatmentMachine()->GetProjection());
			mProjInv.Invert();

			// set up the renderable's modelview matrix
 			CMatrixD<4> mBeamXform = GetBeam()->GetBeamToFixedXform()
 				* CMatrixD<4>(CreateScale(CVectorD<3>(1.0, 1.0, -1.0)))
 				* mProjInv;

			GenerateTriangles(&pBlock->LockVertexMatrix()[0][0], mBeamXform,
				pVertices, 0, NULL); 
			for (int nAt = 1; nAt < nVertCount; nAt++)
			{
				GenerateTriangles(&pBlock->LockVertexMatrix()[nAt][0], mBeamXform,
					pVertices, nAt*2, 
					&pIndices[nAt*2]);
			}
			ASSERT_HRESULT( m_pBlockDivSurfMesh->UnlockIndexBuffer() );
			ASSERT_HRESULT( m_pBlockDivSurfMesh->UnlockVertexBuffer() );
	
			// ASSERT_HRESULT(::SafeArrayUnaccessData(*varVert.pparray));

			// pIBlock->Release();
		}

		m_pBlockDivSurfMesh->DrawSubset(0);
	}
}
