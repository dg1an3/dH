//////////////////////////////////////////////////////////////////////
// Volume.h: declaration of the CVolume class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
#define AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>
#include <VectorN.h>
#include <MatrixD.h>
#include <MatrixNxM.h>

#include "ModelObject.h"

#include "Polygon.h"

#undef max
#undef min

#include <limits>
using namespace std;

//////////////////////////////////////////////////////////////////////
// class CVolume<VOXEL_TYPE>
//
// template class representing an arbitrary-typed volume.
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
class CVolume : public CModelObject
{
public:
	// constructor / destructor
	CVolume();
	CVolume(const CVolume& vFrom);

	virtual ~CVolume();

	// dimension attributes
	int GetWidth() const;
	int GetHeight() const;
	int GetDepth() const;

	// total voxels
	int GetVoxelCount();

	// sets the dimensions
	void SetDimensions(int nWidth, int nHeight, int nDepth);

	// retrieves a single voxel value
	VOXEL_TYPE GetVoxelAt(int nX, int nY, int nZ);

	// direct accessor for the voxels
	VOXEL_TYPE ***GetVoxels();
	const VOXEL_TYPE * const * const *GetVoxels() const;

	// sets the raw voxel pointer
	void SetVoxels(VOXEL_TYPE *pVoxels, int nWidth, int nHeight, int nDepth);

	// sets all voxels to zero
	void ClearVoxels();

	// accumulates another volume
	void Accumulate(const CVolume *pVolume, double weight = 1.0);

	// sum of voxels
	VOXEL_TYPE GetSum();

	// max / min of the volume voxel values
	VOXEL_TYPE GetMax();
	VOXEL_TYPE GetMin();

	// basis for volume
	const CMatrixD<4, REAL>& GetBasis();
	void SetBasis(const CMatrixD<4, REAL>& mBasis);

	// serializes the volume
	virtual void Serialize(CArchive& ar);

	// logging
	virtual void Log(CXMLElement *pElem) const;

private:
	// dimensions
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;

	// pointer to raw voxels
	VOXEL_TYPE *m_pVoxels;

	// array of raw voxels (if no replacement is set)
	CArray<VOXEL_TYPE, VOXEL_TYPE&> m_arrVoxels;

	// Iliffe vectors for voxels
	CArray<VOXEL_TYPE *, VOXEL_TYPE *&> m_arrpVoxels;
	CArray<VOXEL_TYPE **, VOXEL_TYPE **&> m_arrppVoxels;

	// basis matrix for volume
	CMatrixD<4, REAL> m_mBasis;

