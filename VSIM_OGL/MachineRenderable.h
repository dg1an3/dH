// MachineRenderable.h: interface for the CMachineRenderable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MACHINERENDERABLE_H)
#define MACHINERENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include <Value.h>

#include <Beam.h>
#include <TreatmentMachine.h>

#include <Renderable.h>

class CMachineRenderable : public CRenderable 
{
public:
	// constructor/destructor
	CMachineRenderable(CSceneView *pView);
	virtual ~CMachineRenderable();

	// association to the beam for treatment machine parameters
	CBeam *m_pBeam;

	// flag to indicate that the machine is to be rendered as a wireframe
	BOOL m_bWireFrame;

	// rendering routine
	virtual void DescribeOpaque();

	// re-loads the gantry angle
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);

private:
	double privGantryAngle;
};

#endif // !defined(MACHINERENDERABLE_H)
