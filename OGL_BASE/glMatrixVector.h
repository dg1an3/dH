#if !defined(GLMATRIXVECTOR_H)
#define GLMATRIXVECTOR_H

#include <Vector.h>

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

inline CMatrix<4> glGetMatrix(GLenum pname)
{
	GLdouble mArray[16];
	glGetDoublev(pname, mArray);

	CMatrix<4> m;
	m.FromArray(mArray);

	return m;
}

#endif
