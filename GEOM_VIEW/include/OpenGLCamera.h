// OpenGLCamera.h: interface for the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
#define AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>
#include <Matrix.h>

#include <Observer.h>

//////////////////////////////////////////////////////////////////////
// class COpenGLCamera
// 
// a camera that represents the viewpoint for a COpenGLView
//////////////////////////////////////////////////////////////////////
class COpenGLCamera : public CObject
{
public:
	// constructor/destructor
	COpenGLCamera();
	virtual ~COpenGLCamera();

public:
	// the distance along the optical axis from the camera's focal point
	//		to the target point
	double GetDistance() const;
	void SetDistance(double dist);

	// the direction from the camera to the the target point
	double GetTheta() const;
	void SetTheta(double theta);

	double GetPhi() const;
	void SetPhi(double phi);

	// the camera rotation about its optical axis
	double GetRollAngle() const;
	void SetRollAngle(double rollAngle);

	// matrix representing the transform from the model space to the
	//		camera space
	const CMatrix<4>& GetModelXform() const;
	void SetModelXform(const CMatrix<4>& m);

	// the viewing angle for the camera (0 for orthographic camera)
	double GetViewingAngle() const;
	void SetViewingAngle(double angle);

	// the aspect ratio is usually set to the aspect ratio of the viewport
	double GetAspectRatio() const;
	void SetAspectRatio(double aspectRatio);

	// the clipping planes, distance from focal point
	double GetNearPlane() const;
	double GetFarPlane() const;
	void SetClippingPlanes(double nearPlane, double farPlane);

	// sets the near plane, far plane, and viewing angle for a particular
	//		maximum object size
	void SetFieldOfView(double maxObjectSize);

	// matrix representing the camera projection
	const CMatrix<4>& GetPerspective() const;

	// the total matrix for the projection and transform
	const CMatrix<4>& GetProjection() const;

	// returns a reference to the change event for the camera
	CObservableEvent& GetChangeEvent();

protected:
	// call-back to compute the object-to-camera transform
	void RecalcModelXform();

	// call-back to compute the camera projection
	void RecalcProjection();

private:
	// the point being looked at
	CVector<3> m_vTargetPoint;

	// viewing distance
	double m_distance;

	// theta, phi
	double m_theta;
	double m_phi;

	// the roll angle for the camera
	double m_rollAngle;

	// the model transform
	CMatrix<4> m_mModelXform;

	// the viewing angle (along Y direction
	double m_viewingAngle;

	// the aspect ratio for the view (X / Y)
	double m_aspectRatio;

	// the near clipping, distance from focal point
	double m_nearPlane;

	// the far clipping plane, distance from focal point
	double m_farPlane;

	// perspective matrix
	CMatrix<4> m_mPerspective;

	// projection matrix
	CMatrix<4> m_mProjection;

	// the change event for this object
	CObservableEvent m_eventChange;
};

//////////////////////////////////////////////////////////////////////
// COpenGLCamera::GetChangeEvent
// 
// returns a reference to the change event for the camera
//////////////////////////////////////////////////////////////////////
inline CObservableEvent& COpenGLCamera::GetChangeEvent()
{
	return m_eventChange;
}

#endif // !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
