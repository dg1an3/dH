// OpenGLLight.h: interface for the COpenGLLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(OPENGLLIGHT_H)
#define OPENGLLIGHT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector include file
#include <Vector.h>

//////////////////////////////////////////////////////////////////////
// class COpenGLLight
// 
// a light that lights a scene
//////////////////////////////////////////////////////////////////////
class COpenGLLight : public CObject  
{
public:
	// constructor/destructor
	COpenGLLight();
	virtual ~COpenGLLight();

	// accessors for the position of the light
	const CVector<3>& GetPosition() const;
	void SetPosition(const CVector<3>& vPos);

	// diffuse color for the light
	COLORREF GetDiffuseColor() const;
	void SetDiffuseColor(COLORREF color);

	// call to set up the light in the current OpenGL rendering context
	void TurnOn(int nLightNum = 0);

private:
	// stores the position of the light
	CVector<3> m_vPosition;

	// diffuse color for the light
	COLORREF m_diffuseColor;
};

#endif // !defined(OPENGLLIGHT_H)
