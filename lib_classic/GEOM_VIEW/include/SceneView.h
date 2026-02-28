//////////////////////////////////////////////////////////////////////
// SceneView.h: declaration of the CSceneView class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(SCENEVIEW_H)
#define SCENEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <d3dx8.h>
#include <mmsystem.h>

// matrix include file
#include <MatrixD.h>

// helper classes
#include "Renderable.h"
#include "Camera.h"
#include "Light.h"
#include "Tracker.h"

//////////////////////////////////////////////////////////////////////
// class CSceneView
//
// represents a window with an OpenGL rendering context.
//////////////////////////////////////////////////////////////////////
class CSceneView : public CWnd
{
public:
	// constructor/destructor
	CSceneView();           // protected constructor used by dynamic creation
	virtual ~CSceneView();

	// make sure we can create this window dynamically (for splitter window)
	DECLARE_DYNCREATE(CSceneView)

// Attributes
public:
	
	// the background color for the view
	COLORREF GetBackgroundColor() const;
	void SetBackgroundColor(COLORREF color);

	// the view's camera
	CCamera& GetCamera();

	// collection of renderers
	int GetRenderableCount() const;
	CRenderable *GetRenderableAt(int nAt);
	int AddRenderable(CRenderable *pRenderer);

	// collection of lights for the view
	int GetLightCount() const;
	CLight *GetLightAt(int nAt);
	int AddLight(CLight *pRenderer);

	// collection of trackers for left button
	int GetLeftTrackerCount() const;
	CTracker *GetLeftTrackerAt(int nAt);
	int AddLeftTracker(CTracker *pRenderer);

	// collection of trackers for middle button
	int GetMiddleTrackerCount() const;
	CTracker *GetMiddleTrackerAt(int nAt);
	int AddMiddleTracker(CTracker *pRenderer);

// Operations
public:
	// compute the model point from a window coordinate
	CVectorD<3> ModelPtFromWndPt(CPoint wndPt);

	// compute the model point from a window coordinate, using a different
	//		projection matrix
	CVectorD<3> ModelPtFromWndPt(CPoint wndPt, const CMatrixD<4>& mProj);

	// create vertex buffers and meshed
	LPDIRECT3DVERTEXBUFFER8 CreateVertexBuffer(UINT nLength, DWORD dwFVF);
	LPDIRECT3DINDEXBUFFER8 CreateIndexBuffer(UINT nLength);
	LPD3DXMESH CreateMesh(UINT nFaces, UINT nVertices, DWORD dwFVF);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// function to initialize / re-initialize the D3D device
	HRESULT SetupD3D();

	// sorts the renderables from furthest to nearest, based on current camera
	//		position
	void SortRenderables(CObArray *pArray, 
		double (CRenderable::*DistFunc)(const CVectorD<3>& vPoint));

	// change handler for camera changes
	void OnCameraChanged(CObservableEvent *, void *);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSceneView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the background color for the view
	COLORREF m_backgroundColor;

	// the renderables sorted from back-to-front
	//		based on nearest point
	CObArray m_arrRenderablesNearest;	// CRenderables

	// the renderables sorted from back-to-front
	//		based on nearest point
	CObArray m_arrRenderablesFurthest;	// CRenderables

	// the view camera
	CCamera m_camera;

	// the array of lights
	CObArray m_arrLights;			// CLight

	// the array of left button trackers
	CObArray m_arrLeftTrackers;		// CTracker

	// the array of middle button trackers
	CObArray m_arrMiddleTrackers;	// CTracker

	// flag to indicate that the dragging is occurring
	BOOL m_bLeftDragging;
	BOOL m_bMiddleDragging;

	// class global Direct3D object and D3D device count 
	static LPDIRECT3D8 g_pD3D;
	static UINT g_nDeviceCount;

	// D3D device
	LPDIRECT3DDEVICE8 m_pd3dDevice; // Our rendering device
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(SCENEVIEW_H)
