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
#include <Beam.h>

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
	CSurfaceRenderable(CSceneView *pView);
	virtual ~CSurfaceRenderable();

	// Accessors for the surface to be rendered
	CSurface * GetSurface();
	void SetSurface(CSurface *m_pSurface);

	// Accessors for the beam to use for lightfield generation
	CBeam * GetLightFieldBeam();
	void SetLightFieldBeam(CBeam *pBeam);

	// Rendering routines
	virtual void DescribeOpaque();

	// event handler for beam changes
	void OnBeamChanged(CObservableEvent *pEvent, void *pOldValue);

protected:
	// Re-generates the lightfield texture, if necessary
	CTexture * GetLightfieldTexture();

private:
	// pointer to the surface data
	CSurface *m_pSurface;

	// pointer to the beam to use for light field generation
	CBeam * m_pBeam;

	// stores the texture for the lightfield
	CTexture *m_pLightfieldTexture;

	// stores the texture for the end of the mesh
	CTexture *m_pEndTexture;

	// Flag to indicate wire frame mode (project contours, not mesh)
	BOOL m_bWireFrame;

	// Flag to indicate wire frame mode (project contours, not mesh)
	BOOL m_bColorWash;

	// Flag to indicate the bounding surfaces are to be rendered
	BOOL m_bShowBoundsSurface;
};

#endif // !defined(SURFACERENDERABLE_H)
