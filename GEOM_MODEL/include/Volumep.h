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
//#include <MatrixBase.inl>

#include <ippi.h>

#include "ModelObject.h"

#include "Polygon.h"

#undef max
#undef min

#include <limits>
using namespace std;

typedef float VOXEL_REAL;
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

	// sets the dimensions & basis to the same as other volume
	// NOTE: this needs to be defined within the class, because
	//		of limitation of Microsoft compiler
	template<class OTHER_VOXEL_TYPE>
	void ConformTo(const CVolume<OTHER_VOXEL_TYPE> *pVolume)
	{
		SetBasis(pVolume->GetBasis());

		SetDimensions(pVolume->GetWidth(), 
			pVolume->GetHeight(),
			pVolume->GetDepth());

	}	// CVolume<VOXEL_TYPE>::ConformTo

	// retrieves a single voxel value
	VOXEL_TYPE GetVoxelAt(int nX, int nY, int nZ);

	// direct accessor for the voxels
	VOXEL_TYPE ***GetVoxels();
	const VOXEL_TYPE * const * const *GetVoxels() const;
	void VoxelsChanged();

	// sets the raw voxel pointer
	void SetVoxels(VOXEL_TYPE *pVoxels, int nWidth, int nHeight, int nDepth);

	// sets all voxels to zero
	void ClearVoxels();

	// accumulates another volume
	void Accumulate(const CVolume *pVolume, double weight = 1.0, BOOL bUseBoundingBox = FALSE);

	// sum of voxels
	VOXEL_TYPE GetSum();

	// max / min of the volume voxel values
	VOXEL_TYPE GetMax();
	VOXEL_TYPE GetMin();
	VOXEL_TYPE GetAvg();

	// threshold bounds
	VOXEL_TYPE GetThreshold() const { return m_thresh; }
	void SetThreshold(VOXEL_TYPE thresh);
	const CRect& GetThresholdBounds() const;

	// basis for volume
	const CMatrixD<4>& GetBasis() const;
	void SetBasis(const CMatrixD<4>& mBasis);

	// helper for pixel spacing
	CVectorD<3> GetPixelSpacing() const;

	// serializes the volume
	virtual void Serialize(CArchive& ar);

	// logging
	virtual void Log(CXMLElement *pElem) const;
	void LogPlane(int nPlane, CXMLElement *pElem) const;

private:
	// dimensions
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;

	// pointer to raw voxels
	VOXEL_TYPE *m_pVoxels;

	// flag to indicate ownership of voxels
	BOOL m_bFreeVoxels;

	// Iliffe vectors for voxels
	CArray<VOXEL_TYPE *, VOXEL_TYPE *&> m_arrpVoxels;
	CArray<VOXEL_TYPE **, VOXEL_TYPE **&> m_arrppVoxels;

	// basis matrix for volume
	CMatrixD<4> m_mBasis;

	// flag to recompute the volume's sum
	mutable BOOL m_bRecomputeSum;
	mutable VOXEL_TYPE m_sum;

	// caches threshold and threshold bounds
	mutable BOOL m_bRecomputeThresh;
	VOXEL_TYPE m_thresh;
	mutable CRect m_rectThresh;

#if defined(USE_IPP)
	// accumulator helper buffer
	CVolume<VOXEL_TYPE> *m_pAccumBuffer;
#endif

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
		m_pVoxels(NULL),
		m_bFreeVoxels(TRUE)
#if defined(USE_IPP)
		, m_pAccumBuffer(NULL)
#endif
{
}	// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>(const CVolume<VOXEL_TYPE>&)
// 
// copy constructor
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline CVolume<VOXEL_TYPE>::CVolume<VOXEL_TYPE>(const CVolume<VOXEL_TYPE>& from)
	: m_nWidth(0), 
		m_nHeight(0), 
		m_nDepth(0),
		m_pVoxels(NULL),
		m_bFreeVoxels(TRUE)
#if defined(USE_IPP)
		, m_pAccumBuffer(NULL)
#endif
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
	if (m_bFreeVoxels)
	{
		FreeValues(m_pVoxels);
	}

#if defined(USE_IPP)
	delete m_pAccumBuffer;
