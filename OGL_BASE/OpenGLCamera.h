// OpenGLCamera.h: interface for the COpenGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
#define AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COpenGLCamera  
{
public:
	COpenGLCamera();
	virtual ~COpenGLCamera();

public:
	// the point that the camera is looking at
	CValue< CVector<3> > targetPoint;

	// the direction from the camera to the the target point
	CValue< CVector<3> > direction;

	// the camera rotation about its optical axis
	CValue< double > rotation;

	// the distance along the optical axis from the camera's focal point
	//		to the target point
	CValue< double > distance;

	// matrix representing the transform from the object space to the
	//		camera space
	const CMatrix<4>& GetObjectToCameraTransform() const;

	// the viewing angle for the camera (0 for orthographic camera)
	CValue< double > viewingAngle;

	// the near clipping, distance from focal point
	CValue< double > nearPlane;

	// the far clipping plane, distance from focal point
	CValue< double > farPlane;

	// matrix representing the camera projection
	const CMatrix<4>& GetProjection() const;
};

#endif // !defined(AFX_OPENGLCAMERA_H__94F5EA60_85D6_11D5_ABA8_00B0D0AB90D6__INCLUDED_)
