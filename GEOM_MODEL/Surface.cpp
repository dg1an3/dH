//////////////////////////////////////////////////////////////////////
// Surface.cpp: implementation of the CSurface class.
//
// Copyright (C) 2000-2002 Derek G. Lane
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// floating point constants
#include <float.h>

// utility macros
#include <UtilMacros.h>

// class declaration
#include "Surface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSurface::CSurface
// 
// constructs a new surface object
//////////////////////////////////////////////////////////////////////
CSurface::CSurface()
	: m_pRegion(NULL),
		m_bRecomputeBoundsMax(TRUE),
		m_bRecomputeBoundsMin(TRUE)
{
}

//////////////////////////////////////////////////////////////////////
// CSurface::CSurface
// 
// copy constructor
//////////////////////////////////////////////////////////////////////
CSurface::CSurface(const CSurface& fromSurface)
	: m_pRegion(NULL)
{
	// assign the surface
	(*this) = fromSurface;
}

//////////////////////////////////////////////////////////////////////
// CSurface::~CSurface
// 
// destructor
//////////////////////////////////////////////////////////////////////
CSurface::~CSurface()
{
	// delete the contours
	for (int nAt = 0; nAt < m_arrContours.GetSize(); nAt++)
	{
		delete m_arrContours[nAt];
	}

	// and remove the items from the array
	m_arrContours.RemoveAll();

	// delete the region
	delete m_pRegion;
}


//////////////////////////////////////////////////////////////////////
// IMPLEMENT_SERIAL
// 
// provides serialization support for the surface
//////////////////////////////////////////////////////////////////////

#define SURFACE_SCHEMA 4
	// 4 - added the region
	// 3
	// 2
	// 1

IMPLEMENT_SERIAL(CSurface, CModelObject, VERSIONABLE_SCHEMA | SURFACE_SCHEMA)


