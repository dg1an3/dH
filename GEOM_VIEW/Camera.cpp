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
		m_vTargetPoint(CVector<3>(0.0, 0.0, 0.0)),
		m_distance(100.0),
		m_theta(0.0),
		m_phi(0.0),
		m_rollAngle(0.0),
		m_zoom(1.0),
		m_viewingAngle(45.0),
		m_aspectRatio(1.0),
		m_nearPlane(50.0),
		m_farPlane(150.0)
{
	// set up the projection
	RecalcProjection();

	// and model xform matrices
	RecalcXform();
}


//////////////////////////////////////////////////////////////////////
// CCamera::~CCamera
// 
// destroys the camera object
//////////////////////////////////////////////////////////////////////
CCamera::~CCamera()
{
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetDistance
// 
// returns the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
double CCamera::GetDistance() const 
{ 
	return m_distance;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void CCamera::SetDistance(double dist) 
{ 
	// set the distance from the camera to the target point
	m_distance = dist;

	// recalculate the model transform
	RecalcXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetTheta
// 
// returns the theta angle for the camera
//////////////////////////////////////////////////////////////////////
double CCamera::GetTheta() const 
{ 
	return m_theta;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetTheta
// 
// sets the theta angle for the camera
//////////////////////////////////////////////////////////////////////
void CCamera::SetTheta(double theta) 
{ 
	// set theta
	m_theta = theta;

	// and recalculate the model xform
	RecalcXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetPhi
// 
// returns the phi angle (pitch)
//////////////////////////////////////////////////////////////////////
double CCamera::GetPhi() const 
{ 
	return m_phi;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetPhi
// 
// sets the phi angle (pitch)
//////////////////////////////////////////////////////////////////////
void CCamera::SetPhi(double phi) 
{ 
	// set phi
	m_phi = phi;

	// and recalculate the model xform
	RecalcXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetZoom
// 
// returns the camera zoom
//////////////////////////////////////////////////////////////////////
double CCamera::GetZoom() const 
{ 
	return m_zoom;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetZoom
// 
// sets the camera zoom
//////////////////////////////////////////////////////////////////////
void CCamera::SetZoom(double zoom) 
{ 
	// set phi
	m_zoom = zoom;

	// and recalculate the model xform
	RecalcXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}



//////////////////////////////////////////////////////////////////////
// CCamera::GetRollAngle
// 
// returns the camera rotation about its optical axis
//////////////////////////////////////////////////////////////////////
double CCamera::GetRollAngle() const 
{ 
	return m_rollAngle;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetRollAngle
// 
// sets the camera rotation about its optical axis
//////////////////////////////////////////////////////////////////////
void CCamera::SetRollAngle(double rollAngle) 
{ 
	m_rollAngle = rollAngle;

	// and recalculate the model xform
	RecalcXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetXform
// 
// returns matrix representing the transform from the model space to 
//		the camera space
//////////////////////////////////////////////////////////////////////
const CMatrix<4>& CCamera::GetXform() const 
{ 
	return m_mXform;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetXform
// 
// sets the matrix representing the transform from the model space to 
//		the camera space
//////////////////////////////////////////////////////////////////////
void CCamera::SetXform(const CMatrix<4>& m) 
{ 
	// assign the matrix
	m_mXform = m;

	// compute the total projection
	m_mProjection = GetPerspective() * m_mXform;

	// create an orthogonal (rotation) matrix
	CMatrix<3> mOrtho = CMatrix<3>(m);
	mOrtho.Orthogonalize();

	// compute the scale factor
	CMatrix<3> mOrthoInv(mOrtho);
	mOrthoInv.Invert();
	CMatrix<3> mScale = mOrthoInv * CMatrix<3>(m);

	// ensure that there is no warp in the xform
	ASSERT(IS_APPROX_EQUAL(mScale[0][0], mScale[1][1]));
	ASSERT(IS_APPROX_EQUAL(mScale[0][0], mScale[2][2]));

	// set the scale factor
	m_zoom = mScale[0][0];

	// form the rotation angles for the camera direction
	m_phi = acos(mOrtho[2][2]);

	// set the theta and roll angles to zero initially
	m_theta = 0.0;
	m_rollAngle = 0.0;

	// the sine of phi is used to compute the other angles
	double sin_phi = sin(m_phi);
	if (sin_phi > EPS)
	{
		// compute theta
		if (mOrtho[2][0] != 0.0 && mOrtho[2][1] != 0.0)
		{
			m_theta = AngleFromSinCos(
				mOrtho[2][0] / sin_phi,
				mOrtho[2][1] / sin_phi);
		}

		// compute phi
		if (mOrtho[0][2] != 0.0 && mOrtho[1][2] != 0.0)
		{
			m_rollAngle = AngleFromSinCos(
				mOrtho[0][2] / sin_phi,
				-mOrtho[1][2] / sin_phi);
		}
	}

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetViewingAngle
// 
// the viewing angle for the camera (0 for orthographic camera)
//////////////////////////////////////////////////////////////////////
double CCamera::GetViewingAngle() const 
{ 
	return m_viewingAngle;
}

//////////////////////////////////////////////////////////////////////
// CCamera::SetViewingAngle
// 
// the viewing angle for the camera (0 for orthographic camera)
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
const CMatrix<4>& CCamera::GetPerspective() const
{
	return m_mPerspective;
}

//////////////////////////////////////////////////////////////////////
// CCamera::GetProjection
// 
// the total matrix for the projection and transform
//////////////////////////////////////////////////////////////////////
const CMatrix<4>& CCamera::GetProjection() const
{
	return m_mProjection;
}

//////////////////////////////////////////////////////////////////////
// CCamera::RecalcXform
// 
// recalculates the model transform
//////////////////////////////////////////////////////////////////////
void CCamera::RecalcXform() const
{
	// form the rotation matrix for the camera direction
	CMatrix<3> mRotateDir = 
		CreateRotate(GetPhi(), CVector<3>(1.0, 0.0, 0.0))
		* CreateRotate(GetTheta(), CVector<3>(0.0, 0.0, 1.0));

	// form the camera roll rotation matrix
	CMatrix<3> mRotateRoll = CreateRotate(GetRollAngle(), 
		CVector<3>(0.0, 0.0, 1.0));

	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(GetDistance(), 
		CVector<3>(0.0, 0.0, -1.0));

	// form the scale matrix
	CMatrix<3> mScale = 
		CreateScale(CVector<3>(m_zoom, m_zoom, m_zoom));

	// and set the total camera transformation to all three matrices
	m_mXform = mTranslate 
		* CMatrix<4>(mRotateRoll) 
		* CMatrix<4>(mRotateDir)
		* CMatrix<4>(mScale);

	// compute the total projection
	m_mProjection = GetPerspective() * GetXform();
}

//////////////////////////////////////////////////////////////////////
// CCamera::RecalcProjection
// 
// recalculates the projection (perspective and xform)
//////////////////////////////////////////////////////////////////////
void CCamera::RecalcProjection() const
{
	// compute the top edge of the viewing frustum from the nearPlane 
	//		and viewingAngle
	double top = GetNearPlane() * tan(GetViewingAngle() * PI / 360.0);

	// compute the right edge from the top edge and the aspect ratio
	double right = top * GetAspectRatio();

	// now populate the perspective projection matrix
	CMatrix<4> mPersp;
	mPersp[0][0] = 2.0 * GetNearPlane() / (2.0 * right);
	mPersp[1][1] = 2.0 * GetNearPlane() / (2.0 * top);
	mPersp[2][2] = -(GetFarPlane() + GetNearPlane()) 
		/ (GetFarPlane() - GetNearPlane());
	mPersp[2][3] = -2.0 * GetFarPlane() * GetNearPlane()
		/ (GetFarPlane() - GetNearPlane());
	mPersp[3][2] = -1.0;
	mPersp[3][3] = 0.0;

	// and set the projection
	m_mPerspective = mPersp;

	// compute the total projection
	m_mProjection = GetPerspective() * GetXform();
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

	// ensure that the current projection matrix is valid
	MatrixValid<REAL>(GetProjection());

	// compare the model xform to the recalculated one
	CMatrix<4> mXformPre = GetXform();
	RecalcXform();
	ASSERT(mXformPre.IsApproxEqual(GetXform(), 1e-1));

	// compare the projection to the recalculated one
	CMatrix<4> mPerspectivePre = GetPerspective();
	RecalcProjection();
	ASSERT(mPerspectivePre.IsApproxEqual(GetPerspective(), 1e-1));

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

