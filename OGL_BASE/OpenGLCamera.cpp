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
		distance(100.0),
		// direction(CVector<3>(0.0, 0.0, -1.0)),
		theta(0.0),
		phi(0.0),
		rollAngle(0.0),
		viewingAngle(45.0),
		aspectRatio(1.0),
		nearPlane(50.0),
		farPlane(150.0) // ,
		// projection(privProjection)
{
	// add the change listeners for the model-to-camera transform
	targetPoint.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	// direction.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	distance.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	rollAngle.AddObserver(this, (ChangeFunction) OnComputeModelXform);
	theta.AddObserver(this, (ChangeFunction) OnComputeModelXformAngles);
	phi.AddObserver(this, (ChangeFunction) OnComputeModelXformAngles);

	// add the change listeners for the camera direction and roll
	modelXform.AddObserver(this, (ChangeFunction) OnComputeCameraDir);

	// add the change listeners for the camera projection
	viewingAngle.AddObserver(this, (ChangeFunction) OnComputeProjection);
	aspectRatio.AddObserver(this, (ChangeFunction) OnComputeProjection);
	nearPlane.AddObserver(this, (ChangeFunction) OnComputeProjection);
	farPlane.AddObserver(this, (ChangeFunction) OnComputeProjection);

	// set up the total projection matrix
	projection.SyncTo(&(
		perspective * modelXform
	));

	// TEST
	phi.Set(PI / 2);
	theta.Set(PI);
}

COpenGLCamera::~COpenGLCamera()
{
}

void COpenGLCamera::SetFieldOfView(double maxObjectSize)
{
	farPlane.Set(nearPlane.Get() + maxObjectSize * 2.5f);
	distance.Set(nearPlane.Get() + maxObjectSize / 1.2f);
}

void COpenGLCamera::OnComputeModelXform(CObservableObject *pSource, void *pOldValue)
{
	// form the rotation angles for the camera direction
/*	double phi = acos(-direction.Get()[2]);
	double sin_phi = sin(phi);
	double theta = 0.0;
	if (sin_phi > EPS)
		theta = AngleFromSinCos(-direction.Get()[0] / sin_phi,
			direction.Get()[1] / sin_phi);

	TRACE2("Angles in COpenGLCamera: phi = %lf\t theta = %lf\n",
		phi, theta);

	// form the rotation matrix for the camera direction
	CMatrix<4> mRotateDir = CreateRotate(phi, CVector<3>(1.0, 0.0, 0.0))
		* CreateRotate(theta, CVector<3>(0.0, 0.0, 1.0));
	TRACE_MATRIX4("mRotateDir in COpenGLCamera = ", mRotateDir);

	// form the camera roll rotation matrix
	CMatrix<4> mRotateRoll = CreateRotate(rollAngle.Get(), 
		CVector<3>(0.0, 0.0, 1.0));

	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(distance.Get(), 
		CVector<3>(0.0, 0.0, -1.0));

	// and set the total camera transformation to all three matrices
	modelXform.Set(mTranslate * mRotateRoll * 
		mRotateDir);

	// notify listeners that the camera has changed
	FireChange(); */
}

void COpenGLCamera::OnComputeModelXformAngles(CObservableObject *pSource, void *pOldValue)
{
	// form the rotation matrix for the camera direction
	CMatrix<4> mRotateDir = CreateRotate(phi.Get(), CVector<3>(1.0, 0.0, 0.0))
		* CreateRotate(theta.Get(), CVector<3>(0.0, 0.0, 1.0));
	TRACE_MATRIX4("mRotateDir in COpenGLCamera = ", mRotateDir);

	// form the camera roll rotation matrix
	CMatrix<4> mRotateRoll = CreateRotate(rollAngle.Get(), 
		CVector<3>(0.0, 0.0, 1.0));

	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(distance.Get(), 
		CVector<3>(0.0, 0.0, -1.0));

	// and set the total camera transformation to all three matrices
	modelXform.Set(mTranslate * mRotateRoll * 
		mRotateDir);

	// notify listeners that the camera has changed
	FireChange();
}

void COpenGLCamera::OnComputeCameraDir(CObservableObject *pSource, void *pOldValue)
{
	CMatrix<4> mXform = modelXform.Get();
	TRACE_MATRIX4("mXform in COpenGLCamera = ", mXform);

	// form the rotation angles for the camera direction
	double new_phi = acos(mXform[2][2]);
	double sin_phi = sin(phi.Get());
	double new_theta = 0.0;
	double new_rollAngle = 0.0;
	if (sin_phi > EPS)
	{
		if (mXform[2][0] != 0.0 && mXform[2][1] != 0.0)
			new_theta = AngleFromSinCos(
				mXform[2][0] / sin_phi,
				mXform[2][1] / sin_phi);
		if (mXform[0][2] != 0.0 && mXform[1][2] != 0.0)
			new_rollAngle = AngleFromSinCos(
				mXform[0][2] / sin_phi,
				-mXform[1][2] / sin_phi);
	}
	TRACE2("Angles in COpenGLCamera: phi = %lf\t theta = %lf\n",
		new_phi, new_theta);

	// compare to the existing, only replace if different
	if (!IS_APPROX_EQUAL(new_phi, phi.Get()))
		phi.Set(new_phi + 0.1);

	if (!IS_APPROX_EQUAL(new_theta, theta.Get()))
		theta.Set(new_theta);

	if (!IS_APPROX_EQUAL(new_rollAngle, rollAngle.Get()))
		rollAngle.Set(new_rollAngle);

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
	perspective.Set(mPersp);

	// notify listeners that the camera has changed
	FireChange();
}
