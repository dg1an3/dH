//////////////////////////////////////////////////////////////////////
// Texture.h: declaration of the CTexture class
//
// Copyright (C) 2000-2002
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(TEXTURE_H)
#define TEXTURE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixD.h>

#include <Observer.h>

// forward declaration of the view class
class CSceneView;
class CRenderContext;

//////////////////////////////////////////////////////////////////////
// class CTexture
//
// represents a texture for texture-mapping
//////////////////////////////////////////////////////////////////////
class CTexture : public CObject 
{
public:
	// constructor/destructor
	CTexture();
	virtual ~CTexture();

	// width and height of texture image
	void SetWidthHeight(int nWidth, int nHeight);

	// flag to indicate texture wrapping
	BOOL IsWrap() const;
	void SetWrap(BOOL bWrap = FALSE);

	// get a DC for drawing the texture
	CDC * GetDC();
	void DrawBorder(int nWidth = 1);
	void ReleaseDC();

	// loads the texture from a bitmap
	BOOL LoadBitmap(UINT nResourceID);

	// accessors for the texture projection
	const CMatrixD<4>& GetProjection();
	void SetProjection(const CMatrixD<4>& mXform);

	// returns reference to the change event
	CObservableEvent& GetChangeEvent();

protected:
	// friend to access the Bind/Unbind functions
	friend CRenderContext;

	// binds for rendering
	void Bind(CSceneView *pView);
	void Unbind();

	// helper function to process the transparency color
	void ProcessTransparency();

private:
	// the view for the texture
	CSceneView *m_pView;

	// the height and width of the texture
	int m_nWidth;
	int m_nHeight;

	// flag to indicate texture wrapping
	BOOL m_bWrap;

	// texture handle
	int m_nHandle;

	// the display context
	CDC *m_pDC;

	// the bitmap that holds the texture
	CBitmap *m_pBitmap;

	// the old bitmap (bookkeeping purposes)
	CBitmap *m_pOldBitmap;

	// the actual pixel values for the texture
	CArray<COLORREF, COLORREF> m_arrPixels;

	// the texture xform
	CMatrixD<4> m_mProjection;

	// the change event for this object
	CObservableEvent m_eventChange;
};

#endif // !defined(TEXTURE_H)
