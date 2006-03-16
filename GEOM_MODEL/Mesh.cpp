//////////////////////////////////////////////////////////////////////
// Mesh.cpp: implementation of the CMesh class.
//
// Copyright (C) 2000-2002 Derek G. Lane
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// floating point constants
#include <float.h>

// matrix base class
// #include <MatrixBase.inl>

// utility macros
#include <UtilMacros.h>

// class declaration
#include "Mesh.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CMesh::CMesh
// 
// constructs a new surface object
//////////////////////////////////////////////////////////////////////
CMesh::CMesh()
	: // m_pRegion(NULL),
		m_bRecomputeBoundsMax(TRUE),
		m_bRecomputeBoundsMin(TRUE)
{
}

//////////////////////////////////////////////////////////////////////
// CMesh::CMesh
// 
// copy constructor
//////////////////////////////////////////////////////////////////////
CMesh::CMesh(const CMesh& fromSurface)
//	: m_pRegion(NULL)
{
	// assign the surface
	(*this) = fromSurface;
}

//////////////////////////////////////////////////////////////////////
// CMesh::~CMesh
// 
// destructor
//////////////////////////////////////////////////////////////////////
CMesh::~CMesh()
{
	// delete the contours
/*	for (int nAt = 0; nAt < GetContourCount(); nAt++)
	{
		delete m_arrContours[nAt];
	} */

	// delete the region
//	delete m_pRegion;
}


//////////////////////////////////////////////////////////////////////
// IMPLEMENT_SERIAL
// 
// provides serialization support for the surface
//////////////////////////////////////////////////////////////////////

const int MESH_SCHEMA = 4;
	// 4 - added the region

IMPLEMENT_DYNAMIC(CMesh, CModelObject)


//////////////////////////////////////////////////////////////////////
// CMesh::operator=
// 
// assignment operator
//////////////////////////////////////////////////////////////////////
CMesh& CMesh::operator=(const CMesh& fromSurface)
{
	ASSERT(FALSE);

/*	// set the name of the surface
	SetName(fromSurface.GetName());

	// copy the contours
	for (int nAt = 0; nAt < fromSurface.GetContourCount(); nAt++)
	{
		CComObject<CPolygon> *pPoly = NULL;
		CComObject<CPolygon>::CreateInstance(&pPoly);

		m_arrContours.Add(new CPolygon(fromSurface.GetContour(nAt)));
	}

	// copy the reference distances for the contours
	m_arrRefDist.Copy(fromSurface.m_arrRefDist);

	// copy the mesh vertices
	m_arrTriIndex.assign(fromSurface.m_arrTriIndex.begin(),
		fromSurface.m_arrTriIndex.end());
	m_mVertex = fromSurface.m_mVertex;
	m_mNormal = fromSurface.m_mNormal;
*/
	// return a reference to this
	return (*this);
}

/*
//////////////////////////////////////////////////////////////////////
// CMesh::GetContourCount
// 
// returns the number of contours in the mesh
//////////////////////////////////////////////////////////////////////
int CMesh::GetContourCount() const
{
	return m_arrContours.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CMesh::GetContour
// 
// returns the contour at the given index
//////////////////////////////////////////////////////////////////////
CPolygon *CMesh::GetContour(int nIndex)
{
	return (CPolygon *) m_arrContours[nIndex];
}

//////////////////////////////////////////////////////////////////////
// CMesh::GetContourRefDist
// 
// returns the reference distance of the indicated contour
//////////////////////////////////////////////////////////////////////
double CMesh::GetContourRefDist(int nIndex) const
{
	return m_arrRefDist[nIndex];
}
*/

//////////////////////////////////////////////////////////////////////
// CMesh::GetBoundsMin
// 
// returns the minimum of the mesh
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CMesh::GetBoundsMin() const
{
	if (m_bRecomputeBoundsMin)
	{
		// set to max originally
		m_vBoundsMin = CVectorD<3>(FLT_MAX, FLT_MAX, FLT_MAX);

		// accumulate the minimum
		for (int nVertex = 0; nVertex < m_mVertex.GetCols(); nVertex++) 
		{
			m_vBoundsMin[0] = 
				__min(m_mVertex[nVertex][0], m_vBoundsMin[0]);
			m_vBoundsMin[1] = 
				__min(m_mVertex[nVertex][1], m_vBoundsMin[1]);
			m_vBoundsMin[2] = 
				__min(m_mVertex[nVertex][2], m_vBoundsMin[2]);
		}

		m_bRecomputeBoundsMin = FALSE;
	}

	// return the min vector
	return m_vBoundsMin;
}

