// DRRRenderer.cpp: implementation of the CDRRRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DRRRenderer.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <glMatrixVector.h>

#include <OpenGLView.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDRRRenderer::CDRRRenderer(COpenGLView *pView)
: COpenGLRenderer(pView),
	forVolume(NULL)
{

}

CDRRRenderer::~CDRRRenderer()
{

}

void CDRRRenderer::ComputeDRR()
{
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

	short ***pppVoxels = forVolume->GetVoxels();

	int nSteps = 512;
	int nMax = 0;
	int nMin = INT_MAX;
	
	for (int nY = 0; nY < rect.Height(); nY++)
		for (int nX = 0; nX < rect.Width(); nX++)
		{
			// un-project the window coordinates into the model coordinate system
			CVector<3> vStart;
			gluUnProject((GLdouble)nX, (GLdouble)nY, m_pView->GetNearPlane(),
				modelMatrix, projMatrix, viewport, 
				&vStart[0], &vStart[1], &vStart[2]);
			vStart += CVector<3>(0.5, 0.5, 0.5);

			// un-project the window coordinates into the model coordinate system
			CVector<3> vEnd;
			gluUnProject((GLdouble)nX, (GLdouble)nY, m_pView->GetFarPlane(),
				modelMatrix, projMatrix, viewport, 
				&vEnd[0], &vEnd[1], &vEnd[2]);

			CVector<3> vStep = (double) (1.0f / (float) nSteps) * (vEnd - vStart);

			int nPixelAt = nY * rect.Width() + nX;
			m_arrPixels[nPixelAt] = 0;

			CVector<3, int> viStart;
			viStart[0] = vStart[0] * 65536.0;
			viStart[1] = vStart[1] * 65536.0;
			viStart[2] = vStart[2] * 65536.0;

			CVector<3, int> viStep;
			viStep[0] = vStep[0] * 65536.0;
			viStep[1] = vStep[1] * 65536.0;
			viStep[2] = vStep[2] * 65536.0;

			int nWidth = forVolume->width.Get();
			int nHeight = forVolume->height.Get();
			int nDepth = forVolume->depth.Get();
			for (int nAt = 0; nAt < nSteps; nAt++)
			{
				int nVoxelX = viStart[0] >> 16;
				int nVoxelY = viStart[1] >> 16;
				int nVoxelZ = viStart[2] >> 16;
				if (nVoxelX >= 0 && nVoxelX < nWidth
					&& nVoxelY >= 0 && nVoxelY < nHeight
					&& nVoxelZ >= 0 && nVoxelZ < nDepth
					&& pppVoxels[nVoxelZ][nVoxelY][nVoxelX] >= 0)

					m_arrPixels[nPixelAt] += pppVoxels[nVoxelZ][nVoxelY][nVoxelX];

				viStart += viStep;
			}

			nMax = max(m_arrPixels[nPixelAt], nMax);
			nMin = min(m_arrPixels[nPixelAt], nMin);
		}

	for (nY = 0; nY < rect.Height(); nY++)
		for (int nX = 0; nX < rect.Width(); nX++)
		{
			m_arrPixels[nY * rect.Width() + nX] -= nMin;
			m_arrPixels[nY * rect.Width() + nX] *= (INT_MAX / (nMax - nMin)); 
		}
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

		glRasterPos2i(0, 0);

		if (m_arrPixels.GetSize() != rect.Height() * rect.Width())
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
