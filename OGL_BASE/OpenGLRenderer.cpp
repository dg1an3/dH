// OpenGLRenderer.cpp: implementation of the COpenGLRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "OpenGLRenderer.h"
#include "OpenGLView.h"

#include "gl/gl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLRenderer::COpenGLRenderer(COpenGLView *pView)
	: m_pView(pView),
		m_nDrawList(-1),
		myColor(RGB(255, 255, 255)),
		isEnabled(TRUE)
{
	myColor.AddObserver(this);
	myModelviewMatrix.AddObserver(this);
	isEnabled.AddObserver(this);
}

COpenGLRenderer::~COpenGLRenderer()
{
}

void COpenGLRenderer::Invalidate()
{
	if (m_nDrawList != -1)
		glDeleteLists(m_nDrawList, 1);

	m_nDrawList = -1;

	m_pView->Invalidate();
}

void COpenGLRenderer::OnChange(CObservable *pSource)
{
	Invalidate();
}

void COpenGLRenderer::OnRenderScene()
{
}

void COpenGLRenderer::DrawScene()
{
	if (!isEnabled.Get())
		return;

	// see if we need to re-create the draw list
	if (m_nDrawList == -1)
	{
		// generate a new draw list name
		m_nDrawList = glGenLists(1);

		// check for error in creating the draw list
		ASSERT(m_nDrawList != 0);

		// create the list
		glNewList(m_nDrawList, GL_COMPILE);

		// now populate the draw list
		OnRenderScene();

		// finish up the list
		glEndList();

		// make sure nothing bad happened
		GLenum error = glGetError();
		ASSERT(error == GL_NO_ERROR);
	}

	// now actually render the scene
	glCallList(m_nDrawList);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}
