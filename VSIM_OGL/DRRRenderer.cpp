// DRRRenderer.cpp: implementation of the CDRRRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DRRRenderer.h"

#include <float.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include <glMatrixVector.h>

#include <OpenGLView.h>

#define RAY_TRACE_RESOLUTION 128
#define RAY_TRACE_RES_LOG2   7

#define POST_PROCESS
#define COMPUTE_MINMAX
// #define USE_TANDEM_RAYS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDRRRenderer::CDRRRenderer(COpenGLView *pView)
: COpenGLRenderer(pView),
	forVolume(NULL),
	m_bRecomputeDRR(TRUE),
	m_nSteps(RAY_TRACE_RESOLUTION),
	m_nShift(RAY_TRACE_RES_LOG2),
	m_nResDiv(4)
{
	m_pView->projectionMatrix.AddObserver(this, (ChangeFunction) OnChange);
}

CDRRRenderer::~CDRRRenderer()
{

}

#define CREATE_INT_VECTOR(v, vi) \
	CVector<3, int> vi; \
	vi[0] = (int)(v[0] * 65536.0); \
	vi[1] = (int)(v[1] * 65536.0); \
	vi[2] = (int)(v[2] * 65536.0);

///////////////////////////////////////////////////////////////////////////////
bool ClipRaster(int& nDestLength,
			CVector<3, int>& vSourceStart, const CVector<3, int> &vSourceStep, 
			int nD, int nSourceMinD, int nSourceMaxD)
{
	// First, adjust the raster start.

	// if the source start is less than the allowed min,
	if ((vSourceStart[nD] >> 16) < nSourceMinD)
	{
		// if the step size is negative, the raster doesn't intersect the range
		if (vSourceStep[nD] <= 0)
		{
			return false;
		}

		// compute the delta for the destination start and length values, 
		//		and the source start value
		ASSERT(nSourceMinD < (INT_MAX >> 16));

//		int nDelta2 = static_cast<int>(ceil(((double)nSourceMinD * 65536.0 - (double) vSourceStart[nD])
//					/ (double) vSourceStep[nD]));

		int nDelta = ((((nSourceMinD << 16) - vSourceStart[nD]) / vSourceStep[nD]) + 1); //  + 65536) >> 16;
//		ASSERT(nDelta == nDelta2);

		ASSERT(((vSourceStart[nD] + nDelta * vSourceStep[nD]) >> 16) >= nSourceMinD);

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the destination start
//		nDestStart += nDelta;

		// adjust the source start
		// vSourceStart += static_cast<double>(nDelta) * vSourceStep;
		vSourceStart[0] += nDelta * vSourceStep[0];
		vSourceStart[1] += nDelta * vSourceStep[1];
		vSourceStart[2] += nDelta * vSourceStep[2];
	}
	// else if the source start is greater than the allowed max,
	else if ((vSourceStart[nD] >> 16) > (nSourceMaxD -1))
	{
		// if the step size is position, the raster doesn't intersect
		if (vSourceStep[nD] >= 0)
		{
			return false;
		}

		// compute the delta for the destination start and length values,
		//		and the source start value
		ASSERT((nSourceMaxD-1) < (INT_MAX >> 16));

//		int nDelta2 = static_cast<int>
//			(ceil((((double) (nSourceMaxD - 1) * 65536.0) - (double) vSourceStart[nD]) 
//							/ (double) vSourceStep[nD]));

		int nDelta = (((((nSourceMaxD - 1) << 16) - vSourceStart[nD]) / vSourceStep[nD]) + 1); // 65536) >> 16;
//		ASSERT(nDelta == nDelta2);

		ASSERT(((vSourceStart[nD] + nDelta * vSourceStep[nD]) >> 16) <= (nSourceMaxD-1));

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the destination start
//		nDestStart += nDelta;

		// adjust the source start
		// vSourceStart += static_cast<double>(nDelta) * vSourceStep;
		vSourceStart[0] += nDelta * vSourceStep[0];
		vSourceStart[1] += nDelta * vSourceStep[1];
		vSourceStart[2] += nDelta * vSourceStep[2];
	}

	// Next, adjust the raster length.

	// compute the source end
	// double sourceEndD = 
	int nSourceEndD = 
		vSourceStart[nD] + nDestLength * vSourceStep[nD];

	// if the source end is greater than the allowed max,
	if ((nSourceEndD >> 16) > (nSourceMaxD - 1))
	{
		// if the source step is negative, the raster doesn't intersect
		if (vSourceStep[nD] <= 0)
		{
			return false;
		}

		// compute the new destination length
//		int nDestLength2 =  static_cast<int>
//			(floor(((double)(nSourceMaxD - 1) * 65536.0 - (double) vSourceStart[nD]) 
//							/ (double) vSourceStep[nD]));
		nDestLength = (((((nSourceMaxD - 1) << 16) - vSourceStart[nD]) / vSourceStep[nD]) - 0);
//		ASSERT(nDestLength == nDestLength2);
			
		ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) <= (nSourceMaxD - 1));

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}
	}
	// else if the source end is less than the allowed min,
	else if ((nSourceEndD >> 16) < nSourceMinD)
	{
		// if the source step is positive, the raster doesn't intersect
		if (vSourceStep[nD] >= 0)
		{
			return false;
		}

		// compute the new destination length
//		int nDestLength2 =  static_cast<int>
//			 (floor((((double) nSourceMinD * 65536.0 - (double) vSourceStart[nD]))
//							/ (double) vSourceStep[nD]));
					
		nDestLength = ((((nSourceMinD << 16) - vSourceStart[nD]) / vSourceStep[nD]) - 0);
//		ASSERT(nDestLength == nDestLength2);

		ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) >= nSourceMinD);

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}
	}

	ASSERT(((vSourceStart[nD]) >> 16) >= nSourceMinD);
	ASSERT(((vSourceStart[nD]) >> 16) < nSourceMaxD);
	ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) >= nSourceMinD);
	ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) < nSourceMaxD);

	// Completed successfully, so return true
	return true;
}

