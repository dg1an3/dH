// Surface.cpp: implementation of the CSurface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <float.h>

#include <UtilMacros.h>

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
{
}

//////////////////////////////////////////////////////////////////////
// CSurface::CSurface
// 
// copy constructor
//////////////////////////////////////////////////////////////////////
CSurface::CSurface(const CSurface& fromSurface)
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
}


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
CVector<3> CSurface::GetBoundsMin() const
{
	// set to max originally
	CVector<3> vMin(FLT_MAX, FLT_MAX, FLT_MAX);

	// accumulate the minimum
	int nVertex;
	for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++) 
    {
		vMin[0] = min(m_arrVertex[nVertex][0], vMin[0]);
		vMin[1] = min(m_arrVertex[nVertex][1], vMin[1]);
		vMin[2] = min(m_arrVertex[nVertex][2], vMin[2]);
    }

	// return the min vector
	return vMin;
}

//////////////////////////////////////////////////////////////////////
// CSurface::GetBoundsMax
// 
// returns the maximum of the mesh
//////////////////////////////////////////////////////////////////////
CVector<3> CSurface::GetBoundsMax() const
{
	// set to -max originally
	CVector<3> vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// accumulate the maximum
	int nVertex;
	for (nVertex = 0; nVertex < m_arrVertex.GetSize(); nVertex++)
    {
		vMax[0] = max(m_arrVertex[nVertex][0], vMax[0]);
		vMax[1] = max(m_arrVertex[nVertex][1], vMax[1]);
		vMax[2] = max(m_arrVertex[nVertex][2], vMax[2]);
    }

	// return the max vector
	return vMax;
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

