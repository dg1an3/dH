// OpenGLCamera.cpp: implementation of the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// floating point utilities
#include <float.h>

// math utilities
#include <MathUtil.h>

// class interface
#include "OpenGLCamera.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::COpenGLCamera
// 
// constructs a camera object
//////////////////////////////////////////////////////////////////////
COpenGLCamera::COpenGLCamera()
#pragma warning(disable: 4355)
	: m_eventChange(this),
#pragma warning(default: 4355)
		m_vTargetPoint(CVector<3>(0.0, 0.0, 0.0)),
		m_distance(100.0),
		m_theta(0.0),
		m_phi(0.0),
		m_rollAngle(0.0),
		m_viewingAngle(45.0),
		m_aspectRatio(1.0),
		m_nearPlane(50.0),
		m_farPlane(150.0)
{
	// set up the projection
	RecalcProjection();

	// and model xform matrices
	RecalcModelXform();
}


//////////////////////////////////////////////////////////////////////
// COpenGLCamera::~COpenGLCamera
// 
// destroys the camera object
//////////////////////////////////////////////////////////////////////
COpenGLCamera::~COpenGLCamera()
{
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::GetDistance
// 
// returns the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
double COpenGLCamera::GetDistance() const 
{ 
	return m_distance;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetDistance(double dist) 
{ 
	// set the distance from the camera to the target point
	m_distance = dist;

	// recalculat the model transform
	RecalcModelXform();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::GetTheta
// 
// returns the theta angle for the camera
//////////////////////////////////////////////////////////////////////
double COpenGLCamera::GetTheta() const 
{ 
	return m_theta;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetTheta
// 
// sets the theta angle for the camera
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetTheta(double theta) 
{ 
	// set theta
	m_theta = theta;

	// and recalculate the model xform
	RecalcModelXform();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::GetPhi
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
double COpenGLCamera::GetPhi() const 
{ 
	return m_phi;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetPhi
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetPhi(double phi) 
{ 
	// set phi
	m_phi = phi;

	// and recalculate the model xform
	RecalcModelXform();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// the camera rotation about its optical axis
double COpenGLCamera::GetRollAngle() const 
{ 
	return m_rollAngle;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetRollAngle(double rollAngle) 
{ 
	m_rollAngle = rollAngle;

	// and recalculate the model xform
	RecalcModelXform();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// matrix representing the transform from the model space to the
//		camera space
const CMatrix<4>& COpenGLCamera::GetModelXform() const 
{ 
	return m_mModelXform;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetModelXform(const CMatrix<4>& m) 
{ 
	// assign the matrix
	m_mModelXform = m;

	// compute the total projection
	m_mProjection = GetPerspective() * m_mModelXform;

	// form the rotation angles for the camera direction
	m_phi = acos(m[2][2]);

	// set the theta and roll angles to zero initially
	m_theta = 0.0;
	m_rollAngle = 0.0;

	// the sine of phi is used to compute the other angles
	double sin_phi = sin(m_phi);
	if (sin_phi > EPS)
	{
		// compute theta
		if (m[2][0] != 0.0 && m[2][1] != 0.0)
		{
			m_theta = AngleFromSinCos(
				m[2][0] / sin_phi,
				m[2][1] / sin_phi);
		}

		// compute phi
		if (m[0][2] != 0.0 && m[1][2] != 0.0)
		{
			m_rollAngle = AngleFromSinCos(
				m[0][2] / sin_phi,
				-m[1][2] / sin_phi);
		}
	}

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// the viewing angle for the camera (0 for orthographic camera)
double COpenGLCamera::GetViewingAngle() const 
{ 
	return m_viewingAngle;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetViewingAngle(double angle) 
{ 
	m_viewingAngle = angle;

	// and recalculate the projection
	RecalcProjection();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// the aspect ratio is usually set to the aspect ratio of the viewport
double COpenGLCamera::GetAspectRatio() const 
{ 
	return m_aspectRatio;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetAspectRatio(double aspectRatio) 
{ 
	// make sure a valid aspect ratio is passed
	ASSERT(!_isnan(aspectRatio));

	m_aspectRatio = aspectRatio;

	// and recalculate the projection
	RecalcProjection();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// the clipping planes, distance from focal point
double COpenGLCamera::GetNearPlane() const 
{ 
	return m_nearPlane;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
double COpenGLCamera::GetFarPlane() const 
{ 
	return m_farPlane;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetClippingPlanes(double nearPlane, double farPlane) 
{ 
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;

	// and recalculate the projection
	RecalcProjection();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::SetFieldOfView(double maxObjectSize)
{
	SetClippingPlanes(GetNearPlane(), 
			GetNearPlane() + maxObjectSize * 2.5f);
	SetDistance(GetNearPlane() + maxObjectSize / 1.2f);
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// matrix representing the camera projection
const CMatrix<4>& COpenGLCamera::GetPerspective() const
{
	return m_mPerspective;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::SetDistance
// 
// sets the distance along the optical axis from the camera's 
//		focal point to the target point
//////////////////////////////////////////////////////////////////////
// the total matrix for the projection and transform
const CMatrix<4>& COpenGLCamera::GetProjection() const
{
	return m_mProjection;
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::RecalcModelXform
// 
// recalculates the model transform
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::RecalcModelXform()
{
	// form the rotation matrix for the camera direction
	CMatrix<3> mRotateDir = CreateRotate(GetPhi(), CVector<3>(1.0, 0.0, 0.0))
		* CreateRotate(GetTheta(), CVector<3>(0.0, 0.0, 1.0));

	// form the camera roll rotation matrix
	CMatrix<3> mRotateRoll = CreateRotate(GetRollAngle(), 
		CVector<3>(0.0, 0.0, 1.0));

	// form the translation from the target point to the focal point
	CMatrix<4> mTranslate = CreateTranslate(GetDistance(), 
		CVector<3>(0.0, 0.0, -1.0));

	// and set the total camera transformation to all three matrices
	m_mModelXform = 
		mTranslate * CMatrix<4>(mRotateRoll) * CMatrix<4>(mRotateDir);

	// compute the total projection
	m_mProjection = GetPerspective() * GetModelXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::RecalcProjection
// 
// recalculates the projection (perspective and xform)
//////////////////////////////////////////////////////////////////////
void COpenGLCamera::RecalcProjection()
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
	m_mProjection = GetPerspective() * GetModelXform();

	// notify listeners that the camera has changed
	GetChangeEvent().Fire();
}
