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



UINT BackgroundComputeDRR( LPVOID pParam )
{
	CDRRRenderer *pRenderer = (CDRRRenderer *)pParam;

	pRenderer->m_bRecomputeDRR = TRUE;
	pRenderer->ComputeDRR();

	::InvalidateRect(pRenderer->m_pView->m_hWnd, NULL, FALSE);
	pRenderer->m_pThread = NULL;

	::AfxEndThread(0, TRUE);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDRRRenderer::CDRRRenderer(COpenGLView *pView)
: COpenGLRenderer(pView),
	forVolume(NULL),
	m_bRecomputeDRR(TRUE),
	m_nSteps(RAY_TRACE_RESOLUTION),
	m_nShift(RAY_TRACE_RES_LOG2),
	m_nResDiv(4),
	m_pThread(NULL)
{
	m_pView->projectionMatrix.AddObserver(this, (ChangeFunction) OnChange);
}

CDRRRenderer::~CDRRRenderer()
{
	if (m_pThread != NULL)
	{
		m_pThread->SuspendThread();
		delete m_pThread;
		m_pThread = NULL;
	}
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
	TRACE3("Volume dimensions = %i, %i, %i\n",
		nWidth, nHeight, nDepth);

	// size the pixel array
	TRACE2("Setting pixel size to %i, %i\n", m_nImageWidth, m_nImageHeight);
	m_arrPixels.SetSize(m_nImageWidth * m_nImageHeight);

	// retrieve the model and projection matrices
	GLdouble modelMatrix[16];
	volumeTransform.Get().ToArray(modelMatrix);

	GLdouble projMatrix[16];
	m_pView->projectionMatrix.Get().ToArray(projMatrix);

	// compute the near and far planes containing the volume
	CVector<3> vPt;
	double zMin = DBL_MAX, zMax = -DBL_MAX;

	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, 0.0, 0.0, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(0.0, nHeight, 0.0, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(0.0, 0.0, nDepth, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, nHeight, 0.0, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, 0.0, nDepth, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	gluProject(nWidth, nHeight, nDepth, modelMatrix, projMatrix, m_viewport, &vPt[0], &vPt[1], &vPt[2]);
	zMin = min(zMin, vPt[2]);
	zMax = max(zMax, vPt[2]);

	short ***pppVoxels = forVolume->GetVoxels();

	double start = 0.5 * (double) m_nResDiv;
	double step = start + (double)m_nResDiv;

	// un-project the window coordinates into the model coordinate system
	CVector<3> vStart;
	gluUnProject(start, start, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStart[0], &vStart[1], &vStart[2]);

	CVector<3> vStartNextX;
	gluUnProject(step, start, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStartNextX[0], &vStartNextX[1], &vStartNextX[2]);
	CVector<3> vStartStepX = vStartNextX - vStart;
	CREATE_INT_VECTOR(vStartStepX, viStartStepX);

	CVector<3> vStartNextY;
	gluUnProject(start, step, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStartNextY[0], &vStartNextY[1], &vStartNextY[2]);
	CVector<3> vStartStepY = vStartNextY - vStart;
	CREATE_INT_VECTOR(vStartStepY, viStartStepY);

	CREATE_INT_VECTOR(vStart, viStart);

	// un-project the window coordinates into the model coordinate system
	CVector<3> vEnd;
	gluUnProject(start, start, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEnd[0], &vEnd[1], &vEnd[2]);

	CVector<3> vEndNextX;
	gluUnProject(step, start, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEndNextX[0], &vEndNextX[1], &vEndNextX[2]);
	CVector<3> vEndStepX = vEndNextX - vEnd;
	CREATE_INT_VECTOR(vEndStepX, viEndStepX);

	CVector<3> vEndNextY;
	gluUnProject(start, step, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEndNextY[0], &vEndNextY[1], &vEndNextY[2]);
	CVector<3> vEndStepY = vEndNextY - vEnd;
	CREATE_INT_VECTOR(vEndStepY, viEndStepY);

	CREATE_INT_VECTOR(vEnd, viEnd);

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

	for (int nY = 0; nY < m_nImageHeight; nY++)
	{
		CVector<3, int> viStartOld = viStart;
		CVector<3, int> viEndOld = viEnd;

		int nPixelAt = nY * m_nImageWidth;

		for (int nX = 0; nX < m_nImageWidth; nX++, nPixelAt++)
		{
			CVector<3, int> viStartOldX = viStart;

			CVector<3, int> viStep = viEnd - viStart;
			viStep[0] >>= m_nShift;
			viStep[1] >>= m_nShift;
			viStep[2] >>= m_nShift;

			int nDestLength = m_nSteps;

			if (ClipRaster(nDestLength, viStart, viStep, 0, 0, nWidth)
				&& ClipRaster(nDestLength, viStart, viStep, 1, 0, nHeight)
				&& ClipRaster(nDestLength, viStart, viStep, 2, 0, nDepth))
			{

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

			}

#ifdef COMPUTE_MINMAX
			nMax = max(m_arrPixels[nPixelAt], nMax);
			nMin = min(m_arrPixels[nPixelAt], nMin);
#endif

			viStart = viStartOldX + viStartStepX;
			viEnd += viEndStepX;
		}

		viStart = viStartOld + viStartStepY;
		viEnd = viEndOld + viEndStepY;
	}

#ifdef POST_PROCESS
	int nValue;
	int nWindow = nMax - nMin;
	unsigned char (*pRGBA)[4];
	for (int nPixelAt = 0; nPixelAt < m_nImageWidth * m_nImageHeight; nPixelAt++)
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
#ifdef _DEBUG
	return;
#endif

	if (forVolume.Get() != NULL)
	{
		if (m_pThread != NULL)
		{
			m_pThread->SuspendThread();
			delete m_pThread;
			m_pThread = NULL;
		}

		CRect rect;
		m_pView->GetClientRect(&rect);

		if (m_bRecomputeDRR) // || m_arrPixels.GetSize() != rect.Height() * rect.Width())
		{
			m_nSteps = RAY_TRACE_RESOLUTION;
			m_nShift = RAY_TRACE_RES_LOG2;
			m_nResDiv = 4;

			m_nImageWidth = rect.Width() / m_nResDiv;
			m_nImageHeight = rect.Height() / m_nResDiv;

			// retrieve the viewport
			glGetIntegerv(GL_VIEWPORT, m_viewport);

			ComputeDRR();
		}

		glMatrixMode(GL_PROJECTION);

		glPushMatrix();

		glLoadIdentity();
		gluOrtho2D(0.0, (GLfloat) rect.Width(), 0.0, (GLfloat) rect.Height());

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glRasterPos3f(0.0f, 0.0f, -0.99f);

		glDrawBuffer(GL_BACK);

		glDisable(GL_DEPTH_TEST);
		glPixelZoom((float) m_nResDiv, (float) m_nResDiv);
		ASSERT(m_arrPixels.GetSize() == m_nImageWidth * m_nImageHeight);
		glDrawPixels(m_nImageWidth, m_nImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_arrPixels.GetData());
		ASSERT(glGetError() == GL_NO_ERROR);
		glEnable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);

		// now start the background thread
		if (m_nResDiv > 1)
		{
			m_nSteps = RAY_TRACE_RESOLUTION * 2;
			m_nShift = RAY_TRACE_RES_LOG2 + 1;
			m_nResDiv = 1;
			m_nImageWidth = rect.Width() / m_nResDiv;
			m_nImageHeight = rect.Height() / m_nResDiv;

			// retrieve the viewport
			glGetIntegerv(GL_VIEWPORT, m_viewport);

			m_pThread = ::AfxBeginThread(BackgroundComputeDRR, (void *)this, THREAD_PRIORITY_HIGHEST);
		}
	}
}

void CDRRRenderer::OnRenderScene()
{
}

void CDRRRenderer::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if (pSource == &m_pView->projectionMatrix)
	{
		if (m_pThread != NULL)
		{
			m_pThread->SuspendThread();
			delete m_pThread;
			m_pThread = NULL;
		}

		m_bRecomputeDRR = TRUE;
	}
	COpenGLRenderer::OnChange(pSource, pOldValue);
}