	// flag to recompute the volume's sum
	BOOL m_bRecomputeSum;
	VOXEL_TYPE m_sum;

};	// class CVolume


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>
// 
// default constructor
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>()
	: m_nWidth(0), 
		m_nHeight(0), 
		m_nDepth(0),
		m_pVoxels(NULL)
{
}	// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>(const CVolume<VOXEL_TYPE>&)
// 
// copy constructor
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>(const CVolume<VOXEL_TYPE>& from)
{
	SetDimensions(from.GetWidth(), from.GetHeight(), from.GetDepth());
	ClearVoxels();
	Accumulate(&from);

}	// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>(const CVolume<VOXEL_TYPE>&)


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::~CVolume<VOXEL_TYPE>
// 
// destructor
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline CVolume<VOXEL_TYPE>::~CVolume<VOXEL_TYPE>()
{
}	// CVolume<VOXEL_TYPE>::~CVolume<VOXEL_TYPE>


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetWidth
// 
// dimension attributes
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline int CVolume<VOXEL_TYPE>::GetWidth() const
{ 
	return m_nWidth; 

}	// CVolume<VOXEL_TYPE>::GetWidth


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetHeight
// 
// dimension attributes
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline int CVolume<VOXEL_TYPE>::GetHeight() const
{ 
	return m_nHeight; 

}	// CVolume<VOXEL_TYPE>::GetHeight


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetDepth
// 
// dimension attributes
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline int CVolume<VOXEL_TYPE>::GetDepth() const
{ 
	return m_nDepth; 

}	// CVolume<VOXEL_TYPE>::GetDepth


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetVoxelCount
// 
// returns total voxels in volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline int CVolume<VOXEL_TYPE>::GetVoxelCount()
{
	return GetWidth() * GetHeight() * GetDepth();

}	// CVolume<VOXEL_TYPE>::GetVoxelCount


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::SetDimensions
// 
// sets the dimension attributes
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::SetDimensions(int nWidth, 
										int nHeight, int nDepth)
{
	SetVoxels(NULL, nWidth, nHeight, nDepth);

}	// CVolume<VOXEL_TYPE>::SetDimensions


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetVoxelAt
// 
// accessor for single voxel
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetVoxelAt(int nX, int nY, int nZ)
{
	// perform range-checking
	if ((nX >= 0 && nX < m_nWidth)
		&& (nY >= 0 && nY < m_nHeight)
		&& (nZ >= 0 && nZ < m_nDepth))
	{
		// range OK, so return voxel value
		return GetVoxels()[nZ][nY][nX];
	}

	return (VOXEL_TYPE) 0;

}	// CVolume<VOXEL_TYPE>::GetVoxelAt


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetVoxels
// 
// accessor for voxel iliffe
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE ***CVolume<VOXEL_TYPE>::GetVoxels()
{
	return // &m_arrppVoxels[0];
		m_arrppVoxels.GetData();

}	// CVolume<VOXEL_TYPE>::GetVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetVoxels
// 
// accessor for voxel iliffe - const version
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
const VOXEL_TYPE * const * const *CVolume<VOXEL_TYPE>::GetVoxels() const
{
	return m_arrppVoxels.GetData();

}	// CVolume<VOXEL_TYPE>::GetVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::SetVoxels
// 
// sets the raw voxel pointer
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::SetVoxels(VOXEL_TYPE *pVoxels, 
								int nWidth, int nHeight, int nDepth)
{
	// set the default array of voxels
	m_arrVoxels.SetSize(nDepth * nHeight * nWidth);

	// set the raw voxel pointer
	if (pVoxels == NULL)
	{
		m_pVoxels = (m_arrVoxels.GetSize() > 0) ? &m_arrVoxels[0] : NULL;
	}
	else
	{
		m_pVoxels = pVoxels;
	}

	// now set the size of the Iliffe vectors
	m_arrpVoxels.SetSize(nDepth * nHeight);
	m_arrppVoxels.SetSize(nDepth);

	// only set up Iliffe vectors if there is non-zero voxel count
	if (nDepth > 0 && nHeight > 0 && nWidth > 0)
	{
		for (int nAtZ = 0; nAtZ < nDepth; nAtZ++)
		{
			// set the elements in the outer indirection array
			m_arrppVoxels[nAtZ] = &m_arrpVoxels[nAtZ * m_nHeight];


			for (int nAtY = 0; nAtY < nHeight; nAtY++)
			{
				// set the elements in the inner indirection array
				m_arrpVoxels[nAtZ * nHeight + nAtY] =
					&m_pVoxels[((nAtZ * nHeight) + nAtY) * nWidth];
			}
		}
	}

	// now set the dimension member variables
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nDepth = nDepth;

	m_bRecomputeSum = TRUE;

}	// CVolume<VOXEL_TYPE>::SetVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::ClearVoxels
// 
// clears all voxel values
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::ClearVoxels()
{
	// clear the memory
	memset(m_arrVoxels.GetData(), 0, 
			GetVoxelCount() * sizeof(VOXEL_TYPE));

	GetChangeEvent().Fire();

}	// CVolume<VOXEL_TYPE>::ClearVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::Accumulate
// 
// accumulates voxel values -- other volume must be conformant
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::Accumulate(const CVolume<VOXEL_TYPE> *pVolume, 
											double weight)
{
	ASSERT(GetWidth() == pVolume->GetWidth());
	ASSERT(GetHeight() == pVolume->GetHeight());
	ASSERT(GetDepth() == pVolume->GetDepth());

	VOXEL_TYPE *pDst = &GetVoxels()[0][0][0];
	const VOXEL_TYPE *pSrc = &pVolume->GetVoxels()[0][0][0];

	int nCount = GetVoxelCount();
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		pDst[nAt] += weight * pSrc[nAt];
	}

