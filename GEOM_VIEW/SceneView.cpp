//////////////////////////////////////////////////////////////////////
// SceneView.cpp: implementation of the CSceneView class.
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// math routines
#include <math.h>

// inline matrix routines
#include <MatrixBase.inl>

// HRESULT processing
#include "Results.h"

// class declaration
#include "SceneView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// default values for timer 
const int SCENEVIEW_TIMER_ID = 11;
const int SCENEVIEW_TIMER_ELAPSED = 50;

// constant for setting alpha to opaque
const double MAX_ALPHA = 0.99;

const DWORD FRONT_FACE = D3DCULL_CCW;
const DWORD BACK_FACE = D3DCULL_CW;

// global D3D interface
LPDIRECT3D8 CSceneView::g_pD3D = NULL; // Used to create the D3DDevice

// global device count
UINT CSceneView::g_nDeviceCount	 = 0;

D3DXCOLOR FromCOLORREF(const COLORREF& color, double alpha = 0.0)
{
	return D3DXCOLOR(GetRValue(color) / 255.0,
		GetGValue(color) / 255.0,
		GetBValue(color) / 255.0,
		alpha );
}

/////////////////////////////////////////////////////////////////////////////
// IMPLEMENT_DYNCREATE -- implements the dynamic creation mechanism for
//		CSceneView
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSceneView, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSceneView::CSceneView
// 
// constructs a new CSceneView.
/////////////////////////////////////////////////////////////////////////////
CSceneView::CSceneView()
	: m_pd3dDevice(NULL),
		m_backgroundColor(RGB(0, 0, 0)),
		m_bLeftDragging(FALSE),
		m_bMiddleDragging(FALSE)
{
	// add this as a listener on the camera
	::AddObserver<CSceneView>(&GetCamera().GetChangeEvent(), 
		this, OnCameraChanged);

}	// CSceneView::CSceneView


/////////////////////////////////////////////////////////////////////////////
// CSceneView::~CSceneView
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
CSceneView::~CSceneView()
{
	// delete the renderables
	for (int nAt = 0; nAt < GetRenderableCount(); nAt++)
	{
		delete m_arrRenderablesNearest[nAt];
	}

	// delete the lights
	for (nAt = 0; nAt < GetLightCount(); nAt++)
	{
		delete m_arrLights[nAt];
	}

	// delete the trackers
	for (nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		delete m_arrLeftTrackers[nAt];
	}

	// delete the middle trackers
	for (nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		delete m_arrMiddleTrackers[nAt];
	}

}	// CSceneView::~CSceneView


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetBackgroundColor
// 
// the background color for the view
/////////////////////////////////////////////////////////////////////////////
COLORREF CSceneView::GetBackgroundColor() const
{
	return m_backgroundColor;

}	// CSceneView::GetBackgroundColor


/////////////////////////////////////////////////////////////////////////////
// CSceneView::SetBackgroundColor
// 
// the background color for the view
/////////////////////////////////////////////////////////////////////////////
void CSceneView::SetBackgroundColor(COLORREF color)
{
	m_backgroundColor = color;

}	// CSceneView::SetBackgroundColor


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetCamera
// 
// returns a reference to the view's camera
/////////////////////////////////////////////////////////////////////////////
CCamera& CSceneView::GetCamera()
{
	return m_camera;

}	// CSceneView::GetCamera


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetRenderableCount
// 
// collection of renderables
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetRenderableCount() const
{
	return m_arrRenderablesNearest.GetSize();

}	// CSceneView::GetRenderableCount


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetRenderableAt
// 
// returns the renderable at the given index
/////////////////////////////////////////////////////////////////////////////
CRenderable *CSceneView::GetRenderableAt(int nAt)
{
	return (CRenderable *) m_arrRenderablesNearest[nAt];

}	// CSceneView::GetRenderableAt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddRenderable
// 
// adds a new renderable to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddRenderable(CRenderable *pRenderable) 
{ 
	// set this as the scene view
	pRenderable->m_pView = this;

	// add to both renderable arrays
	m_arrRenderablesFurthest.Add(pRenderable);
	return m_arrRenderablesNearest.Add(pRenderable);

}	// CSceneView::AddRenderable


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLightCount
// 
// collection of lights for the view
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetLightCount() const 
{ 
	return m_arrLights.GetSize();

}	// CSceneView::GetLightCount


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLightAt
// 
// returns the light at the given index
/////////////////////////////////////////////////////////////////////////////
CLight *CSceneView::GetLightAt(int nAt)
{ 
	return (CLight *) m_arrLights[nAt];

}	// CSceneView::GetLightAt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddLight
// 
// adds a light to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddLight(CLight *pLight) 
{ 
	return m_arrLights.Add(pLight);

}	// CSceneView::AddLight


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLeftTrackerCount
// 
// collection of trackers for left button
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetLeftTrackerCount() const 
{ 
	return m_arrLeftTrackers.GetSize();

}	// CSceneView::GetLeftTrackerCount


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetLeftTrackerAt
// 
// returns the left tracker at the given index
/////////////////////////////////////////////////////////////////////////////
CTracker *CSceneView::GetLeftTrackerAt(int nAt) 
{ 
	return (CTracker *) m_arrLeftTrackers[nAt];

}	// CSceneView::GetLeftTrackerAt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddLeftTracker
// 
// adds a new left tracker to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddLeftTracker(CTracker *pRenderer) 
{ 
	return m_arrLeftTrackers.Add(pRenderer);

}	// CSceneView::AddLeftTracker


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetMiddleTrackerCount
// 
// collection of trackers for middle button
/////////////////////////////////////////////////////////////////////////////
int CSceneView::GetMiddleTrackerCount() const 
{ 
	return m_arrMiddleTrackers.GetSize();

}	// CSceneView::GetMiddleTrackerCount


