// Surface.cpp: implementation of the CSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Surface.h"

#include <float.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSurface::CSurface()
{

}

CSurface::CSurface(const CSurface& fromSurface)
{
	(*this) = fromSurface;
}

CSurface::~CSurface()
{
	for (int nAt = 0; nAt < m_arrContours.GetSize(); nAt++)
		delete m_arrContours[nAt];
	m_arrContours.RemoveAll();
}

CSurface& CSurface::operator=(const CSurface& fromSurface)
{
	name.Set(fromSurface.name.Get());

	for (int nAt = 0; nAt < fromSurface.GetContourCount(); nAt++)
		m_arrContours.Add(new CPolygon(fromSurface.GetContour(nAt)));
	m_arrRefDist.Copy(fromSurface.m_arrRefDist);

	m_arrVertIndex.Copy(fromSurface.m_arrVertIndex);
	m_arrVertex.Copy(fromSurface.m_arrVertex);
	m_arrNormal.Copy(fromSurface.m_arrNormal);

	return (*this);
}

int CSurface::GetContourCount() const
{
	return m_arrContours.GetSize();
}

CPolygon& CSurface::GetContour(int nIndex) const
{
	return *(CPolygon *)m_arrContours[nIndex];
}

double CSurface::GetContourRefDist(int nIndex) const
{
	return m_arrRefDist[nIndex];
}

CVector<3> CSurface::GetBoundsMin() const
{
	CVector<3> vMin(FLT_MAX, FLT_MAX, FLT_MAX);

	int nVertex;
	for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++) 
    {
		vMin[0] = min(m_arrVertex[nVertex][0], vMin[0]);
		vMin[1] = min(m_arrVertex[nVertex][1], vMin[1]);
		vMin[2] = min(m_arrVertex[nVertex][2], vMin[2]);
    }

	return vMin;
}

CVector<3> CSurface::GetBoundsMax() const
{
	CVector<3> vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	int nVertex;
	for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++)
    {
		vMax[0] = max(m_arrVertex[nVertex][0], vMax[0]);
		vMax[1] = max(m_arrVertex[nVertex][1], vMax[1]);
		vMax[2] = max(m_arrVertex[nVertex][2], vMax[2]);
    }

	return vMax;
}

double CSurface::GetMaxSize()
{
	return (GetBoundsMax() - GetBoundsMin()).GetLength() 
		/ sqrt(2.0); 
}

/* void CSurface::AdjustSurface()
{
	CVector<3> vMin = GetBoundsMin();
	CVector<3> vMax = GetBoundsMax();
	CVector<3> vAvg = (vMin + vMax) * 0.5;

	int nVertex;
	for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++) 
    {
		m_arrVertex[nVertex] -= vAvg;
    }
}
*/

IMPLEMENT_SERIAL(CSurface, CObject, 3)

void CSurface::Serialize(CArchive &ar)
{
	name.Serialize(ar);

	m_arrContours.Serialize(ar);
	m_arrRefDist.Serialize(ar);

	m_arrVertIndex.Serialize(ar);
	m_arrVertex.Serialize(ar);
	m_arrNormal.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CSeries diagnostics

#ifdef _DEBUG
void CSurface::AssertValid() const
{
	CObject::AssertValid();

	m_arrVertIndex.AssertValid();
	m_arrVertex.AssertValid();
	m_arrNormal.AssertValid();
}

void CSurface::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	CREATE_TAB(dc.GetDepth());

	DC_TAB(dc) << "Number of triangles = " << m_arrVertIndex.GetSize() << "\n";

	CVector<3> vMin = GetBoundsMin();
	DC_TAB(dc) << "Bounding box min = <" << vMin[0] 
		<< ", " << vMin[1] 
		<< ", " << vMin[2] << ">\n";

	CVector<3> vMax = GetBoundsMax();
	DC_TAB(dc) << "Bounding box max = <" << vMax[0] 
		<< ", " << vMax[1] 
		<< ", " << vMax[2] << ">\n";
}
#endif //_DEBUG

