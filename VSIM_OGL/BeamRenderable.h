// BeamRenderable.h: interface for the CBeamRenderable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(BEAMRENDERABLE_H)
#define BEAMRENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Beam.h>

#include <Renderable.h>

class CBeamRenderable : public CRenderable
{
public:
	CBeamRenderable(CSceneView *pView);
	virtual ~CBeamRenderable();

	CBeam *GetBeam();
	void SetBeam(CBeam *pBeam);

	void SetREVBeam();

	BOOL m_bCentralAxisEnabled;
	BOOL m_bDivergenceSurfacesEnabled;
	BOOL m_bGraticuleEnabled;

	virtual void DescribeOpaque();

private:
	// association to the beam
	CBeam *m_pBeam;
};

#endif // !defined(BEAMRENDERABLE_H)
