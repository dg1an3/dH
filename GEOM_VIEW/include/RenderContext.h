// RenderContext.h: interface for the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_)
#define AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>

#include <Polygon.h>
#include <Surface.h>

#include "Texture.h"

class CSceneView;

class CRenderContext  
{
public:
	CRenderContext(CSceneView *pSceneView);
	virtual ~CRenderContext();

	// begin describing geometrics
	void BeginLines();
	void BeginLineLoop();
	void BeginTriangles();
	void BeginQuads();
	void BeginQuadStrip();
	void BeginPolygon();

	// generic end
	void End();

	// vertex description
	void Vertex(const CVector<2>& v);
	void Vertex(const CVector<3>& v);
	void Vertex(const CVector<4>& v);
	void VertexArray();

	// normal description
	void Normal(const CVector<3>& n);
	void Normal(const CVector<4>& n);
	void NormalArray();

	// higher-level descriptions
	void LineLoopFromPolygon(CPolygon& poly);
	void TrianglesFromSurface(CSurface& surf);

	// color and alpha 
	void Color(COLORREF color);
	void Alpha(double alpha);

	// rendering parameters
	void SetSmoothShading(BOOL bSmooth = TRUE);
	void SetLighting(BOOL bLighting = TRUE);

	// setup for default rendering modes
	void SetupLines();
	void SetupOpaqueSurface();
	void SetupTransparentFrontFaces();
	void SetupTransparentBackFaces();

	// modelview matrix operations
	void LoadMatrix(const CMatrix<4, double>& m);
	void MultMatrix(const CMatrix<4, double>& m);

	void Translate(const CVector<3> vOffset);
	void Rotate(double angle,  const CVector<3> vAxis);

	void PushMatrix();
	void PopMatrix();

	// texture operations
	void Bind(CTexture *pTexture);
	void Unbind();

private:
	// scene view for this context
	CSceneView *m_pSceneView;

	COLORREF m_color;
	double m_alpha;

	// pointer to the bound texture
	CTexture *m_pTexture;
};

#endif // !defined(AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_)
