//////////////////////////////////////////////////////////////////////
// Renderable.cpp: implementation of the CRenderable class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// OpenGL includes
#include <gl/gl.h>

// render context
#include "RenderContext.h"

// the parent class
#include "SceneView.h"

// class declaration
#include "Renderable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CRenderable::CRenderable
// 
// a renderable is able to describe itself given a rendering context
//////////////////////////////////////////////////////////////////////
CRenderable::CRenderable()
	: m_pObject(NULL),
		m_pView(NULL),
		m_nDrawListOpaque(-1),
		m_nDrawListTransparent(-1),
		m_color(RGB(255, 255, 255)),
		m_alpha(1.0),
		m_bEnabled(TRUE)
{
}

//////////////////////////////////////////////////////////////////////
// CRenderable::~CRenderable
// 
// destroys the renderable
//////////////////////////////////////////////////////////////////////
CRenderable::~CRenderable()
{
}

//////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE
// 
// implements the dynamic create behavior for the renderables
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CRenderable, CObject);


//////////////////////////////////////////////////////////////////////
// CRenderable::GetObject
// 
// returns the object being rendered
//////////////////////////////////////////////////////////////////////
// sets the object being rendered
CObject *CRenderable::GetObject()
{
	return m_pObject;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::SetObject
// 
// sets the object being rendered
//////////////////////////////////////////////////////////////////////
void CRenderable::SetObject(CObject *pObject)
{
	m_pObject = pObject;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::GetColor
// 
// color for the renderer (use depends on objects being rendered)
//////////////////////renderable///////////////////////////////////////////////
COLORREF CRenderable::GetColor() const
{
	return m_color;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::SetColor
// 
// color for the renderable (use depends on objects being rendered)
//////////////////////////////////////////////////////////////////////
void CRenderable::SetColor(COLORREF color)
{
	// set the color
	m_color = color;

	// invalidate to redraw
	Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetAlpha
// 
// alpha transparency for the renderable (use depends on objects being 
//		rendered)
//////////////////////////////////////////////////////////////////////
double CRenderable::GetAlpha() const
{
	return m_alpha;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetAlpha
// 
// alpha transparency for the renderable (use depends on objects being 
//		rendered)
//////////////////////////////////////////////////////////////////////
void CRenderable::SetAlpha(double alpha)
{
	// set the color
	m_alpha = alpha;

	// invalidate to redraw
	Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetCentroid
// 
// returns the centroid of this renderable, used to determine
//		order for rendering
//////////////////////////////////////////////////////////////////////
// accessors for the renderable's centroid (used for sorting)
const CVector<3>& CRenderable::GetCentroid() const
{
	return m_vCentroid;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetCentroid
// 
// sets the centroid of this renderable, used to determine
//		order for rendering
//////////////////////////////////////////////////////////////////////
void CRenderable::SetCentroid(const CVector<3>& vCentroid)
{
	// set the renderable centroid
	m_vCentroid = vCentroid;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetNearestDistance
// 
// returns the nearest distance from the passed point to the object
//		being rendered.  Default implementation returns distance
//		from the point to the centroid
//////////////////////////////////////////////////////////////////////
double CRenderable::GetNearestDistance(const CVector<3>& vPoint)
{
	return (vPoint - GetCentroid()).GetLength();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetFurthestDistance
// 
// returns the furthest distance from the passed point to the object
//		being rendered.  Default implementation returns distance
//		from the point to the centroid
//////////////////////////////////////////////////////////////////////
double CRenderable::GetFurthestDistance(const CVector<3>& vPoint)
{
	return (vPoint - GetCentroid()).GetLength();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetModelviewMatrix
// 
// the modelview matrix for this renderable
//////////////////////////////////////////////////////////////////////
const CMatrix<4>& CRenderable::GetModelviewMatrix() const
{
	return m_mModelview;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetModelviewMatrix
// 
// the modelview matrix for this renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::SetModelviewMatrix(const CMatrix<4>& m)
{
	// set the modelview matrix
	m_mModelview = m;

	// invalidate if we are live
	if (NULL != m_pView)
	{
		// and invalidate the view to redraw without re-rendering
		m_pView->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////
// CRenderable::IsEnabled
// 
// flag to enable/disable the renderable
//////////////////////////////////////////////////////////////////////
BOOL CRenderable::IsEnabled() const
{
	return m_bEnabled;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetEnabled
// 
// flag to enable/disable the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::SetEnabled(BOOL bEnabled)
{
	// set the enabled flag
	m_bEnabled = bEnabled;

	// invalidate if we are live
	if (NULL != m_pView)
	{
		// and invalidate the view to redraw without re-rendering
		m_pView->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////
// CRenderable::Invalidate
// 
// triggers re-describing the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::Invalidate(CObservableEvent *pEvent, void *pValue)
{
	// only invalidate if it is attached to a CSceneView
	if (NULL == m_pView)
	{
		return;
	}

	// set the current HGLRC
	m_pView->MakeCurrentGLRC();

	// if the draw list exists,
	if (-1 != m_nDrawListOpaque)
	{
		// delete it
		glDeleteLists(m_nDrawListOpaque, 1);

		// set it to -1
		m_nDrawListOpaque = -1;
	}

	// if the draw list exists,
	if (-1 != m_nDrawListTransparent)
	{
		// delete it
		glDeleteLists(m_nDrawListTransparent, 1);

		// set it to -1
		m_nDrawListTransparent = -1;
	}

	// invalidate the view, which will trigger re-rendering
	m_pView->Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetupRenderingContext
// 
// sets up the rendering context
//////////////////////////////////////////////////////////////////////
void CRenderable::SetupRenderingContext(CRenderContext *pRC)
{
	// load the renderer's modelview matrix
	pRC->LoadMatrix(GetModelviewMatrix());

	// set the drawing color
	pRC->Color(GetColor());
	pRC->Alpha(1.0 - sqrt(1.0 - GetAlpha()));
	// glColor(GetColor(), );

	// Set the shading model
	glShadeModel(GL_SMOOTH);

	// enable lighting
	// glEnable(GL_LIGHTING);
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeOpaque
// 
// call to describe the opaque part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawOpaque(CRenderContext *pRC)
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeTransparent
// 
// call to describe the transparent part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawTransparent(CRenderContext *pRC)
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeOpaqueDrawList
// 
// forms a call list for the opaque parts, and then calls it
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawOpaqueList(CRenderContext *pRC)
{
	// only draw if enabled
	if (!IsEnabled())
	{
		return;
	}

	// see if we need to re-create the draw list
	if (-1 == m_nDrawListOpaque)
	{
		// generate a new draw list name
		m_nDrawListOpaque = glGenLists(1);

		// create the list
		glNewList(m_nDrawListOpaque, GL_COMPILE);

		// set up the rendering context
		SetupRenderingContext(pRC);

		// now populate the draw list
		DrawOpaque(pRC);

		// finish up the list
		glEndList();
	}

	// now actually render the scene
	glCallList(m_nDrawListOpaque);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeAlphaDrawList
// 
// forms a call list for the alpha parts, and then calls it
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawTransparentList(CRenderContext *pRC)
{
	// only draw if enabled
	if (!IsEnabled())
	{
		return;
	}

	// see if we need to re-create the draw list
	if (-1 == m_nDrawListTransparent)
	{
		// generate a new draw list name
		m_nDrawListTransparent = glGenLists(1);

		// create the list
		glNewList(m_nDrawListTransparent, GL_COMPILE);

		// set up the rendering context
		SetupRenderingContext(pRC);

		// now populate the draw list
		DrawTransparent(pRC);

		// finish up the list
		glEndList();
	}

	// now actually render the scene
	glCallList(m_nDrawListTransparent);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}
