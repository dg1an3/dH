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

// TODO: give generic Texture mapping facility
//			over-ride ^^^ for light field display

class CSurfaceRenderer : public COpenGLRenderer // , public CObserver
{
public:
	CSurfaceRenderer(COpenGLView *pView);
	virtual ~CSurfaceRenderer();

	CSurface * GetSurface();
	void SetSurface(CSurface *m_pSurface);

	CBeam * GetLightFieldBeam();
	void SetLightFieldBeam(CBeam *pBeam);

	virtual void OnRenderScene();

	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

	CValue< BOOL > isWireFrame;

	static CVector<3> m_vXlate;

private:
	// pointer to the surface data
	CSurface *m_pSurface;

	// pointer to the beam to use for light field generation
	CBeam * m_pBeam;

	// stores the texture for the lightfield
	COpenGLTexture *m_pLightfieldTexture;
protected:
	COpenGLTexture * GetLightfieldTexture();
};

#endif // !defined(AFX_SURFACERENDERER_H__0E2B2435_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
