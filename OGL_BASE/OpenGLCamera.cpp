// OpenGLCamera.cpp: implementation of the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <ScalarFunction.h>

#include "OpenGLCamera.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLCamera::COpenGLCamera()
	: targetPoint(CVector<3>(0.0, 0.0, 0.0)),
		direction(CVector<3>(0.0, 0.0, -1.0)),
		distance(100.0),
		rollAngle(0.0),
		viewingAngle(45.0),
		aspectRatio(1.0),
		nearPlane(50.0),
		farPlane(150.0)
{
	// add the change listeners for the model-to-camera transform
	targetPoint.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	direction.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	distance.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	rollAngle.AddObserver(this, (ChangeFunction) OnComputeModelXform);

	// add the change listeners for the camera projection
	viewingAngle.AddObserver(this, (ChangeFunction) OnComputeProjection);
	aspectRatio.AddObserver(this, (ChangeFunction) OnComputeProjection);
	nearPlane.AddObserver(this, (ChangeFunction) OnComputeProjection);
	farPlane.AddObserver(this, (ChangeFunction) OnComputeProjection);
}

COpenGLCamera::~COpenGLCamera()
{
}

void COpenGLCamera::OnComputeModelXform(CObservableObject *pSource, void *pOldValue)
{
	// form the rotation angles for the camera direction
	double phi = acos(-direction.Get()[2]);
	double sin_phi = sin(phi);
	double theta = 0.0;
	if (sin_phi > EPS)
		theta = AngleFromSinCos(direction.Get()[0] / sin_phi,
			-direction.Get()[1] / sin_phi);

	// form the rotation matrix for the camera direction
	CMatrix<4> mRotateDir = CreateRotate(phi, CVector<3>(1.0, 0.0, 0.0))
		* CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0));

	// form the camera roll rotation matrix
	CMatrix<4> mRotateRoll = CreateRotate(0.0, CVector<3>(0.0, 0.0, 1.0));

	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(distance.Get(), 
		CVector<3>(0.0, 0.0, -1.0));

	// and set the total camera transformation to all three matrices
	modelXform.Set(mTranslate); // * mRotateRoll * mRotateDir);

	// notify listeners that the camera has changed
	FireChange();
}

void COpenGLCamera::OnComputeProjection(CObservableObject *pSource, void *pOldValue)
{
	// compute the top edge of the viewing frustum from the nearPlane 
	//		and viewingAngle
	double top = nearPlane.Get() * tan(viewingAngle.Get() * PI / 360.0);

	// compute the right edge from the top edge and the aspect ratio
	double right = top * aspectRatio.Get();

	// now populate the perspective projection matrix
	CMatrix<4> mPersp;
	mPersp[0][0] = 2.0 * nearPlane.Get() 
		/ (2.0 * right);
	mPersp[1][1] = 2.0 * nearPlane.Get()
		/ (2.0 * top);
	mPersp[2][2] = -(farPlane.Get() + nearPlane.Get())
		/ (farPlane.Get() - nearPlane.Get());
	mPersp[2][3] = -2.0 * farPlane.Get() * nearPlane.Get()
		/ (farPlane.Get() - nearPlane.Get());
	mPersp[3][2] = -1.0;
	mPersp[3][3] = 0.0;

	// and set the projection
	projection.Set(mPersp);

	// notify listeners that the camera has changed
	FireChange();
}
