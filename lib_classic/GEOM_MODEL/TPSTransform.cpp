//////////////////////////////////////////////////////////////////////
// TPSTransform.cpp: implementation of the CTPSTransform class.
//
// Copyright (C) 2002
// $Id$
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <MatrixNxM.h>
// #include <MatrixBase.inl>

#include "TPSTransform.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// distance_function
// 
// returns the log-distance squared betweeen two landmarks
//////////////////////////////////////////////////////////////////////
double distance_function(const CVectorD<3>& vL1, const CVectorD<3>& vL2)
{
	// compute the euclidean distance
	double r = (vL1 - vL2).GetLength();

	// pass through the log-squared
	return (r > 0.0) ? (r * r * log(r)) : 0.0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CTPSTransform::CTPSTransform
// 
// constructs a CTPSTransform object with the given name
//////////////////////////////////////////////////////////////////////
CTPSTransform::CTPSTransform()
	: m_bRecalc(TRUE)
{
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::CTPSTransform
// 
// destroys the TPSTransform
//////////////////////////////////////////////////////////////////////
CTPSTransform::~CTPSTransform()
{
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::GetLandmarkCount
// 
// returns the number of defined landmarks for the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::GetLandmarkCount()
{
	return m_arrLandmarks[0].size();
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::GetLandmark
// 
// returns a landmark
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CTPSTransform::GetLandmark(int nDataSet, int nIndex)
{
	// return the landmark at the given index
	return m_arrLandmarks[nDataSet].at(nIndex);
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::SetLandmark
// 
// constructs a CTPSTransform object with the given name
//////////////////////////////////////////////////////////////////////
void CTPSTransform::SetLandmark(int nDataSet, int nIndex, const CVectorD<3>& vLandmark)
{
	// assign the landmark
	m_arrLandmarks[nDataSet].at(nIndex) = vLandmark;

	// set the flag to indicate recalculation is needed
	m_bRecalc = TRUE;

	// fire a change
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::AddLandmark
// 
// adds a new landmark to the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::AddLandmark(const CVectorD<3>& vLandmark)
{
	// return the index of the new landmark
	return AddLandmark(vLandmark, vLandmark);
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::AddLandmark
// 
// adds a new landmark to the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::AddLandmark(const CVectorD<3>& vLandmark1, 
							   const CVectorD<3>& vLandmark2)
{
	// add the landmark to both data sets
	m_arrLandmarks[0].push_back(vLandmark1);
	m_arrLandmarks[1].push_back(vLandmark2);

	// set the flag to indicate recalculation is needed
	m_bRecalc = TRUE;

	// fire a change
	GetChangeEvent().Fire();

	// return the index of the new landmark
	return GetLandmarkCount()-1;
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::RemoveAllLandmarks
// 
// removes all landmarks from the transform
//////////////////////////////////////////////////////////////////////
void CTPSTransform::RemoveAllLandmarks()
{
	m_arrLandmarks[0].clear();
	m_arrLandmarks[1].clear();
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::Eval
// 
// evaluates the vector field at a point
//////////////////////////////////////////////////////////////////////
void CTPSTransform::Eval(const CVectorD<3>& vPos, CVectorD<3>& vPosTrans)
{
	// start with transformed equal to input
	vPosTrans = vPos;

	// don't compute without at least three landmarks
	if (GetLandmarkCount() >= 3)
	{
		// see if a recalc is needed
		if (m_bRecalc)
		{
			// recalculate the weight vectors
			RecalcWeights();
		}

		// add the weight vector displacements
		for (int nAt = 0; nAt < GetLandmarkCount(); nAt++)
		{
			// distance to the first landmark
			double d = distance_function(vPos, GetLandmark(0, nAt));

			// add weight vector displacements
			vPosTrans[0] += d * m_vWx[nAt];
			vPosTrans[1] += d * m_vWy[nAt];
		}

		// add the affine displacements
		vPosTrans[0] += 
			m_vWx[GetLandmarkCount() + 0] 
				+ m_vWx[GetLandmarkCount() + 1] * vPos[0] 
				+ m_vWx[GetLandmarkCount() + 2] * vPos[1];

		vPosTrans[1] += 
			m_vWy[GetLandmarkCount() + 0] 
				+ m_vWy[GetLandmarkCount() + 1] * vPos[0] 
				+ m_vWy[GetLandmarkCount() + 2] * vPos[1];
	}
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::Resample
// 
// resamples a source image through the transform
//////////////////////////////////////////////////////////////////////
void CTPSTransform::Resample(CDib *pImageSrc, CDib *pImageDst)
{
	// get the number of bytes-per-pixel for each
	BITMAP srcBitmap;
	pImageSrc->GetBitmap(&srcBitmap);
	int nSrcBytesPixel = srcBitmap.bmBitsPixel / 8;

	// ensure the scan line width is on a LONG boundary
	int nSrcWidthBytes = sizeof(LONG) 
		* ((srcBitmap.bmWidthBytes - 1) / sizeof(LONG) + 1);

	BITMAP dstBitmap;
	pImageDst->GetBitmap(&dstBitmap);
	int nDstBytesPixel = dstBitmap.bmBitsPixel / 8;

	// ensure the scan line width is on a LONG boundary
	int nDstWidthBytes = sizeof(LONG) 
		* ((dstBitmap.bmWidthBytes - 1) / sizeof(LONG) + 1);

	// should have same pixel format
	ASSERT(nSrcBytesPixel == nDstBytesPixel);

	// get the size of the source image
	CSize srcSize = pImageSrc->GetSize();
	CSize dstSize = pImageDst->GetSize();

	// retrieve the source and destination pixels
	UCHAR *pSrcPixels = (UCHAR *) pImageSrc->GetDIBits();
	UCHAR *pDstPixels = (UCHAR *) pImageDst->GetDIBits();

	// for each pixel in the image
	for (int nAtRow = 0; nAtRow < dstSize.cy; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < dstSize.cx; nAtCol++)
		{
			// position of the destination
			CVectorD<3> vDstPos((double) nAtCol, (double) nAtRow);

			// position of the source
			CVectorD<3> vSrcPos;
			Eval(vDstPos, vSrcPos);

			// compute the destination position
			int nDstY = dstSize.cy - (int) vDstPos[1] - 1;
			int nDstIndex = nDstBytesPixel * (int) vDstPos[0] 
				+ nDstY * nDstWidthBytes;

			// bounds check source position
			int nSrcY = srcSize.cy - (int) vSrcPos[1] - 1;
			if (vSrcPos[0] >= 0.0 && vSrcPos[0] < srcSize.cx
				&& nSrcY >= 0 && nSrcY < srcSize.cy)
			{
				// compute the positions
				int nSrcIndex = nSrcBytesPixel * (int) vSrcPos[0] 
					+ nSrcY * nSrcWidthBytes;

				// and resample
				memcpy(&pDstPixels[nDstIndex], &pSrcPixels[nSrcIndex], 
					nDstBytesPixel);
			}
			else
			{
				// set to zero
				memset(&pDstPixels[nDstIndex], 0, nDstBytesPixel);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::RecalcWeights
// 
// recalcs the TPS weights from the landmarks
//////////////////////////////////////////////////////////////////////
void CTPSTransform::RecalcWeights()
{
	// don't compute without at least three landmarks
	if (GetLandmarkCount() < 3)
	{
		return;
	}

	// stores the L matrix
	CMatrixNxM<> mL(GetLandmarkCount() + 3, GetLandmarkCount() + 3);

	// iterate over the rows and columns of the L matrix
	int nAtRow;
	int nAtCol;
	for (nAtRow = 0; nAtRow < GetLandmarkCount(); nAtRow++)
	{
		for (nAtCol = 0; nAtCol < GetLandmarkCount(); nAtCol++)
		{
			// are we off-diagonal?
			if (nAtRow != nAtCol)
			{
				// populate the K part of the matrix
				mL[nAtCol][nAtRow] = 
					distance_function(GetLandmark(0, nAtRow), 
						GetLandmark(0, nAtCol));
			}
			else
			{
				// zeros on the diagonal
				mL[nAtCol][nAtRow] = 0.0;
			}

			// populate the Q^T part of the matrix
			mL[nAtCol][GetLandmarkCount() + 0] = 1.0;
			mL[nAtCol][GetLandmarkCount() + 1] = GetLandmark(0, nAtCol)[0];
			mL[nAtCol][GetLandmarkCount() + 2] = GetLandmark(0, nAtCol)[1];
		}

		// populate the Q part of the matrix
		mL[GetLandmarkCount() + 0][nAtRow] = 1.0;
		mL[GetLandmarkCount() + 1][nAtRow] = GetLandmark(0, nAtRow)[0];
		mL[GetLandmarkCount() + 2][nAtRow] = GetLandmark(0, nAtRow)[1];
	}

	// fill the lower-right 3x3 with zeros
	for (nAtRow = GetLandmarkCount(); nAtRow < GetLandmarkCount()+3; nAtRow++)
	{
		for (nAtCol = GetLandmarkCount(); nAtCol < GetLandmarkCount()+3; nAtCol++)
		{
			mL[nAtCol][nAtRow] = 0.0;
		}
	}

	// form the inverse of L
	CMatrixNxM<> mL_inv = mL;
	mL_inv.Invert();

	// compute the x- and y-direction "heights"
	CVectorN<> vHx(GetLandmarkCount() + 3);
	CVectorN<> vHy(GetLandmarkCount() + 3);
	for (int nAtLandmark = 0; nAtLandmark < GetLandmarkCount(); nAtLandmark++)
	{
		vHx[nAtLandmark] = GetLandmark(1, nAtLandmark)[0] 
				- GetLandmark(0, nAtLandmark)[0];

		vHy[nAtLandmark] = GetLandmark(1, nAtLandmark)[1] 
				- GetLandmark(0, nAtLandmark)[1];
	}

	// compute the weight vectors
	m_vWx     = mL_inv * vHx;
	m_vWy     = mL_inv * vHy;

	// unset flag
	m_bRecalc = FALSE;

#ifdef _DEBUG
	// now check to ensure the offsets at each landmark is correct
	for (int nAtLandmark = 0; nAtLandmark < GetLandmarkCount(); nAtLandmark++)
	{
		const CVectorD<3>& vL0 = GetLandmark(0, nAtLandmark);
		const CVectorD<3>& vL1 = GetLandmark(1, nAtLandmark);

		CVectorD<3> vL0_xform;
		Eval(vL0, vL0_xform);

		ASSERT(vL0_xform.IsApproxEqual(vL1));
	}
#endif
}
