// OpenGLCamera.h: interface for the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
#define AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>

#include <Vector.h>
#include <Matrix.h>

class COpenGLCamera : public CObservableObject
{
public:
	COpenGLCamera();
	virtual ~COpenGLCamera();

public:
	// the point that the camera is looking at
	CValue< CVector<3> > targetPoint;

	// the direction from the camera to the the target point
	CValue< CVector<3> > direction;

	// the distance along the optical axis from the camera's focal point
	//		to the target point
	CValue< double > distance;

	// the camera rotation about its optical axis
	CValue< double > rollAngle;

	// matrix representing the transform from the model space to the
	//		camera space
	CValue< CMatrix<4> > modelXform;

	// the viewing angle for the camera (0 for orthographic camera)
	CValue< double > viewingAngle;

	// the aspect ratio is usually set to the aspect ratio of the viewport
	CValue< double > aspectRatio;

	// the near clipping, distance from focal point
	CValue< double > nearPlane;

	// the far clipping plane, distance from focal point
	CValue< double > farPlane;

	// matrix representing the camera projection
	CValue< CMatrix<4> > projection;

protected:
	// call-back to compute the object-to-camera transform
	void OnComputeModelXform(CObservableObject *pSource, 
		void *pOldValue);

	// call-back to compute the camera projection
	void OnComputeProjection(CObservableObject *pSource, 
		void *pOldValue);
};

#endif // !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
