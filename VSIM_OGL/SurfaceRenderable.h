// SurfaceRenderable.h: interface for the CSurfaceRenderable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SURFACERENDERABLE_H)
#define SURFACERENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Renderable.h"
#include "Surface.h"
#include "Beam.h"
#include "Texture.h"	// Added by ClassView

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

	// Flag to indicate wire frame mode (project contours, not mesh)
	BOOL m_bWireFrame;

	// Flag to indicate wire frame mode (project contours, not mesh)
	BOOL m_bColorWash;

	// Flag to indicate the bounding surfaces are to be rendered
	BOOL m_bShowBoundsSurface;

	// Couch rotation angle
	double m_couchAngle;

	// Translation vector 
	CVector<3> m_vTableOffset;

	// Rendering routines
	virtual void DescribeOpaque();

	// Capture changes from the surface and/or lightfield beam
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

	// Capture changes from the couch angle or table offset
	void OnPositionChange(CObservableObject *pFromObject, void *pOldValue);

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
};

#endif // !defined(SURFACERENDERABLE_H)
