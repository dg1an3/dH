//////////////////////////////////////////////////////////////////////
// SurfaceRenderable.cpp: declaration of the CSurfaceRenderable class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "stdafx.h"

// floating-point constants
#include <float.h>

// OpenGL includes
#include "glMatrixVector.h"

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
		m_bColorWash(FALSE),
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
double CSurfaceRenderable::GetNearestDistance(const CVector<3>& vPoint)
{
	return CRenderable::GetNearestDistance(vPoint);

	CVector<3> vMin = GetSurface()->GetBoundsMin();
	CVector<3> vMax = GetSurface()->GetBoundsMax();
	
	double nearestDist = FLT_MAX;
	for (int nAtZ = 0; nAtZ < 2; nAtZ++)
	{
		for (int nAtY = 0; nAtY < 2; nAtY++)
		{
			for (int nAtX = 0; nAtX < 2; nAtX++)
			{
				CVector<3> vCorner;
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
double CSurfaceRenderable::GetFurthestDistance(const CVector<3>& vPoint)
{
	return CRenderable::GetFurthestDistance(vPoint);

	CVector<3> vMin = GetSurface()->GetBoundsMin();
	CVector<3> vMax = GetSurface()->GetBoundsMax();
	
	double furthestDist = 0.0;
	for (int nAtZ = 0; nAtZ < 2; nAtZ++)
	{
		for (int nAtY = 0; nAtY < 2; nAtY++)
		{
			for (int nAtX = 0; nAtX < 2; nAtX++)
			{
				CVector<3> vCorner;
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
// CSurfaceRenderable::DescribeOpaque
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DescribeOpaque()
{
	DescribeWireframe();
}


///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DescribeAlpha
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DescribeAlpha()
{
	// DescribeBoundsSurface();

	DescribeSurface();
}


///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DescribeWireframe
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DescribeWireframe()
{
	if (m_bWireFrame)
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		// glColor(GetColor());

		for (int nAt = 0; nAt < GetSurface()->GetContourCount(); nAt++)
		{
			CPolygon& polygon = GetSurface()->GetContour(nAt);

			glPushMatrix();

			// translate to the appropriate reference distance
			glTranslated(0.0, GetSurface()->GetContourRefDist(nAt), 0.0);

			// after we rotate the data into the X-Z plane
			glRotated(90.0, 1.0, 0.0, 0.0);

			// use the polygon's vertex data as the data array
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_DOUBLE, 0, 
				polygon.GetVertexArray().GetData());

			// and draw the loop
			glDrawArrays(GL_LINE_LOOP, 0, polygon.GetVertexCount());

			glDisableClientState(GL_VERTEX_ARRAY);

			glPopMatrix();
		}

		glEnable(GL_LIGHTING);
	}
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DescribeSurface
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DescribeSurface()
{
	if (m_bColorWash)
	{
		// disable lighting
		glDisable(GL_LIGHTING);

		// set the depth mask to read-only
		glDepthMask(GL_FALSE);

		// set up the accumulation buffer, using the current transparency
		glAccum(GL_LOAD, 0.75f);
	}

	// set the array for vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0,
		GetSurface()->GetVertexArray().GetData()-1);

	// set the array for normals
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_DOUBLE, 0, 
		GetSurface()->GetNormalArray().GetData()-1);

	// enable blending; this is redundant but needed because
	//		of the draw list
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (GetTexture() != NULL)
	{
		GetTexture()->Bind(m_pView);

		glMatrixMode(GL_TEXTURE);

		// load the texture adjustment onto the matrix stack
		glLoadMatrix(GetTexture()->GetProjection());

		// enable texture coordinate mode
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// set the texture coordinate pointer
		glTexCoordPointer(3, GL_DOUBLE, 0, GetSurface()->GetVertexArray().GetData()-1);

		glMatrixMode(GL_MODELVIEW);

		// make sure no errors occurred
		ASSERT(glGetError() == GL_NO_ERROR);
	}

	// now draw the surface from the arrays of data
	glDrawElements(GL_TRIANGLES, GetSurface()->GetTriangleCount() * 3, 
		GL_UNSIGNED_INT, (void *)GetSurface()->GetIndexArray().GetData());

	// disable the use of arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	if (NULL != GetTexture()) 
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		GetTexture()->Unbind();
	}

	// make the depth mask read-only
	glDepthMask(GL_TRUE);

	if (m_bColorWash)
	{
		// set up the accumulation buffer, using the current transparency
		glAccum(GL_ACCUM, 0.25f);
		glAccum(GL_RETURN, 1.0f);

		// set the depth mask to writeable
		glDepthMask(GL_TRUE);

		// re-enable lighting
		glEnable(GL_LIGHTING);
	}
}

///////////////////////////////////////////////////////////////////////
// CSurfaceRenderable::DescribeBoundsSurface
// 
// 
//////////////////////////////////////////////////////////////////////
void CSurfaceRenderable::DescribeBoundsSurface()
{
	if (m_bShowBoundsSurface)
	{
		// draw the boundary surfaces
		glColor(RGB(0, 0, 128));

		double yMin = GetSurface()->GetBoundsMin()[1];
		double yMax = GetSurface()->GetBoundsMax()[1];
		for (int nAtTri = 0; nAtTri < GetSurface()->GetTriangleCount(); nAtTri++)
		{
			const double *vVert0 = GetSurface()->GetTriangleVertex(nAtTri, 0);
			const double *vVert1 = GetSurface()->GetTriangleVertex(nAtTri, 1);
			const double *vVert2 = GetSurface()->GetTriangleVertex(nAtTri, 2);

			if (vVert0[1] == yMin && vVert1[1] == yMin && vVert2[1] == yMin)
			{
				glBegin(GL_TRIANGLES);

				glVertex3dv(vVert0);
				glNormal3dv(GetSurface()->GetTriangleNormal(nAtTri, 0));
				glTexCoord3dv(vVert0);

				glVertex3dv(vVert1);
				glNormal3dv(GetSurface()->GetTriangleNormal(nAtTri, 1));
				glTexCoord3dv(vVert1);

				glVertex3dv(vVert2);
				glNormal3dv(GetSurface()->GetTriangleNormal(nAtTri, 2));
				glTexCoord3dv(vVert2);

				glEnd();
			}
		}
	}
}
