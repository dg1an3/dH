// RenderContext.cpp: implementation of the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// OpenGL includes
#include <gl/gl.h>

// class declaration
#include "RenderContext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenderContext::CRenderContext(LPDIRECT3DDEVICE8 pd3dDevice)
: // m_pSceneView(pSceneView),
	m_pd3dDevice(pd3dDevice),
	m_pTexture(NULL)
{

}

CRenderContext::~CRenderContext()
{

}

LPDIRECT3DVERTEXBUFFER8 CRenderContext::CreateVertexBuffer(UINT nLength, DWORD dwFVF)
{
	LPDIRECT3DVERTEXBUFFER8 lpVertexBuffer;

	m_pd3dDevice->CreateVertexBuffer(nLength, 0, dwFVF, 
		D3DPOOL_DEFAULT, &lpVertexBuffer);

	return lpVertexBuffer;
}

void CRenderContext::BeginLines()
{
	glBegin(GL_LINES);
}

void CRenderContext::BeginLineLoop()
{
	glBegin(GL_LINE_LOOP);
}

void CRenderContext::BeginTriangles()
{
	glBegin(GL_TRIANGLES);
}

void CRenderContext::BeginQuads()
{
	glBegin(GL_QUADS);
}

void CRenderContext::BeginQuadStrip()
{
	glBegin(GL_QUAD_STRIP);
}

void CRenderContext::BeginPolygon()
{
	glBegin(GL_POLYGON);
}

void CRenderContext::End()
{
	glEnd();
}

void CRenderContext::Vertex(const CVectorD<2, double>& v)
{
	glVertex2dv(v);
}

void CRenderContext::Vertex(const CVectorD<3, double>& v)
{
	glVertex3dv(v);
}

void CRenderContext::Vertex(const CVectorD<4, double>& v)
{
	glVertex4dv(v);
}

void CRenderContext::Normal(const CVectorD<3, double>& n)
{
	glNormal3dv(n);
}

void CRenderContext::Normal(const CVectorD<4, double>& n)
{
	Normal(FromHomogeneous<3, double>(n));
}

/* void CRenderContext::LineLoopFromPolygon(CPolygon& poly)
{
	CMatrixNxM<>& mVertex = poly.LockVertexMatrix();
	// use the polygon's vertex data as the data array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_DOUBLE, 0, (REAL *) mVertex);

	// and draw the loop
	glDrawArrays(GL_LINE_LOOP, 0, poly.GetVertexCount());

	glDisableClientState(GL_VERTEX_ARRAY);
	poly.UnlockVertexMatrix();
}
*/
void CRenderContext::TrianglesFromSurface(CMesh& surf)
{
	// set the array for vertices
	glEnableClientState(GL_VERTEX_ARRAY);
//	glVertexPointer(3, GL_DOUBLE, 0, surf.GetVertexArray().GetData()-1);

	// set the array for normals
	glEnableClientState(GL_NORMAL_ARRAY);
//	glNormalPointer(GL_DOUBLE, 0, surf.GetNormalArray().GetData()-1);

	// if there is a texture bound,
	if (NULL != m_pTexture)
	{
		// switch to texture matrix
		glMatrixMode(GL_TEXTURE);

		// load the texture adjustment onto the matrix stack
		// LoadMatrix(m_pTexture->GetProjection());

		// enable texture coordinate mode
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// set the texture coordinate pointer
//		glTexCoordPointer(3, GL_DOUBLE, 0, surf.GetVertexArray().GetData()-1);

		// switch back to modelview matrix
		glMatrixMode(GL_MODELVIEW);

		// make sure no errors occurred
		ASSERT(glGetError() == GL_NO_ERROR);
	}

	// now draw the surface from the arrays of data
//	glDrawElements(GL_TRIANGLES, surf.GetTriangleCount() * 3, 
//		GL_UNSIGNED_INT, (void *) surf.GetIndexArray().GetData());

	// disable the use of arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	// if there is a texture bound,
	if (NULL != m_pTexture) 
	{
		// disable the texture coordinate array
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void CRenderContext::Color(COLORREF color)
{
	m_color = color;

	glColor4ub(GetRValue(m_color), GetGValue(m_color), GetBValue(m_color),
		(GLubyte) 255.0 * m_alpha);
}

void CRenderContext::Alpha(double alpha)
{
	m_alpha = alpha;

	glColor4ub(GetRValue(m_color), GetGValue(m_color), GetBValue(m_color),
		(GLubyte) 255.0 * m_alpha);
}

// set up for surface rendering
void CRenderContext::SetSmoothShading(BOOL bSmooth)
{
	glShadeModel(bSmooth ? GL_SMOOTH : GL_FLAT);
}


void CRenderContext::SetLighting(BOOL bLighting)
{
	if (bLighting)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}

void CRenderContext::SetupLines()
{
	// no lighting
	glDisable(GL_LIGHTING);

	// line defaults
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// set up polygons for wire frame rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void CRenderContext::SetupOpaqueSurface()
{
	// no lighting
	glEnable(GL_LIGHTING);

	// set up polygons for wire frame rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CRenderContext::SetupTransparentFrontFaces()
{
}

void CRenderContext::SetupTransparentBackFaces()
{
}

void CRenderContext::PushMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void CRenderContext::PopMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CRenderContext::Translate(const CVectorD<3> vOffset)
{
	glMatrixMode(GL_MODELVIEW);
	glTranslated(vOffset[0], vOffset[1], vOffset[2]);
}

void CRenderContext::Rotate(double angle,  const CVectorD<3> vAxis)
{
	glMatrixMode(GL_MODELVIEW);
	glRotated(angle, vAxis[0], vAxis[1], vAxis[2]);
}

void CRenderContext::Bind(CTexture *pTexture)
{
	m_pTexture = pTexture;
	// m_pTexture->Bind(m_pSceneView);
}

void CRenderContext::Unbind()
{
	m_pTexture->Unbind();

	m_pTexture = NULL;
}

// helper functions for matrix manipulation
void CRenderContext::LoadMatrix(const CMatrixD<4, double>& m)
{
	glLoadMatrixd(m);
}


void CRenderContext::MultMatrix(const CMatrixD<4, double>& m)
{
	glMultMatrixd(m);
}

