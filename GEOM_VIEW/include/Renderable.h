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
	CRenderable();
	virtual ~CRenderable();

	// allow for dynamic creation
	DECLARE_DYNCREATE(CRenderable)

	// sets the object being rendered
	CObject *GetObject();
	virtual void SetObject(CObject *pObject);

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

	// helper function to set up the rendering context
	void SetupRenderingContext();

	// describes the opaque part of the object
	virtual void DescribeOpaque();

	// describes the part controlled by the alpha parameter
	virtual void DescribeAlpha();

	// draw list management functions
	void DescribeOpaqueDrawList();
	void DescribeAlphaDrawList();

private:
	// the object being described
	CObject *m_pObject;

	// the rendering color
	COLORREF m_color;	

	// the rendering transparency
	double m_alpha;	

	// the centroid of the renderable; used to determine rendering order
	CVector<3> m_vCentroid;

	// the modelview matrix for the object
	CMatrix<4> m_mModelview;

	// the enabled flag
	BOOL m_bEnabled;

	// the draw list for opaque draw compilation
	int m_nDrawListOpaque;

	// the draw list for alpha draw compilation
	int m_nDrawListAlpha;
};

#endif // !defined(RENDERABLE_H)
