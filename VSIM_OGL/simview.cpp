// SimView1.cpp : implementation file
//

#include "stdafx.h"
#include "VSIM_OGL.h"
#include "SimView.h"

#include <math.h>

#include "gl/gl.h"
#include "glMatrixVector.h"

#include "RotateTracker.h"
#include "ZoomTracker.h"
#include "BeamRenderable.h"
#include "SurfaceRenderable.h"
#include "MachineRenderable.h"
#include "DRRRenderable.h"

#include "BeamParamPosCtrl.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// function CreateRotate
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
static CMatrix<4> CreateRotateHG(const double& theta, 
							   const CVector<3>& vAxis)
{
	// start with an identity matrix
	CMatrix<3> mRotate = CreateRotate(theta, vAxis);

	CMatrix<4> mRotateHG(mRotate);

	return mRotateHG;
}

//////////////////////////////////////////////////////////////////////
// function CreateScale
//
// creates a rotation matrix given an angle and an axis of rotation
//////////////////////////////////////////////////////////////////////
static CMatrix<4> CreateScaleHG(const CVector<3>& vScale)
{
	// start with an identity matrix
	CMatrix<3> mScale = CreateScale(vScale);

	CMatrix<4> mScaleHG(mScale);

	return mScaleHG;
}


/////////////////////////////////////////////////////////////////////////////
// CSimView

IMPLEMENT_DYNCREATE(CSimView, CView)

CSimView::CSimView()
	: m_pCurrentBeam(NULL),
		m_pBeamRenderable(NULL),
		m_pSurfaceRenderable(NULL),
		m_pDRRRenderable(NULL),
		m_bPatientEnabled(TRUE),
		m_bWireFrame(FALSE),
		m_bColorWash(FALSE)
{
}

CSimView::~CSimView()
{
}


BEGIN_MESSAGE_MAP(CSimView, CView)
	//{{AFX_MSG_MAP(CSimView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_BEAM, OnViewBeam)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BEAM, OnUpdateViewBeam)
	ON_COMMAND(ID_VIEW_LIGHTPATCH, OnViewLightfield)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTPATCH, OnUpdateViewLightfield)
	ON_COMMAND(ID_VIEW_WIREFRAME, OnViewWireframe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WIREFRAME, OnUpdateViewWireframe)
	ON_COMMAND(ID_VIEW_COLORWASH, OnViewColorwash)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLORWASH, OnUpdateViewColorwash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimView drawing

void CSimView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	CRect rect;
	GetClientRect(&rect);

	CBrush brush(RGB(0, 0, 255));
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	pDC->Rectangle(rect);
	pDC->SelectObject(pOldBrush);
}

void CSimView::OnChange(CObservableObject *pFromObject, void *pOldValue)
{
	// if (pFromObject //->GetParent() 
	//		== &m_wndBEV.GetCamera())
	{
		CMatrix<4> mBeamToPatientXform(m_wndBEV.GetCamera().GetXform());
		mBeamToPatientXform.Invert();

		m_pCurrentBeam->SetBeamToPatientXform(mBeamToPatientXform);
		m_wndREV.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	if (pFromObject // ->GetParent() 
		!= &m_wndBEV.GetCamera().GetChangeEvent())
	{
		CBeam *pBeam = (CBeam *)pFromObject->GetParent();
		SetBEVPerspective(*pBeam);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSimView diagnostics

#ifdef _DEBUG
void CSimView::AssertValid() const
{
	CView::AssertValid();
}

void CSimView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPlan* CSimView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPlan)));
	return (CPlan*)m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimView message handlers

CRotateTracker *pRotateTracker;

int CSimView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndREV.Create(NULL, NULL, lpCreateStruct->style, CRect(0, 0, 0, 0), this, 0))
		return -1;

	m_wndREV.GetCamera().SetClippingPlanes(1.0, 200.0);
	m_wndREV.GetCamera().SetFieldOfView(20.0f);
	m_wndREV.GetCamera().SetPhi(PI);
	m_wndREV.GetCamera().SetTheta(0.0); // PI / 2.0);

	TRACE_MATRIX("CSimView::OnCreate Projection Matrix", 
		m_wndREV.GetCamera().GetProjection());

	pRotateTracker = new CRotateTracker(&m_wndREV);
	m_wndREV.AddLeftTracker(pRotateTracker);
	m_wndREV.AddMiddleTracker(new CZoomTracker(&m_wndREV));

	if (!m_wndBEV.Create(NULL, NULL, lpCreateStruct->style, CRect(0, 0, 0, 0), this, 0))
		return -1;
	
	m_wndBEV.GetCamera().SetClippingPlanes(1.0, 200.0);
	m_wndBEV.GetCamera().SetFieldOfView(20.0f);
	CRotateTracker *pBEVRotateTracker = new CRotateTracker(&m_wndBEV);
	m_wndBEV.AddLeftTracker(pBEVRotateTracker);

	return 0;
}

