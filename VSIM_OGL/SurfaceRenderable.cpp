// SurfaceRenderable.cpp: implementation of the CSurfaceRenderable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SurfaceRenderable.h"

#include <glMatrixVector.h>

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

// CVector<3> CSurfaceRenderable::m_vXlate;

//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
static CMatrix<4> CreateScaleHG(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<3> mScale = CreateScale(vScale);

	CMatrix<4> mScaleHG(mScale);

	return mScaleHG;
}

//////////////////////////////////////////////////////////////////////
// function CreateRotate
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
static CMatrix<4> CreateRotateHG(const double& theta, 
							   const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<3> mRotate = CreateRotate(theta, vAxis);

	CMatrix<4> mRotateHG(mRotate);

	return mRotateHG;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSurfaceRenderable::CSurfaceRenderable(CSceneView *pView)
	: CRenderable(pView),
		m_pSurface(NULL),
		m_pBeam(NULL),
		m_pLightfieldTexture(NULL),
		m_pEndTexture(NULL),
		m_bWireFrame(FALSE),
		m_bColorWash(FALSE),
		m_bShowBoundsSurface(FALSE)
{
	SetColor(RGB(192, 192, 192));
	// isWireFrame.AddObserver(this, (ChangeFunction) OnChange);
	// isColorWash.AddObserver(this, (ChangeFunction) OnChange);

	// couchAngle.AddObserver(this, (ChangeFunction) OnPositionChange);
	// tableOffset.AddObserver(this, (ChangeFunction) OnPositionChange);
}

CSurfaceRenderable::~CSurfaceRenderable()
{
	if (m_pLightfieldTexture)
	{
		delete m_pLightfieldTexture;
		m_pLightfieldTexture = NULL;
	}	
}

CSurface * CSurfaceRenderable::GetSurface()
{
	return m_pSurface;
}

void CSurfaceRenderable::SetSurface(CSurface *pSurface)
{
	if (m_pSurface != NULL)
	{
		// remove this as an observer of surface changes
		::RemoveObserver<CSurfaceRenderable>(&m_pSurface->GetChangeEvent(),
			this, OnChange);
	}

	m_pSurface = pSurface;

	if (m_pSurface != NULL)
	{
		// add this as an observer of surface changes
		::AddObserver<CSurfaceRenderable>(&m_pSurface->GetChangeEvent(),
			this, OnChange);

		// set the renderables centroid to the center of the bounding box
		SetCentroid(0.5 * (m_pSurface->GetBoundsMin() + m_pSurface->GetBoundsMax()));
	}

	// trigger re-rendering
	Invalidate();
}

CBeam * CSurfaceRenderable::GetLightFieldBeam() 
{
	return m_pBeam; 
}

void CSurfaceRenderable::SetLightFieldBeam(CBeam *pBeam)
{
	if (m_pBeam != NULL)
		m_pBeam->GetChangeEvent().RemoveObserver(this, (ChangeFunction) OnChange);

	m_pBeam = pBeam;

	if (m_pBeam != NULL)
		m_pBeam->GetChangeEvent().AddObserver(this, (ChangeFunction) OnChange);

	Invalidate();
}

void CSurfaceRenderable::DescribeOpaque()
{
	// now translate drawing to that center
	// glRotated(couchAngle.Get() * 180.0 / PI, 
	//	0.0, 0.0, 1.0);
	// glTranslate(-1.0 * tableOffset.Get());

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

	if (m_bWireFrame)
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		glColor(GetColor());

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
			glVertexPointer(2, GL_DOUBLE, 0, 
				polygon.GetVertexArray().GetData());

			// and draw the loop
			glDrawArrays(GL_LINE_LOOP, 0, polygon.GetVertexCount());

			glDisableClientState(GL_VERTEX_ARRAY);

			glPopMatrix();
		}

		glEnable(GL_LIGHTING);

		return;
	}

	if (m_bColorWash)
	{
		// disable lighting
		glDisable(GL_LIGHTING);

		// set the depth mask to read-only
		glDepthMask(GL_FALSE);

		// set up the accumulation buffer, using the current transparency
		glAccum(GL_LOAD, 0.75f);
	}

	if (FALSE) // m_bShowBoundsSurface)
	{
		// draw the boundary surfaces
		glColor(RGB(0, 0, 128));

		double yMin = m_pSurface->GetBoundsMin()[1];
		double yMax = m_pSurface->GetBoundsMax()[1];
		for (int nAtTri = 0; nAtTri < m_pSurface->GetTriangleCount(); nAtTri++)
		{
			const double *vVert0 = m_pSurface->GetTriangleVertex(nAtTri, 0);
			const double *vVert1 = m_pSurface->GetTriangleVertex(nAtTri, 1);
			const double *vVert2 = m_pSurface->GetTriangleVertex(nAtTri, 2);

			if (vVert0[1] == yMin && vVert1[1] == yMin && vVert2[1] == yMin)
			{
				glBegin(GL_TRIANGLES);

				glVertex3dv(vVert0);
				glNormal3dv(m_pSurface->GetTriangleNormal(nAtTri, 0));
				glTexCoord3dv(vVert0);

				glVertex3dv(vVert1);
				glNormal3dv(m_pSurface->GetTriangleNormal(nAtTri, 1));
				glTexCoord3dv(vVert1);

				glVertex3dv(vVert2);
				glNormal3dv(m_pSurface->GetTriangleNormal(nAtTri, 2));
				glTexCoord3dv(vVert2);

				glEnd();
			}
		}
	}

	// set the array for vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0,
		m_pSurface->GetVertexArray().GetData()-1);

	// set the array for normals
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_DOUBLE, 0, 
		m_pSurface->GetNormalArray().GetData()-1);

	// make the depth mask read-only
	glDepthMask(GL_FALSE);

	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	// individual vertices of the faces, etc... 
	GLfloat specular [] = { 0.5, 0.5, 0.5, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	GLfloat shininess [] = { 20.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	// Set the GL_AMBIENT_AND_DIFFUSE color state variable to be the
	// one referred to by all following calls to glColor
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// set the color of the surface
	glColor(GetColor()); // , GetAlpha());

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
		glMultMatrix(m_pBeam->GetTreatmentMachine()->m_projection);

		// transform from patient -> beam coordinates
		CMatrix<4> mXform = m_pBeam->GetBeamToPatientXform();
		mXform.Invert();
		glMultMatrix(mXform);

		// now translate the texture for the model center
		// glTranslate(m_vXlate);

		// enable texture coordinate mode
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// set the texture coordinate pointer
		glTexCoordPointer(3, GL_DOUBLE, 0, m_pSurface->GetVertexArray().GetData()-1);

		glMatrixMode(GL_MODELVIEW);

		// make sure no errors occurred
		ASSERT(glGetError() == GL_NO_ERROR);
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

	// make the depth mask read-only
	glDepthMask(GL_TRUE);

	if (m_bColorWash)
	{
		// set up the accumulation buffer, using the current transparency
		glAccum(GL_ACCUM, 0.25f);
		glAccum(GL_RETURN, 1.0f);

		// set the depth mask to writeable
		glDepthMask(GL_TRUE);

		// re-enable lighting
		glEnable(GL_LIGHTING);
	}
}

void CSurfaceRenderable::OnChange(CObservableObject *pFromObject, void *pOldValue)
{
	if (pFromObject->GetParent() == GetLightFieldBeam())
	{
		if (m_pLightfieldTexture != NULL)
		{
			// trigger re-generation of the lightfield texture
			delete m_pLightfieldTexture;
			m_pLightfieldTexture = NULL;
		}
	}

	CRenderable::Invalidate();
}

void CSurfaceRenderable::OnPositionChange(CObservableObject *pFromObject, void *pOldValue)
{
	// update the modelview matrix
	SetModelviewMatrix(
		CreateRotateHG(m_couchAngle, CVector<3>(0.0, 0.0, 1.0)) 
		* CreateTranslate(-1.0 * m_vTableOffset)
	);
}

CTexture * CSurfaceRenderable::GetLightfieldTexture()
{
	if (m_pLightfieldTexture == NULL)
	{
		m_pLightfieldTexture = new CTexture();
		m_pLightfieldTexture->SetWidthHeight(TEX_RESOLUTION, TEX_RESOLUTION);

		CDC *pDC = m_pLightfieldTexture->GetDC();

		// default pen == thick green
		CPen pen(PS_SOLID, PEN_THICKNESS, RGB(0, 255, 0));
		CPen *pOldPen = pDC->SelectObject(&pen);

		// hollow brush
		CBrush *pOldBrush = (CBrush *)pDC->SelectStockObject(HOLLOW_BRUSH);

		CVector<2> vMin = m_pBeam->GetCollimMin();
		CVector<2> vMax = m_pBeam->GetCollimMax();

		vMin *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
		vMin += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

		vMax *= (double)(TEX_RESOLUTION - 5) / TEX_SIZE;
		vMax += CVector<2>(TEX_RESOLUTION/2, TEX_RESOLUTION/2);

		pDC->Rectangle((int)vMin[0], (int)vMin[1], (int)vMax[0], (int)vMax[1]);

		CBrush brush(RGB(128, 128, 128));
		pDC->SelectObject(&brush);
		pDC->SetPolyFillMode(WINDING);
		for (int nAt = 0; nAt < m_pBeam->GetBlockCount(); nAt++)
		{
			CPolygon& polygon = *m_pBeam->GetBlockAt(nAt);
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
