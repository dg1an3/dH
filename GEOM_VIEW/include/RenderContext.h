// RenderContext.h: interface for the CRenderContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_)
#define AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

#include <d3d8.h>

// #include <Polygon.h>
#include <Mesh.h>

#include "Texture.h"

// class CSceneView;

class CRenderContext  
{
public:
	CRenderContext(LPDIRECT3DDEVICE8 pd3dDevice); // CSceneView *pSceneView);

public:
	virtual ~CRenderContext();

	LPDIRECT3DVERTEXBUFFER8 CreateVertexBuffer(UINT nLength, DWORD dwFVF);

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
	void Vertex(const CVectorD<2>& v);
	void Vertex(const CVectorD<3>& v);
	void Vertex(const CVectorD<4>& v);
	void VertexArray();

	// normal description
	void Normal(const CVectorD<3>& n);
	void Normal(const CVectorD<4>& n);
	void NormalArray();

	// TODO: move these functions to the Renderable base class
	// higher-level descriptions
	// void LineLoopFromPolygon(CPolygon& poly);
	void TrianglesFromSurface(CMesh& surf);

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
	void LoadMatrix(const CMatrixD<4, double>& m);
	void MultMatrix(const CMatrixD<4, double>& m);

	void Translate(const CVectorD<3> vOffset);
	void Rotate(double angle,  const CVectorD<3> vAxis);

	void PushMatrix();
	void PopMatrix();

	// texture operations
	void Bind(CTexture *pTexture);
	void Unbind();

private:
	// scene view for this context
	// CSceneView *m_pSceneView;

	LPDIRECT3DDEVICE8 m_pd3dDevice;

	COLORREF m_color;
	double m_alpha;

	// pointer to the bound texture
	CTexture *m_pTexture;
};

#endif // !defined(AFX_RENDERCONTEXT_H__53C73296_5B7E_4706_81D3_CB8A158778A8__INCLUDED_)
