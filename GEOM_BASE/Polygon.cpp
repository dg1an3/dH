// Polygon.cpp: implementation of the CPolygon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Polygon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPolygon::CPolygon()
{

}

CPolygon::CPolygon(const CPolygon& fromPoly)
{
	(*this) = fromPoly;
}

IMPLEMENT_SERIAL(CPolygon, CObject, 1)

CPolygon::~CPolygon()
{
}

CPolygon& CPolygon::operator=(const CPolygon& fromPoly)
{
	m_arrVertex.Copy(fromPoly.m_arrVertex);

	return (*this);
}

int CPolygon::GetVertexCount()
{
	return m_arrVertex.GetSize();
}

const CVector<2>& CPolygon::GetVertex(int nIndex)
{
	return m_arrVertex[(nIndex + m_arrVertex.GetSize()) % m_arrVertex.GetSize()];
}

int CPolygon::AddVertex(CVector<2>& v)
{
	int nIndex = m_arrVertex.Add(v);

	FireChange();

	return nIndex;
}

void CPolygon::RemoveVertex(int nIndex)
{
	m_arrVertex.RemoveAt((nIndex + m_arrVertex.GetSize()) % m_arrVertex.GetSize());

	FireChange();
}

double CPolygon::GetSignedArea()
{
	double area = 0.0;
	for (int nAt = 0; nAt < GetVertexCount()-2; nAt++)
		area += Cross(GetVertex(nAt), GetVertex(nAt+1));
	area += Cross(GetVertex(nAt), GetVertex(0));

	return area / 2.0;
}

void CPolygon::MakeConvexHull()
{
	if (GetVertexCount() < 3)
		return;

	// now get the signed area of this polygon
	double signedArea = GetSignedArea();

	// for each vertex
	for (int nAt = 0; nAt <= GetVertexCount(); nAt++)
	{
		// compute the signed area of the current triangle
		CVector<2> vBase = GetVertex(nAt);
		CVector<2> v1 = GetVertex(nAt+1) - vBase;
		CVector<2> v2 = GetVertex(nAt+2) - vBase;

		if (Cross(v1, v2) * signedArea < 0)
		{
			// eliminate the middle vertex
			RemoveVertex(nAt+1);
		}
	}

	for (nAt = GetVertexCount(); nAt >= 0; nAt--)
	{
		// compute the signed area of the current triangle
		CVector<2> vBase = GetVertex(nAt);
		CVector<2> v1 = GetVertex(nAt-1) - vBase;
		CVector<2> v2 = GetVertex(nAt-2) - vBase;

		if (Cross(v1, v2) * signedArea > 0)
		{
			// eliminate the middle vertex
			RemoveVertex(nAt-1);
		}
	}
}

void CPolygon::Serialize(CArchive &ar)
{
	m_arrVertex.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CBeam diagnostics

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


CArray<CVector<2>, CVector<2>&>& CPolygon::GetVertexArray()
{
	return m_arrVertex;
}
