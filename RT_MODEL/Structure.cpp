// Structure.cpp: implementation of the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Structure.h"
#include <Series.h>


CVolume<REAL> CStructure::m_kernel;


/////////////////////////////////////////////////////////////////////////////
// CSeries

IMPLEMENT_SERIAL(CStructure, CModelObject, 1)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CStructure::CStructure
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CStructure::CStructure(const CString& strName)
	: CModelObject(strName),
		m_pSeries(NULL),
		m_pMesh(NULL)
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
// <description>
///////////////////////////////////////////////////////////////////////////////
CStructure::~CStructure()
{
	delete m_pMesh;

	for (int nAt = 0; nAt < m_arrRegions.GetSize(); nAt++)
	{
		delete m_arrRegions[nAt];
	}

}	// CStructure::~CStructure

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetMesh
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CMesh * CStructure::GetMesh()
{
	return m_pMesh;

}	// CStructure::GetMesh

///////////////////////////////////////////////////////////////////////////////
// CStructure::GetRegion
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<REAL> * CStructure::GetRegion(int nScale)
{
	if (m_arrRegions.GetSize() <= nScale)
	{
		BEGIN_LOG_SECTION(CStructure::GetRegion);

		CVolume<REAL> *pNewRegion = new CVolume<REAL>();
		if (nScale == 0)
		{
			// set size of region
			pNewRegion->ConformTo(m_pSeries->m_pDens);
			// ContoursToRegion(pNewRegion);
		}
		else
		{
			CVolume<REAL> *pPrevRegion = GetRegion(nScale - 1);

			CVolume<REAL> convRegion;
			convRegion.SetBasis(pPrevRegion->GetBasis());
			Convolve(pPrevRegion, &m_kernel, &convRegion);

			Decimate(&convRegion, pNewRegion);
		}

		m_arrRegions.Add(pNewRegion);

		END_LOG_SECTION();
	}
	
	return (CVolume<REAL> *) m_arrRegions[nScale];

}	// CStructure::GetRegion

///////////////////////////////////////////////////////////////////////////////
// CStructure::InitFilter
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CStructure::InitFilter()
{
}	// CStructure::InitFilter


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
// CStructure::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CStructure::Serialize(CArchive& ar)
{
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

}	// CStructure::Serialize


///////////////////////////////////////////////////////////////////////////////
// CStructure::ContoursToRegion
// 
// converts the contours to a region
///////////////////////////////////////////////////////////////////////////////
void CStructure::ContoursToRegion(CVolume<REAL> *pRegion)
{
	CreateRegion((CPolygon *) m_arrContours.GetData(), 
		m_arrContours.GetSize(), pRegion);

/*	int nWidth = 0;
	int nHeight = 0;
	for (int nAt = 0; nAt < GetContourCount(); nAt++)
	{
		CPolygon *pPoly = GetContour(nAt);

		nWidth = __max(ceil(pPoly->GetMax()[0]), nWidth);
		nHeight = __max(ceil(pPoly->GetMax()[1]), nHeight);
	}

	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	for (nAt = 0; nAt < GetContourCount(); nAt++)
	{
		CPolygon *pPoly = GetContour(nAt);

		static CArray<CPoint, CPoint&> arrPoints;
		arrPoints.SetSize(pPoly->GetVertexCount());
		for (int nAt = 0; nAt < pPoly->GetVertexCount(); nAt++)
		{
			arrPoints[nAt].x = (int) floor(pPoly->GetVertexAt(nAt)[0] + 0.5);
			arrPoints[nAt].y = (int) floor(pPoly->GetVertexAt(nAt)[1] + 0.5);
		}
		dc.Polygon(arrPoints.GetData(), pPoly->GetVertexCount());
	}

	dc.SelectObject(pOldBitmap);

	dc.DeleteDC();

	BITMAP bm;
	bitmap.GetBitmap(&bm);

	static CArray<BYTE, BYTE> arrBuffer;
	arrBuffer.SetSize(nHeight * bm.bmWidthBytes);
	int nByteCount = bitmap.GetBitmapBits(arrBuffer.GetSize(), arrBuffer.GetData());

	// now populate region
	pRegion->SetDimensions(nWidth, nHeight, 1);
	pRegion->ClearVoxels();

	for (int nY = 0; nY < pRegion->GetHeight(); nY++)
	{
		for (int nX = 0; nX < pRegion->GetWidth(); nX++)
		{
			if ((arrBuffer[nY * bm.bmWidthBytes + nX / 8] >> (nX % 8)) & 0x01)
			{
				pRegion->GetVoxels()[0][nY][nX] = 1.0;
			}
		}
	}

	bitmap.DeleteObject(); */

}	// CStructure::ContoursToRegion
