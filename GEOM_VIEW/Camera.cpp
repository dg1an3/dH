//////////////////////////////////////////////////////////////////////
// Camera.cpp: implementation of the CCamera class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// floating point utilities
#include <float.h>

// math utilities
#include <MathUtil.h>

// class interface
#include "Camera.h"

// include the Matrix inline routines
#include <MatrixBase.inl>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CCamera::CCamera
// 
// constructs a camera object
//////////////////////////////////////////////////////////////////////
CCamera::CCamera()
#pragma warning(disable: 4355)
	: m_eventChange(this),
#pragma warning(default: 4355)
		m_vDirection(CVectorD<3>(0.0, 0.0, -1.0)),
		m_distance(100.0),
		m_vUpDirection(CVectorD<3>(0.0, 1.0, 0.0)),
		m_viewingAngle(45.0),
		m_aspectRatio(1.0),
		m_nearPlane(50.0),
		m_farPlane(150.0)
{
	// set up the projection
	RecalcProjection();

	// and model xform matrices
	RecalcCameraToModel();
}


//////////////////////////////////////////////////////////////////////
// CCamera::~CCamera
// 
// destroys the camera object
//////////////////////////////////////////////////////////////////////
CCamera::~CCamera()
{
}

// camera direction vector
const CVectorD<3>& CCamera::GetTarget() const
{
	return m_vTargetPoint;
}

