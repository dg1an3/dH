// OpenGLRenderable.cpp: implementation of the COpenGLRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <gl/gl.h>

#include "OpenGLRenderable.h"
#include "OpenGLView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLRenderable::COpenGLRenderable(COpenGLView *pView)
	: m_pView(pView),
		m_nDrawList(-1),
		m_color(RGB(255, 255, 255)),
		m_bEnabled(TRUE)
{
}

COpenGLRenderable::~COpenGLRenderable()
{
}

// color for the renderer (use depends on objects being rendered)
COLORREF COpenGLRenderable::GetColor() const
{
	return m_color;
}

void COpenGLRenderable::SetColor(COLORREF color)
{
	// set the color
	m_color = color;

	// invalidate to redraw
	Invalidate();
}

// defines the modelview matrix for this renderer
const CMatrix<4>& COpenGLRenderable::GetModelviewMatrix() const
{
	return m_mModelview;
}

void COpenGLRenderable::SetModelviewMatrix(const CMatrix<4>& m)
{
	// set the modelview matrix
	m_mModelview = m;

	// and invalidate the view to redraw without re-rendering
	m_pView->Invalidate();
}


// turns on/off the renderer;
BOOL COpenGLRenderable::IsEnabled() const
{
	return m_bEnabled;
}

void COpenGLRenderable::SetEnabled(BOOL bEnabled)
{
	// set the enabled flag
	m_bEnabled = bEnabled;

	// and invalidate the view to redraw without re-rendering
	m_pView->Invalidate();
}


void COpenGLRenderable::Invalidate(CObservableEvent *pEvent, void *pValue)
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

void COpenGLRenderable::RenderOpaque()
{
}

void COpenGLRenderable::RenderTransparent(double scale)
{
}

void COpenGLRenderable::Render()
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
		GLenum error = glGetError();
		ASSERT(error == GL_NO_ERROR);

		// create the list
		glNewList(m_nDrawList, GL_COMPILE);

		// now populate the draw list
		RenderOpaque();

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
