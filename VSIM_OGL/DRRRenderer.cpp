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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDRRRenderer::CDRRRenderer(COpenGLView *pView)
: COpenGLRenderer(pView),
	forVolume(NULL),
	m_bRecomputeDRR(TRUE),
	m_nSteps(16),
	m_nShift(4)
{
	m_pView->myProjectionMatrix.AddObserver(this);
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
//  Function clipRaster(int *pnDestStart, int *pnDestLength,
//			Vector *pvSourceStart, const Vector &vSourceStep, int nD, 
//			int nSourceMinD, int nSourceMaxD)
//
//  Helper function to clip a raster associated with a 3-D chain of regularly 
//  spaced positions, to the given "source min" and "source max" values, for 
//  the given dimension.
//
//  Parameters:
//		pnDestStart - the starting value, which is possibly modified by the 
//			clipping
//		pnDestLength - the length of the raster, which is possibly modified
//			by the clipping
//		pvSourceStart - the starting source value
//		vSourceStep - the source step value
//		nD - the dimension to clip
//		nSourceMinD - the min for the dimension (in source coordinates)
//		nSourceMaxD - the max for the dimension (in source coordinates
//  Returns:
//		true if the clipping results in a valid raster
//	Exceptions/Errors:
//		none
//
///////////////////////////////////////////////////////////////////////////////
bool ClipRaster(int&, // nDestStart, 
				int& nDestLength,
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
	int nWidth = forVolume->width.Get();
	int nHeight = forVolume->height.Get();
	int nDepth = forVolume->depth.Get();

	CRect rect;
	m_pView->GetClientRect(&rect);
	m_arrPixels.SetSize(rect.Width() * rect.Height());

	// retrieve the model and projection matrices
	GLdouble modelMatrix[16];
	volumeTransform.Get().ToArray(modelMatrix);

	GLdouble projMatrix[16];
	m_pView->myProjectionMatrix.Get().ToArray(projMatrix);

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
	gluUnProject((GLdouble)1, (GLdouble)0, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStartNextX[0], &vStartNextX[1], &vStartNextX[2]);
	CVector<3> vStartStepX = vStartNextX - vStart;
	CREATE_INT_VECTOR(vStartStepX, viStartStepX);

	CVector<3> vStartNextY;
	gluUnProject((GLdouble)0, (GLdouble)1, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStartNextY[0], &vStartNextY[1], &vStartNextY[2]);
	CVector<3> vStartStepY = vStartNextY - vStart;
	CREATE_INT_VECTOR(vStartStepY, viStartStepY);

//	vStart += CVector<3>(0.5, 0.5, 0.5);
	CREATE_INT_VECTOR(vStart, viStart);

	// un-project the window coordinates into the model coordinate system
	CVector<3> vEnd;
	gluUnProject((GLdouble)0, (GLdouble)0, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEnd[0], &vEnd[1], &vEnd[2]);

	CVector<3> vEndNextX;
	gluUnProject((GLdouble)1, (GLdouble)0, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEndNextX[0], &vEndNextX[1], &vEndNextX[2]);
	CVector<3> vEndStepX = vEndNextX - vEnd;
	CREATE_INT_VECTOR(vEndStepX, viEndStepX);

	CVector<3> vEndNextY;
	gluUnProject((GLdouble)0, (GLdouble)1, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEndNextY[0], &vEndNextY[1], &vEndNextY[2]);
	CVector<3> vEndStepY = vEndNextY - vEnd;
	CREATE_INT_VECTOR(vEndStepY, viEndStepY);

//	vEnd += CVector<3>(0.5, 0.5, 0.5);
	CREATE_INT_VECTOR(vEnd, viEnd);

	int nMax = 0;
	int nMin = INT_MAX;
	
	int nImageHeight = rect.Height();
	int nImageWidth = rect.Width();

	for (int nY = 0; nY < nImageHeight; nY++)
	{
		CVector<3, int> viStartOld = viStart;
		CVector<3, int> viEndOld = viEnd;

		int nPixelAt = nY * nImageWidth;

		for (int nX = 0; nX < nImageWidth; nX++, nPixelAt++)
		{
			m_arrPixels[nPixelAt] = 0;

			CVector<3, int> viStartOldX = viStart;

			CVector<3, int> viStep = viEnd - viStart;
			viStep[0] >>= m_nShift;
			viStep[1] >>= m_nShift;
			viStep[2] >>= m_nShift;

			int nDestStart = 0;
			int nDestLength = m_nSteps;
			if (ClipRaster(nDestStart, nDestLength, viStart, viStep, 0, 0, nWidth)
				&& ClipRaster(nDestStart, nDestLength, viStart, viStep, 1, 0, nHeight)
				&& ClipRaster(nDestStart, nDestLength, viStart, viStep, 2, 0, nDepth))
			{

#define USE_MMX
#ifdef USE_MMX
				static int viStepMMX[6];
				viStepMMX[0] = viStep[0];
				viStepMMX[1] = viStep[1];
				viStepMMX[2] = viStep[2];
				viStepMMX[3] = 0; // viStep[0];
//					viStepMMX[4] = viStep[1];
//					viStepMMX[5] = viStep[2];

				static int viStartMMX[6];
				viStartMMX[0] = viStart[0];
				viStartMMX[1] = viStart[1];
				viStartMMX[2] = viStart[2];
				viStartMMX[3] = 0; // viStart[0];
//					viStartMMX[4] = viStart[1];
//					viStartMMX[5] = viStart[2];

				char *pVoxels = (char *)&pppVoxels[0][0][0];
				int *pCurrentPixel = (int *)&m_arrPixels[nPixelAt];

				__int64 mult_mask = 512L << 1L;
				mult_mask <<= 32;
				mult_mask |= (512L << 17L) | 2L;

				__asm
				{
					// set up the MMX registers

					emms						// clear MMX state 

					movq mm0, viStepMMX + 0		// set up the offset vector as MMX register 0 
					movq mm1, viStartMMX + 0	// set up the position vector as MMX register 1 

					movq mm2, viStepMMX[2 * 4]
					movq mm3, viStartMMX[2 * 4]

//					movq mm4, viStepMMX[4 * 4]
//					movq mm5, viStartMMX[4 * 4]

					mov esi, pVoxels
					mov edi, pCurrentPixel

					mov ecx, nDestLength
					mov eax, 0

LOOP1:

					// compute the address of the pixel
					movq mm6, mm1			// move the x- and y- coordinates to work register
					psrad mm6, 16			// adjust for fixed-point
					movq mm7, mm3
					psrad mm7, 16
					packssdw mm6, mm7

					movq mm7, mult_mask
					pmaddwd mm6, mm7		// perform the packed add-and-multiply

					movd edx, mm6
					psrlq mm6, 32
					movd ebx, mm6
					shl ebx, 9
					add edx, ebx

					mov ebx, 0
					mov bx, [esi+edx]

					add eax, ebx
					// add eax, [edi]
					// mov [edi], eax

					paddd mm1, mm0			// increment the offset vector */
					// movq viStartMMX[0], mm1

					paddd mm3, mm2
					// movq viStartMMX[2 * 4], mm3

//						paddd mm5, mm4
//						movq viStartMMX[4 * 4], mm5

					loop LOOP1

					mov [edi], eax

					emms					// clear MMX state 
				}

#else	// ifdef USE_MMX
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

					m_arrPixels[nPixelAt] += pppVoxels[nVoxelZ][nVoxelY][nVoxelX];
#else	// ifdef _DEBUG
					m_arrPixels[nPixelAt] += pppVoxels[viStart[2] >> 16][viStart[1] >> 16][viStart[0] >> 16];
#endif	// ifdef _DEBUG

					viStart += viStep;
				}

#endif	// ifdef USE_MMX

			}

			nMax = max(m_arrPixels[nPixelAt], nMax);
			nMin = min(m_arrPixels[nPixelAt], nMin);

			viStart = viStartOldX + viStartStepX;
			viEnd += viEndStepX;
		}

		viStart = viStartOld + viStartStepY;
		viEnd = viEndOld + viEndStepY;
	}

	for (nY = 0; nY < rect.Height(); nY++)
		for (int nX = 0; nX < rect.Width(); nX++)
		{
			m_arrPixels[nY * rect.Width() + nX] -= nMin;
			m_arrPixels[nY * rect.Width() + nX] *= (INT_MAX / (nMax - nMin)); 
		}

	m_bRecomputeDRR = FALSE;
}

void CDRRRenderer::DrawScene()
{
	if (forVolume.Get() != NULL)
	{
		CRect rect;
		m_pView->GetClientRect(&rect);

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
		glDrawPixels(rect.Width(), rect.Height(), GL_LUMINANCE, GL_INT, m_arrPixels.GetData());
		ASSERT(glGetError() == GL_NO_ERROR);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
	}
}

void CDRRRenderer::OnRenderScene()
{
}

void CDRRRenderer::OnChange(CObservable *pSource)
{
	COpenGLRenderer::OnChange(pSource);
	if (pSource == &m_pView->myProjectionMatrix)
	{
		m_nSteps = 8;
		m_nShift = 3;
		m_bRecomputeDRR = TRUE;
	}
}
