// OpenGLLight.h: interface for the COpenGLLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
#define AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Observer.h>
#include <Value.h>

#include <Vector.h>

class COpenGLLight : public CObservableObject  
{
public:
	COpenGLLight();
	virtual ~COpenGLLight();

public:
	// stores the position of the light
	CValue< CVector<3> > position;

	// diffuse color for the light
	CValue< COLORREF > diffuseColor;

	// call to set up the light in the current OpenGL rendering context
	void TurnOn(int nLightNum = 0);
};

#endif // !defined(AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
