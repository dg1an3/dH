// SurfaceRenderer.h: interface for the CSurfaceRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SURFACERENDERER_H__0E2B2435_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_SURFACERENDERER_H__0E2B2435_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLRenderer.h"
#include "Surface.h"
#include "Beam.h"
#include "OpenGLTexture.h"	// Added by ClassView

class CSurfaceRenderer : public COpenGLRenderer
{
public:
	// Constructors/destructores
	CSurfaceRenderer(COpenGLView *pView);
	virtual ~CSurfaceRenderer();

	// Accessors for the surface to be rendered
	CSurface * GetSurface();
	void SetSurface(CSurface *m_pSurface);

	// Accessors for the beam to use for lightfield generation
	CBeam * GetLightFieldBeam();
	void SetLightFieldBeam(CBeam *pBeam);

	// Flag to indicate wire frame mode (project contours, not mesh)
	CValue< BOOL > isWireFrame;

	// Translation vector 
	static CVector<3> m_vXlate;

	// Rendering routines
	virtual void OnRenderScene();

	// Capture changes from the surface and/or lightfield beam
	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

protected:
	// Re-generates the lightfield texture, if necessary
	COpenGLTexture * GetLightfieldTexture();

private:
	// pointer to the surface data
	CSurface *m_pSurface;

	// pointer to the beam to use for light field generation
	CBeam * m_pBeam;

	// stores the texture for the lightfield
	COpenGLTexture *m_pLightfieldTexture;
};

#endif // !defined(AFX_SURFACERENDERER_H__0E2B2435_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
