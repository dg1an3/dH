//////////////////////////////////////////////////////////////////////
// MachineRenderable.h: declaration of the CMachineRenderable class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(MACHINERENDERABLE_H)
#define MACHINERENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// the beam class
#include <Beam.h>

// base class
#include <Renderable.h>


//////////////////////////////////////////////////////////////////////
// class CMachineRenderable
//
// describes the treatment machine in a CSceneView
//////////////////////////////////////////////////////////////////////
class CMachineRenderable : public CRenderable 
{
public:
	// constructor/destructor
	CMachineRenderable();
	virtual ~CMachineRenderable();

	// beam for treatment machine parameters
	CBeam *GetBeam();
	void SetBeam(CBeam *pBeam);

	// over-ride to register change listeners, etc.
	virtual void SetObject(CObject *pObject);

	// rendering routine
	virtual void DescribeOpaque();

	// rendering the parts of the machine
	void DescribeTable();
	void DescribeGantry(double axisToCollim);
	void DescribeCollimator(double axisToCollim);

private:
	// flag to indicate that the machine is to be rendered as a wireframe
	BOOL m_bWireFrame;
};

#endif // !defined(MACHINERENDERABLE_H)
