// SurfaceRenderer.cpp: implementation of the CSurfaceRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "cube.h"
#include "SurfaceRenderer.h"

#include "gl\gl.h"
#include "gl\glu.h"
#include "glMatrixVector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// texture resolution (in pixels)
#define TEX_RESOLUTION 512

// maximum texture size (in mm)
#define TEX_SIZE 40.0

#define PEN_THICKNESS (TEX_RESOLUTION / 100)

CVector<3> CSurfaceRenderer::m_vXlate;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSurfaceRenderer::CSurfaceRenderer(COpenGLView *pView)
	: COpenGLRenderer(pView),
		m_pSurface(NULL),
		m_pBeam(NULL),
		m_pLightfieldTexture(NULL),
		isWireFrame(TRUE)
{
	myColor.Set(RGB(192, 192, 192));
	isWireFrame.AddObserver(this);
}

CSurfaceRenderer::~CSurfaceRenderer()
{
	if (m_pLightfieldTexture)
	{
		delete m_pLightfieldTexture;
		m_pLightfieldTexture = NULL;
	}	
}

CSurface * CSurfaceRenderer::GetSurface()
{
	return m_pSurface;
}

void CSurfaceRenderer::SetSurface(CSurface *pSurface)
{
	if (m_pSurface != NULL)
		m_pSurface->RemoveObserver(this);

	m_pSurface = pSurface;

	if (m_pSurface != NULL)
		m_pSurface->AddObserver(this);

	Invalidate();
}

CBeam * CSurfaceRenderer::GetLightFieldBeam() 
{
	return m_pBeam; 
}

void CSurfaceRenderer::SetLightFieldBeam(CBeam *pBeam)
{
	if (m_pBeam != NULL)
		m_pBeam->RemoveObserver(this);

	m_pBeam = pBeam;

	if (m_pBeam != NULL)
		m_pBeam->AddObserver(this);

	Invalidate();
}

void CSurfaceRenderer::OnRenderScene()
{
	// now translate drawing to that center
	glTranslate(m_vXlate);

#ifdef DRAW_CROSSHAIRS
	// draw cross-hairs
	glBegin(GL_LINES);

	glColor3f(0.0f, 1.0f, 0.0f);

	glVertex3d(-1000.0, 0.0, 0.0);
	glVertex3d(1000.0, 0.0, 0.0);

	glVertex3d(0.0, -1000.0, 0.0);
	glVertex3d(0.0, 1000.0, 0.0);

	glVertex3d(0.0, 0.0, -1000.0);
	glVertex3d(0.0, 0.0, 1000.0);

	glEnd(); 
#endif DRAW_CROSSHAIRS

	if (isWireFrame.Get())
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		glColor(myColor.Get());

		for (int nAt = 0; nAt < m_pSurface->GetContourCount(); nAt++)
		{
			CPolygon& polygon = m_pSurface->GetContour(nAt);

			glPushMatrix();

			// translate to the appropriate reference distance
			glTranslated(0.0, m_pSurface->GetContourRefDist(nAt), 0.0);

			// after we rotate the data into the X-Z plane
			glRotated(90.0, 1.0, 0.0, 0.0);

			// use the polygon's vertex data as the data array
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_DOUBLE, 0, polygon.GetVertexArray().GetData());

			// and draw the loop
			glDrawArrays(GL_LINE_LOOP, 0, polygon.GetVertexCount());

			glDisableClientState(GL_VERTEX_ARRAY);

			glPopMatrix();
		}

		glEnable(GL_LIGHTING);

		return;
	}

	// set the array for vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, m_pSurface->GetVertexArray().GetData()-1);

	// set the array for normals
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_DOUBLE, 0, m_pSurface->GetNormalArray().GetData()-1);

	// set the color of the surface
	glColor(myColor.Get());

	if (m_pBeam != NULL)
	{
		GetLightfieldTexture()->Bind(m_pView);

		glMatrixMode(GL_TEXTURE);

		// compute the texture adjustment
		CMatrix<4> mTex;
		mTex[0][0] = -1.0 / (TEX_SIZE + 1);
		mTex[1][1] = -1.0 / (TEX_SIZE + 1);
		mTex[0][3] = 0.5;
		mTex[1][3] = 0.5;

		// load the texture adjustment onto the matrix stack
		glLoadMatrix(mTex);

		// beam projection
		glMultMatrix(m_pBeam->forMachine->GetProjection());

		// transform from patient -> beam coordinates
		CMatrix<4> mXform = m_pBeam->GetBeamToPatientXform();
		mXform.Invert();
		glMultMatrix(mXform);

		// now translate the texture for the model center
		glTranslate(m_vXlate);

		// enable texture coordinate mode
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// set the texture coordinate pointer
		glTexCoordPointer(3, GL_DOUBLE, 0, m_pSurface->GetVertexArray().GetData()-1);

		glMatrixMode(GL_MODELVIEW);
	}

	// now draw the surface from the arrays of data
	glDrawElements(GL_TRIANGLES, m_pSurface->GetTriangleCount() * 3, 
		GL_UNSIGNED_INT, (void *)m_pSurface->GetIndexArray().GetData());

	// disable the use of arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	if (m_pBeam != NULL) 
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		GetLightfieldTexture()->Unbind();
	}
}