/*	for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
	{
		for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
		{
			for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
			{
				GetVoxels()[nAtZ][nAtY][nAtX] += weight * 
					pVolume->GetVoxels()[nAtZ][nAtY][nAtX];
			}
		}
	} */

	GetChangeEvent().Fire();

}	// CVolume<VOXEL_TYPE>::Accumulate


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetSum
// 
// forms the sum of the volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetSum()
{
	VOXEL_TYPE sum = (VOXEL_TYPE) 0;
	VOXEL_TYPE *pVoxels = &GetVoxels()[0][0][0];

	int nCount = GetVoxelCount();
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		sum += pVoxels[nAt];
	}

	return sum;

}	// CVolume<VOXEL_TYPE>::GetSum


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetMax
// 
// forms the max of the volume voxel values
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetMax()
{
	VOXEL_TYPE nMaxValue = -numeric_limits<VOXEL_TYPE>::max();

	VOXEL_TYPE *pVoxels = &GetVoxels()[0][0][0];

	int nCount = GetVoxelCount();
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		nMaxValue = __max(nMaxValue, pVoxels[nAt]); 
	}

	return nMaxValue;

}	// CVolume<VOXEL_TYPE>::GetMax


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetMin
// 
// forms the min of the volume voxel values
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetMin()
{
	VOXEL_TYPE nMinValue = numeric_limits<VOXEL_TYPE>::max();

	VOXEL_TYPE *pVoxels = &GetVoxels()[0][0][0];

	int nCount = GetVoxelCount();
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		nMinValue = __min(nMinValue, pVoxels[nAt]); 
	}

	return nMinValue;

}	// CVolume<VOXEL_TYPE>::GetMin


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::Serialize
// 
// serializes the volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_nWidth;
		ar << m_nHeight;
		ar << m_nDepth;

		UINT nBytes = m_nDepth * m_nHeight * m_nWidth * sizeof(VOXEL_TYPE);
		ar.Write(m_pVoxels, nBytes);
	}
	else
	{
		ar >> m_nWidth;
		ar >> m_nHeight;
		ar >> m_nDepth;

		SetVoxels(m_pVoxels, m_nWidth, m_nHeight, m_nDepth);

		UINT nBytes = m_nDepth * m_nHeight * m_nWidth * sizeof(VOXEL_TYPE);
		UINT nActBytes = ar.Read(m_pVoxels, nBytes);
		ASSERT(nActBytes == nBytes);
	}

}	// CVolume<VOXEL_TYPE>::Serialize


///////////////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::Log
// 
// log the volume object
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::Log(CXMLElement *pElem) const
{
	LOG_EXPR(m_nWidth);
	LOG_EXPR(m_nHeight);
	LOG_EXPR(m_nDepth);

	CMatrixNxM<VOXEL_TYPE> mPlane(GetWidth(), GetHeight());

	VOXEL_TYPE ***pppVoxels = ((CVolume<VOXEL_TYPE>&)(*this)).GetVoxels();
	for (int nAt = 0; nAt < m_nDepth; nAt++)
	{
		for (int nAtCol = 0; nAtCol < GetWidth(); nAtCol++)
			for (int nAtRow = 0; nAtRow < GetHeight(); nAtRow++)
				mPlane[nAtCol][nAtRow] = pppVoxels[nAt][nAtRow][nAtCol];
		LogExprExt(mPlane, FMT("mPlane %i", nAt), "");
		// LOG_EXPR_EXT(mPlane);
	}

}	// Log