//////////////////////////////////////////////////////////////////////
// CSurface::operator=
// 
// assignment operator
//////////////////////////////////////////////////////////////////////
CSurface& CSurface::operator=(const CSurface& fromSurface)
{
	// set the name of the surface
	SetName(fromSurface.GetName());

	// copy the contours
	for (int nAt = 0; nAt < fromSurface.GetContourCount(); nAt++)
	{
		m_arrContours.Add(new CPolygon(fromSurface.GetContour(nAt)));
	}

	// copy the reference distances for the contours
	m_arrRefDist.Copy(fromSurface.m_arrRefDist);

	// copy the mesh vertices
	m_arrVertIndex.Copy(fromSurface.m_arrVertIndex);
	m_arrVertex.Copy(fromSurface.m_arrVertex);
	m_arrNormal.Copy(fromSurface.m_arrNormal);

	// return a reference to this
	return (*this);
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetContourCount
// 
// returns the number of contours in the mesh
//////////////////////////////////////////////////////////////////////
int CSurface::GetContourCount() const
{
	return m_arrContours.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetContour
// 
// returns the contour at the given index
//////////////////////////////////////////////////////////////////////
CPolygon& CSurface::GetContour(int nIndex) const
{
	return *(CPolygon *)m_arrContours[nIndex];
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetContourRefDist
// 
// returns the reference distance of the indicated contour
//////////////////////////////////////////////////////////////////////
double CSurface::GetContourRefDist(int nIndex) const
{
	return m_arrRefDist[nIndex];
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetBoundsMin
// 
// returns the minimum of the mesh
//////////////////////////////////////////////////////////////////////
CVectorD<3> CSurface::GetBoundsMin() const
{
	if (m_bRecomputeBoundsMin)
	{
		// set to max originally
		m_vBoundsMin = CVectorD<3>(FLT_MAX, FLT_MAX, FLT_MAX);

		// accumulate the minimum
		int nVertex;
		for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++) 
		{
			m_vBoundsMin[0] = 
				min(m_arrVertex[nVertex][0], m_vBoundsMin[0]);
			m_vBoundsMin[1] = 
				min(m_arrVertex[nVertex][1], m_vBoundsMin[1]);
			m_vBoundsMin[2] = 
				min(m_arrVertex[nVertex][2], m_vBoundsMin[2]);
		}

		m_bRecomputeBoundsMin = FALSE;
	}

	// return the min vector
	return m_vBoundsMin;
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetBoundsMax
// 
// returns the maximum of the mesh
//////////////////////////////////////////////////////////////////////
CVectorD<3> CSurface::GetBoundsMax() const
{
	if (m_bRecomputeBoundsMax)
	{
		// set to -max originally
		m_vBoundsMax = CVectorD<3>(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		// accumulate the maximum
		int nVertex;
		for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++)
		{
			m_vBoundsMax[0] = 
				max(m_arrVertex[nVertex][0], m_vBoundsMax[0]);
			m_vBoundsMax[1] = 
				max(m_arrVertex[nVertex][1], m_vBoundsMax[1]);
			m_vBoundsMax[2] = 
				max(m_arrVertex[nVertex][2], m_vBoundsMax[2]);
		}

		m_bRecomputeBoundsMax = FALSE;
	}

	// return the max vector
	return m_vBoundsMax;
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetMaxSize
// 
// returns the largest dimension of the bounding box
//////////////////////////////////////////////////////////////////////
double CSurface::GetMaxSize()
{
	return (GetBoundsMax() - GetBoundsMin()).GetLength() 
		/ sqrt(2.0); 
}

//////////////////////////////////////////////////////////////////////
// CSurface::OrientNextFace
// 
// orients a face of the surface, and returns TRUE if more
//		passes are needed
//////////////////////////////////////////////////////////////////////
BOOL CSurface::OrientNextFace()
{
	if (m_arrIsOriented.GetSize() < GetTriangleCount())
	{
		// array of flags to keep track of which triangles have been oriented
		m_arrIsOriented.SetSize(GetTriangleCount());

		// set the first triangle flag to oriented by default
		m_arrIsOriented[0] = TRUE;
		for (int nAt = 1; nAt < GetTriangleCount(); nAt++)
		{
			m_arrIsOriented[nAt] = FALSE;
		}

		// inside-out
		ReverseTriangleOrientation(0);
		m_nAtTri = 1;
		m_nUnorientedCount = 0;
	}

	// if we are finished with this pass
	if (m_nAtTri > GetTriangleCount())
	{
		// see if another pass is needed
		if (m_nUnorientedCount == 0)
		{
			// no, so return FALSE
			return FALSE;
		}

		// yes, so reset counters
		m_nAtTri = 1;
		m_nUnorientedCount = 0;
	}

	// iterate through the vertices of the current triangle
	for (int nAtVert = 0; !m_arrIsOriented[m_nAtTri] && nAtVert < 3; nAtVert++)
	{
		// flag to indicate that the two vertices are in the same
		//		orientation as passed
		BOOL bSameOrientation;

		// search for the triangle with two matching sides
		int nNeighborVertex;
		int nAtNeighbor = FindTriangleWithVertices(m_nAtTri, 
			GetTriVert(m_nAtTri, (nAtVert + 1) % 3),
			GetTriVert(m_nAtTri, nAtVert), 
			&bSameOrientation, &nNeighborVertex);

		// if a neighbor was found, and it is already oriented,
		if (nAtNeighbor > -1 && m_arrIsOriented[nAtNeighbor])
		{
			// if the triangle is oriented incorrectly
			if (!bSameOrientation)
			{
				// reverse its orientation
				ReverseTriangleOrientation(m_nAtTri);
			}

			// and mark as oriented
			m_arrIsOriented[m_nAtTri] = TRUE;
		}
	}

	// if we still aren't oriented,
	if (!m_arrIsOriented[m_nAtTri])
	{
		// set the flag to false because one unoriented triangle was found
		m_nUnorientedCount++;
	}

	// increment the triangle counter
	m_nAtTri++;

	// and return TRUE for more passes
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSurface::Serialize
// 
// returns the number of triangles in the mesh
//////////////////////////////////////////////////////////////////////
void CSurface::Serialize(CArchive &ar)
{
	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : SURFACE_SCHEMA;

	// serialize the surface name
	// name.Serialize(ar);
	if (ar.IsLoading())
	{
		CString strName;
		ar >> strName;
		SetName(strName);
	}
	else
	{
		ar << GetName();
	}

	// serialize the contours
	m_arrContours.Serialize(ar);

	// serialize the reference distance
	m_arrRefDist.Serialize(ar);

	// serialize vertices
	m_arrVertIndex.Serialize(ar);
	m_arrVertex.Serialize(ar);
	m_arrNormal.Serialize(ar);

	// if schema >= 4
	if (nSchema >= 4)
	{
		// serialize the region also
		if (m_pRegion == NULL)
		{
			m_pRegion = new CVolume<int>();
		}
		m_pRegion->Serialize(ar);
	}
}

//////////////////////////////////////////////////////////////////////
// CSurface::ReverseTriangleOrientation
// 
// helper function to reverse the orientation of a face
//////////////////////////////////////////////////////////////////////
void CSurface::ReverseTriangleOrientation(int nAt)
{
	int nTemp = m_arrVertIndex[nAt * 3 + 1];
	m_arrVertIndex[nAt * 3 + 1] = m_arrVertIndex[nAt*3 + 2];
	m_arrVertIndex[nAt * 3 + 2] = nTemp;
}

//////////////////////////////////////////////////////////////////////
// CSurface::FindTriangleWithVertices
// 
// helper function to find a neighbor triangle
//////////////////////////////////////////////////////////////////////
int CSurface::FindTriangleWithVertices(int nSkip, const CVectorD<3>& v1, 
		const CVectorD<3>& v2, BOOL *bOriented, int *pNeighborVertex)
{
	// search for the vertices
	for (int nAt = nSkip-1; nAt >= 0; nAt--)
	{
		// see if the first vertex matches any
		for (int nv1Match = 0; nv1Match < 3
			&& !v1.IsApproxEqual(GetTriVert(nAt, nv1Match));
			nv1Match++);

		// see if the second vertex matches any
		for (int nv2Match = 0; nv2Match < 3
			&& !v2.IsApproxEqual(GetTriVert(nAt, nv2Match));
			nv2Match++);

		// if a match was found for both
		if (nv1Match < 3 && nv2Match < 3)
		{
			// set the flag if the order of the two vertices matched the 
			//		passed order
			(*bOriented) = ((nv1Match + 1) % 3) == nv2Match;

			// set match to first vertex
			(*pNeighborVertex) = nv1Match;

			// and return the triangle number
			return nAt;
		}
	}

	// search for the vertices
	for (nAt = nSkip+1; nAt < GetTriangleCount(); nAt++)
	{
		// see if the first vertex matches any
		for (int nv1Match = 0; nv1Match < 3
			&& !v1.IsApproxEqual(GetTriVert(nAt, nv1Match));
			nv1Match++);

		// see if the second vertex matches any
		for (int nv2Match = 0; nv2Match < 3
			&& !v2.IsApproxEqual(GetTriVert(nAt, nv2Match));
			nv2Match++);

		// if a match was found for both
		if (nv1Match < 3 && nv2Match < 3)
		{
			// set the flag if the order of the two vertices matched the 
			//		passed order
			(*bOriented) = ((nv1Match + 1) % 3) == nv2Match;

			// set match to first vertex
			(*pNeighborVertex) = nv1Match;

			// and return the triangle number
			return nAt;
		}
	}

	// none found, return -1
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// CSurface diagnostics

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////
// CSurface::AssertValid
// 
// asserts validity of the surface object
//////////////////////////////////////////////////////////////////////
void CSurface::AssertValid() const
{
	// base class assert
	CObject::AssertValid();

	// array assert
	m_arrVertIndex.AssertValid();
	m_arrVertex.AssertValid();
	m_arrNormal.AssertValid();
}

//////////////////////////////////////////////////////////////////////
// CSurface::Dump
// 
// helper function to find a neighbor triangle
//////////////////////////////////////////////////////////////////////
void CSurface::Dump(CDumpContext& dc) const
{
	// dump base class
	CObject::Dump(dc);

	// tab out
	CREATE_TAB(dc.GetDepth());

	// output triangle count
	DC_TAB(dc) << "Number of triangles = " << m_arrVertIndex.GetSize() << "\n";

	// output bounding box
	CVectorD<3> vMin = GetBoundsMin();
	DC_TAB(dc) << "Bounding box min = <" << vMin[0] 
		<< ", " << vMin[1] 
		<< ", " << vMin[2] << ">\n";

	CVectorD<3> vMax = GetBoundsMax();
	DC_TAB(dc) << "Bounding box max = <" << vMax[0] 
		<< ", " << vMax[1] 
		<< ", " << vMax[2] << ">\n";
}
#endif //_DEBUG

