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
CRenderable::CRenderable(CSceneView *pView)
	: m_pView(pView),
		m_nDrawList(-1),
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

	// and invalidate the view to redraw without re-rendering
	m_pView->Invalidate();
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

	// and invalidate the view to redraw without re-rendering
	m_pView->Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::Invalidate
// 
// triggers re-describing the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::Invalidate(CObservableEvent *pEvent, void *pValue)
{
	// set the current HGLRC
	m_pView->MakeCurrentGLRC();

	// if the draw list exists,
	if (m_nDrawList != -1)
	{
		// delete it
		glDeleteLists(m_nDrawList, 1);

		// set it to -1
		m_nDrawList = -1;
	}

	// invalidate the view, which will trigger re-rendering
	m_pView->Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeOpaque
// 
// call to describe the opaque part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DescribeOpaque()
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeTransparent
// 
// call to describe the transparent part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DescribeTransparent(double scale)
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::Render
// 
// creates a draw list for the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::Render()
{
	// only draw if enabled
	if (!IsEnabled())
	{
		return;
	}

	// see if we need to re-create the draw list
	if (m_nDrawList == -1)
	{
		// generate a new draw list name
		m_nDrawList = glGenLists(1);

		// check for error in creating the draw list
		ASSERT(m_nDrawList != 0);
		ASSERT(glIsList(m_nDrawList) == GL_TRUE);

		// check for other errors
		GLenum error = glGetError();
		ASSERT(error == GL_NO_ERROR);

		// create the list
		glNewList(m_nDrawList, GL_COMPILE);

		// now populate the draw list
		DescribeOpaque();

		// finish up the list
		glEndList();

		// make sure nothing bad happened
		error = glGetError();
		ASSERT(error == GL_NO_ERROR);
	}

	// now actually render the scene
	glCallList(m_nDrawList);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}
