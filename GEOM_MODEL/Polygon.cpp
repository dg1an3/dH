//////////////////////////////////////////////////////////////////////
// Polygon.cpp: implementation of the CPolygon class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <UtilMacros.h>

#include "Polygon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CPolygon::CPolygon
// 
// constructs an empty polygon
//////////////////////////////////////////////////////////////////////
CPolygon::CPolygon()
{
}

//////////////////////////////////////////////////////////////////////
// CPolygon::CPolygon(const CPolygon& fromPoly)
// 
// constructs a copy of the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::CPolygon(const CPolygon& fromPoly)
{
	(*this) = fromPoly;
}

//////////////////////////////////////////////////////////////////////
// declares CPolygon as a serializable class
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CPolygon, CObject, 1)

//////////////////////////////////////////////////////////////////////
// CPolygon::~CPolygon
// 
// destroys the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::~CPolygon()
{
}

//////////////////////////////////////////////////////////////////////
// CPolygon::operator=
// 
// assignment operator
//////////////////////////////////////////////////////////////////////
CPolygon& CPolygon::operator=(const CPolygon& fromPoly)
{
	// copy the vertices
	m_arrVertex.Copy(fromPoly.m_arrVertex);

	// return a reference to this
	return (*this);
}

//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexCount
// 
// returns the number of vertices in the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::GetVertexCount()
{
	return m_arrVertex.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertex
// 
// returns the vertex at the given index.  forms the modulus index
//////////////////////////////////////////////////////////////////////
const CPackedVector<2>& CPolygon::GetVertex(int nIndex)
{
	// form the modulus index
	int nModIndex = nIndex;
	while (nModIndex < 0)
		nModIndex += m_arrVertex.GetSize();

	nModIndex %= m_arrVertex.GetSize();

	// return the desired vertex
	return m_arrVertex[nModIndex];
}

//////////////////////////////////////////////////////////////////////
// CPolygon::AddVertex
// 
// adds a new vertex to the end of the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::AddVertex(CVector<2>& v)
{
	// add the vertex
	int nIndex = m_arrVertex.Add(CPackedVector<2>(v));

	// fire a change
	GetChangeEvent().Fire();

	// return the index of the new vertex
	return nIndex;
}

//////////////////////////////////////////////////////////////////////
// CPolygon::RemoveVertex
// 
// removes a vertex from the polygon
//////////////////////////////////////////////////////////////////////
void CPolygon::RemoveVertex(int nIndex)
{
	// form the modulus index
	int nModIndex = nIndex;
	while (nModIndex < 0)
		nModIndex += m_arrVertex.GetSize();

	nModIndex %= m_arrVertex.GetSize();

	// remove the vertex at the modulus index
	m_arrVertex.RemoveAt(nModIndex);

	// fire a change event
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexArray
// 
// returns a CHANGEABLE reference to the polygon vertices
//////////////////////////////////////////////////////////////////////
CArray<CPackedVector<2>, CPackedVector<2>&>& CPolygon::GetVertexArray()
{
	return m_arrVertex;
}

//////////////////////////////////////////////////////////////////////
// CPolygon::GetSignedArea
// 
// computes the signed area of the polygon
//////////////////////////////////////////////////////////////////////
double CPolygon::GetSignedArea()
{
	// initialize the area to zero
	double area = 0.0;

	// for each vertex, form the cross product of it with the next
	//		vertex
	for (int nAt = 0; nAt < GetVertexCount(); nAt++)
		area += Cross(CVector<2>(GetVertex(nAt)), 
			CVector<2>(GetVertex(nAt+1)));

	// the actual signed area is one-half of the sum
	return area / 2.0;
}

//////////////////////////////////////////////////////////////////////
// CPolygon::MakeConvexHull
// 
// computes the convex hull of the polygon.  doesn't work right now
//////////////////////////////////////////////////////////////////////
void CPolygon::MakeConvexHull()
{
	// only works if more than three vertices are present
	if (GetVertexCount() < 3)
		return;

	// now get the signed area of this polygon
	double signedArea = GetSignedArea();

	// for each vertex
	for (int nAt = 0; nAt <= GetVertexCount(); nAt++)
	{
		// compute the signed area of the current triangle
		CVector<2> vBase = GetVertex(nAt);
		CVector<2> v1 = CVector<2>(GetVertex(nAt+1)) - vBase;
		CVector<2> v2 = CVector<2>(GetVertex(nAt+2)) - vBase;

		if (Cross(v1, v2) * signedArea < 0)
		{
			// eliminate the middle vertex
			RemoveVertex(nAt+1);
			nAt--;
		}
	}

	for (nAt = GetVertexCount(); nAt >= 0; nAt--)
	{
		// compute the signed area of the current triangle
		CVector<2> vBase = GetVertex(nAt);
		CVector<2> v1 = CVector<2>(GetVertex(nAt-1)) - vBase;
		CVector<2> v2 = CVector<2>(GetVertex(nAt-2)) - vBase;

		if (Cross(v1, v2) * signedArea > 0)
		{
			// eliminate the middle vertex
			RemoveVertex(nAt-1);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CPolygon::Serialize
// 
// serializes the polygon
//////////////////////////////////////////////////////////////////////
void CPolygon::Serialize(CArchive &ar)
{
	m_arrVertex.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CPolygon diagnostics

#ifdef _DEBUG
void CPolygon::AssertValid() const
{
	CObject::AssertValid();
}

void CPolygon::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	CREATE_TAB(dc.GetDepth());

	DC_TAB(dc) << "Number of vertices = " << m_arrVertex.GetSize() << "\n";
}
#endif //_DEBUG
