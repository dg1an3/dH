// OpenGLCamera.cpp: implementation of the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpenGLCamera.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLCamera::COpenGLCamera()
	: targetPoint(CVector<3>(0.0, 0.0, 0.0)),
		direction(CVector<3>(0.0, 0.0, 1.0)),
		distance(100.0),
		rotation(0.0),
		angle(45.0),
		nearPlane(50.0),
		farPlane(150.0)
{
}

COpenGLCamera::~COpenGLCamera()
{
}

const CMatrix<4>& COpenGLCamera::GetObjectToCameraTransform() const
{
	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(distance.Get(), 
		CVector<3>(0.0, 0.0, 1.0));

	// form the rotation angles for the camera position
	CMatrix<4> mRotate = CreateRotate();

	// form the optiical axis rotation
}
