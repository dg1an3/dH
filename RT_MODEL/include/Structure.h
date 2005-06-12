// Structure.h: interface for the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRUCTURE_H__489BF55D_7352_4E75_9689_6B1818CC8D0D__INCLUDED_)
#define AFX_STRUCTURE_H__489BF55D_7352_4E75_9689_6B1818CC8D0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ModelObject.h>

#include <Polygon.h>
#include <Mesh.h>
#include <Volumep.h>

class CSeries;

class CStructure : public CModelObject  
{
public:
	BOOL m_bVisible;
	COLORREF m_color;
	CStructure(const CString& strName = "");
	virtual ~CStructure();

	DECLARE_SERIAL(CStructure);
	virtual void Serialize(CArchive& ar);

	// contour accessors
	int GetContourCount() const;
	CPolygon *GetContour(int nAt);
	REAL GetContourRefDist(int nIndex) const;

	void AddContour(CPolygon *pPoly, REAL refDist);

	CMesh * GetMesh();
	
	CVolume<VOXEL_REAL> * GetRegion(int nLevel = 0);

	void SetRegionResolution(REAL pixelScale = 1.0);

	CSeries *m_pSeries;

protected:
	static void InitFilter();
	void ContoursToRegion(CVolume<VOXEL_REAL> *pRegion);

private:
	// contours for the structure
	CTypedPtrArray<CObArray, CPolygon*> m_arrContours;

	// reference distances for the contours
	CArray<REAL, REAL> m_arrRefDist;

	CMesh * m_pMesh;

	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>*> m_arrRegions;

	static CVolume<VOXEL_REAL> m_kernel;
};

#endif // !defined(AFX_STRUCTURE_H__489BF55D_7352_4E75_9689_6B1818CC8D0D__INCLUDED_)