//////////////////////////////////////////////////////////////////////
// CMesh::GetBoundsMax
// 
// returns the maximum of the mesh
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CMesh::GetBoundsMax() const
{
	if (m_bRecomputeBoundsMax)
	{
		// set to -max originally
		m_vBoundsMax = CVectorD<3>(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		// accumulate the maximum
		for (int nVertex = 0; nVertex < m_mVertex.GetCols(); nVertex++) 
		{
			m_vBoundsMax[0] = 
				__max(m_mVertex[nVertex][0], m_vBoundsMax[0]);
			m_vBoundsMax[1] = 
				__max(m_mVertex[nVertex][1], m_vBoundsMax[1]);
			m_vBoundsMax[2] = 
				__max(m_mVertex[nVertex][2], m_vBoundsMax[2]);
		}

		m_bRecomputeBoundsMax = FALSE;
	}

	// return the max vector
	return m_vBoundsMax;
}

/*
//////////////////////////////////////////////////////////////////////
// CMesh::GetRegion
// 
// returns the region
//////////////////////////////////////////////////////////////////////
CVolume<double> *CMesh::GetRegion()
{
	if (NULL == m_pRegion)
	{
		m_pRegion = new CVolume<double>;
	}

	return m_pRegion;
}
*/

//////////////////////////////////////////////////////////////////////
// CMesh::GetMaxSize
// 
// returns the largest dimension of the bounding box
//////////////////////////////////////////////////////////////////////
double CMesh::GetMaxSize() const
{
	return (GetBoundsMax() - GetBoundsMin()).GetLength() 
		/ sqrt(2.0); 
}

//////////////////////////////////////////////////////////////////////
// CMesh::OrientNextFace
// 
// orients a face of the surface, and returns TRUE if more
//		passes are needed
//////////////////////////////////////////////////////////////////////
BOOL CMesh::OrientNextFace()
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
// CMesh::Serialize
// 
// returns the number of triangles in the mesh
//////////////////////////////////////////////////////////////////////
void CMesh::Serialize(CArchive &ar)
{
	UINT nSchema = MESH_SCHEMA;
	SERIALIZE_VALUE(ar, nSchema);

	// serialize the surface name
	SERIALIZE_VALUE(ar, m_strName);

	DWORD nContourCount = 0; // GetContourCount();
	SERIALIZE_VALUE(ar, nContourCount);

/*	// serialize the contours -- first prepare the array
	if (ar.IsLoading())
	{
		// delete any existing contours
		for (int nAt = 0; nAt < GetContourCount(); nAt++)
		{
			delete GetContour(nAt);
		}
		m_arrContours.SetSize(0);

		// create new contours
		for (nAt = 0; nAt < nContourCount; nAt++)
		{
			// and add it to the array
			m_arrContours.Add(new CPolygon());
		}
	}

	// now serialize the blocks
	for (int nAt = 0; nAt < nContourCount; nAt++)
	{
		GetContour(nAt)->Serialize(ar);
	} */

	// serialize the reference distance
	CArray<double, double> arrRefDist;
	arrRefDist.Serialize(ar);

	UINT nSize = m_arrTriIndex.size();
	SERIALIZE_VALUE(ar, nSize);

	// serialize vertices
	if (ar.IsLoading())
	{
		m_arrTriIndex.resize(nSize);
		ar.Read((void *) &m_arrTriIndex[0][0], 
			m_arrTriIndex.size() * sizeof(CTriIndex<>));
	}
	else
	{
		ar.Write((const void *) &m_arrTriIndex[0][0], 
			m_arrTriIndex.size() * sizeof(CTriIndex<>));
	}

	SERIALIZE_VALUE(ar, m_mVertex);
	SERIALIZE_VALUE(ar, m_mNormal);

	if (nSchema >= 4)
	{
		CVolume<double> region;
		region.Serialize(ar);

/*		// serialize the region also
		if (m_pRegion == NULL)
		{
			m_pRegion = new CVolume<double>();
		}
		m_pRegion->Serialize(ar); */
	}
}

//////////////////////////////////////////////////////////////////////
// CMesh::ReverseTriangleOrientation
// 
// helper function to reverse the orientation of a face
//////////////////////////////////////////////////////////////////////
void CMesh::ReverseTriangleOrientation(int nAt)
{
	int nTemp = m_arrTriIndex[nAt][1];
	m_arrTriIndex[nAt][1] = m_arrTriIndex[nAt][2];
	m_arrTriIndex[nAt][2] = nTemp;
}

//////////////////////////////////////////////////////////////////////
// CMesh::FindTriangleWithVertices
// 
// helper function to find a neighbor triangle
//////////////////////////////////////////////////////////////////////
int CMesh::FindTriangleWithVertices(int nSkip, const CVectorD<3>& v1, 
		const CVectorD<3>& v2, BOOL *bOriented, int *pNeighborVertex)
{
	// search for the vertices
	for (int nAt = nSkip-1; nAt >= 0; nAt--)
	{
		// see if the first vertex matches any
		int nv1Match;
		for (nv1Match = 0; nv1Match < 3
			&& !v1.IsApproxEqual(GetTriVert(nAt, nv1Match));
			nv1Match++);

		// see if the second vertex matches any
		int nv2Match;
		for (nv2Match = 0; nv2Match < 3
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
	for (int nAt = nSkip+1; nAt < GetTriangleCount(); nAt++)
	{
		// see if the first vertex matches any
		int nv1Match;
		for (nv1Match = 0; nv1Match < 3
			&& !v1.IsApproxEqual(GetTriVert(nAt, nv1Match));
			nv1Match++);

		// see if the second vertex matches any
		int nv2Match;
		for (nv2Match = 0; nv2Match < 3
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
// CMesh diagnostics

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////
// CMesh::AssertValid
// 
// asserts validity of the surface object
//////////////////////////////////////////////////////////////////////
void CMesh::AssertValid() const
{
	// base class assert
	CObject::AssertValid();

	// array assert
	// m_arrTriIndex.AssertValid();
	ASSERT(m_mVertex.GetRows() == 3);
	ASSERT(m_mNormal.GetRows() == 3);
	ASSERT(m_mNormal.GetCols() == m_mVertex.GetCols());
}

//////////////////////////////////////////////////////////////////////
// CMesh::Dump
// 
// helper function to find a neighbor triangle
//////////////////////////////////////////////////////////////////////
void CMesh::Dump(CDumpContext& dc) const
{
	// dump base class
	CObject::Dump(dc);

	// tab out
	CREATE_TAB(dc.GetDepth());

	// output triangle count
	DC_TAB(dc) << "Number of triangles = " << m_arrTriIndex.size() << "\n";

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

