// Polygon.h: interface for the CPolygon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
#define AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"
#include "Vector.h"

class CPolygon : public CObservableObject
{
public:
	// constructor
	CPolygon();

	// copy constructor
	CPolygon(const CPolygon& fromPoly);

	// destructor
	virtual ~CPolygon();

	// serialization support for the polygon
	DECLARE_SERIAL(CPolygon)

	// assignment operator
	CPolygon& operator=(const CPolygon& fromPoly);

	// vertex accessors
	int GetVertexCount();
	const CVector<2>& GetVertex(int nIndex);
	int AddVertex(CVector<2>& v);
	void RemoveVertex(int nIndex);

	// computes the signed area of the polygon
	double GetSignedArea();

	// turns this polygon into its own convex hull
	void MakeConvexHull();

	// serializes the polygon
	void Serialize(CArchive &ar);

	// direct access to vertex array
	// WARNING: if you change the vertices through this reference, you
	//		MUST call CPolygon::FireChange() as soon as possible to
	//		notify observers of the change
	CArray<CVector<2>, CVector<2>&>& GetVertexArray();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// the polygon's vertex array
	CArray<CVector<2>, CVector<2>&> m_arrVertex;
};

#endif // !defined(AFX_POLYGON_H__AAA9A385_F0B7_11D4_9E39_00B0D0609AB0__INCLUDED_)
