//////////////////////////////////////////////////////////////////////
// BeamRenderable.h: declaration of the CBeamRenderable class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(BEAMRENDERABLE_H)
#define BEAMRENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// the beam class
#include <Beam.h>

// base class
#include <Renderable.h>


//////////////////////////////////////////////////////////////////////
// class CBeamRenderable
//
// describes the beam in a CSceneView
//////////////////////////////////////////////////////////////////////
class CBeamRenderable : public CRenderable
{
public:
	// construction/destruction
	CBeamRenderable();
	virtual ~CBeamRenderable();

	// allow for dynamic creation
	DECLARE_DYNCREATE(CBeamRenderable)

	// accessors for the beam to be rendered
	CBeam *GetBeam();
	void SetBeam(CBeam *pBeam);

	// over-ride to the set the object to a beam
	virtual void SetObject(CObject *pObject);

	// renders the beam with lines
	virtual void DrawOpaque(CRenderContext *pRC);

	// description of items on the collimator plane
	void DrawGraticule(CRenderContext *pRC, double size = 0.5);
	void DrawField(CRenderContext *pRC);
	void DrawBlocks(CRenderContext *pRC);

	// description of divergence lines
	void DrawCentralAxis(CRenderContext *pRC);
	void DrawFieldDivergenceLines(CRenderContext *pRC);

	// renders the beam surfaces
	virtual void DrawTransparent(CRenderContext *pRC);

	// description of divergence surfaces
	void DrawBlockDivergenceSurfaces(CRenderContext *pRC);
	void DrawFieldDivergenceSurfaces(CRenderContext *pRC);

	// event handler
	void OnBeamChanged(CObservableEvent *pEvent, void *pOldValue);

private:
	// holds the four corners of the field during description
	CVectorD<2> m_vMin;
	CVectorD<2> m_vMax;
	CVectorD<2> m_vMinXMaxY;
	CVectorD<2> m_vMaxXMinY;

	// flags to control rendering
	BOOL m_bCentralAxisEnabled;
	BOOL m_bGraticuleEnabled;
	BOOL m_bFieldDivergenceSurfacesEnabled;
	BOOL m_bBlockDivergenceSurfacesEnabled;
};

#endif // !defined(BEAMRENDERABLE_H)
