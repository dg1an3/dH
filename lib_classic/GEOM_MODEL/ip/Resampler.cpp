// Resampler.cpp: implementation of the CResampler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resampler.h"

///////////////////////////////////////////////////////////////////////////////

bool ClipRaster(int& nDestLength,
			CVectorD<3, int>& vSourceStart, 
			const CVectorD<3, int> &vSourceStep, 
			int nD, 
			int nSourceMinD, 
			int nSourceMaxD)
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
		int nDelta = ((((nSourceMinD << 16) - vSourceStart[nD]) / vSourceStep[nD]) + 1);
		ASSERT(((vSourceStart[nD] + nDelta * vSourceStep[nD]) >> 16) >= nSourceMinD);

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the source start
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
		int nDelta = (((((nSourceMaxD - 1) << 16) - vSourceStart[nD]) / vSourceStep[nD]) + 1);
		ASSERT(((vSourceStart[nD] + nDelta * vSourceStep[nD]) >> 16) <= (nSourceMaxD-1));

		// adjust the destination length
		nDestLength -= nDelta;

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}

		// adjust the source start
		vSourceStart[0] += nDelta * vSourceStep[0];
		vSourceStart[1] += nDelta * vSourceStep[1];
		vSourceStart[2] += nDelta * vSourceStep[2];
	}

	// Next, adjust the raster length.

	// compute the source end
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
		nDestLength = (((((nSourceMaxD - 1) << 16) - vSourceStart[nD]) / vSourceStep[nD]) - 0);
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
		nDestLength = ((((nSourceMinD << 16) - vSourceStart[nD]) / vSourceStep[nD]) - 0);
		ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) >= nSourceMinD);

		// if the length has become negative, the raster doesn't intersect
		if (nDestLength < 0)
		{
			return false;
		}
	}

	// check our work
	ASSERT(((vSourceStart[nD]) >> 16) >= nSourceMinD);
	ASSERT(((vSourceStart[nD]) >> 16) < nSourceMaxD);
	ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) >= nSourceMinD);
	ASSERT(((vSourceStart[nD] + nDestLength * vSourceStep[nD]) >> 16) < nSourceMaxD);

	// Completed successfully, so return true
	return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<class VOXEL_TYPE>
void CResampler<VOXEL_TYPE>::Resample()
{
/*	// get copies of the volume dimensions
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
	m_pView->camera.projection.Get().ToArray(projMatrix);

	// compute the near and far planes containing the volume
	CVectorD<3> vPt;
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
	CVectorD<3> vStart;
	gluUnProject(start, start, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStart[0], &vStart[1], &vStart[2]);

	CVectorD<3> vStartNextX;
	gluUnProject(step, start, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStartNextX[0], &vStartNextX[1], &vStartNextX[2]);
	CVectorD<3> vStartStepX = vStartNextX - vStart;
	CREATE_INT_VECTOR(vStartStepX, viStartStepX);

	CVectorD<3> vStartNextY;
	gluUnProject(start, step, zMin,
		modelMatrix, projMatrix, m_viewport, 
		&vStartNextY[0], &vStartNextY[1], &vStartNextY[2]);
	CVectorD<3> vStartStepY = vStartNextY - vStart;
	CREATE_INT_VECTOR(vStartStepY, viStartStepY);

	CREATE_INT_VECTOR(vStart, viStart);
	viStart[0] += 32768;
	viStart[1] += 32768;
	viStart[2] += 32768;

	// un-project the window coordinates into the model coordinate system
	CVectorD<3> vEnd;
	gluUnProject(start, start, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEnd[0], &vEnd[1], &vEnd[2]);

	CVectorD<3> vEndNextX;
	gluUnProject(step, start, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEndNextX[0], &vEndNextX[1], &vEndNextX[2]);
	CVectorD<3> vEndStepX = vEndNextX - vEnd;
	CREATE_INT_VECTOR(vEndStepX, viEndStepX);

	CVectorD<3> vEndNextY;
	gluUnProject(start, step, zMax,
		modelMatrix, projMatrix, m_viewport, 
		&vEndNextY[0], &vEndNextY[1], &vEndNextY[2]);
	CVectorD<3> vEndStepY = vEndNextY - vEnd;
	CREATE_INT_VECTOR(vEndStepY, viEndStepY);

	CREATE_INT_VECTOR(vEnd, viEnd);
	viEnd[0] += 32768;
	viEnd[1] += 32768;
	viEnd[2] += 32768;

#ifdef COMPUTE_MINMAX
	int nMax = -INT_MAX;
	int nMin = INT_MAX;
#else
	int nMax = 65000 / 64 * m_nSteps; // 0;
	int nMin = 15000 / 64 * m_nSteps; // INT_MAX;
#endif

#ifdef POST_PROCESS
	int level_offset = -nMin;
	int window_div = (nMax - nMin) / 256;
#endif

	short nMaxVoxel = forVolume->GetMax();
	TRACE1("Max voxel value = %i\n", nMaxVoxel);

	for (int nY = 0; nY < m_nImageHeight; nY++)
	{
		CVectorD<3, int> viStartOld = viStart;
		CVectorD<3, int> viEndOld = viEnd;

		int nPixelAt = nY * m_nImageWidth;

		for (int nX = 0; nX < m_nImageWidth; nX++, nPixelAt++)
		{
			CVectorD<3, int> viStartOldX = viStart;

			CVectorD<3, int> viStep = viEnd - viStart;
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

					nPixelValue += (int) pppVoxels[nVoxelZ][nVoxelY][nVoxelX];
#else	// ifdef _DEBUG
					nPixelValue += (int) pppVoxels[viStart[2] >> 16][viStart[1] >> 16][viStart[0] >> 16];
#endif	// ifdef _DEBUG

					viStart += viStep;
				}
				if (nPixelValue >  m_nSteps * (int)(nMaxVoxel) / 2)
				{
					bOverMax = true;
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
*/
}

static CResampler<short> shortResampler;
static CResampler<int> intResampler;
