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

// matrix include file
#include <Matrix.h>

// observable events
#include <Observer.h>

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
	COLORREF GetColor() const;
	void SetColor(COLORREF);

	// defines the modelview matrix for this renderer
	const CMatrix<4>& GetModelviewMatrix() const;
	void SetModelviewMatrix(const CMatrix<4>& m);

	// turns on/off the renderer;
	BOOL IsEnabled() const;
	void SetEnabled(BOOL bEnabled = TRUE);

	// Renders the scene -- called by DrawScene to create the drawlist
	virtual void OnRenderScene();

	// called to draw the scene -- don't over-ride this unless the
	//		drawlist logic needs to be overridden
	virtual void DrawScene();

	// invalidates the object -- is a CObservableEvent compatible
	//		member function (ListenerFunction)
	void Invalidate(CObservableEvent *pEvent = NULL, void *pValue = NULL);

protected:
	// the view for this class
	COpenGLView * m_pView;

private:
	// the rendering color
	COLORREF m_color;	

	// the modelview matrix for the object
	CMatrix<4> m_mModelview;

	// the enabled flag
	BOOL m_bEnabled;

	// the draw list for draw compilation
	int m_nDrawList;
};

#endif // !defined(AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