/////////////////////////////////////////////////////////////////////////////
// CSceneView::GetMiddleTrackerAt
// 
// returns one of the current middle trackers
/////////////////////////////////////////////////////////////////////////////
CTracker *CSceneView::GetMiddleTrackerAt(int nAt) 
{ 
	return (CTracker *) m_arrMiddleTrackers[nAt];

}	// CSceneView::GetMiddleTrackerAt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::AddMiddleTracker
// 
// adds a new middle tracker to the sceneview
/////////////////////////////////////////////////////////////////////////////
int CSceneView::AddMiddleTracker(CTracker *pRenderer)
{
	return m_arrMiddleTrackers.Add(pRenderer);

}	// CSceneView::AddMiddleTracker


/////////////////////////////////////////////////////////////////////////////
// CSceneView diagnostics
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////
// CSceneView::AssertValid
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
void CSceneView::AssertValid() const
{
	CWnd::AssertValid();

}	// CSceneView::AssertValid


/////////////////////////////////////////////////////////////////////////////
// CSceneView::Dump
// 
// destroys the CSceneView
/////////////////////////////////////////////////////////////////////////////
void CSceneView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);

}	// CSceneView::Dump
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSceneView::ModelPtFromWndPt
// 
// converts a point in viewport coordinates to a 3-d point using the
//		current projection matrix
/////////////////////////////////////////////////////////////////////////////
CVectorD<3> CSceneView::ModelPtFromWndPt(CPoint wndPt)
{
	return ModelPtFromWndPt(wndPt, 
		GetCamera().GetProjection() * GetCamera().GetXform());

}	// CSceneView::ModelPtFromWndPt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::ModelPtFromWndPt
// 
// converts a point in viewport coordinates to a 3-d point given 
//		a projection matrix
/////////////////////////////////////////////////////////////////////////////
CVectorD<3> CSceneView::ModelPtFromWndPt(CPoint wndPt, 
										 const CMatrixD<4>& mProj)
{
	CVectorD<4> vModelPt(ToHomogeneous<3, REAL>(wndPt)); 

	D3DVIEWPORT8 vwport;
	HRESULT hr = m_pd3dDevice->GetViewport(&vwport);

	// adjust for the viewport
	vModelPt[0] -= (vwport.X + vwport.Width / 2);
	vModelPt[0] /= (REAL) vwport.Width / 2.0;

	vModelPt[1] -= (vwport.Y + vwport.Height / 2);
	vModelPt[1] /= (REAL) vwport.Height / -2.0;

	vModelPt[2] =  (REAL) vwport.MaxZ;

	// inverse project
	CMatrixD<4> mInvProj = mProj;
	mInvProj.Invert();
	vModelPt = mInvProj * vModelPt;

	return FromHomogeneous<3, REAL>(vModelPt);

}	// CSceneView::ModelPtFromWndPt