#define BORDER 3

void CSimView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	cx -= BORDER;
	cy -= BORDER;

	m_wndREV.MoveWindow(0 + BORDER, 0 + BORDER, 
		cx // 2 
			- BORDER, cy - BORDER);	

	m_wndBEV.MoveWindow(cx / 2 + BORDER, 0 + BORDER, 
		cx / 2 - BORDER, cy - BORDER);	

	if (GetDocument()->GetBeamCount() > 0)
	{
		CBeam& beam = *GetDocument()->GetBeamAt(0);
		SetBEVPerspective(beam);
	}
}

COLORREF arrColors[] = 
{   RGB(192, 192, 192),
	RGB(240,   0,   0),
	RGB(  0, 240,   0),
	RGB(  0,   0, 255),
	RGB(255,   0, 255),
	RGB(  0, 255, 255),
	RGB(240, 240,   0),
	RGB(240,   0,   0),
	RGB(  0, 240,   0),
	RGB(  0,   0, 255),
	RGB(255,   0, 255),
	RGB(  0, 255, 255),
	RGB(240, 240,   0), 
};

UINT arrIconResourceIDs[] =
{
	IDB_CONTOUR_GREY,
	IDB_CONTOUR_RED,
	IDB_CONTOUR_GREEN,
	IDB_CONTOUR_BLUE,
	IDB_CONTOUR_MAGENTA,
	IDB_CONTOUR_CYAN,
	IDB_CONTOUR_YELLOW,
	IDB_CONTOUR_RED,
	IDB_CONTOUR_GREEN,
	IDB_CONTOUR_BLUE,
	IDB_CONTOUR_MAGENTA,
	IDB_CONTOUR_CYAN,
	IDB_CONTOUR_YELLOW,
};

void CSimView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (GetDocument() == NULL)
		return;

	CBeam *pMainBeam = NULL;
	if (GetDocument()->GetBeamCount() > 0)
	{
		CBeam& beam = *GetDocument()->GetBeamAt(0);
		pMainBeam = &beam;

		beam.SetName("AP");

		m_pCurrentBeam = &beam;
		m_pDRRRenderable = new CDRRRenderable(&m_wndBEV);
		m_pDRRRenderable->m_pVolume = &GetDocument()->GetSeries()->volume;
		m_pDRRRenderable->m_mVolumeTransform = GetDocument()->GetSeries()->m_volumeTransform;
		m_pDRRRenderable->SetEnabled(FALSE);
		m_wndBEV.AddRenderable(m_pDRRRenderable);

		CBeamRenderable *pBEVBeamRenderable = new CBeamRenderable(&m_wndBEV);
		pBEVBeamRenderable->SetBeam(&beam);
		pBEVBeamRenderable->m_bGraticuleEnabled = TRUE;
		m_wndBEV.AddRenderable(pBEVBeamRenderable);

		beam.GetChangeEvent().AddObserver(this, (ChangeFunction) OnChange);
	}

// #define ADD_EXTRA_BEAMS
#ifdef ADD_EXTRA_BEAMS
	// add some extra beams
	CBeam *pNewBeam = new CBeam();
	pNewBeam->name.Set("PA");
	GetDocument()->beams.Add(pNewBeam);

	pNewBeam = new CBeam();
	pNewBeam->name.Set("RL");
	GetDocument()->beams.Add(pNewBeam);

	pNewBeam = new CBeam();
	pNewBeam->name.Set("LR");
	GetDocument()->beams.Add(pNewBeam);
#endif

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CObjectExplorer *pExplorer = &pFrame->m_wndExplorerCtrl.m_ExplorerCtrl;
	CBeamParamPosCtrl *pPosCtrl = &pFrame->m_wndPosCtrl;

#ifdef USE_HIERARCHY
	CObjectTreeItem *pPatientItem = new CObjectTreeItem();
	pPatientItem->label.Set("Patient: Doe, John");
	pPatientItem->Create(pExplorer);
#endif

	if (GetDocument()->GetSeries() == NULL)
		return;

	CSeries *pSeries = GetDocument()->GetSeries();

	CObjectTreeItem *pSeriesItem = new CObjectTreeItem();

#ifdef USE_HIERARCHY
	pSeriesItem->m_strLabel = "Series: " + pSeries->GetFileRoot();
	pPatientItem->children.Add(pSeriesItem);
#else
	pSeriesItem->m_strLabel = "Structures";
	pSeriesItem->Create(pExplorer);
