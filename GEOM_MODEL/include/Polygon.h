//////////////////////////////////////////////////////////////////////
// Polygon.h: declaration of the CPolygon class
//
// Copyright (C) 2000-2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>
#include <MatrixNxM.h>

#include "ModelObject.h"

//////////////////////////////////////////////////////////////////////
// class CPolygon
//
// represents a polygon on a plane; includes change notification
// and some computational geometry
//////////////////////////////////////////////////////////////////////
class CPolygon : public CModelObject
{
public:
	// constructors / destructors
	CPolygon();
	CPolygon(const CPolygon& fromPoly);
	virtual ~CPolygon();

	// serialization support for the polygon
	DECLARE_SERIAL(CPolygon)

	// assignment operator
	CPolygon& operator=(const CPolygon& fromPoly);

	// vertex accessors
	int GetVertexCount() const;

	const CVectorD<2>& GetVertexAt(int nIndex) const;
	void SetVertexAt(int nIndex, const CVectorD<2>& v);

	int AddVertex(const CVectorD<2>& v);
	void RemoveVertex(int nIndex);

	// direct access to vertex array 
	// WARNING: Lock calls must be matched by unlock calls
	CMatrixNxM<>& LockVertexMatrix();
	void UnlockVertexMatrix(BOOL bChanged = TRUE);

	// computes the signed area of the polygon
	double GetSignedArea() const;

	// turns this polygon into its own convex hull
	void MakeConvexHull();

	// serializes the polygon
	void Serialize(CArchive &ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	// the polygon's vertex array
	CMatrixNxM<REAL> m_mVertex;
};

#endif // !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
