//////////////////////////////////////////////////////////////////////
// Camera.h: declaration of the CCamera class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(CAMERA_H)
#define CAMERA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector and matrix include file
#include <VectorD.h>
#include <MatrixD.h>

// event management
#include <Observer.h>

//////////////////////////////////////////////////////////////////////
// class CCamera
// 
// a camera that represents the viewpoint for a CSceneView
//////////////////////////////////////////////////////////////////////
class CCamera : public CObject
{
public:
	// constructor/destructor
	CCamera();
	virtual ~CCamera();

public:
	// camera direction vector
	const CVectorD<3>& GetTarget() const;
	void SetTarget(const CVectorD<3>& vDir);

	// camera direction vector
	const CVectorD<3>& GetDirection() const;
	void SetDirection(const CVectorD<3>& vDir);

	// the distance along the optical axis from the camera's focal point
	//		to the target point
	double GetDistance() const;
	void SetDistance(double dist);

	// sets the "up" direction for the camera
	const CVectorD<3>& GetUpDirection() const;
	void SetUpDirection(const CVectorD<3>& vDir);

	// matrix representing the transform from the model space to the
	//		camera space
	const CMatrixD<4>& GetXform() const;

	// the viewing angle for the camera in radians
	//		(0 for orthographic camera)
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
	const CMatrixD<4>& GetProjection() const;

	// the total matrix for the projection and transform
	// const CMatrixD<4>& GetProjection() const;

	// returns a reference to the change event for the camera
	CObservableEvent& GetChangeEvent();

protected:
	// call-back to compute the object-to-camera transform
	void RecalcCameraToModel() const;

	// call-back to compute the camera projection
	void RecalcProjection() const;

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// the point being looked at
	CVectorD<3> m_vTargetPoint;

	// direction
	CVectorD<3> m_vDirection;

	// viewing distance
	double m_distance;

	// direction
	CVectorD<3> m_vUpDirection;

	// the model transform
	mutable CMatrixD<4> m_mXform;

	// the viewing angle (along Y direction
	double m_viewingAngle;

	// the aspect ratio for the view (X / Y)
	double m_aspectRatio;

	// the near clipping, distance from focal point
	double m_nearPlane;

	// the far clipping plane, distance from focal point
	double m_farPlane;

	// perspective matrix
	mutable CMatrixD<4> m_mProjection;

	// the change event for this object
	CObservableEvent m_eventChange;
};

//////////////////////////////////////////////////////////////////////
// CCamera::GetChangeEvent
// 
// returns a reference to the change event for the camera
//////////////////////////////////////////////////////////////////////
inline CObservableEvent& CCamera::GetChangeEvent()
{
	return m_eventChange;
}

#endif // !defined(CAMERA_H)
