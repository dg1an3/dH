// OpenGLLight.cpp: implementation of the COpenGLLight class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpenGLLight.h"

#include "glMatrixVector.h"

static GLenum arrLightID[] =
{
	GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
	GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7,
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLLight::COpenGLLight()
	: m_diffuseColor(RGB(160, 160, 160))
{
}

COpenGLLight::~COpenGLLight()
{
}

// accessors for the position of the light
const CVector<3>& COpenGLLight::GetPosition() const
{
	return m_vPosition;
}

void COpenGLLight::SetPosition(const CVector<3>& vPos)
{
	m_vPosition = vPos;
}

// diffuse color for the light
COLORREF COpenGLLight::GetDiffuseColor() const
{
	return m_diffuseColor;
}

void COpenGLLight::SetDiffuseColor(COLORREF color)
{
	m_diffuseColor = color;
}

void COpenGLLight::TurnOn(int nLightNum)
{
	// enable lighting
	glEnable(GL_LIGHTING);

	// set the light's direction
	glLightDirection(arrLightID[nLightNum], GetPosition());

	// set the light's diffuse color
	GLfloat diffuse[4];
	diffuse[0] = (float) GetRValue(GetDiffuseColor()) / 255.0f;
	diffuse[1] = (float) GetGValue(GetDiffuseColor()) / 255.0f;
	diffuse[2] = (float) GetBValue(GetDiffuseColor()) / 255.0f;
	diffuse[3] = 1.0f;
	glLightfv(arrLightID[nLightNum], GL_DIFFUSE, diffuse);

	// enable this light
	glEnable(arrLightID[nLightNum]);
}
