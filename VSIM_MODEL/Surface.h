// Surface.h: interface for the CSurface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>
#include <Value.h>

#include <Vector.h>
#include <Polygon.h>

class CSurface : public CModelObject
{
public:
	CSurface();
	CSurface(const CSurface& fromSurface);
	DECLARE_SERIAL(CSurface)

	virtual ~CSurface();

	CSurface& operator=(const CSurface& fromSurface);

//	CValue< CString > name;

	int GetContourCount() const;
	CPolygon& GetContour(int nIndex) const;
	double GetContourRefDist(int nIndex) const;

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
	CArray<CVector<3>, CVector<3>&>& 
		GetVertexArray() { return m_arrVertex; }
	CArray<CVector<3>, CVector<3>&>& 
		GetNormalArray() { return m_arrNormal; }

	void Serialize(CArchive &ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CObArray m_arrContours;
	CArray<double, double> m_arrRefDist;

private:
	CArray<int, int> m_arrVertIndex;
	CArray<CVector<3>, CVector<3>&> m_arrVertex;
	CArray<CVector<3>, CVector<3>&> m_arrNormal;
};

inline int CSurface::GetTriangleCount()
{
	return m_arrVertIndex.GetSize() / 3;
}

inline const double *CSurface::GetTriangleVertex(int nTriangle, int nVertex)
{
	return m_arrVertex[m_arrVertIndex[nTriangle*3 + nVertex]-1];
}

inline const double *CSurface::GetTriangleNormal(int nTriangle, int nVertex)
{
	return m_arrNormal[m_arrVertIndex[nTriangle*3 + nVertex]-1];
}

#endif // !defined(AFX_SURFACE_H__C7A6AA31_E5D9_11D4_9E2F_00B0D0609AB0__INCLUDED_)
