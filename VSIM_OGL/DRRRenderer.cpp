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
	m_bRecomputeDRR(TRUE)
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
bool ClipRaster(int& nDestStart, int& nDestLength,
			CVector<3>& vSourceStart, const CVector<3> &vSourceStep, 
			int nD, int nSourceMinD, int nSourceMaxD)
{
	// First, adjust the raster start.

	// if the source start is less than the allowed min,
	if (vSourceStart[nD] < nSourceMinD)
	{
		// if the step size is negative, the raster doesn't intersect the range
		if (vSourceStep[nD] <= 0.0)
		{
			return false;
		}

		// compute the delta for the destination start and length values, 
		//		and the source start value
		int nDelta = static_cast<int>(ceil((nSourceMinD - vSourceStart[nD])
												/ vSourceStep[nD]));

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the destination start
		nDestStart += nDelta;

		// adjust the source start
		vSourceStart += static_cast<double>(nDelta) * vSourceStep;
	}
	// else if the source start is greater than the allowed max,
	else if (vSourceStart[nD] > (nSourceMaxD -1))
	{
		// if the step size is position, the raster doesn't intersect
		if (vSourceStep[nD] >= 0.0)
		{
			return false;
		}

		// compute the delta for the destination start and length values,
		//		and the source start value
		int nDelta = static_cast<int>
			(ceil(((nSourceMaxD - 1) - vSourceStart[nD]) 
							/ vSourceStep[nD]));

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the destination start
		nDestStart += nDelta;

		// adjust the source start
		vSourceStart += static_cast<double>(nDelta) * vSourceStep;
	}

	// Next, adjust the raster length.

	// compute the source end
	double sourceEndD = 
		vSourceStart[nD] + nDestLength * vSourceStep[nD];

	// if the source end is greater than the allowed max,
	if (sourceEndD > (nSourceMaxD - 1))
	{
		// if the source step is negative, the raster doesn't intersect
		if (vSourceStep[nD] <= 0.0)
		{
			return false;
		}

		// compute the new destination length
		nDestLength = static_cast<int>
			(floor(((nSourceMaxD - 1) - vSourceStart[nD]) 
							/ vSourceStep[nD]));

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}
	}
	// else if the source end is less than the allowed min,
	else if (sourceEndD < nSourceMinD)
	{
		// if the source step is positive, the raster doesn't intersect
		if (vSourceStep[nD] >= 0.0)
		{
			return false;
		}

		// compute the new destination length
		nDestLength = static_cast<int>
			(floor(((nSourceMinD - vSourceStart[nD]))
							/ vSourceStep[nD]));

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}
	}

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

	TRACE2("Window size = %i, %i\n", rect.Width(), rect.Height());

#ifdef _DEBUG
	int nClipped = 0;
#endif

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
//	CREATE_INT_VECTOR(vStartStepX, viStartStepX);

	CVector<3> vStartNextY;
	gluUnProject((GLdouble)0, (GLdouble)1, zMin,
		modelMatrix, projMatrix, viewport, 
		&vStartNextY[0], &vStartNextY[1], &vStartNextY[2]);
	CVector<3> vStartStepY = vStartNextY - vStart;
//	CREATE_INT_VECTOR(vStartStepY, viStartStepY);

	vStart += CVector<3>(0.5, 0.5, 0.5);
//	CREATE_INT_VECTOR(vStart, viStart);

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
//	CREATE_INT_VECTOR(vEndStepX, viEndStepX);

	CVector<3> vEndNextY;
	gluUnProject((GLdouble)0, (GLdouble)1, zMax,
		modelMatrix, projMatrix, viewport, 
		&vEndNextY[0], &vEndNextY[1], &vEndNextY[2]);
	CVector<3> vEndStepY = vEndNextY - vEnd;
//	CREATE_INT_VECTOR(vEndStepY, viEndStepY);

	vEnd += CVector<3>(0.5, 0.5, 0.5);
//	CREATE_INT_VECTOR(vEnd, viEnd);

	const int nSteps = 64;
	const int nShift = 5;
	int nMax = 0;
	int nMin = INT_MAX;
	
	int nImageHeight = rect.Height();
	int nImageWidth = rect.Width();

	for (int nY = 0; nY < nImageHeight; nY++)
	{
		CVector<3> vStartOld = vStart;
		CVector<3> vEndOld = vEnd;

		int nPixelAt = nY * nImageWidth;

		for (int nX = 0; nX < nImageWidth; nX++, nPixelAt++)
		{
			m_arrPixels[nPixelAt] = 0;

			CVector<3> vStartOldX = vStart;

			CVector<3> vStep = vEnd - vStart;
			vStep[0] /= nSteps;
			vStep[1] /= nSteps;
			vStep[2] /= nSteps;

			int nDestStart = 0;
			int nDestLength = nSteps;
			if (ClipRaster(nDestStart, nDestLength, vStart, vStep, 0, 0, nWidth)
				&& ClipRaster(nDestStart, nDestLength, vStart, vStep, 1, 0, nHeight)
				&& ClipRaster(nDestStart, nDestLength, vStart, vStep, 2, 0, nDepth))
			{

				for (int nAt = 0; nAt < nDestLength; nAt++)
				{
					int nVoxelX = (int) vStart[0]; // viStart[0] >> 16;
					int nVoxelY = (int) vStart[1]; // >> 16;
					int nVoxelZ = (int) vStart[2]; // >> 16;

					ASSERT(nVoxelX >= 0 && nVoxelX < nWidth
						&& nVoxelY >= 0 && nVoxelY < nHeight
						&& nVoxelZ >= 0 && nVoxelZ < nDepth);

					m_arrPixels[nPixelAt] += pppVoxels[nVoxelZ][nVoxelY][nVoxelX];

					vStart += vStep;
				}
			}

			nMax = max(m_arrPixels[nPixelAt], nMax);
			nMin = min(m_arrPixels[nPixelAt], nMin);

			vStart = vStartOldX + vStartStepX;
			vEnd += vEndStepX;
		}

		vStart = vStartOld + vStartStepY;
		vEnd = vEndOld + vEndStepY;
	}

	for (nY = 0; nY < rect.Height(); nY++)
		for (int nX = 0; nX < rect.Width(); nX++)
		{
			m_arrPixels[nY * rect.Width() + nX] -= nMin;
			m_arrPixels[nY * rect.Width() + nX] *= (INT_MAX / (nMax - nMin)); 
		}

	TRACE2("%i points out of %i were clipped\n", nClipped, rect.Height() * rect.Width() * nSteps);

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
		m_bRecomputeDRR = TRUE;
}
