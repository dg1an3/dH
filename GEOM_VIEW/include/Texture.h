// Texture.h: interface for the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(TEXTURE_H)
#define TEXTURE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forward declaration of the view class
class CSceneView;

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

	// get a DC for drawing the texture
	CDC * GetDC();
	void DrawBorder(int nWidth = 1);
	void ReleaseDC();

	// loads the texture from a bitmap
	BOOL LoadBitmap(UINT nResourceID);

	// binds for rendering
	void Bind(CSceneView *pView, BOOL bWrap = FALSE);
	void Unbind();

protected:
	// helper function to process the transparency color
	void ProcessTransparency();

private:
	// the view for the texture
	CSceneView *m_pView;

	// the height and width of the texture
	int m_nWidth;
	int m_nHeight;

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
};

#endif // !defined(TEXTURE_H)
