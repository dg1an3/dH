// OpenGLTexture.cpp: implementation of the COpenGLTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"
#include "OpenGLTexture.h"

#include "gl/gl.h"
#include "gl/glu.h"

#include <Dib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLTexture::COpenGLTexture()
:	m_nWidth(0), m_nHeight(0),
	m_pDC(NULL),
	m_pBitmap(NULL),
	m_pOldBitmap(NULL),
	m_pView(NULL),
	m_nHandle(-1)
{

}

COpenGLTexture::~COpenGLTexture()
{
	if (m_pDC != NULL)
		ReleaseDC();

	if (m_pView != NULL)
		Unbind();
}

void COpenGLTexture::SetWidthHeight(int nWidth, int nHeight)
{
	ASSERT(m_pDC == NULL);
	ASSERT(m_pBitmap == NULL);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	// delete any pixels that are cached, so they will be re-allocated
	m_arrPixels.SetSize(nWidth * nHeight);
}

CDC * COpenGLTexture::GetDC()
{
	/////////////////////////////////////////////////////////////////

	ASSERT(m_pDC == NULL);
	ASSERT(m_pBitmap == NULL);

	CDC currDC;
	currDC.Attach(::GetDC(NULL));

	m_pDC = new CDC();
	m_pDC->CreateCompatibleDC(&currDC);

	m_pBitmap = new CBitmap();
	m_pBitmap->CreateCompatibleBitmap(&currDC, m_nWidth, m_nHeight);

	currDC.Detach();

	m_pOldBitmap = m_pDC->SelectObject(m_pBitmap);

	return m_pDC;
}

void COpenGLTexture::DrawBorder(int nWidth)
{
	ASSERT(m_pDC != NULL);

	// post-process by drawing transparent border
	CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen *pOldPen = m_pDC->SelectObject(&blackPen);
	CBrush *pOldBrush = (CBrush *)m_pDC->SelectStockObject(HOLLOW_BRUSH);

	m_pDC->Rectangle(0, 0, m_nWidth-1, m_nHeight-1);

	m_pDC->SelectObject(pOldPen);
	m_pDC->SelectObject(pOldBrush);
}

void COpenGLTexture::ReleaseDC()
{
	/////////////////////////////////////////////////////////////////

	// delete the objects
	m_pDC->SelectObject(m_pOldBitmap);
	m_pOldBitmap = NULL;

	delete m_pDC;
	m_pDC = NULL;

	// store the pixels
	m_pBitmap->GetBitmapBits(m_nWidth * m_nHeight * 4, m_arrPixels.GetData());

	delete m_pBitmap;
	m_pBitmap = NULL;

	ProcessTransparency();
}

// ONLY PROCESS TRANSPARENCY ONCE!!!
void COpenGLTexture::ProcessTransparency()
{
	COLORREF *pPixels = m_arrPixels.GetData();

	// color in the transparency
	for (int nAt = 0; nAt < m_nWidth * m_nHeight; nAt++)
	{
		if (pPixels[nAt] == RGB(0, 0, 0))
		{
			pPixels[nAt] = 0x00000000;
		}
		else if (pPixels[nAt] == RGB(128, 128, 128))
		{
			pPixels[nAt] = 0x7fffffff;
		}
		else
		{
			pPixels[nAt] |= 0xff000000;
		}
	}
}

BOOL COpenGLTexture::LoadBitmap(UINT nResourceID)
{
	CDib dib;
	BOOL bResult = dib.Load(GetModuleHandle(NULL), nResourceID);
	if (bResult)
	{
		BITMAP bm;
		dib.GetBitmap(&bm);
		m_nHeight = bm.bmHeight;
		m_nWidth = bm.bmWidth;

		int nSize = m_nHeight * m_nWidth;

		char *pPixels = new char[nSize * 3];
		dib.GetBitmapBits(nSize * 3, pPixels);

		// now load the pixels
		m_arrPixels.RemoveAll();
		for (int nAt = 0; nAt < nSize*3; nAt += 3)
			m_arrPixels.Add(RGB(pPixels[nAt+2], pPixels[nAt+1], pPixels[nAt+0]));

		ProcessTransparency();
	}

	return bResult;
}

void COpenGLTexture::Bind(COpenGLView *pView, BOOL bWrap)
{
	m_pView = pView;

	// now bind the texture
	m_pView->MakeCurrentGLRC();

	ASSERT(m_nHandle == -1);
	glGenTextures(1, (unsigned int *)&m_nHandle);

    glBindTexture (GL_TEXTURE_2D, m_nHandle);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	if (bWrap)
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);     

	// now create a texture
	glTexImage2D(GL_TEXTURE_2D, 
		0,				// GLint level,
		GL_RGBA,			// GLint components,       
		m_nWidth,			// GLsizei width,          
		m_nHeight,			// GLsizei height,       
		0,				// GLint border,           
		GL_RGBA,		    // GLenum format,
		GL_UNSIGNED_BYTE,	// GLenum type,          
		m_arrPixels.GetData());

	glEnable(GL_TEXTURE_2D); // enable texture mapping 

	ASSERT(glGetError() == GL_NO_ERROR);
}

void COpenGLTexture::Unbind()
{
	ASSERT(m_pView != NULL);

	m_pView->MakeCurrentGLRC();

	glDisable (GL_TEXTURE_2D); // disable texture mapping 

    glBindTexture (GL_TEXTURE_2D, 0);

	if (m_nHandle != -1)
	{
		glDeleteTextures(1, (unsigned int *)&m_nHandle);
		m_nHandle = -1;
	}

	m_pView = NULL;
	ASSERT(glGetError() == GL_NO_ERROR);
}