///////////////////////////////////////////////////////////////////////////////
// Convolve
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void Convolve(CVolume<VOXEL_TYPE> *pVol, CVolume<VOXEL_TYPE> *pKernel, 
					 CVolume<VOXEL_TYPE> *pRes)
{
	pRes->SetDimensions(pVol->GetWidth(), pVol->GetHeight(), pVol->GetDepth());
	pRes->ClearVoxels();

	ASSERT(pKernel->GetWidth() % 2 == 1);
	ASSERT(pKernel->GetHeight() % 2 == 1);

	int nKernelBase = pKernel->GetWidth() / 2;

	VOXEL_TYPE ***pppVoxels = pVol->GetVoxels();
	VOXEL_TYPE ***pppKernel = pKernel->GetVoxels();
	VOXEL_TYPE ***pppRes = pRes->GetVoxels();
	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth(); nAtCol++)
		{
			for (int nAtKernRow = -nKernelBase; nAtKernRow <= nKernelBase; nAtKernRow++)
			{
				for (int nAtKernCol = -nKernelBase; nAtKernCol < nKernelBase; nAtKernCol++)
				{
					if (nAtRow + nAtKernRow > 0 
						&& nAtRow + nAtKernRow < pRes->GetHeight()
						&& nAtCol + nAtKernCol > 0
						&& nAtCol + nAtKernCol < pRes->GetWidth())
					{
						pppRes[0][nAtRow + nAtKernRow][nAtCol + nAtKernCol] += 
							pppVoxels[0][nAtRow][nAtCol] 
								* pppKernel[0][nKernelBase + nAtKernRow][nKernelBase + nAtKernCol];
					}
				}
			}
		}
	}    

}	// Convolve


///////////////////////////////////////////////////////////////////////////////
// CalcBinomialFilter
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void CalcBinomialFilter(CVolume<TYPE> *pVol)
{
	// find largest dimension
	int nMaxDim = __max(pVol->GetDepth(), 
		__max(pVol->GetWidth(), pVol->GetHeight()));

	// calc coeffecients
	CVectorN<TYPE> vCoeff;
	vCoeff.SetDim(nMaxDim);
	CalcBinomialCoeff(vCoeff);
	TYPE norm = (TYPE) pow(2, 2 * (vCoeff.GetDim()-1));

	// populate volume
	TYPE ***pppVoxels = pVol->GetVoxels();
	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth(); nAtCol++)
		{
			pppVoxels[0][nAtRow][nAtCol] = vCoeff[nAtRow] * vCoeff[nAtCol] 
				/ norm;
		}
	}
}	// CalcBinomialFilter


///////////////////////////////////////////////////////////////////////////////
// Decimate
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void Decimate(CVolume<VOXEL_TYPE> *pVol, CVolume<VOXEL_TYPE> *pRes)
{
	int nBase = pVol->GetWidth() / 2;
	int nBaseDec = nBase / 2;

	pRes->SetDimensions(nBaseDec * 2 +1, nBaseDec * 2 + 1, 1);
	
	VOXEL_TYPE ***pppVoxels = pVol->GetVoxels();
	VOXEL_TYPE ***pppRes = pRes->GetVoxels();
	for (int nAtRow = -nBaseDec; nAtRow <= nBaseDec; nAtRow++)
	{
		for (int nAtCol = -nBaseDec; nAtCol <= nBaseDec; nAtCol++)
		{
			pppRes[0][nAtRow + nBaseDec][nAtCol + nBaseDec] = 
				pppVoxels[0][nAtRow*2 + nBase][nAtCol*2 + nBase];
		}
	}

}	// Decimate


///////////////////////////////////////////////////////////////////////////////
// Rotate
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void Rotate(CVolume<VOXEL_TYPE> *pOrig, CVectorD<2> vCenterOrig, 
			REAL angle, CVolume<VOXEL_TYPE> *pNew, CVectorD<2> vCenterNew)
{
	pNew->ClearVoxels();

	CMatrixD<2, VOXEL_TYPE> mRot = ::CreateRotate(angle);
	
	// iterate over new volume's pixels
	for (int nAtRow = 0; nAtRow < pNew->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pNew->GetWidth(); nAtCol++)
		{
			// transform coordinate
			CVectorD<2, VOXEL_TYPE> vAt((REAL) nAtCol, (REAL) nAtRow);

			vAt = mRot * (vAt - vCenterNew) + vCenterOrig;

#ifdef NN_INTERP
			// populate new voxel
			pNew->GetVoxels()[0][nAtRow][nAtCol] = 
				pOrig->GetVoxels()[0][floor(vAt[1]+0.5)][floor(vAt[0]+0.5)];

#else
			REAL xfrac = vAt[0] - floor(vAt[0]);
			REAL yfrac = vAt[1] - floor(vAt[1]);

			pNew->GetVoxels()[0][nAtRow][nAtCol] = 
				(1.0 - yfrac) * (1.0 - xfrac)
					* pOrig->GetVoxelAt((int) floor(vAt[0]), (int) floor(vAt[1]), 0)
				+ (1.0 - yfrac) * xfrac
					* pOrig->GetVoxelAt((int) ceil(vAt[0]), (int) floor(vAt[1]), 0)
				+ yfrac * (1.0 - xfrac)
					* pOrig->GetVoxelAt((int) floor(vAt[0]), (int) ceil(vAt[1]), 0)
				+ yfrac * xfrac 
					* pOrig->GetVoxelAt((int) ceil(vAt[0]), (int) ceil(vAt[1]), 0);
#endif
		}
	}

}	// Rotate