/////////////////////////////////////////////////////////////////////////////
// CSceneView::CreateVertexBuffer
// 
// creates a vertex buffer for rendering
/////////////////////////////////////////////////////////////////////////////
LPDIRECT3DVERTEXBUFFER8 CSceneView::CreateVertexBuffer(UINT nLength, DWORD dwFVF)
{
	// initialize a buffer pointer
	LPDIRECT3DVERTEXBUFFER8 lpVertexBuffer;

	// only create if the device exists
	if (NULL != m_pd3dDevice)
	{
		ASSERT_HRESULT( m_pd3dDevice->CreateVertexBuffer(nLength, 0, dwFVF, 
			D3DPOOL_DEFAULT, &lpVertexBuffer) );
	}

	// return the new buffer
	return lpVertexBuffer;

}	// CSceneView::CreateVertexBuffer


/////////////////////////////////////////////////////////////////////////////
// CSceneView::CreateIndexBuffer
// 
// creates an index buffer for rendering
/////////////////////////////////////////////////////////////////////////////
LPDIRECT3DINDEXBUFFER8 CSceneView::CreateIndexBuffer(UINT nLength)
{
	// initialize a buffer pointer
	LPDIRECT3DINDEXBUFFER8 lpIndexBuffer;

	// only create if the device exists
	if (NULL != m_pd3dDevice)
	{
		ASSERT_HRESULT( m_pd3dDevice->CreateIndexBuffer(
			nLength * sizeof(UINT),		// size, in bytes
			0,							// usage
			D3DFMT_INDEX16,				// format
			D3DPOOL_DEFAULT,			// pool
			&lpIndexBuffer				// return
		) );
	}

	// return the new buffer
	return lpIndexBuffer;

}	// CSceneView::CreateIndexBuffer


/////////////////////////////////////////////////////////////////////////////
// CSceneView::CreateMesh
// 
// creates a mesh for rendering
/////////////////////////////////////////////////////////////////////////////
LPD3DXMESH CSceneView::CreateMesh(UINT nFaces, UINT nVertices, DWORD dwFVF)
{
	// initialize a mesh pointer
	LPD3DXMESH lpMesh = NULL;

	// only create if the device exists
	if (NULL != m_pd3dDevice)
	{
		// create the mesh
		ASSERT_HRESULT( D3DXCreateMeshFVF(nFaces, nVertices, 
			D3DXMESH_MANAGED, // | D3DXMESH_32BIT, 
			dwFVF,
			m_pd3dDevice, &lpMesh) );
	}

	// return the new mesh
	return lpMesh;

}	// CSceneView::CreateMesh


/////////////////////////////////////////////////////////////////////////////
// CSceneView::PreCreateWindow
// 
// ensures that the necessary window styles are applied to this window 
//		during creation
/////////////////////////////////////////////////////////////////////////////
BOOL CSceneView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// An OpenGL window must be created with the following flags and must not
	// include CS_PARENTDC for the class style. Refer to SetPixelFormat
	// documentation in the "Comments" section for further information.
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CWnd::PreCreateWindow(cs);

}	// CSceneView::PreCreateWindow


/////////////////////////////////////////////////////////////////////////////
// CSceneView::SetupD3D
// 
// initializes or re-initializes the D3D device
/////////////////////////////////////////////////////////////////////////////
HRESULT CSceneView::SetupD3D()
{
    // create the D3D object.
    if( NULL == g_pD3D)
	{
		g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
	}

    // get the current desktop display mode, so we can set up a back
    //		buffer of the same format
    D3DDISPLAYMODE d3ddm;
    CHECK_HRESULT( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, 
		&d3ddm) );

	// set up the presentation parameters for D3D
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// if the device has not yet been created,
	if (!m_pd3dDevice)
	{
		// default behavior is software vertex processing
		DWORD dwBehaviorFlag = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

		// check the device capabilities
		D3DCAPS8 d3dcaps;
		ZeroMemory( &d3dcaps, sizeof(d3dcaps) );
		CHECK_HRESULT( g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, 
			&d3dcaps) );

		if (d3dcaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS != 0
			&& d3dcaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN  != 0)
		{
			dwBehaviorFlag = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}

		// create the D3DDevice
		CHECK_HRESULT( g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			m_hWnd, 
			dwBehaviorFlag,
			&d3dpp, 
			&m_pd3dDevice) );

		// add to the global device count
		g_nDeviceCount++;
	}
	else 
	{
		// re-initialize
		CHECK_HRESULT( m_pd3dDevice->Reset(&d3dpp) );
	}

	return S_OK;

}	// CSceneView::SetupD3D


