// BeamRenderer.h: interface for the CBeamRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BEAMRENDERER_H__385F4714_E652_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_BEAMRENDERER_H__385F4714_E652_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Association.h>

#include <Beam.h>

#include <OpenGLRenderer.h>

class CBeamRenderer : public COpenGLRenderer
{
public:
	CBeamRenderer(COpenGLView *pView);
	virtual ~CBeamRenderer();

	// association to the beam
	CAssociation< CBeam > forBeam;

	CBeam *GetBeam();
	void SetBeam(CBeam *pBeam);

	CValue< BOOL > isCentralAxisEnabled;
	CValue< BOOL > isDivergenceSurfacesEnabled;
	CValue< BOOL > isGraticuleEnabled;

	virtual void OnRenderScene();

private:
	// private synced copy of the current beams B2P xform
	CAutoSyncValue< CBeam, CMatrix<4> > privBeamToPatientXform;

	// private synced copy of the machine projection
	CAutoSyncValue< CBeam, CMatrix<4> > privMachineProjection;
};

#endif // !defined(AFX_BEAMRENDERER_H__385F4714_E652_11D4_9E2F_00B0D0609AB0__INCLUDED_)
