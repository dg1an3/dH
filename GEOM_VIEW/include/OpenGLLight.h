// OpenGLLight.h: interface for the COpenGLLight class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
#define AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>

class COpenGLLight : public CObject  
{
public:
	// constructor/destructor
	COpenGLLight();
	virtual ~COpenGLLight();

public:
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

#endif // !defined(AFX_OPENGLLIGHT_H__61F32F74_8AC7_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
