// MachineRenderer.h: interface for the CMachineRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MACHINERENDERER_H__F8B84E0C_8074_11D5_ABA5_00B0D0AB90D6__INCLUDED_)
#define AFX_MACHINERENDERER_H__F8B84E0C_8074_11D5_ABA5_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Association.h>

#include <Beam.h>
#include <TreatmentMachine.h>

#include <OpenGLRenderer.h>

class CMachineRenderer : public COpenGLRenderer  
{
public:
	// constructor/destructor
	CMachineRenderer(COpenGLView *pView);
	virtual ~CMachineRenderer();

	// association to the beam for treatment machine parameters
	CAssociation< CBeam > forBeam;

	// rendering routine
	virtual void OnRenderScene();

	// re-loads the gantry angle
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);

private:
	CValue<double> privGantryAngle;
};

#endif // !defined(AFX_MACHINERENDERER_H__F8B84E0C_8074_11D5_ABA5_00B0D0AB90D6__INCLUDED_)
