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

class CStructure : public CModelObject  
{
public:
	CStructure();
	virtual ~CStructure();

	// contour accessors
	int GetContourCount() const;
	CPolygon *GetContour(int nAt);
	double GetContourRefDist(int nIndex) const;

	CMesh * GetMesh();
	
	CVolume<double> * GetRegion(int nScale = 0);

	void SetRegionResolution(double pixelScale = 1.0);

protected:
	static void InitFilter();
	void ContoursToRegion(CVolume<double> *pRegion);

private:
	// contours for the structure
	CObArray m_arrContours;

	// reference distances for the contours
	CArray<double, double> m_arrRefDist;

	CMesh * m_pMesh;

	CObArray m_arrRegions;

	static CVolume<double> m_filterBinomial;
};

#endif // !defined(AFX_STRUCTURE_H__489BF55D_7352_4E75_9689_6B1818CC8D0D__INCLUDED_)
