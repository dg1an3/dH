//////////////////////////////////////////////////////////////////////
// Polygon.cpp: implementation of the CPolygon class.
//
// Copyright (C) 2000-2003 Derek G. Lane
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// utility macros
#include <UtilMacros.h>

// class declaration
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
}	// CPolygon::CPolygon


//////////////////////////////////////////////////////////////////////
// CPolygon::CPolygon(const CPolygon& fromPoly)
// 
// constructs a copy of the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::CPolygon(const CPolygon& fromPoly)
{
	(*this) = fromPoly;

}	// CPolygon::CPolygon(const CPolygon& fromPoly)


//////////////////////////////////////////////////////////////////////
// declares CPolygon as a serializable class
//////////////////////////////////////////////////////////////////////
#define POLYGON_SCHEMA 2
	// Schema 1: geometry description, blocks
IMPLEMENT_SERIAL(CPolygon, CObject, VERSIONABLE_SCHEMA | POLYGON_SCHEMA)

//////////////////////////////////////////////////////////////////////
// CPolygon::~CPolygon
// 
// destroys the polygon
//////////////////////////////////////////////////////////////////////
CPolygon::~CPolygon()
{
}	// CPolygon::~CPolygon


//////////////////////////////////////////////////////////////////////
// CPolygon::operator=
// 
// assignment operator
//////////////////////////////////////////////////////////////////////
CPolygon& CPolygon::operator=(const CPolygon& fromPoly)
{
	// copy the vertices
	// m_arrVertex.Copy(fromPoly.m_arrVertex);
	m_mVertex = fromPoly.m_mVertex;

	// return a reference to this
	return (*this);

}	// CPolygon::operator=


//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexCount
// 
// returns the number of vertices in the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::GetVertexCount() const
{
	return m_mVertex.GetCols();

}	// CPolygon::GetVertexCount


//////////////////////////////////////////////////////////////////////
// CPolygon::GetVertexAt
// 
// returns the vertex at the given index.  forms the modulus index
//////////////////////////////////////////////////////////////////////
const CVectorD<2>& CPolygon::GetVertexAt(int nIndex) const
{
	// form the modulus index
	int nModIndex = nIndex;
	while (nModIndex < 0)
	{
		nModIndex += GetVertexCount();
	}

	nModIndex %= GetVertexCount();

	// return the desired vertex
	return static_cast< const CVectorD<2>& >(m_mVertex[nModIndex]);

}	// CPolygon::GetVertexAt


//////////////////////////////////////////////////////////////////////
// CPolygon::AddVertex
// 
// adds a new vertex to the end of the polygon
//////////////////////////////////////////////////////////////////////
int CPolygon::AddVertex(const CVectorD<2>& v)
{
	// vertex index = current column count
	int nIndex = m_mVertex.GetCols();

	// reshape the matrix
	m_mVertex.Reshape(m_mVertex.GetCols()+1, 2);

	// fire a change
	GetChangeEvent().Fire();

	// return the index of the new vertex
	return nIndex;

}	// CPolygon::AddVertex


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
		nModIndex += GetVertexCount();

	nModIndex %= GetVertexCount();

	// remove the vertex at the modulus index
	// m_arrVertex.RemoveAt(nModIndex);
	ASSERT(FALSE);

	// fire a change event
	GetChangeEvent().Fire();

}	// CPolygon::RemoveVertex


//////////////////////////////////////////////////////////////////////
// CPolygon::LockVertexMatrix
// 
// returns a CHANGEABLE reference to the polygon vertices -- must
//		be paired with an UnlockVertexMatrix call
//////////////////////////////////////////////////////////////////////
CMatrixNxM<REAL>& CPolygon::LockVertexMatrix()
{
	return m_mVertex;

}	// CPolygon::LockVertexMatrix


//////////////////////////////////////////////////////////////////////
// CPolygon::UnlockVertexMatrix
// 
// returns a CHANGEABLE reference to the polygon vertices -- must
//		be paired with an UnlockVertexMatrix call
//////////////////////////////////////////////////////////////////////
void CPolygon::UnlockVertexMatrix(BOOL bChanged)
{
	if (bChanged)
	{
		GetChangeEvent().Fire();
	}

}	// CPolygon::UnlockVertexMatrix


//////////////////////////////////////////////////////////////////////
// CPolygon::GetSignedArea
// 
// computes the signed area of the polygon
//////////////////////////////////////////////////////////////////////
double CPolygon::GetSignedArea() const
{
	// initialize the area to zero
	double area = 0.0;

	// for each vertex, form the cross product of it with the next
	//		vertex
	for (int nAt = 0; nAt < GetVertexCount(); nAt++)
	{
		area += Cross(GetVertexAt(nAt), GetVertexAt(nAt+1));
	}

	// the actual signed area is one-half of the sum
	return area / 2.0;

}	// CPolygon::GetSignedArea


//////////////////////////////////////////////////////////////////////
// CPolygon::MakeConvexHull
// 
// computes the convex hull of the polygon.  doesn't work right now
//////////////////////////////////////////////////////////////////////
void CPolygon::MakeConvexHull()
{
	// only works if more than three vertices are present
	if (GetVertexCount() < 3)
	{
		return;
	}

	// now get the signed area of this polygon
	double signedArea = GetSignedArea();

	// for each vertex
	for (int nAt = 0; nAt <= GetVertexCount(); nAt++)
	{
		// compute the signed area of the current triangle
		CVectorD<2> v1 = GetVertexAt(nAt+1);
		v1 -= GetVertexAt(nAt);
		CVectorD<2> v2 = GetVertexAt(nAt+2);
		v2 -= GetVertexAt(nAt);

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
		CVectorD<2> v1 = GetVertexAt(nAt-1);
		v1 -= GetVertexAt(nAt);
		CVectorD<2> v2 = GetVertexAt(nAt-2);
		v2 -= GetVertexAt(nAt);


		if (Cross(v1, v2) * signedArea > 0)
		{
			// eliminate the middle vertex
			RemoveVertex(nAt-1);
		}
	}

}	// CPolygon::MakeConvexHull


//////////////////////////////////////////////////////////////////////
// CPolygon::Serialize
// 
// serializes the polygon
//////////////////////////////////////////////////////////////////////
void CPolygon::Serialize(CArchive &ar)
{
	// store the schema for the beam object
	UINT nSchema = // ar.IsLoading() ? ar.GetObjectSchema() :
			POLYGON_SCHEMA;
	SERIALIZE_VALUE(ar, nSchema);

	SERIALIZE_VALUE(ar, m_mVertex);

	/*
	if (nSchema == 1)
	{
		CArray<CPackedVectorD<2>, CPackedVectorD<2>&> arrVertex;

		if (!ar.IsLoading())
		{
			arrVertex.SetSize(m_mVertex.GetCols());
			memcpy(arrVertex.GetData(), m_mVertex,  
				2 * m_mVertex.GetCols() * sizeof(REAL));
		}

		arrVertex.Serialize(ar);

		if (ar.IsLoading())
		{
			m_mVertex.Reshape(arrVertex.GetSize(), 2);
			memcpy(m_mVertex, arrVertex.GetData(), 
				2 * arrVertex.GetSize() * sizeof(REAL));
		}
	}
	else if (nSchema == 2)
	{
		SERIALIZE_VALUE(ar, m_mVertex);
	} */

}	// CPolygon::Serialize


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

	DC_TAB(dc) << "Number of vertices = " << GetVertexCount() << "\n";
}
#endif //_DEBUG