///////////////////////////////////////////////////////////////////////////////
// CreateRegion
// 
// Creates a region (bit mask) from a polygon
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void CreateRegion(CPolygon arrPoly[], int nPolyCount,
				  CVolume<VOXEL_TYPE> *pRegion, BOOL bSetDim = TRUE)
{
	if (bSetDim)
	{
		int nWidth = 0;
		int nHeight = 0;
		for (int nAtPoly = 0; nAtPoly < nPolyCount; nAtPoly++)
		{
			nWidth = __max(ceil(arrPoly[nAtPoly].GetMax()[0]), nWidth);
			nHeight = __max(ceil(arrPoly[nAtPoly].GetMax()[1]), nHeight);
		}

		pRegion->SetDimensions(nWidth, nHeight, 1);
	}

	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(pRegion->GetWidth(), pRegion->GetHeight(), 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	for (int nAtPoly = 0; nAtPoly < nPolyCount; nAtPoly++)
	{
		static CArray<CPoint, CPoint&> arrPoints;
		arrPoints.SetSize(arrPoly[nAtPoly].GetVertexCount());
		for (int nAt = 0; nAt < arrPoly[nAtPoly].GetVertexCount(); nAt++)
		{
			arrPoints[nAt] = arrPoly[nAtPoly].GetVertexAt(nAt);
		}
		dc.Polygon(arrPoints.GetData(), arrPoly[nAtPoly].GetVertexCount());
	}

	// finished with DC
	dc.SelectObject(pOldBitmap);
	dc.DeleteDC();

	// now get the bitmap descriptor (for scan width
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	// resize the buffer
	static CArray<BYTE, BYTE> arrBuffer;
	arrBuffer.SetSize(pRegion->GetHeight() * bm.bmWidthBytes);
	int nByteCount = bitmap.GetBitmapBits(arrBuffer.GetSize(), arrBuffer.GetData());

	// now populate region
	pRegion->ClearVoxels();

	// now populate the region
	for (int nY = 0; nY < pRegion->GetHeight(); nY++)
	{
		for (int nX = 0; nX < pRegion->GetWidth(); nX++)
		{
			if ((arrBuffer[nY * bm.bmWidthBytes + nX / 8] >> (nX % 8)) & 0x01)
			{
				pRegion->GetVoxels()[0][nY][nX] = (VOXEL_TYPE) 1.0;
			}
		}
	}

	// done with bitmap
	bitmap.DeleteObject();

}	// CreateRegion


template<class VOXEL_TYPE>
void WriteCSV(const char *pszFilename, CVolume<VOXEL_TYPE> *pVol, int nAtDepth)
{
	FILE *hFile = fopen(pszFilename, "wt");

	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth(); nAtCol++)
		{
			fprintf(hFile, "%f", pVol->GetVoxels()[nAtDepth][nAtRow][nAtCol]);
			if (nAtCol < pVol->GetWidth()-1)
				fprintf(hFile, ",");
		}

		fprintf(hFile, "\n");
	}

	fclose(hFile);
}

//////////////////////////////////////////////////////////////////////
// DEPRECATED
// function operator>>(CArchive &ar, CVolume<VOXEL_TYPE>*&)
//
// CArchive de-serialization of a volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
CArchive& operator>>(CArchive& ar, CVolume<VOXEL_TYPE>*&pOb) 
{ 
	// if no pointer is passed in,
	if (pOb == NULL)
	{
		// allocate a new volume
		pOb = new CVolume<VOXEL_TYPE>; 
	}

	// serialize the object
	pOb->Serialize(ar);

	// return the archive
	return ar; 
} 

#endif // !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