#endif

	for (int nAtSurf = 0; nAtSurf < pSeries->m_arrStructures.GetSize(); nAtSurf++)
	{
		CSurface *pSurface = (CSurface *) GetDocument()->GetSeries()->m_arrStructures.GetAt(nAtSurf);

		// add the surface to the object explorer
		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->m_pObject = pSurface;

		pNewItem->m_nImageResourceID = arrIconResourceIDs[nAtSurf % 13];
		pNewItem->m_nSelectedImageResourceID = arrIconResourceIDs[nAtSurf % 13];

		pNewItem->m_bChecked = TRUE;

		pSeriesItem->m_arrChildren.Add(pNewItem);

		CSurfaceRenderable *pSurfaceRenderable = new CSurfaceRenderable(&m_wndREV);
		// pSurfaceRenderable->m_bWireFrame.SyncTo(&isWireFrame);

		pSurfaceRenderable->SetColor(arrColors[nAtSurf]);
		pSurfaceRenderable->SetSurface(pSurface);
		// pSurfaceRenderable->tableOffset.SyncTo(&pMainBeam->tableOffset);
		// pSurfaceRenderable->couchAngle.SyncTo(&pMainBeam->couchAngle);
		m_wndREV.AddRenderable(pSurfaceRenderable);

		// pSurfaceRenderable->isEnabled.SyncTo(&pNewItem->isChecked);

		if (nAtSurf == 0)	// patient surface
		{
			m_pSurfaceRenderable = pSurfaceRenderable;
			m_pSurfaceRenderable->m_bShowBoundsSurface = TRUE;

			// compute the center of the patient surface
			CVector<3> vMin = pSurface->GetBoundsMin();
			CVector<3> vMax = pSurface->GetBoundsMax();
			// CSurfaceRenderable::m_vXlate = (vMin + vMax) * -0.5; 

			m_wndREV.GetCamera().SetFieldOfView((float) (8.5 * pSurface->GetMaxSize())); 
			m_wndBEV.GetCamera().SetFieldOfView((float) (2.5 * pSurface->GetMaxSize())); 
		}

		if (nAtSurf > 0)
		{
			CSurfaceRenderable *pBEVSurfaceRenderable = new CSurfaceRenderable(&m_wndBEV);
			// pBEVSurfaceRenderable->m_bWireFrame.SyncTo(&isWireFrame);
			// pBEVSurfaceRenderable->m_bColorWash.SyncTo(&isColorWash);
			pBEVSurfaceRenderable->SetColor(arrColors[nAtSurf]);
			pBEVSurfaceRenderable->SetSurface(pSurface);
			m_wndBEV.AddRenderable(pBEVSurfaceRenderable);
			//pBEVSurfaceRenderable->isEnabled.SyncTo(&pNewItem->isChecked);
		}
	}

	CObjectTreeItem *pRefPtItem = new CObjectTreeItem();

#ifdef USE_HIERARCHY
#else
	pRefPtItem->m_strLabel = "Ref Pts";
	pRefPtItem->Create(pExplorer);
#endif

	{
		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->m_strLabel = "pt1";
		pNewItem->m_nImageResourceID = IDB_POINT_BLUE;
		pNewItem->m_nSelectedImageResourceID = IDB_POINT_BLUE;

		pRefPtItem->m_arrChildren.Add(pNewItem);

		pNewItem = new CObjectTreeItem();

		pNewItem->m_strLabel = "pt2";
		pNewItem->m_nImageResourceID = IDB_POINT_YELLOW;
		pNewItem->m_nSelectedImageResourceID = IDB_POINT_YELLOW;

		pRefPtItem->m_arrChildren.Add(pNewItem);
	}

	if (GetDocument()->GetBeamCount() > 0)
	{
		CBeam& beam = *GetDocument()->GetBeamAt(0);
		SetBEVPerspective(beam);
	}

	CObjectTreeItem *pPlanItem = new CObjectTreeItem();

#ifdef USE_HIERARCHY
	pPlanItem->m_strLabel = "Plan: " + GetDocument()->GetFileRoot();
	pPatientItem->children.Add(pPlanItem);
#else
	pPlanItem->m_strLabel = "Beams";
	pPlanItem->Create(pExplorer);