void CDRRRenderer::ComputeDRR()
{
	// get copies of the volume dimensions
	int nWidth = forVolume->width.Get();
	int nHeight = forVolume->height.Get();
	int nDepth = forVolume->depth.Get();

	// get the view rectangle
	CRect rect;
	m_pView->GetClientRect(&rect);
	int nImageWidth = rect.Width() / m_nResDiv;
	int nImageHeight = rect.Height() / m_nResDiv;
	m_arrPixels.SetSize(nImageWidth * nImageHeight);

	// retrieve the model and projection matrices
	GLdouble modelMatrix[16];
	volumeTransform.Get().ToArray(modelMatrix);

	GLdouble projMatrix[16];
	m_pView->projectionMatrix.Get().ToArray(projMatrix);

	// retrieve the viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// compute the near and far planes containing the volume
	CVector<3> vPt;
	double zMin = DBL_MAX, zMax = -DBL_MAX;

	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, 0.0, 0.0, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(0.0, nHeight, 0.0, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(0.0, 0.0, nDepth, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, nHeight, 0.0, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, 0.0, nDepth, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, nHeight, nDepth, modelMatrix, projMatrix, viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	short ***pppVoxels = forVolume->GetVoxels();

	// un-project the window coordinates into the model coordinate system
	CVector<3> vStart;
	gluUnProject((GLdouble)0, (GLdouble)0, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStart[0], &vStart[1], &vStart[2]);

	CVector<3> vStartNextX;
	gluUnProject((GLdouble)m_nResDiv, (GLdouble)0, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStartNextX[0], &vStartNextX[1], &vStartNextX[2]);
	CVector<3> vStartStepX = vStartNextX - vStart;
	CREATE_INT_VECTOR(vStartStepX, viStartStepX);

	CVector<3> vStartNextY;
	gluUnProject((GLdouble)0, (GLdouble)m_nResDiv, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStartNextY[0], &vStartNextY[1], &vStartNextY[2]);
	CVector<3> vStartStepY = vStartNextY - vStart;
	CREATE_INT_VECTOR(vStartStepY, viStartStepY);

	CREATE_INT_VECTOR(vStart, viStart);

	// un-project the window coordinates into the model coordinate system
	CVector<3> vEnd;
	gluUnProject((GLdouble)0, (GLdouble)0, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEnd[0], &vEnd[1], &vEnd[2]);

	CVector<3> vEndNextX;
	gluUnProject((GLdouble)m_nResDiv, (GLdouble)0, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEndNextX[0], &vEndNextX[1], &vEndNextX[2]);
	CVector<3> vEndStepX = vEndNextX - vEnd;
	CREATE_INT_VECTOR(vEndStepX, viEndStepX);

	CVector<3> vEndNextY;
	gluUnProject((GLdouble)0, (GLdouble)m_nResDiv, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEndNextY[0], &vEndNextY[1], &vEndNextY[2]);
	CVector<3> vEndStepY = vEndNextY - vEnd;
	CREATE_INT_VECTOR(vEndStepY, viEndStepY);

	CREATE_INT_VECTOR(vEnd, viEnd);

	__int64 mult_mask = nWidth << 1L;	// WATCH THIS: we need to shift by
										//		the log2(nHeight) also
	mult_mask <<= 32;
	mult_mask |= (nWidth << 17L) | (1 << 1L);

#ifdef USE_TANDEM_RAYS
	__int64 mult_mask2 = nWidth << 1L;	// WATCH THIS: we need to shift by
										//		the log2(nHeight) also
	mult_mask2 <<= 48;
	mult_mask2 |= (nWidth << 17L) | (1 << 1L);
#endif

#ifdef COMPUTE_MINMAX
	int nMax = 0;
	int nMin = INT_MAX;
#else
	int nMax = 65000 / 64 * m_nSteps; // 0;
	int nMin = 15000 / 64 * m_nSteps; // INT_MAX;
#endif

#ifdef POST_PROCESS
	int level_offset = -nMin;
	int window_div = (nMax - nMin) / 256;
#endif

	for (int nY = 0; nY < nImageHeight; nY++)
	{
		CVector<3, int> viStartOld = viStart;
		CVector<3, int> viEndOld = viEnd;

		int nPixelAt = nY * nImageWidth;

#ifdef USE_TANDEM_RAYS
		for (int nX = 0; nX < nImageWidth; nX += 2, nPixelAt += 2)
		{
#else
		for (int nX = 0; nX < nImageWidth; nX++, nPixelAt++)
		{
#endif
//			m_arrPixels[nPixelAt] = 0;
//			m_arrPixels[nPixelAt+1] = 0;

			CVector<3, int> viStartOldX = viStart;

			CVector<3, int> viStep = viEnd - viStart;
			viStep[0] >>= m_nShift;
			viStep[1] >>= m_nShift;
			viStep[2] >>= m_nShift;

#ifdef USE_TANDEM_RAYS
			CVector<3, int> viStart2 = viStart + viStartStepX;
			viEnd += viEndStepX;
			CVector<3, int> viStep2 = viEnd - viStart2;
			viStep2[0] >>= m_nShift;
			viStep2[1] >>= m_nShift;
			viStep2[2] >>= m_nShift;
#endif

			int nDestLength = m_nSteps;
#ifdef USE_TANDEM_RAYS
			int nDestLength2 = m_nSteps;
#endif
			if (ClipRaster(nDestLength, viStart, viStep, 0, 0, nWidth)
				&& ClipRaster(nDestLength, viStart, viStep, 1, 0, nHeight)
				&& ClipRaster(nDestLength, viStart, viStep, 2, 0, nDepth)
#ifdef USE_TANDEM_RAYS
				&& ClipRaster(nDestLength, viStart2, viStep2, 0, 0, nWidth)
				&& ClipRaster(nDestLength, viStart2, viStep2, 1, 0, nHeight)
				&& ClipRaster(nDestLength, viStart2, viStep2, 2, 0, nDepth)
#endif
				)
			{

// #define USE_MMX
#ifdef USE_MMX

#ifdef USE_TANDEM_RAYS
//				nDestLength = min(nDestLength, nDestLength2);
#endif

				static int viStepMMX[6];
				viStepMMX[0] = viStep[0];
				viStepMMX[1] = viStep[1];
				viStepMMX[2] = viStep[2];
#ifndef USE_TANDEM_RAYS
				viStepMMX[3] = 0;
#else
				viStepMMX[3] = viStep2[2];
				viStepMMX[4] = viStep2[0];
				viStepMMX[5] = viStep2[1];
#endif
				static int viStartMMX[6];
				viStartMMX[0] = viStart[0];
				viStartMMX[1] = viStart[1];
				viStartMMX[2] = viStart[2];
#ifndef USE_TANDEM_RAYS
				viStartMMX[3] = 0;
#else
				viStartMMX[3] = viStart2[2];
				viStartMMX[4] = viStart2[0];
				viStartMMX[5] = viStart2[1];
#endif

				char *pVoxels = (char *)&pppVoxels[0][0][0];
				int *pCurrentPixel = (int *)&m_arrPixels[nPixelAt];

				__asm
				{
					// set up the MMX registers

					emms						// clear MMX state 

					movq mm0, viStepMMX + 0		// set up the offset vector as MMX register 0 
					movq mm1, viStartMMX + 0	// set up the position vector as MMX register 1 

					movq mm2, viStepMMX[2 * 4]
					movq mm3, viStartMMX[2 * 4]

#ifdef USE_TANDEM_RAYS
					movq mm4, viStepMMX[4 * 4]
					movq mm5, viStartMMX[4 * 4]
#endif
					mov esi, pVoxels

					mov ecx, nDestLength
					mov eax, 0

#ifdef USE_TANDEM_RAYS
					mov edi, 0
#endif

LOOP1:

					// compute the address of the current voxel
					//

					movq mm6, mm1			// move the first ray's x- and y- coordinates to 
											//		work register mm6
					movq mm7, mm3			// move the first ray's z- to work register mm7

					psrad mm6, 16			// adjust for fixed-point
					psrad mm7, 16			// adjust for fixed-point
						
					packssdw mm6, mm7		// pack the four 32-bit values into four 16-bit 
											//		values

					movq mm7, mult_mask		// set up the multiplication mask for the four 
											//		values
					pmaddwd mm6, mm7		// perform the packed add-and-multiply

					movd edx, mm6			// now store the x- and y- offset

					psrlq mm6, 32			// shift the z-offset down

					movd ebx, mm6			// store the z-offset
					shl ebx, 9				// multiply by nHeight
					add edx, ebx			// and add the two offsets

					mov ebx, 0				// set up ebx to convert voxel to 32-bit value
					mov bx, [esi+edx]		// retrieve the voxel value

					add eax, ebx			// add the voxel in ebx to the accumulated voxel
											//		values in eax

#ifdef USE_TANDEM_RAYS
					xchg eax, edi			// swap the stored pixel value

					// compute the address of the current voxel
					//

					movq mm6, mm5			// move the first ray's x- and y- coordinates to 
											//		work register mm6
					movq mm7, mm3			// move the first ray's z- to work register mm7

					psrad mm6, 16			// adjust for fixed-point
					psrad mm7, 16			// adjust for fixed-point
						
					packssdw mm6, mm7		// pack the four 32-bit values into four 16-bit 
											//		values

					movq mm7, mult_mask2	// set up the multiplication mask for the four 
											//		values
					pmaddwd mm6, mm7		// perform the packed add-and-multiply

					movd edx, mm6			// now store the x- and y- offset

					psrlq mm6, 32			// shift the z-offset down
					movd ebx, mm6			// store the z-offset
					shl ebx, 9				// multiply by nHeight
					add edx, ebx			// and add the two offsets

					mov ebx, 0				// set up ebx to convert voxel to 32-bit value
					mov bx, [esi+edx]		// retrieve the voxel value

					add eax, ebx			// add the voxel in ebx to the accumulated voxel
											//		values in eax

					xchg eax, edi
#endif

					paddd mm1, mm0			// increment the first ray's x- and y-
					paddd mm3, mm2			// increment the first ray's z- and the
											//		second ray's x-
#ifdef USE_TANDEM_RAYS
					paddd mm5, mm4			// increment the second ray's y- and z-
#endif

					loop LOOP1				// now loop, decrementing the ECX count

#ifdef USE_TANDEM_RAYS
					mov ecx, edi
#endif

#ifndef POST_PROCESS
					add eax, level_offset
					mov edx, 0
					div window_div
					and eax, 0x000000ff

					mov edx, eax
					shl edx, 8
					or eax, edx
					shl edx, 8
					or eax, edx
#endif

					mov edi, pCurrentPixel
					mov [edi], eax			// store the accumulated value

#ifdef USE_TANDEM_RAYS

					inc edi
					inc edi
					inc edi
					inc edi

#ifndef POST_PROCESS
					mov eax, ecx

					add eax, level_offset
					mov edx, 0
					div window_div
					and eax, 0x000000ff

					mov edx, eax
					shl edx, 8
					or eax, edx
					shl edx, 8
					or eax, edx

					mov [edi], eax
#else
					mov [edi], ecx
#endif

#endif

					emms					// clear MMX state 
				}

#else	// ifdef USE_MMX
				int nPixelValue = 0;
				for (int nAt = 0; nAt < nDestLength; nAt++)
				{
#ifdef _DEBUG
					// for debugging, check to ensure that the voxel is inside the volume
					int nVoxelX = viStart[0] >> 16;
					int nVoxelY = viStart[1] >> 16;
					int nVoxelZ = viStart[2] >> 16;

					ASSERT(nVoxelX >= 0 && nVoxelX < nWidth
						&& nVoxelY >= 0 && nVoxelY < nHeight
						&& nVoxelZ >= 0 && nVoxelZ < nDepth);

					nPixelValue += pppVoxels[nVoxelZ][nVoxelY][nVoxelX];
#else	// ifdef _DEBUG
					nPixelValue += pppVoxels[viStart[2] >> 16][viStart[1] >> 16][viStart[0] >> 16];
#endif	// ifdef _DEBUG

					viStart += viStep;
				}
				m_arrPixels[nPixelAt] = nPixelValue;

#endif	// ifdef USE_MMX

			}

#ifdef COMPUTE_MINMAX
			nMax = max(m_arrPixels[nPixelAt], nMax);
			nMin = min(m_arrPixels[nPixelAt], nMin);
#endif

#ifndef USE_TANDEM_RAYS
			viStart = viStartOldX + viStartStepX;
#else
			viStart = viStartOldX + viStartStepX + viStartStepX;
#endif
			viEnd += viEndStepX;
		}

		viStart = viStartOld + viStartStepY;
		viEnd = viEndOld + viEndStepY;
	}

#ifdef POST_PROCESS
	int nValue;
	int nWindow = nMax - nMin;
	unsigned char (*pRGBA)[4];
	for (int nPixelAt = 0; nPixelAt < nImageWidth * nImageHeight; nPixelAt++)
	{
		nValue = m_arrPixels[nPixelAt];
		nValue -= nMin;
		nValue <<= 8;
		nValue /= nWindow; // (nMax - nMin);
		nValue = min(255, nValue);
		nValue = max(0, nValue);

		pRGBA = (unsigned char (*)[4])&m_arrPixels[nPixelAt];
		(*pRGBA)[0] = nValue;
		(*pRGBA)[1] = nValue;
		(*pRGBA)[2] = nValue;
		(*pRGBA)[3] = 0;
	}
#endif

	m_bRecomputeDRR = FALSE;
}

void CDRRRenderer::DrawScene()
{
	if (forVolume.Get() != NULL)
	{
		CRect rect;
		m_pView->GetClientRect(&rect);
		int nImageWidth = rect.Width() / m_nResDiv;
		int nImageHeight = rect.Height() / m_nResDiv;

		glMatrixMode(GL_PROJECTION);

		glPushMatrix();

		glLoadIdentity();
		gluOrtho2D(0.0, (GLfloat) rect.Width(), 0.0, (GLfloat) rect.Height());

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glRasterPos3f(0.0f, 0.0f, -0.99f);

		if (m_bRecomputeDRR || m_arrPixels.GetSize() != rect.Height() * rect.Width())
			ComputeDRR();

		glDrawBuffer(GL_BACK);

		glDisable(GL_DEPTH_TEST);
		glPixelZoom((float) m_nResDiv, (float) m_nResDiv);
		glDrawPixels(nImageWidth, nImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_arrPixels.GetData());
		ASSERT(glGetError() == GL_NO_ERROR);
		glEnable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	}
}

void CDRRRenderer::OnRenderScene()
{
}

void CDRRRenderer::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if (pSource == &m_pView->projectionMatrix)
	{
		m_nSteps = RAY_TRACE_RESOLUTION;
		m_nShift = RAY_TRACE_RES_LOG2;
		m_bRecomputeDRR = TRUE;
	}
	COpenGLRenderer::OnChange(pSource, pOldValue);
}
