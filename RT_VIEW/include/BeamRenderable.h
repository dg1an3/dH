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

	// renders the beam
	virtual void DescribeOpaque();

	// description of items on the collimator plane
	void DescribeGraticule(double size = 0.5);
	void DescribeField();
	void DescribeBlocks();

	// description of divergence lines
	void DescribeCentralAxis();
	void DescribeFieldDivergenceLines();

	// description of divergence surfaces
	void DescribeBlockDivergenceSurfaces();
	void DescribeFieldDivergenceSurfaces();

	// event handler
	void OnBeamChanged(CObservableEvent *pEvent, void *pOldValue);

private:
	// holds the four corners of the field during description
	CVector<2> m_vMin;
	CVector<2> m_vMax;
	CVector<2> m_vMinXMaxY;
	CVector<2> m_vMaxXMinY;

	// flags to control rendering
	BOOL m_bCentralAxisEnabled;
	BOOL m_bGraticuleEnabled;
	BOOL m_bFieldDivergenceSurfacesEnabled;
	BOOL m_bBlockDivergenceSurfacesEnabled;
};

#endif // !defined(BEAMRENDERABLE_H)