#endif
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
	// make sure divisible by 4
	nWidth = (((nWidth - 1) / 16) + 1) * 16;

	if (nWidth != GetWidth()
		|| nHeight != GetHeight()
		|| nDepth != GetDepth())
	{
		SetVoxels(NULL, nWidth, nHeight, nDepth);
	}

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
	return m_arrppVoxels.GetData();

}	// CVolume<VOXEL_TYPE>::GetVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetVoxels
// 
// accessor for voxel iliffe - const version
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline const VOXEL_TYPE * const * const *CVolume<VOXEL_TYPE>::GetVoxels() const
{
	return m_arrppVoxels.GetData();

}	// CVolume<VOXEL_TYPE>::GetVoxels


///////////////////////////////////////////////////////////////////////////////
// VoxelsChanged
// 
// notifies that voxels have changed
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::VoxelsChanged() 
{
	m_bRecomputeSum = TRUE;
	m_bRecomputeThresh = TRUE;

	GetChangeEvent().Fire();

}	// VoxelsChanged

//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::SetVoxels
// 
// sets the raw voxel pointer
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::SetVoxels(VOXEL_TYPE *pVoxels, 
								int nWidth, int nHeight, int nDepth)
{
	// check width
	// ASSERT(nWidth % (32 / sizeof(VOXEL_TYPE)) == 0);

	// set the raw voxel pointer
	if (pVoxels == NULL)
	{
		if (m_bFreeVoxels)
		{
			FreeValues(m_pVoxels);
		}
		AllocValues(nWidth * nHeight * nDepth, m_pVoxels);
		m_bFreeVoxels = TRUE;
	}
	else
	{
		m_pVoxels = pVoxels;
		m_bFreeVoxels = FALSE;
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
			m_arrppVoxels[nAtZ] = &m_arrpVoxels[nAtZ * nHeight];


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

	VoxelsChanged();

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
	memset(m_pVoxels, // m_arrVoxels.GetData(), 
			0, 
			GetVoxelCount() * sizeof(VOXEL_TYPE));

	VoxelsChanged();

}	// CVolume<VOXEL_TYPE>::ClearVoxels


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::Accumulate
// 
// accumulates voxel values -- other volume must be conformant
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::Accumulate(const CVolume<VOXEL_TYPE> *pVolume, 
											double weight /* = 1.0 */, 
											BOOL bUseBoundingBox /* = FALSE */ )
{
	ASSERT(GetWidth() == pVolume->GetWidth());
	ASSERT(GetHeight() == pVolume->GetHeight());
	ASSERT(GetDepth() == pVolume->GetDepth());

//	ASSERT(GetBasis().IsApproxEqual(pVolume->GetBasis()));

	CRect rectBounds(0, 0, pVolume->GetWidth()-1, pVolume->GetHeight()-1);
	if (bUseBoundingBox)
	{
		rectBounds = pVolume->GetThresholdBounds();
	}

#if defined(USE_IPP)
	if (!m_pAccumBuffer)
	{
		m_pAccumBuffer = new CVolume<VOXEL_TYPE>();
	}
	m_pAccumBuffer->ConformTo(this);
	IppiSize roiSize = { rectBounds.Width(), rectBounds.Height() };
	IppStatus stat = ippiMulC_32f_C1R(
		&pVolume->GetVoxels()[0][rectBounds.top][rectBounds.left], 
		pVolume->GetWidth() * sizeof(VOXEL_REAL),
		(VOXEL_REAL) weight,
		&m_pAccumBuffer->GetVoxels()[0][rectBounds.top][rectBounds.left], 
		m_pAccumBuffer->GetWidth() * sizeof(VOXEL_REAL),
		roiSize);

	stat = ippiAdd_32f_C1IR(
		&m_pAccumBuffer->GetVoxels()[0][rectBounds.top][rectBounds.left],
		m_pAccumBuffer->GetWidth() * sizeof(VOXEL_REAL),
		&GetVoxels()[0][rectBounds.top][rectBounds.left],
		GetWidth() * sizeof(VOXEL_REAL),
		roiSize);

#elif defined(ACCUMULATE_FLAT_LOOP)
	VOXEL_TYPE *pDst = &GetVoxels()[0][0][0];
	const VOXEL_TYPE *pSrc = &pVolume->GetVoxels()[0][0][0];

	int nCount = GetVoxelCount();
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		pDst[nAt] += weight * pSrc[nAt];
	}
#else
	VOXEL_TYPE ***pppDstVoxels = GetVoxels();
	const VOXEL_TYPE * const * const *pppSrc = pVolume->GetVoxels();
	for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
	{
		for (int nAtY = rectBounds.top; nAtY <= rectBounds.bottom; nAtY++)
		{
			for (int nAtX = rectBounds.left; nAtX <= rectBounds.right; nAtX++)
			{
				pppDstVoxels[nAtZ][nAtY][nAtX] += weight * 
					pppSrc[nAtZ][nAtY][nAtX];
			}
		}
	}
#endif

	VoxelsChanged();

}	// CVolume<VOXEL_TYPE>::Accumulate


