// Structure.cpp: implementation of the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\include\structure.h"

#include <Series.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CStructure::CStructure
// 
// constructs a structure
///////////////////////////////////////////////////////////////////////////////
CStructure::CStructure(const CString& strName)
	: CModelObject(strName)
		, m_pSeries(NULL)
		, m_pMesh(NULL)
		, m_bVisible(TRUE)
		, m_type(eNONE)
{
	if (m_kernel.GetWidth() == 0)
	{
		m_kernel.SetDimensions(5, 5, 1);
		CalcBinomialFilter(&m_kernel);
	}

}	// CStructure::CStructure

///////////////////////////////////////////////////////////////////////////////
// CStructure::~CStructure
// 
// destroys structure
///////////////////////////////////////////////////////////////////////////////
CStructure::~CStructure()
{
	delete m_pMesh;

	for (int nAt = 0; nAt < m_arrContours.GetSize(); nAt++)
	{
		delete m_arrContours[nAt];
	}

	for (int nAt = 0; nAt < m_arrRegions.GetSize(); nAt++)
	{
		delete m_arrRegions[nAt];
	}

	for (int nAt = 0; nAt < m_arrConformRegions.GetSize(); nAt++)
	{
		delete m_arrConformRegions[nAt];
	}

}	// CStructure::~CStructure

#define STRUCTURE_SCHEMA 4
	// Schema 1: initial structure schema
	// Schema 2: + structure color
	// Schema 3: + structure type
	// Schema 4: + structure visible

IMPLEMENT_SERIAL(CStructure, CModelObject, VERSIONABLE_SCHEMA | STRUCTURE_SCHEMA)


///////////////////////////////////////////////////////////////////////////////
// CStructure::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CStructure::Serialize(CArchive& ar)
{
	// schema for the plan object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : STRUCTURE_SCHEMA;

	CModelObject::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_pSeries;	
	}
	else
	{
		ar << m_pSeries;
	}

	m_arrContours.Serialize(ar);
	m_arrRefDist.Serialize(ar);

	if (nSchema >= 2)
	{
		if (ar.IsLoading())
		{
			ar >> m_color;	
		}
		else
		{
			ar << m_color;
		}
	}

	if (nSchema >= 3)
	{
		if (ar.IsLoading())
		{
			int nIntType;
			ar >> nIntType;
			m_type = (StructType) nIntType;
		}
		else
		{
			ar << (int) m_type;
		}
	}

	if (nSchema >= 4)
	{
		if (ar.IsLoading())
		{
			ar >> m_bVisible;
		}
		else
		{
			ar << m_bVisible;
		}
	}

}	// CStructure::Serialize



///////////////////////////////////////////////////////////////////////////////
// CStructure::GetContourCount
// 
// returns the number of contours in the mesh
///////////////////////////////////////////////////////////////////////////////
int CStructure::GetContourCount() const
{
	return m_arrContours.GetSize();

}	// CStructure::GetContourCount

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetContour
// 
// returns the contour at the given index
///////////////////////////////////////////////////////////////////////////////
CPolygon *CStructure::GetContour(int nIndex)
{
	return (CPolygon *) m_arrContours[nIndex];

}	// CStructure::GetContour

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetContourRefDist
// 
// returns the reference distance of the indicated contour
///////////////////////////////////////////////////////////////////////////////
REAL CStructure::GetContourRefDist(int nIndex) const
{
	return m_arrRefDist[nIndex];

}	// CStructure::GetContourRefDist

///////////////////////////////////////////////////////////////////////////////
// CStructure::AddContour
// 
// adds a new contour to the structure
///////////////////////////////////////////////////////////////////////////////
void CStructure::AddContour(CPolygon *pPoly, REAL refDist)
{
	m_arrContours.Add(pPoly);
	m_arrRefDist.Add(refDist);

}	// CStructure::AddContour


