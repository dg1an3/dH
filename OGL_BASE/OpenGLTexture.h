// OpenGLTexture.h: interface for the COpenGLTexture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLTEXTURE_H__88503651_F301_11D4_9E3D_00B0D0609AB0__INCLUDED_)
#define AFX_OPENGLTEXTURE_H__88503651_F301_11D4_9E3D_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLView.h"

class COpenGLTexture  
{
public:
	COpenGLTexture();
	virtual ~COpenGLTexture();

	void SetWidthHeight(int nWidth, int nHeight);

	CDC * GetDC();
	void DrawBorder(int nWidth = 1);
	void ReleaseDC();

	void Bind(COpenGLView *pView);
	void Unbind();

private:
	COpenGLView * m_pView;
	int m_nWidth, m_nHeight;

	int m_nHandle;

	CDC * m_pDC;

	CBitmap *m_pBitmap;

	CBitmap *m_pOldBitmap;

	CArray<COLORREF, COLORREF&> m_arrPixels;

protected:
	void ProcessTransparency();
};

#endif // !defined(AFX_OPENGLTEXTURE_H__88503651_F301_11D4_9E3D_00B0D0609AB0__INCLUDED_)