void CSurfaceRenderer::OnChange(CObservable *pFromObject)
{
	if (pFromObject == GetLightFieldBeam())
	{
		if (m_pLightfieldTexture != NULL)
		{
			// trigger re-generation of the lightfield texture
			delete m_pLightfieldTexture;
			m_pLightfieldTexture = NULL;
		}
	}

	COpenGLRenderer::OnChange(pFromObject);
}

COpenGLTexture * CSurfaceRenderer::GetLightfieldTexture()
{
	if (m_pLightfieldTexture == NULL)
	{
		m_pLightfieldTexture = new COpenGLTexture();
		m_pLightfieldTexture->SetWidthHeight(TEX_RESOLUTION, TEX_RESOLUTION);

		CDC *pDC = m_pLightfieldTexture->GetDC();

		// default pen == thick green
		CPen pen(PS_SOLID, PEN_THICKNESS, RGB(0, 255, 0));
		CPen *pOldPen = pDC->SelectObject(&pen);

		// hollow brush
		CBrush *pOldBrush = (CBrush *)pDC->SelectStockObject(HOLLOW_BRUSH);

		CVector<2> vMin = m_pBeam->collimMin.Get();
		CVector<2> vMax = m_pBeam->collimMax.Get();

		vMin *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
		vMin += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

		vMax *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
		vMax += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

		pDC->Rectangle((int)vMin[0], (int)vMin[1], (int)vMax[0], (int)vMax[1]);

		CBrush brush(RGB(128, 128, 128));
		pDC->SelectObject(&brush);
		pDC->SetPolyFillMode(WINDING);
		for (int nAt = 0; nAt < m_pBeam->blocks.GetSize(); nAt++)
		{
			CPolygon& polygon = *m_pBeam->blocks.Get(nAt);
			CArray<CPoint, CPoint&> arrBlock;
			arrBlock.SetSize(polygon.GetVertexCount());
			for (int nAtVert = 0; nAtVert < polygon.GetVertexCount(); nAtVert++)
			{
				CVector<2> v = polygon.GetVertex(nAtVert);
				v *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
				v += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

				arrBlock[nAtVert] = v;
			}

			pDC->Polygon(arrBlock.GetData(), polygon.GetVertexCount());
		}

		// draw the central axis
		CVector<2> vCenter(0.0, 0.0);
		vCenter *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
		vCenter += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

		pDC->MoveTo(vCenter - CVector<2>(8.0, 0.0));
		pDC->LineTo(vCenter + CVector<2>(8.0, 0.0));
		pDC->MoveTo(vCenter - CVector<2>(0.0, 8.0));
		pDC->LineTo(vCenter + CVector<2>(0.0, 8.0));

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);

		m_pLightfieldTexture->DrawBorder();

		m_pLightfieldTexture->ReleaseDC();
	}

	return m_pLightfieldTexture;
}
