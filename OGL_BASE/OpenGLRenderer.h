//////////////////////////////////////////////////////////////////////
// OpenGLRenderer.h: declaration of the COpenGLRenderer class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"
#include "Matrix.h"

// forward declaration of the COpenGLView class 
class COpenGLView;

//////////////////////////////////////////////////////////////////////
// class COpenGLRenderer
//
// represents a renderer that belongs to a COpenGLView window
//////////////////////////////////////////////////////////////////////
class COpenGLRenderer : public CObject
{
public:
	// constructor/destructor
	COpenGLRenderer(COpenGLView *pView);
	virtual ~COpenGLRenderer();

	// color for the renderer (use depends on objects being rendered)
	CValue< COLORREF > color;	

	// defines the modelview matrix for this renderer
	CValue< CMatrix<4> > modelviewMatrix;

	// turns on/off the renderer;
	CValue< BOOL > isEnabled;

	// triggers invalidation and re-rendering
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);

	// triggers invalidation (without full re-render)
	virtual void OnChangeNoRender(CObservableObject *pSource, void *pOldValue);

	// invalidates the 
	void Invalidate();

	// called to draw the scene
	virtual void DrawScene();

	// Renders the scene -- called by DrawScene to create the drawlist
	virtual void OnRenderScene();

protected:
	COpenGLView * m_pView;

private:
	int m_nDrawList;
};

#endif // !defined(AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
