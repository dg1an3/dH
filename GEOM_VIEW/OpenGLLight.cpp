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
	: diffuseColor(RGB(160, 160, 160))
{

}

COpenGLLight::~COpenGLLight()
{

}

void COpenGLLight::TurnOn(int nLightNum)
{
	// enable lighting
	glEnable(GL_LIGHTING);

	// set the light's direction
	glLightDirection(arrLightID[nLightNum], position.Get());

	// set the light's diffuse color
	GLfloat diffuse[4];
	diffuse[0] = (float) GetRValue(diffuseColor.Get()) / 255.0f;
	diffuse[1] = (float) GetGValue(diffuseColor.Get()) / 255.0f;
	diffuse[2] = (float) GetBValue(diffuseColor.Get()) / 255.0f;
	diffuse[3] = 1.0f;
	glLightfv(arrLightID[nLightNum], GL_DIFFUSE, diffuse);

	// enable this light
	glEnable(arrLightID[nLightNum]);
}
