// Structure.cpp: implementation of the CStructure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Structure.h"


CVolume<double> CStructure::m_filterBinomial;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStructure::CStructure()
	: m_pMesh(NULL)
{
	InitFilter();
}

CStructure::~CStructure()
{
	delete m_pMesh;

	for (int nAt = 0; nAt < m_arrRegions.GetSize(); nAt++)
	{
		delete m_arrRegions[nAt];
	}
}

CMesh * CStructure::GetMesh()
{
	return m_pMesh;
}

CVolume<double> * CStructure::GetRegion(int nScale)
{
	if (m_arrRegions.GetSize() <= nScale)
	{
		BEGIN_LOG_SECTION(CStructure::GetRegion);

		CVolume<double> *pNewRegion = new CVolume<double>();
		if (nScale == 0)
		{
			// set size of region
			ContoursToRegion(pNewRegion);
		}
		else
		{
			CVolume<double> *pPrevRegion = GetRegion(nScale - 1);

			CVolume<double> convRegion;
			Convolve(pPrevRegion, &m_filterBinomial, &convRegion);

			Decimate(&convRegion, pNewRegion);
		}

		m_arrRegions.Add(pNewRegion);

		END_LOG_SECTION();
	}
	
	return (CVolume<double> *) m_arrRegions[nScale];
}

void CStructure::InitFilter()
{
	if (m_filterBinomial.GetWidth() == 0)
	{
		double filtElem[] = { 1.0, 4.0, 6.0, 4.0, 1.0 };

		m_filterBinomial.SetDimensions(5, 5, 1);

		double sum = 0.0;
		for (int nAtRow = 0; nAtRow < 5; nAtRow++)
		{
			for (int nAtCol = 0; nAtCol < 5; nAtCol++)
			{
				// double value = Gauss2D<double>(((double) nAtRow - 3.0) / 2.0, 
				//		((double) nAtCol - 3.0) / 2.0, 1.0, 1.0);

				double value = filtElem[nAtRow] * filtElem[nAtCol] / 256.0;
				m_filterBinomial.GetVoxels()[0][nAtRow][nAtCol] = value;
				sum += value;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CMesh::GetContourCount
// 
// returns the number of contours in the mesh
//////////////////////////////////////////////////////////////////////
int CStructure::GetContourCount() const
{
	return m_arrContours.GetSize();
}

//////////////////////////////////////////////////////////////////////
// CMesh::GetContour
// 
// returns the contour at the given index
//////////////////////////////////////////////////////////////////////
CPolygon *CStructure::GetContour(int nIndex)
{
	return (CPolygon *) m_arrContours[nIndex];
}

//////////////////////////////////////////////////////////////////////
// CMesh::GetContourRefDist
// 
// returns the reference distance of the indicated contour
//////////////////////////////////////////////////////////////////////
double CStructure::GetContourRefDist(int nIndex) const
{
	return m_arrRefDist[nIndex];
}


void CStructure::ContoursToRegion(CVolume<double> *pRegion)
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
}