/////////////////////////////////////////////////////////////////////////////
// CompareRenderablesByDist
// 
// sorting helper function
/////////////////////////////////////////////////////////////////////////////
int CompareRenderablesByDist(const void *pElem1, const void *pElem2)
{
	// extract the two renderable pointers
	CRenderable *pRenderable1 = (CRenderable *) pElem1;
	CRenderable *pRenderable2 = (CRenderable *) pElem2;

	// compare using the distance
	return (pRenderable1->m_tempDistance > pRenderable2->m_tempDistance) 
		? 1 : -1;

}	// CompareRenderablesByDist


/////////////////////////////////////////////////////////////////////////////
// CSceneView::SortRenderables
//
// sorts the renderables from the furthest to the nearest based on the
//		current camera position
/////////////////////////////////////////////////////////////////////////////
void CSceneView::SortRenderables(CObArray *pArray, 
		double (CRenderable::*DistFunc)(const CVectorD<3>& vPoint))
{
	// current camera position
	CMatrixD<4> mInvXform = GetCamera().GetXform();
	mInvXform.Invert();
	CVectorD<3> vCameraPos = FromHG<3, REAL>(mInvXform
		* ToHG<3, REAL>(CVectorD<3>(0.0, 0.0, 0.0)));

	// compute the nearest distances
	for (int nAt = 0; nAt < pArray->GetSize(); nAt++)
	{
		// get the renderable
		CRenderable *pRenderable = (CRenderable *) pArray->GetAt(nAt);

		// add to the array
		pRenderable->m_tempDistance = (pRenderable->*DistFunc)(vCameraPos);
	}

	// now sort the renderables
	qsort((void *) pArray->GetData(), pArray->GetSize(), sizeof(CObject *),
		CompareRenderablesByDist);

}	// CSceneView::SortRenderables


//////////////////////////////////////////////////////////////////////////////
// CSceneView::OnCameraChanged
// 
// change handler for camera changes
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnCameraChanged(CObservableEvent *, void *)
{
	// invalidate the window to redraw
	Invalidate();

}	// CSceneView::OnCameraChanged



/////////////////////////////////////////////////////////////////////////////
// CSceneView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSceneView, CWnd)
	//{{AFX_MSG_MAP(CSceneView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP() 
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP() 
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneView message handlers

