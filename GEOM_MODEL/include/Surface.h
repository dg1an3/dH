//////////////////////////////////////////////////////////////////////
// Surface.h: interface for the CSurface class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vectors
#include <Vector.h>

// modelobject base class
#include "ModelObject.h"

// polygons
#include "Polygon.h"

// volume (for region)
#include "Volumep.h"


//////////////////////////////////////////////////////////////////////
// class CSurface
//
// represents a meshed surface
//////////////////////////////////////////////////////////////////////
class CSurface : public CModelObject
{
public:
	// constructor
	CSurface();

	// copy constructor
	CSurface(const CSurface& fromSurface);

	// destructor
	virtual ~CSurface();

	// serialization support
	DECLARE_SERIAL(CSurface)

	// assignment
	CSurface& operator=(const CSurface& fromSurface);

	// contour accessors
	int GetContourCount() const;
	CPolygon& GetContour(int nIndex) const;
	double GetContourRefDist(int nIndex) const;

	// triangle accessors
	int GetTriangleCount();
	const double *GetTriangleVertex(int nTriangle, int nVertex);
	const double *GetTriangleNormal(int nTriangle, int nVertex);

	// volume bounds for the surface
	CVector<3> GetBoundsMin() const;
	CVector<3> GetBoundsMax() const;

	// returns largest dimension
	double GetMaxSize();

	// direct access to index, vertex, and normal array
	// WARNING: if you change the values through these references, you
	//		MUST call CSurface::FireChange() as soon as possible to
	//		notify observers of the change
	CArray<int, int>& GetIndexArray() { return m_arrVertIndex; }
	CArray<CPackedVector<3>, CPackedVector<3>&>& 
		GetVertexArray() { return m_arrVertex; }
	CArray<CPackedVector<3>, CPackedVector<3>&>& 
		GetNormalArray() { return m_arrNormal; }

	// accessor for the "region" -- volume with a 1.0 everywhere
	//		that is in the structure
	CVolume<int> *m_pRegion;

	// serialization
	void Serialize(CArchive &ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// contours for the structure
	CObArray m_arrContours;

	// reference distances for the contours
	CArray<double, double> m_arrRefDist;

	// mesh vertices
	CArray<int, int> m_arrVertIndex;
	CArray<CPackedVector<3>, CPackedVector<3>&> m_arrVertex;
	CArray<CPackedVector<3>, CPackedVector<3>&> m_arrNormal;
};

//////////////////////////////////////////////////////////////////////
// CSurface::GetTriangleCount
// 
// returns the number of triangles in the mesh
//////////////////////////////////////////////////////////////////////
inline int CSurface::GetTriangleCount()
{
	return m_arrVertIndex.GetSize() / 3;
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetTriangleVertex
// 
// returns the vertex for a specific triangle
//////////////////////////////////////////////////////////////////////
inline const double *CSurface::GetTriangleVertex(int nTriangle, int nVertex)
{
	return m_arrVertex[m_arrVertIndex[nTriangle*3 + nVertex]-1];
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetTriangleNormal
// 
// returns the normal for a specific triangle
//////////////////////////////////////////////////////////////////////
inline const double *CSurface::GetTriangleNormal(int nTriangle, int nVertex)
{
	return m_arrNormal[m_arrVertIndex[nTriangle*3 + nVertex]-1];
}

#endif // !defined(AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
