//////////////////////////////////////////////////////////////////////
// Renderable.h: declaration of the CRenderable class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(RENDERABLE_H)
#define RENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// matrix include file
#include <Matrix.h>

// observable events
#include <Observer.h>

// forward declaration of the CSceneView class 
class CSceneView;

//////////////////////////////////////////////////////////////////////
// class CRenderable
//
// represents a renderer that belongs to a CSceneView window
//////////////////////////////////////////////////////////////////////
class CRenderable : public CObject
{
public:
	// constructor/destructor
	CRenderable(CSceneView *pView);
	virtual ~CRenderable();

	// color for the renderer (use depends on objects being rendered)
	COLORREF GetColor() const;
	void SetColor(COLORREF);

	// alpha transparency for the transparent part of the object
	double GetAlpha() const;
	void SetAlpha(double alpha);

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
	friend CSceneView;

	// the view for this class
	CSceneView *m_pView;

	// Renders the scene -- called by DrawScene to create the drawlist
	virtual void DescribeOpaque();
	virtual void DescribeTransparent(double scale = 0.5);

	// called to draw the scene -- don't over-ride this unless the
	//		drawlist logic needs to be overridden
	virtual void Render();

private:
	// the rendering color
	COLORREF m_color;	

	// the rendering transparency
	COLORREF m_alpha;	

	// the modelview matrix for the object
	CMatrix<4> m_mModelview;

	// the enabled flag
	BOOL m_bEnabled;

	// the draw list for draw compilation
	int m_nDrawList;
};

#endif // !defined(RENDERABLE_H)
