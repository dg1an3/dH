//////////////////////////////////////////////////////////////////////
// Renderable.cpp: implementation of the CRenderable class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// OpenGL includes
#include <gl/gl.h>

// render context
#include "RenderContext.h"

// the parent class
#include "SceneView.h"

// class declaration
#include "Renderable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CRenderable::CRenderable
// 
// a renderable is able to describe itself given a rendering context
//////////////////////////////////////////////////////////////////////
CRenderable::CRenderable()
	: m_pObject(NULL),
		m_pView(NULL),
		m_nDrawListOpaque(-1),
		m_nDrawListTransparent(-1),
		m_color(RGB(255, 255, 255)),
		m_alpha(1.0),
		m_bEnabled(TRUE)
{
}

//////////////////////////////////////////////////////////////////////
// CRenderable::~CRenderable
// 
// destroys the renderable
//////////////////////////////////////////////////////////////////////
CRenderable::~CRenderable()
{
}

//////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE
// 
// implements the dynamic create behavior for the renderables
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CRenderable, CObject);


//////////////////////////////////////////////////////////////////////
// CRenderable::GetObject
// 
// returns the object being rendered
//////////////////////////////////////////////////////////////////////
// sets the object being rendered
CObject *CRenderable::GetObject()
{
	return m_pObject;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::SetObject
// 
// sets the object being rendered
//////////////////////////////////////////////////////////////////////
void CRenderable::SetObject(CObject *pObject)
{
	m_pObject = pObject;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::GetColor
// 
// color for the renderer (use depends on objects being rendered)
//////////////////////renderable///////////////////////////////////////////////
COLORREF CRenderable::GetColor() const
{
	return m_color;
}

//////////////////////////////////////////////////////////////////////
// CRenderable::SetColor
// 
// color for the renderable (use depends on objects being rendered)
//////////////////////////////////////////////////////////////////////
void CRenderable::SetColor(COLORREF color)
{
	// set the color
	m_color = color;

	// invalidate to redraw
	Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetAlpha
// 
// alpha transparency for the renderable (use depends on objects being 
//		rendered)
//////////////////////////////////////////////////////////////////////
double CRenderable::GetAlpha() const
{
	return m_alpha;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetAlpha
// 
// alpha transparency for the renderable (use depends on objects being 
//		rendered)
//////////////////////////////////////////////////////////////////////
void CRenderable::SetAlpha(double alpha)
{
	// set the color
	m_alpha = alpha;

	// invalidate to redraw
	Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetCentroid
// 
// returns the centroid of this renderable, used to determine
//		order for rendering
//////////////////////////////////////////////////////////////////////
// accessors for the renderable's centroid (used for sorting)
const CVectorD<3>& CRenderable::GetCentroid() const
{
	return m_vCentroid;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetCentroid
// 
// sets the centroid of this renderable, used to determine
//		order for rendering
//////////////////////////////////////////////////////////////////////
void CRenderable::SetCentroid(const CVectorD<3>& vCentroid)
{
	// set the renderable centroid
	m_vCentroid = vCentroid;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetNearestDistance
// 
// returns the nearest distance from the passed point to the object
//		being rendered.  Default implementation returns distance
//		from the point to the centroid
//////////////////////////////////////////////////////////////////////
double CRenderable::GetNearestDistance(const CVectorD<3>& vPoint)
{
	return (vPoint - GetCentroid()).GetLength();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetFurthestDistance
// 
// returns the furthest distance from the passed point to the object
//		being rendered.  Default implementation returns distance
//		from the point to the centroid
//////////////////////////////////////////////////////////////////////
double CRenderable::GetFurthestDistance(const CVectorD<3>& vPoint)
{
	return (vPoint - GetCentroid()).GetLength();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::GetModelviewMatrix
// 
// the modelview matrix for this renderable
//////////////////////////////////////////////////////////////////////
const CMatrixD<4>& CRenderable::GetModelviewMatrix() const
{
	return m_mModelview;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetModelviewMatrix
// 
// the modelview matrix for this renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::SetModelviewMatrix(const CMatrixD<4>& m)
{
	// set the modelview matrix
	m_mModelview = m;

	// invalidate if we are live
	if (NULL != m_pView)
	{
		// and invalidate the view to redraw without re-rendering
		m_pView->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////
// CRenderable::IsEnabled
// 
// flag to enable/disable the renderable
//////////////////////////////////////////////////////////////////////
BOOL CRenderable::IsEnabled() const
{
	return m_bEnabled;
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetEnabled
// 
// flag to enable/disable the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::SetEnabled(BOOL bEnabled)
{
	// set the enabled flag
	m_bEnabled = bEnabled;

	// invalidate if we are live
	if (NULL != m_pView)
	{
		// and invalidate the view to redraw without re-rendering
		m_pView->Invalidate();
	}
}

///////////////////////////////////////////////////////////////////////
// CRenderable::Invalidate
// 
// triggers re-describing the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::Invalidate(CObservableEvent *pEvent, void *pValue)
{
	// only invalidate if it is attached to a CSceneView
	if (NULL == m_pView)
	{
		return;
	}

	// set the current HGLRC
//	m_pView->MakeCurrentGLRC();

	// if the draw list exists,
	if (-1 != m_nDrawListOpaque)
	{
		// delete it
		glDeleteLists(m_nDrawListOpaque, 1);

		// set it to -1
		m_nDrawListOpaque = -1;
	}

	// if the draw list exists,
	if (-1 != m_nDrawListTransparent)
	{
		// delete it
		glDeleteLists(m_nDrawListTransparent, 1);

		// set it to -1
		m_nDrawListTransparent = -1;
	}

	// invalidate the view, which will trigger re-rendering
	m_pView->Invalidate();
}

///////////////////////////////////////////////////////////////////////
// CRenderable::SetupRenderingContext
// 
// sets up the rendering context
//////////////////////////////////////////////////////////////////////
void CRenderable::SetupRenderingContext(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC)
{
/*	// load the renderer's modelview matrix
	pRC->LoadMatrix(GetModelviewMatrix());

	// set the drawing color
	pRC->Color(GetColor());
	pRC->Alpha(1.0 - sqrt(1.0 - GetAlpha()));
	// glColor(GetColor(), );

	// Set the shading model
	glShadeModel(GL_SMOOTH);

	// enable lighting
	// glEnable(GL_LIGHTING); */
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeOpaque
// 
// call to describe the opaque part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawOpaque(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC)
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeTransparent
// 
// call to describe the transparent part of the renderable
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawTransparent(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC)
{
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeOpaqueDrawList
// 
// forms a call list for the opaque parts, and then calls it
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawOpaqueList(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC)
{
	// only draw if enabled
	if (!IsEnabled())
	{
		return;
	}

	// see if we need to re-create the draw list
	if (-1 == m_nDrawListOpaque)
	{
		// generate a new draw list name
		m_nDrawListOpaque = glGenLists(1);

		// create the list
		glNewList(m_nDrawListOpaque, GL_COMPILE);

		// set up the rendering context
		SetupRenderingContext(pd3dDev); // pRC);

		// now populate the draw list
		DrawOpaque(pd3dDev); // pRC);

		// finish up the list
		glEndList();
	}

	// now actually render the scene
	glCallList(m_nDrawListOpaque);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}

///////////////////////////////////////////////////////////////////////
// CRenderable::DescribeAlphaDrawList
// 
// forms a call list for the alpha parts, and then calls it
//////////////////////////////////////////////////////////////////////
void CRenderable::DrawTransparentList(LPDIRECT3DDEVICE8 pd3dDev) // CRenderContext *pRC)
{
	// only draw if enabled
	if (!IsEnabled())
	{
		return;
	}

	// see if we need to re-create the draw list
	if (-1 == m_nDrawListTransparent)
	{
		// generate a new draw list name
		m_nDrawListTransparent = glGenLists(1);

		// create the list
		glNewList(m_nDrawListTransparent, GL_COMPILE);

		// set up the rendering context
		SetupRenderingContext(pd3dDev); // pRC);

		// now populate the draw list
		DrawTransparent(pd3dDev); // pRC);

		// finish up the list
		glEndList();
	}

	// now actually render the scene
	glCallList(m_nDrawListTransparent);

	// make sure nothing bad happened
	GLenum glerror = glGetError();
	ASSERT(glerror == GL_NO_ERROR);
}


void CRenderable::DestroyBuffers()
{
	m_pVertexBuffer->Release();
}

LPD3DXMESH CRenderable::CreateMeshFromSurface(CMesh *pSurface)
{
	const vector< CTriIndex<int> >& arrSrcIndices = 
		pSurface->LockIndexArray();

	const CMatrixNxM<>& mSrcVertices = 
		pSurface->LockVertexMatrix();
	const CMatrixNxM<>& mSrcNormals = 
		pSurface->LockNormalMatrix();

	LPD3DXMESH pMesh = m_pView->CreateMesh(
		arrSrcIndices.size(),
		mSrcVertices.GetCols(), 
		D3DFVF_CUSTOMVERTEX_POS_NORM );

	unsigned short *pDstIndices = NULL;
	HRESULT hr = pMesh->LockIndexBuffer(0 /*D3DLOCK_DISCARD */, (BYTE **) &pDstIndices); 
	
	// populate the mesh indices
	for (int nAt = 0; nAt < arrSrcIndices.size(); nAt++)
	{
		pDstIndices[nAt*3+0] = arrSrcIndices[nAt][0]-1;
		pDstIndices[nAt*3+1] = arrSrcIndices[nAt][1]-1;
		pDstIndices[nAt*3+2] = arrSrcIndices[nAt][2]-1;
	}
	hr = pMesh->UnlockIndexBuffer();

	CUSTOMVERTEX_POS_NORM *pDstVertices = NULL;
	hr = pMesh->LockVertexBuffer(0, (BYTE **) &pDstVertices);

	// populate the mesh indices
	for (nAt = 0; nAt < mSrcVertices.GetCols(); nAt++)
	{
		pDstVertices[nAt].position.x = mSrcVertices[nAt][0];
		pDstVertices[nAt].position.y = mSrcVertices[nAt][1];
		pDstVertices[nAt].position.z = mSrcVertices[nAt][2];
		pDstVertices[nAt].normal.x = mSrcNormals[nAt][0];
		pDstVertices[nAt].normal.y = mSrcNormals[nAt][1];
		pDstVertices[nAt].normal.z = mSrcNormals[nAt][2];
	}
	hr = pMesh->UnlockVertexBuffer();

	return pMesh;
}

HRESULT CRenderable::CreateVertFromSurface(CMesh *pSurface, 
		LPDIRECT3DVERTEXBUFFER8 *pVertexBuffer,
		LPDIRECT3DINDEXBUFFER8 *pIndexBuffer)
{
	ASSERT(FALSE);

/*	(*pVertexBuffer) = m_pView->CreateVertexBuffer(
		pSurface->GetVertexArray().GetSize() 
			* sizeof(CUSTOMVERTEX_POS_NORM),
		D3DFVF_CUSTOMVERTEX_POS_NORM );
	(*pIndexBuffer) = m_pView->CreateIndexBuffer(
		pSurface->GetIndexArray().GetSize());

	const CArray<int, int>& arrSrcIndices = pSurface->GetIndexArray();

	unsigned short *pDstIndices = NULL;
	HRESULT hr = (*pIndexBuffer)->Lock(0, 0, (BYTE **) &pDstIndices, 0); 
	
	// populate the mesh indices
	for (int nAt = 0; nAt < arrSrcIndices.GetSize(); nAt++)
	{
		pDstIndices[nAt] = arrSrcIndices[nAt]-1;
	}
	hr = (*pIndexBuffer)->Unlock();

	const CArray<CPackedVectorD<3>, CPackedVectorD<3>&>& arrSrcVertices = 
		pSurface->GetVertexArray();
	const CArray<CPackedVectorD<3>, CPackedVectorD<3>&>& arrSrcNormals = 
		pSurface->GetNormalArray();

	CUSTOMVERTEX_POS_NORM *pDstVertices = NULL;
	hr = (*pVertexBuffer)->Lock(0, 0, (BYTE **) &pDstVertices, 0);

	// populate the mesh indices
	for (nAt = 0; nAt < arrSrcVertices.GetSize(); nAt++)
	{
		pDstVertices[nAt].position.x = arrSrcVertices[nAt][0];
		pDstVertices[nAt].position.y = arrSrcVertices[nAt][1];
		pDstVertices[nAt].position.z = arrSrcVertices[nAt][2];
		pDstVertices[nAt].normal.x = arrSrcNormals[nAt][0];
		pDstVertices[nAt].normal.y = arrSrcNormals[nAt][1];
		pDstVertices[nAt].normal.z = arrSrcNormals[nAt][2];
	}
	hr = (*pVertexBuffer)->Unlock();
*/
	return S_OK;
}