///////////////////////////////////////////////////////////////////////////////
// CStructure::GetMesh
// 
// returns the mesh
///////////////////////////////////////////////////////////////////////////////
CMesh * CStructure::GetMesh()
{
	return m_pMesh;

}	// CStructure::GetMesh

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetRegion
// 
// forms a new region and returns at requested scale
///////////////////////////////////////////////////////////////////////////////
CVolume<VOXEL_REAL> * CStructure::GetRegion(int nScale)
{
	if (m_arrRegions.GetSize() <= nScale)
	{
		BEGIN_LOG_SECTION(CStructure::GetRegion);

		// TODO: use CPyramid for this

		CVolume<VOXEL_REAL> *pNewRegion = new CVolume<VOXEL_REAL>();
		if (nScale == 0)
		{
			// set size of region
			pNewRegion->ConformTo(m_pSeries->m_pDens);
			ContoursToRegion(pNewRegion);
		}
		else
		{
			CVolume<VOXEL_REAL> *pPrevRegion = GetRegion(nScale - 1);

			CVolume<VOXEL_REAL> convRegion;
			convRegion.SetBasis(pPrevRegion->GetBasis());
			Convolve(pPrevRegion, &m_kernel, &convRegion);

			Decimate(&convRegion, pNewRegion);
		}

		m_arrRegions.Add(pNewRegion);

		END_LOG_SECTION();
	}
	
	return (CVolume<VOXEL_REAL> *) m_arrRegions[nScale];

}	// CStructure::GetRegion


///////////////////////////////////////////////////////////////////////////////
// CStructure::GetType
// 
// accessors for struct type
///////////////////////////////////////////////////////////////////////////////
CStructure::StructType CStructure::GetType(void) const
{
	return m_type;

}	// CStructure::GetType


///////////////////////////////////////////////////////////////////////////////
// CStructure::SetType
// 
// accessors for struct type
///////////////////////////////////////////////////////////////////////////////
void CStructure::SetType(CStructure::StructType newType)
{
	m_type = newType;

}	// CStructure::SetType



///////////////////////////////////////////////////////////////////////////////
// CStructure::GetVisible
// 
// accessor for visible flag
///////////////////////////////////////////////////////////////////////////////
bool CStructure::IsVisible(void) const
{
	return m_bVisible;

}	// CStructure::GetVisible


///////////////////////////////////////////////////////////////////////////////
// CStructure::SetVisible
// 
// accessor for visible flag
///////////////////////////////////////////////////////////////////////////////
void CStructure::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;

}	// CStructure::SetVisible

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetColor
// 
// accessor for display color 
///////////////////////////////////////////////////////////////////////////////
COLORREF CStructure::GetColor(void) const
{
	return m_color;

}	// CStructure::GetColor


///////////////////////////////////////////////////////////////////////////////
// CStructure::SetColor
// 
// accessor for display color 
///////////////////////////////////////////////////////////////////////////////
void CStructure::SetColor(COLORREF color)
{
	m_color = color;

}	// CStructure::SetColor


///////////////////////////////////////////////////////////////////////////////
// CStructure::ContoursToRegion
// 
// converts the contours to a region
///////////////////////////////////////////////////////////////////////////////
void CStructure::ContoursToRegion(CVolume<VOXEL_REAL> *pRegion)
{
	REAL slicePos = pRegion->GetBasis()[3][2];

	CArray<CPolygon *, CPolygon *> arrContours;
	for (int nAt = 0; nAt < GetContourCount(); nAt++)
	{
		REAL zPos = GetContourRefDist(nAt);
		if (IsApproxEqual(zPos, slicePos))
		{
			arrContours.Add(GetContour(nAt));
		}
	}

	CreateRegion(arrContours, pRegion);

}	// CStructure::ContoursToRegion


// stores the convolution kernel
CVolume<VOXEL_REAL> CStructure::m_kernel;

///////////////////////////////////////////////////////////////////////////////
// CStructure::InitFilter
// 
// initializes the kernel
///////////////////////////////////////////////////////////////////////////////
void CStructure::InitFilter()
{
}	// CStructure::InitFilter

