//////////////////////////////////////////////////////////////////////
// SurfaceRenderable.h: declaration of the CSurfaceRenderable class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(SURFACERENDERABLE_H)
#define SURFACERENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// rendered objects
#include <Surface.h>

// texture for light patch
#include <Texture.h>

// base class
#include <Renderable.h>


//////////////////////////////////////////////////////////////////////
// class CSurfaceRenderable
//
// describes the surface in a CSceneView
//////////////////////////////////////////////////////////////////////
class CSurfaceRenderable : public CRenderable
{
public:
	// Constructors/destructores
	CSurfaceRenderable();
	virtual ~CSurfaceRenderable();

	// allow for dynamic creation
	DECLARE_DYNCREATE(CSurfaceRenderable)

	// Accessors for the surface to be rendered
	CSurface * GetSurface();
	void SetSurface(CSurface *m_pSurface);

	// accessors for the surface texture
	CTexture *GetTexture();
	void SetTexture(CTexture *pTexture);

	// over-ride to register change listeners, etc.
	virtual void SetObject(CObject *pObject);

	// computes the nearest and furthest distances to the
	//	given point
	virtual double GetNearestDistance(const CVectorD<3>& vPoint);
	virtual double GetFurthestDistance(const CVectorD<3>& vPoint);

	// opaque rendering routines
	virtual void DrawOpaque(CRenderContext *pRC);

	// opaque rendering helper functions
	void DrawWireframe(CRenderContext *pRC);

	// alpha-blended rendering
	virtual void DrawTransparent(CRenderContext *pRC);

	// alpha-blended helper functions
	void DrawSurface(CRenderContext *pRC);
	void DrawBoundsSurface(CRenderContext *pRC);

private:
	// stores the texture for the surface
	CTexture *m_pTexture;

	// Flag to indicate wire frame mode (project contours, not mesh)
	BOOL m_bWireFrame;

	// Flag to indicate the bounding surfaces are to be rendered
	BOOL m_bShowBoundsSurface;
};

#endif // !defined(SURFACERENDERABLE_H)
