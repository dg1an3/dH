//////////////////////////////////////////////////////////////////////
// Mesh.h: interface for the CMesh class.
//
// Copyright (C) 2000-2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(_MESH_H_)
#define _MESH_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
using namespace std;

// vectors
#include <VectorD.h>
#include <MatrixNxM.h>

// modelobject base class
#include "ModelObject.h"

// polygons
#include "Polygon.h"

// volume (for region)
#include "Volumep.h"

template<class TYPE = int>
class CTriIndex
{
public:
	CTriIndex() { memset(m_index, 0, sizeof(TYPE) * 3); }

	TYPE& operator[](int nIndex) { return m_index[nIndex]; }
	const TYPE& operator[](int nIndex) const { return m_index[nIndex]; }

private:
	TYPE m_index[3];
};

///////////////////////////////////////////////////////////////////////////////
// class CMesh
//
// represents a meshed surface
///////////////////////////////////////////////////////////////////////////////
class CMesh : public CModelObject
{
public:
	// constructors / destructor
	CMesh();
	CMesh(const CMesh& fromSurface);
	virtual ~CMesh();

	// serialization support
	DECLARE_DYNAMIC(CMesh)

	// assignment
	CMesh& operator=(const CMesh& fromSurface);

	// contour accessors
	int GetContourCount() const;
	CPolygon *GetContour(int nAt);
	double GetContourRefDist(int nIndex) const;

	// triangle accessors
	int GetTriangleCount();

	// accessors for triangle vertices and normals
	const CVectorD<3>& GetTriVert(int nTriangle, int nVertex) const;
	const CVectorD<3>& GetTriNorm(int nTriangle, int nVertex) const;

	// returns the adjacent triangle
	int GetAdjacentTri(int nTriangle, int nSide) const;

	// volume bounds for the surface
	const CVectorD<3>& GetBoundsMin() const;
	const CVectorD<3>& GetBoundsMax() const;

	// returns largest dimension
	double GetMaxSize() const;

	// direct access to index, vertex, and normal arrays
	// WARNING: calls to Lock* must be matched by calls to Unlock*
	vector< CTriIndex<int> >& LockIndexArray();
	void UnlockIndexArray(BOOL bChanged = TRUE);

	CMatrixNxM<>& LockVertexMatrix();
	void UnlockVertexMatrix(BOOL bChanged = TRUE);

	CMatrixNxM<>& LockNormalMatrix();
	void UnlockNormalMatrix(BOOL bChanged = TRUE);

	// function to ensure the orientation of the triangle orientations are 
	//		consistent; returns FALSE when all are consistent
	BOOL OrientNextFace();

	// serialization
	void Serialize(CArchive &ar);

protected:
	// helper function to reverse the orientation of a face
	void ReverseTriangleOrientation(int nAt);

	// helper function to find a neighbor triangle
	int FindTriangleWithVertices(int nStart, const CVectorD<3>& v1, 
			const CVectorD<3>& v2, BOOL *bOriented, int *pNeighborVertex);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// contours for the structure
	CObArray m_arrContours;

	// reference distances for the contours
	CArray<double, double> m_arrRefDist;

	// mesh vertices
	vector< CTriIndex<> > m_arrTriIndex;
	CMatrixNxM<> m_mVertex;
	CMatrixNxM<> m_mNormal;

	// surface bounding box
	mutable CVectorD<3> m_vBoundsMax;
	mutable CVectorD<3> m_vBoundsMin;

	// flags to recompute the bounding box
	mutable BOOL m_bRecomputeBoundsMax;
	mutable BOOL m_bRecomputeBoundsMin;

	// accessor for the "region" -- volume with a 1 everywhere
	//		that is in the structure
	CVolume<int> *m_pRegion;

	// face orientation array -- indicates which faces are oriented
	CArray<BOOL, BOOL> m_arrIsOriented;

	// current position for face orientation
	int m_nAtTri;
	
	// current count of unoriented faces
	int m_nUnorientedCount;
};

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetTriangleCount
// 
// returns the number of triangles in the mesh
///////////////////////////////////////////////////////////////////////////////
inline int CMesh::GetTriangleCount()
{
	return m_arrTriIndex.size() / 3;
}

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetTriVert
// 
// returns the vertex for a specific triangle as a CVectorD<3>
///////////////////////////////////////////////////////////////////////////////
inline const CVectorD<3>& CMesh::GetTriVert(int nTriangle, int nVertex) const
{
	int nVertIndex = m_arrTriIndex[nTriangle][nVertex]-1;
	return static_cast< const CVectorD<3>& >(m_mVertex[nVertIndex]);
}

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetTriNorm
// 
// returns the normal for a specific triangle as a CVectorD<3>
///////////////////////////////////////////////////////////////////////////////
inline const CVectorD<3>& CMesh::GetTriNorm(int nTriangle, int nVertex) const
{
	int nVertIndex = m_arrTriIndex[nTriangle][nVertex]-1;
	return static_cast< const CVectorD<3>& >(m_mNormal[nVertIndex]);
}

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetIndexArray
// 
// returns a reference to the index array
///////////////////////////////////////////////////////////////////////////////
inline vector< CTriIndex<int> >& CMesh::LockIndexArray() 
{ 
	return m_arrTriIndex; 
}

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetVertexArray
// 
// returns a reference to the vertex array
///////////////////////////////////////////////////////////////////////////////
inline CMatrixNxM<>& CMesh::LockVertexMatrix() 
{ 
	return m_mVertex; 
}

///////////////////////////////////////////////////////////////////////////////
// CMesh::GetNormalArray
// 
// returns a reference to the normal array
///////////////////////////////////////////////////////////////////////////////
inline CMatrixNxM<>& CMesh::LockNormalMatrix() 
{ 
	return m_mNormal; 
}

#endif // !defined(_MESH_H_)