#endif

	int nAtBeam;
	for (nAtBeam = 0; nAtBeam < GetDocument()->GetBeamCount(); nAtBeam++)
	{
		CBeam *pBeam = GetDocument()->GetBeamAt(nAtBeam);

		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->m_pObject = pBeam;

		pNewItem->m_nImageResourceID = IDB_BEAM_GREEN;
		pNewItem->m_nSelectedImageResourceID = IDB_BEAM_MAGENTA;

		pPlanItem->m_arrChildren.Add(pNewItem);

		// create and add the machine Renderable to the REV
		CMachineRenderable *pMachineRenderable = new CMachineRenderable(&m_wndREV);
		pMachineRenderable->m_pBeam = pBeam;
		m_wndREV.AddRenderable(pMachineRenderable);

		// create and add the beam Renderable to the REV
		m_pBeamRenderable = new CBeamRenderable(&m_wndREV);
		m_pBeamRenderable->SetBeam(pBeam);
		m_pBeamRenderable->SetREVBeam();

		m_wndREV.AddRenderable(m_pBeamRenderable);
		// m_pBeamRenderable->isEnabled.SyncTo(&pNewItem->isChecked);
	}

#ifdef USE_HIERARCHY
	CObjectTreeItem *pAltPatientItem = new CObjectTreeItem();
	pAltPatientItem->label.Set("Patient: Sprat, Jack");
	pAltPatientItem->isChecked.Set(FALSE);
	pAltPatientItem->Create(pExplorer);
#endif

	// rotate the view to a standard position
	pRotateTracker->OnButtonDown(0, CPoint(111, 45));
	pRotateTracker->OnMouseDrag(0, CPoint(250, 405));

	pRotateTracker->OnButtonDown(0, CPoint(75, 109));
	pRotateTracker->OnMouseDrag(0, CPoint(123, 154));
}

void CSimView::OnViewBeam() 
{
	m_pBeamRenderable->SetEnabled(!m_pBeamRenderable->IsEnabled());
}

void CSimView::OnUpdateViewBeam(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pBeamRenderable != NULL);
	if (m_pBeamRenderable != NULL)
	{
		pCmdUI->SetCheck(m_pBeamRenderable->IsEnabled() ? 1 : 0);
	}
}

void CSimView::OnViewLightfield() 
{
	if (m_pSurfaceRenderable->GetLightFieldBeam() == NULL)
		m_pSurfaceRenderable->SetLightFieldBeam(GetDocument()->GetBeamAt(0));
	else
		m_pSurfaceRenderable->SetLightFieldBeam(NULL);
}

void CSimView::OnUpdateViewLightfield(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pSurfaceRenderable != NULL);
	if (m_pSurfaceRenderable != NULL)
	{
		pCmdUI->SetCheck(m_pSurfaceRenderable->GetLightFieldBeam() ? 1 : 0);
	}
}

BOOL CSimView::OnEraseBkgnd(CDC* pDC) 
{
	// over-ride default to prevent flicker
	return TRUE;
}

void CSimView::OnViewWireframe() 
{
	m_bWireFrame = !m_bWireFrame;
}

void CSimView::OnUpdateViewWireframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pSurfaceRenderable != NULL);
	if (m_pSurfaceRenderable != NULL)
	{
		pCmdUI->SetCheck(m_bWireFrame ? 1 : 0);
	}
}

CMatrix<4> CSimView::ComputeProjection(CBeam& beam)
{
	CMatrix<4> mProj;

	// translate forward slightly (will not affect projection, since it occurs _after_ the perspective)
	mProj *= CreateTranslate(0.01, CVector<3>(0.0, 0.0, 1.0));

	mProj *= beam.GetTreatmentMachine()->m_projection;

	CVector<2> vMin = beam.GetCollimMin();
	CVector<2> vMax = beam.GetCollimMax();

	double collimHeight = __max(fabs(vMin[1]), fabs(vMax[1])) + 10.0;
	double collimWidth = __max(fabs(vMin[0]), fabs(vMax[0])) + 10.0;

	// get the window's client rectangle
	CRect rect;
	m_wndBEV.GetClientRect(&rect);
	float aspect = (float) rect.Width() / (float) rect.Height();

	double height = __max(collimWidth / aspect, collimHeight);
	double width = __max(collimHeight * aspect, collimWidth);

	// scale, including a slight dilation along the z-axis to avoid clipping the collimator plane
	mProj *= CreateScaleHG(CVector<3>(1.0 / width, 1.0 / height, 1.0));

	// now rotate by 180 degrees about Y
	mProj *= CreateRotateHG(PI, CVector<3>(0.0, 1.0, 0.0));

	return mProj;
}

void CSimView::SetBEVPerspective(CBeam& beam)
{
	// compute the projection
	CMatrix<4> mProj = ComputeProjection(beam);
	// m_wndBEV.GetCamera().SetPerspective(mProj);

	// and inverse transform from beam to patient
	CMatrix<4> mB2P = beam.GetBeamToPatientXform();
	mB2P.Invert();

	m_wndBEV.GetCamera().SetXform(mB2P);
}

void CSimView::OnViewColorwash() 
{
	m_bColorWash = !m_bColorWash;
}

void CSimView::OnUpdateViewColorwash(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bColorWash ? 1 : 0);	
}
