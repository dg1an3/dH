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

CRenderContext::CRenderContext(CSceneView *pSceneView)
: m_pSceneView(pSceneView),
	m_pTexture(NULL)
{

}

CRenderContext::~CRenderContext()
{

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

void CRenderContext::Vertex(const CVector<2>& v)
{
	glVertex2dv(v);
}

void CRenderContext::Vertex(const CVector<3>& v)
{
	glVertex3dv(v);
}

void CRenderContext::Vertex(const CVector<4>& v)
{
	glVertex4dv(v);
}

void CRenderContext::Normal(const CVector<3>& n)
{
	glNormal3dv(n);
}

void CRenderContext::Normal(const CVector<4>& n)
{
	Normal(FromHomogeneous<3, double>(n));
}

void CRenderContext::LineLoopFromPolygon(CPolygon& poly)
{
	// use the polygon's vertex data as the data array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_DOUBLE, 0, poly.GetVertexArray().GetData());

	// and draw the loop
	glDrawArrays(GL_LINE_LOOP, 0, poly.GetVertexCount());

	glDisableClientState(GL_VERTEX_ARRAY);
}

void CRenderContext::TrianglesFromSurface(CSurface& surf)
{
	// set the array for vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, surf.GetVertexArray().GetData()-1);

	// set the array for normals
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_DOUBLE, 0, surf.GetNormalArray().GetData()-1);

	// if there is a texture bound,
	if (NULL != m_pTexture)
	{
		// switch to texture matrix
		glMatrixMode(GL_TEXTURE);

		// load the texture adjustment onto the matrix stack
		LoadMatrix(m_pTexture->GetProjection());

		// enable texture coordinate mode
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// set the texture coordinate pointer
		glTexCoordPointer(3, GL_DOUBLE, 0, surf.GetVertexArray().GetData()-1);

		// switch back to modelview matrix
		glMatrixMode(GL_MODELVIEW);

		// make sure no errors occurred
		ASSERT(glGetError() == GL_NO_ERROR);
	}

	// now draw the surface from the arrays of data
	glDrawElements(GL_TRIANGLES, surf.GetTriangleCount() * 3, 
		GL_UNSIGNED_INT, (void *) surf.GetIndexArray().GetData());

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

void CRenderContext::Translate(const CVector<3> vOffset)
{
	glMatrixMode(GL_MODELVIEW);
	glTranslated(vOffset[0], vOffset[1], vOffset[2]);
}

void CRenderContext::Rotate(double angle,  const CVector<3> vAxis)
{
	glMatrixMode(GL_MODELVIEW);
	glRotated(angle, vAxis[0], vAxis[1], vAxis[2]);
}

void CRenderContext::Bind(CTexture *pTexture)
{
	m_pTexture = pTexture;
	m_pTexture->Bind(m_pSceneView);
}

void CRenderContext::Unbind()
{
	m_pTexture->Unbind();

	m_pTexture = NULL;
}

// helper functions for matrix manipulation
void CRenderContext::LoadMatrix(const CMatrix<4, double>& m)
{
	double mArray[16];
	m.ToArray(mArray);
	glLoadMatrixd(mArray);
}


void CRenderContext::MultMatrix(const CMatrix<4, double>& m)
{
	double mArray[16];
	m.ToArray(mArray);
	glMultMatrixd(mArray);
}