void CCamera::SetTarget(const CVectorD<3>& vTarget)
{
	m_vTargetPoint = vTarget;

	// recalculate the model transform
	RecalcCameraToModel();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

// camera direction vector
const CVectorD<3>& CCamera::GetDirection() const
{
	return m_vDirection;
}

void CCamera::SetDirection(const CVectorD<3>& vDir)
{
	m_vDirection = vDir;
	m_vDirection.Normalize();

	// recalculate the model transform
	RecalcCameraToModel();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}


//////////////////////////////////////////////////////////////////////
// CCamera::GetDistance
// 
// returns the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
REAL CCamera::GetDistance() const 
{ 
	return m_distance;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void CCamera::SetDistance(REAL dist) 
{ 
	// set the distance from the camera to the target point
	m_distance = dist;

	// recalculate the model transform
	RecalcCameraToModel();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

// sets the "up" direction for the camera
const CVectorD<3>& CCamera::GetUpDirection() const
{
	return m_vUpDirection;
}

void CCamera::SetUpDirection(const CVectorD<3>& vDir)
{
	// set the distance from the camera to the target point
	m_vUpDirection = vDir;
	m_vUpDirection.Normalize();

	// recalculate the model transform
	RecalcCameraToModel();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}


//////////////////////////////////////////////////////////////////////
// CCamera::GetXform
// 
// returns matrix representing the transform from the model space to 
//		the camera space
//////////////////////////////////////////////////////////////////////
const CMatrixD<4>& CCamera::GetXform() const 
{ 
	return m_mXform;
}


//////////////////////////////////////////////////////////////////////
// CCamera::GetViewingAngle
// 
// the viewing angle for the camera in radians 
//		(0 for orthographic camera)
//////////////////////////////////////////////////////////////////////
double CCamera::GetViewingAngle() const 
{ 
	return m_viewingAngle;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetViewingAngle
// 
// the viewing angle for the camera in radians 
//		(0 for orthographic camera)
//////////////////////////////////////////////////////////////////////
void CCamera::SetViewingAngle(double angle) 
{ 
	m_viewingAngle = angle;

	// and recalculate the projection
	RecalcProjection();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetAspectRatio
// 
// the aspect ratio is usually set to the aspect ratio of the viewport
//////////////////////////////////////////////////////////////////////
double CCamera::GetAspectRatio() const 
{ 
	return m_aspectRatio;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetAspectRatio
// 
// the aspect ratio is usually set to the aspect ratio of the viewport
//////////////////////////////////////////////////////////////////////
void CCamera::SetAspectRatio(double aspectRatio) 
{ 
	// make sure a valid aspect ratio is passed
	ASSERT(!_isnan(aspectRatio));

	m_aspectRatio = aspectRatio;

	// and recalculate the projection
	RecalcProjection();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetNearPlane
// 
// the clipping planes, distance from focal point
//////////////////////////////////////////////////////////////////////
double CCamera::GetNearPlane() const 
{ 
	return m_nearPlane;
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetFarPlane
// 
// the clipping planes, distance from focal point
//////////////////////////////////////////////////////////////////////
double CCamera::GetFarPlane() const 
{ 
	return m_farPlane;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetClippingPlanes
// 
// the clipping planes, distance from focal point
//////////////////////////////////////////////////////////////////////
void CCamera::SetClippingPlanes(double nearPlane, double farPlane) 
{ 
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;

	// and recalculate the projection
	RecalcProjection();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetFieldOfView
// 
// sets the field of view based on the maximum object size to be 
//		viewed.
//////////////////////////////////////////////////////////////////////
void CCamera::SetFieldOfView(double maxObjectSize)
{
	SetClippingPlanes(GetNearPlane(), 
			GetNearPlane() + maxObjectSize * 2.5f);
	SetDistance(GetNearPlane() + maxObjectSize / 1.2f);
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetPerspective
// 
// returns the matrix representing the camera perspective projection
//////////////////////////////////////////////////////////////////////
const CMatrixD<4>& CCamera::GetProjection() const
{
	return m_mProjection;
}

//////////////////////////////////////////////////////////////////////
// CCamera::RecalcXform
// 
// recalculates the model transform
//////////////////////////////////////////////////////////////////////
void CCamera::RecalcCameraToModel() const
{
	// form the camera coordinate system basis
	CMatrixD<3> mCameraBasis;
	mCameraBasis[2] = GetTarget() - (GetDistance() * GetDirection());
	mCameraBasis[2].Normalize();

	mCameraBasis[0] = Cross(mCameraBasis[2], GetUpDirection());
	mCameraBasis[0].Normalize();

	mCameraBasis[1] = Cross(mCameraBasis[0], mCameraBasis[2]);
	mCameraBasis[1].Normalize();

	mCameraBasis.Transpose();

	CVectorD<3> vTranslate = 
		((REAL) -1.0) * mCameraBasis * (GetDistance() * GetDirection());

	m_mXform = CMatrixD<4>(mCameraBasis);
	m_mXform[3][0] = vTranslate[0];
	m_mXform[3][1] = vTranslate[1];
	m_mXform[3][2] = vTranslate[2];
	m_mXform[3][3] = 1.0;
}

//////////////////////////////////////////////////////////////////////
// CCamera::RecalcProjection
// 
// recalculates the projection (perspective and xform)
//////////////////////////////////////////////////////////////////////
void CCamera::RecalcProjection() const
{
	m_mProjection.SetIdentity();

	double h = 1.0 / tan(GetViewingAngle() / 2.0);
	double w = h / GetAspectRatio();

	m_mProjection[0][0] = w;
	m_mProjection[1][1] = h;

	m_mProjection[2][2] = GetFarPlane()
		/ (GetFarPlane() - GetNearPlane());
	m_mProjection[3][2] = - GetFarPlane() * GetNearPlane()
		/ (GetFarPlane() - GetNearPlane());

	m_mProjection[2][3] = 1.0;
	m_mProjection[3][3] = 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// CCamera diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////
// CCamera::AssertValid
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
void CCamera::AssertValid() const
{
	CObject::AssertValid();

	// ensure camera-to-model xform is valid
	MatrixValid<REAL>(GetXform());

	// ensure that the current projection matrix is valid
	MatrixValid<REAL>(GetProjection());
}

/////////////////////////////////////////////////////////////////////////////
// CCamera::Dump
// 
// dumps the contents of the CCamera
/////////////////////////////////////////////////////////////////////////////
void CCamera::Dump(CDumpContext& dc) const
{
}
#endif //_DEBUG

