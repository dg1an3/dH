#if !defined(GLMATRIXVECTOR_H)
#define GLMATRIXVECTOR_H

#include <gl/gl.h>

#include <Vector.h>
#include <Matrix.h>

inline void glVertex(const CVector<2, double>& v)
{
	glVertex2dv(v);
}

inline void glVertex(const CVector<3, double>& v)
{
	glVertex3dv(v);
}

inline void glVertex(const CVector<4, double>& v)
{
	glVertex4dv(v);
}

inline void glNormal(const CVector<3, double>& v)
{
	glNormal3dv(v);
}

inline void glNormal(const CVector<4, double>& v)
{
	glNormal(FromHomogeneous<3, double>(v));
}

inline void glTranslate(const CVector<2, double>& v)
{
	glTranslated(v[0], v[1], 0.0);
}

inline void glTranslate(const CVector<3, double>& v)
{
	glTranslated(v[0], v[1], v[2]);
}

inline void glLoadMatrix(const CMatrix<4, double>& m)
{
	double mArray[16];
	m.ToArray(mArray);
	glLoadMatrixd(mArray);
}

inline void glMultMatrix(const CMatrix<4, double>& m)
{
	double mArray[16];
	m.ToArray(mArray);
	glMultMatrixd(mArray);
}

inline void glColor(COLORREF forColor)
{
	glColor3ub(GetRValue(forColor), GetGValue(forColor), GetBValue(forColor));
}

inline void glClearColor(COLORREF forColor)
{
	glClearColor(
		(float) GetRValue(forColor) / 255.0f, 
		(float) GetGValue(forColor) / 255.0f, 
		(float) GetBValue(forColor) / 255.0f, 
		1.0f);
}

inline void glLightDirection(GLenum light, CVector<3> vParam)
{
	GLfloat param[4];
	param[0] = (float) vParam[0];
	param[1] = (float) vParam[1];
	param[2] = (float) vParam[2];
	param[3] = 0.0f;

	glLightfv(light, GL_POSITION, param);
}

inline CMatrix<4> glGetMatrix(GLenum pname)
{
	GLdouble mArray[16];
	glGetDoublev(pname, mArray);

	CMatrix<4> m;
	m.FromArray(mArray);

	return m;
}

#endif
