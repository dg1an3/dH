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

#include <d3d8.h>

// vector include file
#include <VectorD.h>

//////////////////////////////////////////////////////////////////////
// class CLight
// 
// represents a light illuminating a CSceneView
//////////////////////////////////////////////////////////////////////
class CLight : public CObject, public D3DLIGHT8
{
public:
	// constructor/destructor
	CLight();
	virtual ~CLight();

	// accessors for the position of the light
	CVectorD<3> GetPosition() const;
	void SetPosition(const CVectorD<3>& vPos);

	// diffuse color for the light
	COLORREF GetDiffuseColor() const;
	void SetDiffuseColor(COLORREF color);
};

#endif // !defined(LIGHT_H)
