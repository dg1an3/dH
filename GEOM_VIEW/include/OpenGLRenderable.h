//////////////////////////////////////////////////////////////////////
// OpenGLRenderable.h: declaration of the COpenGLRenderable class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(OPENGLRENDERABLE_H)
#define OPENGLRENDERABLE_H

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
// class COpenGLRenderable
//
// represents a renderer that belongs to a COpenGLView window
//////////////////////////////////////////////////////////////////////
class COpenGLRenderable : public CObject
{
public:
	// constructor/destructor
	COpenGLRenderable(COpenGLView *pView);
	virtual ~COpenGLRenderable();

	// color for the renderer (use depends on objects being rendered)
	COLORREF GetColor() const;
	void SetColor(COLORREF);

	// accessors for the renderable's centroid (used for sorting)
	const CVector<3>& GetCentroid() const;
	void SetCentroid(const CVector<3>& vCentroid);

	// defines the modelview matrix for this renderer
	const CMatrix<4>& GetModelviewMatrix() const;
	void SetModelviewMatrix(const CMatrix<4>& m);

	// turns on/off the renderer;
	BOOL IsEnabled() const;
	void SetEnabled(BOOL bEnabled = TRUE);

	// turns on/off the renderer;
	BOOL IsHighlighted() const;
	void SetHighlighted(BOOL bHighlighted = TRUE);

	// invalidates the object -- is a CObservableEvent compatible
	//		member function (ListenerFunction)
	void Invalidate(CObservableEvent *pEvent = NULL, void *pValue = NULL);

protected:
	// view class is a friend
	friend COpenGLView;

	// the view for this class
	COpenGLView * m_pView;

	// Renders the scene -- called by DrawScene to create the drawlist
	virtual void RenderOpaque();
	virtual void RenderTransparent(double scale = 0.5);

	// called to draw the scene -- don't over-ride this unless the
	//		drawlist logic needs to be overridden
	virtual void Render();

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

#endif // !defined(OPENGLRENDERABLE_H)