//////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetSum
// 
// forms the sum of the volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetSum()
{
	if (m_bRecomputeSum)
	{
		m_sum = (VOXEL_TYPE) 0;
		VOXEL_TYPE *pVoxels = &GetVoxels()[0][0][0];

		int nCount = GetVoxelCount();
		for (int nAt = 0; nAt < nCount; nAt++)
		{
			m_sum += pVoxels[nAt];
		}
		m_bRecomputeSum = FALSE;
	}

	return m_sum;

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
// CVolume<VOXEL_TYPE>::GetMin
// 
// forms the min of the volume voxel values
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline VOXEL_TYPE CVolume<VOXEL_TYPE>::GetAvg()
{
	return GetSum() / (VOXEL_TYPE) GetVoxelCount();
}


///////////////////////////////////////////////////////////////////////////////
// SetThreshold
// 
// sets the threshold for the bounding box
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::SetThreshold(VOXEL_TYPE thresh)
{
	m_thresh = thresh;
	m_bRecomputeThresh = TRUE;

}	// SetThreshold


///////////////////////////////////////////////////////////////////////////////
// GetThresholdBounds
// 
// returns the bounding box for the given threshold value
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline const CRect& CVolume<VOXEL_TYPE>::GetThresholdBounds() const
{
	if (m_bRecomputeThresh)
	{
		m_rectThresh.left = GetWidth();
		m_rectThresh.right = 0;
		m_rectThresh.top = GetHeight();
		m_rectThresh.bottom = 0;
		
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					if (GetVoxels()[nAtZ][nAtY][nAtX] > m_thresh)
					{
						m_rectThresh.left = __min(m_rectThresh.left, nAtX);
						m_rectThresh.top = __min(m_rectThresh.top, nAtY);

						m_rectThresh.right = __max(m_rectThresh.right, nAtX);
						m_rectThresh.bottom = __max(m_rectThresh.bottom, nAtY);
					}
				}
			}
		}

		m_bRecomputeThresh = FALSE;
	}

	//// TODO: reenable thershold boudns
	//m_rectThresh.left = 0; 
	//m_rectThresh.right = GetWidth()-1;
	//m_rectThresh.top = 0; 
	//m_rectThresh.bottom = GetHeight()-1;

	return m_rectThresh;

}	// GetThresholdBounds


///////////////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetBasis
// 
// basis for volume
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
const CMatrixD<4>& CVolume<VOXEL_TYPE>::GetBasis() const
{
	return m_mBasis;

}	// CVolume<VOXEL_TYPE>::GetBasis


///////////////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::SetBasis
// 
// sets volume's basis matrix
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void CVolume<VOXEL_TYPE>::SetBasis(const CMatrixD<4>& mBasis)
{
	m_mBasis = mBasis;
	GetChangeEvent().Fire();

}	// CVolume<VOXEL_TYPE>::SetBasis


