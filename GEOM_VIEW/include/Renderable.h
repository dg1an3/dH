//////////////////////////////////////////////////////////////////////
// Renderable.h: declaration of the CRenderable class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(RENDERABLE_H)
#define RENDERABLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <d3d8.h>
#include <d3dx8.h>

// matrix include file
#include <MatrixD.h>

// observable events
#include <Observer.h>

// mesh 
#include <Mesh.h>

// A structure for our custom vertex type. We added a normal, and omitted the
// color (which is provided by the material)
struct CUSTOMVERTEX_POS_NORM
{
    D3DXVECTOR3 position; // The 3D position for the vertex
    D3DXVECTOR3 normal;   // The surface normal for the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX_POS_NORM (D3DFVF_XYZ|D3DFVF_NORMAL)


// forward declaration of the CSceneView class 
class CSceneView;

//////////////////////////////////////////////////////////////////////
// class CRenderable
//
// represents a renderer that belongs to a CSceneView window
//////////////////////////////////////////////////////////////////////
class CRenderable : public CObject
{
public:
	// constructor/destructor
	CRenderable();
	virtual ~CRenderable();

	// allow for dynamic creation
	DECLARE_DYNCREATE(CRenderable)

	// sets the object being rendered
	CObject *GetObject();
	virtual void SetObject(CObject *pObject);

	LPD3DXMESH CreateMeshFromSurface(CMesh *pSurface);
	HRESULT CreateVertFromSurface(CMesh *pSurface, 
		LPDIRECT3DVERTEXBUFFER8 *pVertexBuffer,
		LPDIRECT3DINDEXBUFFER8 *pIndexBuffer);

	// color for the renderer (use depends on objects being rendered)
	COLORREF GetColor() const;
	void SetColor(COLORREF);

	// alpha transparency for the transparent part of the object
	double GetAlpha() const;
	void SetAlpha(double alpha);

	// accessors for the renderable's centroid (used for sorting)
	const CVectorD<3>& GetCentroid() const;
	void SetCentroid(const CVectorD<3>& vCentroid);

	// computes the nearest and furthest distances to the
	//	given point
	virtual double GetNearestDistance(const CVectorD<3>& vPoint);
	virtual double GetFurthestDistance(const CVectorD<3>& vPoint);

	// defines the modelview matrix for this renderer
	const CMatrixD<4>& GetModelviewMatrix() const;
	void SetModelviewMatrix(const CMatrixD<4>& m);

	// turns on/off the renderer;
	BOOL IsEnabled() const;
	void SetEnabled(BOOL bEnabled = TRUE);

	// turns on/off the renderer;
	BOOL IsHighlighted() const;
	void SetHighlighted(BOOL bHighlighted = TRUE);

	// invalidates the object -- is a CObservableEvent compatible
	//		member function (ListenerFunction)
	void Invalidate(CObservableEvent *pEvent = NULL, void *pValue = NULL);

	// temporary distance variable, for sorting purposes
	double m_tempDistance;

protected:
	// view class is a friend
	friend CSceneView;

	// the view for this class
	CSceneView *m_pView;

	// helper function to set up the rendering context
	void SetupRenderingContext(LPDIRECT3DDEVICE8 pd3dDev); // CRenderContext *pRC);

	// describes the opaque part of the object
	virtual void DrawOpaque(LPDIRECT3DDEVICE8 pd3dDev); // CRenderContext *pRC);

	// describes the part controlled by the alpha parameter
	virtual void DrawTransparent(LPDIRECT3DDEVICE8 pd3dDev); // CRenderContext *pRC);

	// draw list management functions
	void DrawOpaqueList(LPDIRECT3DDEVICE8 pd3dDev); // CRenderContext *pRC);
	void DrawTransparentList(LPDIRECT3DDEVICE8 pd3dDev); // CRenderContext *pRC);

	// triggers renderable to destroy any buffers it may have allocated
	//		from the render context
	void DestroyBuffers();

	LPDIRECT3DVERTEXBUFFER8 m_pVertexBuffer;

private:
	// the object being described
	CObject *m_pObject;

	// the rendering color
	COLORREF m_color;	

	// the rendering transparency
	double m_alpha;	

	// the centroid of the renderable; used to determine rendering order
	CVectorD<3> m_vCentroid;

	// the modelview matrix for the object
	CMatrixD<4> m_mModelview;

	// the enabled flag
	BOOL m_bEnabled;

	// the draw list for opaque draw compilation
	int m_nDrawListOpaque;

	// the draw list for alpha draw compilation
	int m_nDrawListTransparent;
};

#endif // !defined(RENDERABLE_H)
