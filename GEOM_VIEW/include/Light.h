//////////////////////////////////////////////////////////////////////
// Light.h: interface for the CLight class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(LIGHT_H)
#define LIGHT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector include file
#include <VectorD.h>

//////////////////////////////////////////////////////////////////////
// class CLight
// 
// represents a light illuminating a CSceneView
//////////////////////////////////////////////////////////////////////
class CLight : public CObject  
{
public:
	// constructor/destructor
	CLight();
	virtual ~CLight();

	// accessors for the position of the light
	const CVectorD<3>& GetPosition() const;
	void SetPosition(const CVectorD<3>& vPos);

	// diffuse color for the light
	COLORREF GetDiffuseColor() const;
	void SetDiffuseColor(COLORREF color);

	// call to set up the light in the current OpenGL rendering context
	void TurnOn(int nLightNum = 0);

private:
	// stores the position of the light
	CVectorD<3> m_vPosition;

	// diffuse color for the light
	COLORREF m_diffuseColor;
};

#endif // !defined(LIGHT_H)
