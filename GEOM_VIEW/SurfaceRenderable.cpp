//////////////////////////////////////////////////////////////////////
// SurfaceRenderable.cpp: declaration of the CSurfaceRenderable class
//
// Copyright (C) 2000-2002 Derek G. Lane
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "stdafx.h"

// floating-point constants
#include <float.h>

// render context for rendering
#include "RenderContext.h"

// class declaration
#include "SurfaceRenderable.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::CSurfaceRenderable
// 
// 
//////////////////////////////////////////////////////////////////////
CSurfaceRenderable::CSurfaceRenderable()
	: m_pTexture(NULL),
		m_bWireFrame(FALSE),
		m_bShowBoundsSurface(FALSE)
{
	SetColor(RGB(192, 192, 192));
	SetAlpha(0.5);
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::~CSurfaceRenderable
// 
// 
//////////////////////////////////////////////////////////////////////
CSurfaceRenderable::~CSurfaceRenderable()
{
	if (m_pTexture)
	{
		delete m_pTexture;
		m_pTexture = NULL;
	}	
}

//////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE
// 
// implements the dynamic create behavior for the renderables
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSurfaceRenderable, CRenderable);

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::GetSurface
// 
// 
//////////////////////////////////////////////////////////////////////
CSurface * CSurfaceRenderable::GetSurface()
{
	return (CSurface *)GetObject();
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::SetSurface
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::SetSurface(CSurface *pSurface)
{
	SetObject(pSurface);
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::GetTexture
// 
// 
//////////////////////////////////////////////////////////////////////
CTexture * CSurfaceRenderable::GetTexture() 
{
	return m_pTexture; 
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::SetTexture
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::SetTexture(CTexture *pTexture)
{
	if (NULL != GetTexture())
	{
		// remove this as an observer of surface changes
		::RemoveObserver<CSurfaceRenderable>(&GetTexture()->GetChangeEvent(),
			this, Invalidate);
	}

	// assign the pointer
	m_pTexture = pTexture;

	if (NULL != GetTexture())
	{
		// add this as an observer of surface changes
		::AddObserver<CSurfaceRenderable>(&GetTexture()->GetChangeEvent(),
			this, Invalidate);
	}

	// trigger re-rendering
	Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::SetObject
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::SetObject(CObject *pObject)
{
	if (NULL != GetSurface())
	{
		// remove this as an observer of surface changes
		::RemoveObserver<CSurfaceRenderable>(&GetSurface()->GetChangeEvent(),
			this, Invalidate);
	}

	// assign the pointer
	CRenderable::SetObject(pObject);

	if (NULL != GetSurface())
	{
		// add this as an observer of surface changes
		::AddObserver<CSurfaceRenderable>(&GetSurface()->GetChangeEvent(),
			this, Invalidate);

		// set the renderables centroid to the center of the bounding box
		SetCentroid(0.5 * (GetSurface()->GetBoundsMin() 
			+ GetSurface()->GetBoundsMax()));
	}

	// trigger re-rendering
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::GetNearestDistance
// 
// computes the nearest distance to the given point
//////////////////////////////////////////////////////////////////////////////
double CSurfaceRenderable::GetNearestDistance(const CVectorD<3>& vPoint)
{
	CVectorD<3> vMin = GetSurface()->GetBoundsMin();
	CVectorD<3> vMax = GetSurface()->GetBoundsMax();
	
	double nearestDist = FLT_MAX;
	for (int nAtZ = 0; nAtZ < 2; nAtZ++)
	{
		for (int nAtY = 0; nAtY < 2; nAtY++)
		{
			for (int nAtX = 0; nAtX < 2; nAtX++)
			{
				CVectorD<3> vCorner;
				vCorner[0] = (nAtX == 0) ? vMin[0] : vMax[0];
				vCorner[1] = (nAtY == 0) ? vMin[1] : vMax[1];
				vCorner[2] = (nAtZ == 0) ? vMin[2] : vMax[2];

				double dist = (vCorner - vPoint).GetLength();
				nearestDist = __min(dist, nearestDist);
			}
		}
	}

	return nearestDist;
}

//////////////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::GetFurthestDistance
// 
// computes the furthest distance to the given point
//////////////////////////////////////////////////////////////////////////////
double CSurfaceRenderable::GetFurthestDistance(const CVectorD<3>& vPoint)
{
	CVectorD<3> vMin = GetSurface()->GetBoundsMin();
	CVectorD<3> vMax = GetSurface()->GetBoundsMax();
	
	double furthestDist = 0.0;
	for (int nAtZ = 0; nAtZ < 2; nAtZ++)
	{
		for (int nAtY = 0; nAtY < 2; nAtY++)
		{
			for (int nAtX = 0; nAtX < 2; nAtX++)
			{
				CVectorD<3> vCorner;
				vCorner[0] = (nAtX == 0) ? vMin[0] : vMax[0];
				vCorner[1] = (nAtY == 0) ? vMin[1] : vMax[1];
				vCorner[2] = (nAtZ == 0) ? vMin[2] : vMax[2];

				double dist = (vCorner - vPoint).GetLength();
				furthestDist = __max(dist, furthestDist);
			}
		}
	}

	return furthestDist;
}


///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DrawOpaque
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DrawOpaque(CRenderContext *pRC)
{
	DrawWireframe(pRC);
}


///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DrawAlpha
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DrawTransparent(CRenderContext *pRC)
{
	// DrawBoundsSurface();

	DrawSurface(pRC);
}


///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DrawWireframe
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DrawWireframe(CRenderContext *pRC)
{
	if (m_bWireFrame)
	{
		// setup line rendering
		pRC->SetupLines();

		// iterate through contours
		for (int nAt = 0; nAt < GetSurface()->GetContourCount(); nAt++)
		{
			// get a reference to the current polygon
			CPolygon& polygon = GetSurface()->GetContour(nAt);

			// save the modelview matrix
			pRC->PushMatrix();

			// translate to the appropriate reference distance
			pRC->Translate(
				CVectorD<3>(0.0, GetSurface()->GetContourRefDist(nAt), 0.0));

			// after we rotate the data into the X-Z plane
			pRC->Rotate(90.0, CVectorD<3>(1.0, 0.0, 0.0));

			// render the polygon as a line loop
			pRC->LineLoopFromPolygon(polygon);

			// restore the modelview matrix
			pRC->PopMatrix();
		}
	}
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DrawSurface
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DrawSurface(CRenderContext *pRC)
{
	if (NULL != GetTexture())
	{
		pRC->Bind(GetTexture());
	}

	// draw the surface
	pRC->TrianglesFromSurface(*GetSurface());

	if (NULL != GetTexture())
	{
		pRC->Unbind();
	}
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DrawBoundsSurface
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DrawBoundsSurface(CRenderContext *pRC)
{
	if (m_bShowBoundsSurface)
	{
		// draw the boundary surfaces
		pRC->Color(RGB(0, 0, 128));

		double yMin = GetSurface()->GetBoundsMin()[1];
		double yMax = GetSurface()->GetBoundsMax()[1];
		for (int nAtTri = 0; nAtTri < GetSurface()->GetTriangleCount(); nAtTri++)
		{
			CPackedVectorD<3> vVert0 = GetSurface()->GetTriVert(nAtTri, 0);
			CPackedVectorD<3> vVert1 = GetSurface()->GetTriVert(nAtTri, 1);
			CPackedVectorD<3> vVert2 = GetSurface()->GetTriVert(nAtTri, 2);

			if (vVert0[1] == yMin && vVert1[1] == yMin && vVert2[1] == yMin)
			{
				CPackedVectorD<3> vNorm0 = GetSurface()->GetTriNorm(nAtTri, 0);
				CPackedVectorD<3> vNorm1 = GetSurface()->GetTriNorm(nAtTri, 1);
				CPackedVectorD<3> vNorm2 = GetSurface()->GetTriNorm(nAtTri, 2);

				pRC->BeginTriangles();

					pRC->Vertex(vVert0);
					pRC->Normal(vNorm0);

					pRC->Vertex(vVert1);
					pRC->Normal(vNorm1);

					pRC->Vertex(vVert2);
					pRC->Normal(vNorm2);

				pRC->End();
			}
		}
	}
}