/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnCreate
// 
// renders the buffer, and swaps back and front buffer
/////////////////////////////////////////////////////////////////////////////
int CSceneView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	// set up some lights
	CLight *pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(255, 255, 255));
	pNewLight->SetPosition(CVectorD<3>(-5.0, -5.0, 3.0));
	AddLight(pNewLight);

	pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(128, 128, 128));
	pNewLight->SetPosition(CVectorD<3>(5.0, -5.0, 3.0));
	AddLight(pNewLight);

	pNewLight = new CLight();
	pNewLight->SetDiffuseColor(RGB(128, 128, 128));
	pNewLight->SetPosition(CVectorD<3>(0.0, 5.0, 3.0));
	AddLight(pNewLight);

	// set a timer event to occur
	SetTimer(SCENEVIEW_TIMER_ID, SCENEVIEW_TIMER_ELAPSED, NULL);

	return 0;

}	// CSceneView::OnCreate


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnDestroy
// 
// removes the rendering context when the window is destroyed
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnDestroy() 
{
	if (NULL != m_pd3dDevice)
	{
		// release the device
		m_pd3dDevice->Release();

		// decrement global instance count
		g_nDeviceCount--;

		// shall we release the Direct3D object?
		if (0 == g_nDeviceCount
			&& NULL != g_pD3D)
		{
			g_pD3D->Release();
			g_pD3D = NULL;
		}
	}

	// call base class destroy
	CWnd::OnDestroy();

}	// CSceneView::OnDestroy


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnEraseBkgnd
// 
// over-ride to do nothing, because double-buffering pre-empts the need
/////////////////////////////////////////////////////////////////////////////
BOOL CSceneView::OnEraseBkgnd(CDC* pDC) 
{
	// over-ride default to prevent flicker
	return TRUE;

}	// CSceneView::OnEraseBkgnd


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnSize
// 
// adjusts the camera aspect ratio for the new size
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnSize(UINT nType, int cx, int cy) 
{
	// let the base class have a chance
	CWnd::OnSize(nType, cx, cy);

	// calculate the aspect ratio for the camera
	if (cx > 0 
		&& cy > 0)
	{
		GetCamera().SetAspectRatio((double) cx / (double) cy);

		// re-initialize the D3D, only if the device has been created
		if (NULL != m_pd3dDevice)
		{
			ASSERT_HRESULT( SetupD3D() );
		}
	}
	else
	{
		GetCamera().SetAspectRatio(1.0);
	}
	
}	// CSceneView::OnSize


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnPaint
// 
// renders the buffer, and swaps back and front buffer
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnPaint()
{
	if ( NULL == m_pd3dDevice )
	{
		// initialize Direct3D
		ASSERT_HRESULT( SetupD3D() );
	}

    // Clear the backbuffer and the zbuffer
    ASSERT_HRESULT( m_pd3dDevice->Clear(0, NULL, 
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(64,64,96), 1.0f, 0) );

	// set the viewing transform
	ASSERT_HRESULT( m_pd3dDevice->SetTransform( D3DTS_VIEW, 
			&(D3DMATRIX) GetCamera().GetXform()) );

	// set the projection
    ASSERT_HRESULT( m_pd3dDevice->SetTransform( D3DTS_PROJECTION, 
		&(D3DMATRIX) GetCamera().GetProjection()) );
	
	// set up the lights
	for (int nAtLight = 0; nAtLight < GetLightCount(); nAtLight++)
	{
		ASSERT_HRESULT( m_pd3dDevice->SetLight(nAtLight, 
			(D3DLIGHT8 *) GetLightAt(nAtLight)) );
		ASSERT_HRESULT( m_pd3dDevice->LightEnable(nAtLight, TRUE) );
	}

	// enable lighting
    ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE) );

    // turn on some ambient light.
    ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00606060) );

	// sort the renderables from furthest to nearest
	SortRenderables(&m_arrRenderablesNearest, 
		CRenderable::GetNearestDistance);
	SortRenderables(&m_arrRenderablesFurthest, 
		CRenderable::GetFurthestDistance);

	// Turn on the zbuffer for all passes
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) );

    // Begin the scene
    ASSERT_HRESULT( m_pd3dDevice->BeginScene() );

	//////////////////////////////////////////////////////////////////////////
	// render opaque objects

	// turn off alpha blending for opaque objects
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 
		FALSE) );

	// Turn off culling
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, 
		D3DCULL_NONE) );

	// for each renderer, 
	for (int nAt = m_arrRenderablesNearest.GetSize()-1; nAt >= 0; nAt--)
	{
		CRenderable *pRenderable = (CRenderable *) m_arrRenderablesNearest[nAt];

		if (!pRenderable->IsEnabled())
		{
			continue;
		}

		// Set up a material. The material here just has the diffuse and ambient
		// colors set to yellow. Note that only one material can be used at a time.
		D3DMATERIAL8 mtrl;
		COLORREF color = pRenderable->GetColor();
		ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
		mtrl.Diffuse = mtrl.Ambient = 
			FromCOLORREF(color, pRenderable->GetAlpha());
		ASSERT_HRESULT( m_pd3dDevice->SetMaterial(&mtrl) );

		// draw its scene
		pRenderable->DrawOpaque(m_pd3dDevice);

		// also draw the alpha part of the renderable, 
		//		if alpha is greater than the max
		if (pRenderable->GetAlpha() > MAX_ALPHA)
		{
			// draw its scene
			pRenderable->DrawTransparent(m_pd3dDevice);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// render alpha-blended objects

	// enable blending
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 
		TRUE) );
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, 
		D3DBLEND_SRCALPHA) );
	ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, 
		D3DBLEND_INVSRCALPHA) );

	// arrays holding parameters for the four passes
	DWORD arrCullFace[] =			
		{ FRONT_FACE, FRONT_FACE, BACK_FACE, BACK_FACE };
	BOOLEAN arrZWriteEnable[] =		
		{ FALSE, TRUE, FALSE, TRUE };

	for (int nAtPass = 0; nAtPass < 4; nAtPass++)
	{
		// use the furthest array for back-faced rendering
		CObArray *pArray = (arrCullFace[nAtPass] == FRONT_FACE)
			? &m_arrRenderablesFurthest : &m_arrRenderablesNearest;

		// set face culling
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, 
			arrCullFace[nAtPass]) );

		// disable lighting if back faces are displayed
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE) );
			// BACK_FACE == arrCullFace[nAtPass] );
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_COLORVERTEX,
			FALSE ) );
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,
			D3DMCS_MATERIAL ) );
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,
			D3DMCS_MATERIAL ) );
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,
			D3DMCS_MATERIAL ) );

		// set depth buffer writing
		ASSERT_HRESULT( m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, 
			arrZWriteEnable[nAtPass]) );

		// for each renderer, 
		for (nAt = 0; nAt < pArray->GetSize(); nAt++)
		{
			// get the renderable
			CRenderable *pRenderable = (CRenderable *) pArray->GetAt(nAt);

			if (pRenderable->IsEnabled()
				&& pRenderable->GetAlpha() <= MAX_ALPHA)
			{
				// Set up a material. The material here just has the diffuse and ambient
				// colors set to yellow. Note that only one material can be used at a time.
				D3DMATERIAL8 mtrl;
				COLORREF color = pRenderable->GetColor();
				ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
				mtrl.Diffuse = mtrl.Ambient = 
					FromCOLORREF(color, pRenderable->GetAlpha());
				ASSERT_HRESULT(m_pd3dDevice->SetMaterial(&mtrl));

				// draw its scene
				pRenderable->DrawTransparent(m_pd3dDevice);
			}
		}
	}

    // End the scene
    ASSERT_HRESULT(m_pd3dDevice->EndScene());

    // Present the backbuffer contents to the display
    ASSERT_HRESULT(m_pd3dDevice->Present(NULL, NULL, NULL, NULL));

	// make sure the window is updated
	ValidateRect(NULL);

}	// CSceneView::OnPaint


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnLButtonDown
// 
// handles a left-button down event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bLeftDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			// found one
			GetLeftTrackerAt(nAt)->OnButtonDown(nFlags, point);
			break;
		}
	}

	// set the capture for this window
	SetCapture();

}	// CSceneView::OnLButtonDown


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnLButtonUp
// 
// handles a left-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnLButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bLeftDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			GetLeftTrackerAt(nAt)->OnButtonUp(nFlags, point);
			break;
		}
	}

	// release the capture
	::ReleaseCapture();

}	// CSceneView::OnLButtonUp


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMButtonDown
// 
// handles a middle-button down event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnMButtonDown(nFlags, point);

	// set the dragging flag to TRUE
	m_bMiddleDragging = TRUE;

	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnButtonDown(nFlags, point);
			break;
		}
	}

	// set the capture for this window
	SetCapture();

}	// CSceneView::OnMButtonDown


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMButtonUp
// 
// handles a middle-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// handle base class
	CWnd::OnMButtonUp(nFlags, point);

	// dragging is over -- clear flag
	m_bMiddleDragging = FALSE;	
	
	// find the tracker which contains the point
	for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnButtonUp(nFlags, point);
			break;
		}
	}

	// release the capture
	::ReleaseCapture();

}	// CSceneView::OnMButtonUp


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnMouseMove
// 
// handles a middle-button up event by passing to the appropriate
//		tracker
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// process base class move event
	CWnd::OnMouseMove(nFlags, point);

	// see if we are in a left-drag
	if (m_bLeftDragging)
	{
		for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
		{
			if (GetLeftTrackerAt(nAt)->IsInside(point))
			{
				GetLeftTrackerAt(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}

		// return if none found
		return;
	}

	// see if we are in a middle drag
	if (m_bMiddleDragging)
	{
		for (int nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
		{
			if (GetMiddleTrackerAt(nAt)->IsInside(point))
			{
				GetMiddleTrackerAt(nAt)->OnMouseDrag(nFlags, point);
				return;
			}
		}

		// return if none found
		return;
	}

	// process regular left move
	for (int nAt = 0; nAt < GetLeftTrackerCount(); nAt++)
	{
		if (GetLeftTrackerAt(nAt)->IsInside(point))
		{
			GetLeftTrackerAt(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}

	// process regular middle move
	for (nAt = 0; nAt < GetMiddleTrackerCount(); nAt++)
	{
		if (GetMiddleTrackerAt(nAt)->IsInside(point))
		{
			GetMiddleTrackerAt(nAt)->OnMouseMove(nFlags, point);
			return;
		}
	}

}	// CSceneView::OnMouseMove


/////////////////////////////////////////////////////////////////////////////
// CSceneView::OnTimer
// 
// handles the timer event
/////////////////////////////////////////////////////////////////////////////
void CSceneView::OnTimer(UINT nIDEvent) 
{
	if (SCENEVIEW_TIMER_ID == nIDEvent)
	{
		// TODO: 
		// iterate over the trackers and invoke their OnTimer event
	}

	CWnd::OnTimer(nIDEvent);

}	// CSceneView::OnTimer