///////////////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::GetPixelSpacing
// 
// Helper function to calculate the pixel spacing for the volume
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
CVectorD<3> CVolume<VOXEL_TYPE>::GetPixelSpacing() const
{
	CMatrixD<4> mUnit;
	mUnit[0][3] = 1.0;
	mUnit[1][3] = 1.0;
	mUnit[2][3] = 1.0;

	CMatrixD<4> mXformUnit = GetBasis() * mUnit;

	CVectorD<3> vSpacing;
	vSpacing[0] = (FromHG<3, REAL>(mXformUnit[0]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();
	vSpacing[1] = (FromHG<3, REAL>(mXformUnit[1]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();
	vSpacing[2] = (FromHG<3, REAL>(mXformUnit[2]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();

	return vSpacing;

}	// CVolume<VOXEL_TYPE>::GetPixelSpacing



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
		ar << m_mBasis;

		ar << m_nWidth;
		ar << m_nHeight;
		ar << m_nDepth;

		UINT nBytes = m_nDepth * m_nHeight * m_nWidth * sizeof(VOXEL_TYPE);
		ar.Write(m_pVoxels, nBytes);
	}
	else
	{
		ar >> m_mBasis;

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
		LogPlane(nAt, pElem);

/*		for (int nAtCol = 0; nAtCol < GetWidth(); nAtCol++)
			for (int nAtRow = 0; nAtRow < GetHeight(); nAtRow++)
				mPlane[nAtCol][nAtRow] = pppVoxels[nAt][nAtRow][nAtCol];
		// LogExprExt(mPlane, FMT("mPlane %i", nAt), "");
		LOG_EXPR_EXT_DESC(mPlane, FMT("mPlane %i", nAt)); */
	}

}	// Log


///////////////////////////////////////////////////////////////////////////////
// CVolume<VOXEL_TYPE>::LogPlane
// 
// log the volume object
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void CVolume<VOXEL_TYPE>::LogPlane(int nPlane, CXMLElement *pElem) const
{
	CMatrixNxM<VOXEL_TYPE> mPlane(GetWidth(), GetHeight());

	VOXEL_TYPE ***pppVoxels = ((CVolume<VOXEL_TYPE>&)(*this)).GetVoxels();

	for (int nAtCol = 0; nAtCol < GetWidth(); nAtCol++)
	{
		for (int nAtRow = 0; nAtRow < GetHeight(); nAtRow++)
		{
			mPlane[nAtCol][nAtRow] = pppVoxels[nPlane][nAtRow][nAtCol];
		}
	}

	LOG_EXPR_EXT_DESC(mPlane, FMT("mPlane %i", nPlane));

}	// LogPlane


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

	// TODO: fix this (caused by memory alignment
	// ASSERT(pKernel->GetWidth() % 2 == 1);
	ASSERT(pKernel->GetHeight() % 2 == 1);

	int nKernelBase = pKernel->GetHeight() / 2;

	VOXEL_TYPE ***pppVoxels = pVol->GetVoxels();
	VOXEL_TYPE ***pppKernel = pKernel->GetVoxels();
	VOXEL_TYPE ***pppRes = pRes->GetVoxels();
	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth(); nAtCol++)
		{
			for (int nAtKernRow = -nKernelBase; nAtKernRow <= nKernelBase; nAtKernRow++)
			{
				for (int nAtKernCol = -nKernelBase; nAtKernCol <= nKernelBase; nAtKernCol++)
				{
					if (nAtRow + nAtKernRow >= 0 
						&& nAtRow + nAtKernRow < pRes->GetHeight()
						&& nAtCol + nAtKernCol >= 0
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

	// flag change
	pRes->VoxelsChanged();

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
		// TODO: fix for with (always / 4)
		// __max(pVol->GetWidth(), 
		pVol->GetHeight());

	// calc coeffecients
	CVectorN<TYPE> vCoeff;
	vCoeff.SetDim(nMaxDim);
	CalcBinomialCoeff(vCoeff);
	TYPE norm = (TYPE) pow(2, 2 * (vCoeff.GetDim()-1));

	// populate volume
	pVol->ClearVoxels();
	TYPE ***pppVoxels = pVol->GetVoxels();
	for (int nAtRow = 0; nAtRow < pVol->GetHeight(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < vCoeff.GetDim(); /* pVol->GetWidth(); */nAtCol++)
		{
			pppVoxels[0][nAtRow][nAtCol] = vCoeff[nAtRow] * vCoeff[nAtCol] 
				/ norm;
		}
	}

	// flag change
	pVol->VoxelsChanged();

}	// CalcBinomialFilter


///////////////////////////////////////////////////////////////////////////////
// Decimate
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
inline void Decimate(CVolume<VOXEL_TYPE> *pVol, CVolume<VOXEL_TYPE> *pRes)
{
	// set new dimensions
	int nBase = pVol->GetWidth() / 2;
	pRes->SetDimensions(nBase, nBase, 1);
	pRes->ClearVoxels();

	// set up basis matrix
	CMatrixD<4> mBasis = pVol->GetBasis();
	mBasis[0] *= 2.0;
	mBasis[1] *= 2.0;
	mBasis[2] *= 2.0;
	pRes->SetBasis(mBasis);

	// now copy voxels
	VOXEL_TYPE ***pppVoxels = pVol->GetVoxels();
	VOXEL_TYPE ***pppRes = pRes->GetVoxels();
	for (int nAtRow = 0; nAtRow < pVol->GetHeight() / 2; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < pVol->GetWidth() / 2; nAtCol++)
		{
			pppRes[0][nAtRow][nAtCol] = 
				pppVoxels[0][nAtRow*2][nAtCol*2];
		}
	}

	// flag change
	pRes->VoxelsChanged();

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
			CVectorD<2, VOXEL_TYPE> vAt((VOXEL_TYPE) nAtCol, (VOXEL_TYPE) nAtRow);

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

	// flag change
	pNew->VoxelsChanged();

}	// Rotate



///////////////////////////////////////////////////////////////////////////////
// ClipRaster
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
inline void ClipRaster(int nDim, 
				int nSrcStart, int nSrcEnd, 
				CVectorD<3,int> vStart, CVectorD<3,int> vOffset, 
				int *pnDstStart, int *pnDstEnd)
{
	if (vOffset[nDim] == 0)
	{
		// see if we're between the start & end
		if (vStart[nDim] < (nSrcStart << 8) || vStart[nDim] > (nSrcEnd << 8))
		{
			int nTemp = (*pnDstStart);
			(*pnDstStart) = (*pnDstEnd);
			(*pnDstEnd) = nTemp;
		}

		return;
	}

	int nLambdaSrcStart = 
		((nSrcStart << 8) - vStart[nDim]) / vOffset[nDim];
	int nLambdaSrcEnd = 
		((nSrcEnd << 8) - vStart[nDim]) / vOffset[nDim];

	(*pnDstStart) = __max(__min(nLambdaSrcStart, nLambdaSrcEnd), (*pnDstStart));
	(*pnDstEnd) = __min(__max(nLambdaSrcStart, nLambdaSrcEnd), (*pnDstEnd));

}	// ClipRaster



///////////////////////////////////////////////////////////////////////////////
// Resample
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
#if defined(USE_IPP)
inline void Resample(CVolume<VOXEL_REAL> *pOrig, CVolume<VOXEL_REAL> *pNew, 
					 BOOL bBilinear = FALSE)
{
	// form translation basis
	CMatrixD<4> mXform = pOrig->GetBasis();
	mXform.Invert();
	mXform *= pNew->GetBasis();

	double coeffs[2][3];
	coeffs[0][0] = mXform[0][0];
	coeffs[0][1] = mXform[1][0];
	coeffs[0][2] = mXform[3][0];
	coeffs[1][0] = mXform[0][1];
	coeffs[1][1] = mXform[1][1];
	coeffs[1][2] = mXform[3][1];

	IppiSize sz = { pOrig->GetWidth(), pOrig->GetHeight() };
	IppiRect rectOrig = {0, 0, pOrig->GetWidth(), pOrig->GetHeight() };
	IppiRect rectNew = {0, 0, pNew->GetWidth(), pNew->GetHeight() };
	IppStatus stat = ippiWarpAffineBack_32f_C1R(&pOrig->GetVoxels()[0][0][0], sz, 
		pOrig->GetWidth() * sizeof(VOXEL_REAL), rectOrig,
		&pNew->GetVoxels()[0][0][0], pNew->GetWidth() * sizeof(VOXEL_REAL), rectNew,
		coeffs, IPPI_INTER_LINEAR);

	// flag change
	pNew->VoxelsChanged();

}	// Resample

#else

template<class VOXEL_TYPE>
inline void Resample(CVolume<VOXEL_TYPE> *pOrig, CVolume<VOXEL_TYPE> *pNew, 
					 BOOL bBilinear = FALSE)
{
	// form translation basis
	CMatrixD<4> mXform = pOrig->GetBasis();
	mXform.Invert();
	mXform *= pNew->GetBasis();

	// extract direction cosines
	CVectorD<3,int> vX;
	vX[0] = mXform[0][0] * 256.0;
	vX[1] = mXform[0][1] * 256.0;
	vX[2] = mXform[0][2] * 256.0;

	CVectorD<3,int> vY;
	vY[0] = mXform[1][0] * 256.0;
	vY[1] = mXform[1][1] * 256.0;
	vY[2] = mXform[1][2] * 256.0;

	CVectorD<3> vOY_real = FromHG<3, REAL>(mXform[3]);
	CVectorD<3,int> vOY;
	vOY[0] = 256 * vOY_real[0];
	vOY[1] = 256 * vOY_real[1];
	vOY[2] = 256 * vOY_real[2];
	if (!bBilinear)
	{
		vOY[0] += 128;
		vOY[1] += 128;
		vOY[2] += 128;

	}

	VOXEL_REAL ***pppDstVoxels = pNew->GetVoxels();
	VOXEL_REAL ***pppSrcVoxels = pOrig->GetVoxels();
	for (int nY = 0; nY < pNew->GetHeight(); nY++)
	{
		CVectorD<3, int> vOX = vOY;

		// now clip
		int nStart = 0;
		int nEnd = pNew->GetWidth();
		ClipRaster(0, 2, pOrig->GetWidth()-2, vOX, vX, &nStart, &nEnd);
		ClipRaster(1, 2, pOrig->GetHeight()-2, vOX, vX, &nStart, &nEnd);
		ASSERT(nStart >= 0);
		ASSERT(nEnd <= pNew->GetWidth());

		// adjust start
		vOX += nStart * vX;

		if (bBilinear)
		{
			for (int nX = nStart; nX < nEnd; nX++)
			{
				REAL xfrac = (REAL) (vOX[0] & 255) / 256.0;
				REAL yfrac = (REAL) (vOX[1] & 255) / 256.0;

				pppDstVoxels[0][nY][nX] =
					(1.0 - yfrac) * (1.0 - xfrac)
						* pppSrcVoxels[0][vOX[1] >> 8][vOX[0] >> 8];

				pppDstVoxels[0][nY][nX] +=
					(1.0 - yfrac) * xfrac
						* pppSrcVoxels[0][vOX[1] >> 8][(vOX[0] >> 8) + 1];

				pppDstVoxels[0][nY][nX] +=
					yfrac * (1.0 - xfrac)
						* pppSrcVoxels[0][(vOX[1] >> 8) + 1][vOX[0] >> 8];

				pppDstVoxels[0][nY][nX] +=
					yfrac * xfrac 
						* pppSrcVoxels[0][(vOX[1] >> 8) + 1][(vOX[0] >> 8) + 1];

				vOX[0] += vX[0]; 
				vOX[1] += vX[1]; 
				vOX[2] += vX[2]; 
			} 

		}
		else
		{
			// now resample
			for (int nX = nStart; nX < nEnd; nX++)
			{
				pppDstVoxels[0][nY][nX] =
					pppSrcVoxels[0][vOX[1] >> 8][vOX[0] >> 8];

				vOX[0] += vX[0]; 
				vOX[1] += vX[1]; 
				vOX[2] += vX[2]; 
			}
		}

		vOY += vY;
	}

	// flag change
	pNew->VoxelsChanged();

}	// Resample

#endif


///////////////////////////////////////////////////////////////////////////////
// CreateRegion
// 
// Creates a region (bit mask) from a polygon
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void CreateRegion(const CArray<CPolygon *, CPolygon *>& arrPolygons,
				  CVolume<VOXEL_TYPE> *pRegion, BOOL bSetDim = FALSE)
{
#ifdef USE_SETDIM
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
#endif

	CMatrixD<4> mBasis = pRegion->GetBasis();

	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(pRegion->GetWidth(), pRegion->GetHeight(), 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	for (int nAtPoly = 0; nAtPoly < arrPolygons.GetSize(); nAtPoly++)
	{
		static CArray<CPoint, CPoint&> arrPoints;
		arrPoints.SetSize(arrPolygons[nAtPoly]->GetVertexCount());
		for (int nAt = 0; nAt < arrPolygons[nAtPoly]->GetVertexCount(); nAt++)
		{
			CVectorD<2> vVert = arrPolygons[nAtPoly]->GetVertexAt(nAt);
			vVert[0] = (vVert[0] - mBasis[3][0]) / mBasis[0][0];
			vVert[1] = (vVert[1] - mBasis[3][1]) / mBasis[1][1];

			arrPoints[nAt] = vVert;
		}
		dc.Polygon(arrPoints.GetData(), arrPolygons[nAtPoly]->GetVertexCount());
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
			if ((arrBuffer[nY * bm.bmWidthBytes + nX / 8] >> (7 - nX % 8)) & 0x01)
			{
				pRegion->GetVoxels()[0][nY][nX] = (VOXEL_TYPE) 1.0;
			}
		}
	}

	// done with bitmap
	bitmap.DeleteObject();

	// flag change
	pRegion->VoxelsChanged();

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
