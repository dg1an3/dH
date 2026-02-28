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

//////////////////////////////////////////////////////////////////////
// class CStructure
//
// represents an anatomical structure / VOI
//////////////////////////////////////////////////////////////////////
class CStructure : public CModelObject  
{
public:
	// constructor / destructor
	CStructure(const CString& strName = "");
	virtual ~CStructure();

	// serialization
	DECLARE_SERIAL(CStructure);
	virtual void Serialize(CArchive& ar);

	// contour accessors
	int GetContourCount() const;
	CPolygon *GetContour(int nAt);
	REAL GetContourRefDist(int nIndex) const;

	void AddContour(CPolygon *pPoly, REAL refDist);

	// mesh accessor
	CMesh * GetMesh();
	
	// multi-scale region accessor
	CVolume<VOXEL_REAL> * GetRegion(int nLevel = 0);

	// forms / returns a region conformant to another volume
	template<class TYPE>
	CVolume<VOXEL_REAL> * GetConformRegion(CVolume<TYPE> *pVolume);

	// enum for structure type
	enum  StructType 
	{ 
		eNONE = 0, 
		eTARGET = 1, 
		eOAR = 2
	};

	// accessors for struct type
	StructType GetType(void) const;
	void SetType(StructType newType);

	// series accessor
	CSeries *m_pSeries;

	// accessor for visible flag
	bool IsVisible(void) const;
	void SetVisible(bool bVisible);

	// accessor for display color
	COLORREF GetColor(void) const;
	void SetColor(COLORREF color);	
	
protected:

	// helper - converts contours to a region
	void ContoursToRegion(CVolume<VOXEL_REAL> *pRegion);

	// initialize the filter for sub-regions
	static void InitFilter();

private:
	// contours for the structure
	CTypedPtrArray<CObArray, CPolygon*> m_arrContours;

	// reference distances for the contours
	CArray<REAL, REAL> m_arrRefDist;

	// mesh represent
	CMesh * m_pMesh;

	// region (binary volume) represent
	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>*> m_arrRegions;

	// stores cache of resampled regions
	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL>*> m_arrConformRegions;

	// indicates whether presc is for none, target or OAR
	StructType m_type;

	// visible flag
	bool m_bVisible;

	// color
	COLORREF m_color;

	// static holds the filter kernel
	// static 
		CVolume<VOXEL_REAL> m_kernel;

};	// class CStructure




///////////////////////////////////////////////////////////////////////////////
// CStructure::GetConformRegion
// 
// forms / returns a resampled region for a given basis
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline CVolume<VOXEL_REAL> * CStructure::GetConformRegion(CVolume<TYPE> *pVolume)
{
	CVolume<VOXEL_REAL> *pConformRegion = NULL;

	// find if there is already a region
	for (int nAt = 0; nAt < m_arrConformRegions.GetSize(); nAt++)
	{
		pConformRegion= m_arrConformRegions[nAt];

		if (pConformRegion->GetBasis().IsApproxEqual(pVolume->GetBasis())
			&& pConformRegion->GetWidth() == pVolume->GetWidth())
		{
			return m_arrConformRegions[nAt];
		}
	}

	// else form the resampled region
	pConformRegion = new CVolume<VOXEL_REAL>();
	pConformRegion->ConformTo(pVolume);
	pConformRegion->ClearVoxels();

	// search for closest level in structure's pyramid
	int nLevel = -1;
	CVectorD<3> vDosePixelSpacing = pConformRegion->GetPixelSpacing();
	CVectorD<3> vRegionPixelSpacing;
	do
	{
		nLevel++;
		vRegionPixelSpacing = GetRegion(nLevel)->GetPixelSpacing();
	} while (
		vRegionPixelSpacing[0] * 2.0 < vDosePixelSpacing[0]
		|| vRegionPixelSpacing[1] * 2.0 < vDosePixelSpacing[1]);

	// now resample to appropriate basis
	::Resample(GetRegion(nLevel), pConformRegion, TRUE);

	// and add to cache map
	m_arrConformRegions.Add(pConformRegion);

	return pConformRegion;

}	// CStructure::GetConformRegion


#endif // !defined(AFX_STRUCTURE_H__489BF55D_7352_4E75_9689_6B1818CC8D0D__INCLUDED_)
