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
#include "BeamRenderer.h"
#include "SurfaceRenderer.h"
#include "MachineRenderer.h"
#include "DRRRenderer.h"

#include "BeamParamPosCtrl.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimView

IMPLEMENT_DYNCREATE(CSimView, CView)

CSimView::CSimView()
	: m_pBeamRenderer(NULL),
		m_pSurfaceRenderer(NULL),
		m_pDRRRenderer(NULL),
		patientEnabled(TRUE),
		isWireFrame(FALSE),
		isColorWash(FALSE)
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
	if (pFromObject == &m_wndBEV.camera.modelXform)
	{
		CMatrix<4> mBeamToPatientXform = Invert(m_wndBEV.camera.modelXform.Get());
		currentBeam->beamToPatientXform.Set(mBeamToPatientXform);
		m_wndREV.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (pFromObject != &m_wndBEV.camera.projection)
	{
		CBeam *pBeam = (CBeam *)pFromObject;
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

	m_wndREV.camera.nearPlane.Set(1.0);
	m_wndREV.camera.farPlane.Set(200.0);
	m_wndREV.camera.SetFieldOfView(20.0f);
	m_wndREV.camera.phi.Set(PI);
	m_wndREV.camera.theta.Set(PI / 2.0);
	pRotateTracker = new CRotateTracker(&m_wndREV);
	m_wndREV.leftTrackers.Add(pRotateTracker);
	m_wndREV.middleTrackers.Add(new CZoomTracker(&m_wndREV));

	if (!m_wndBEV.Create(NULL, NULL, lpCreateStruct->style, CRect(0, 0, 0, 0), this, 0))
		return -1;
	
	m_wndBEV.camera.nearPlane.Set(1.0);
	m_wndBEV.camera.farPlane.Set(200.0);
	m_wndBEV.camera.SetFieldOfView(20.0f);
	CRotateTracker *pBEVRotateTracker = new CRotateTracker(&m_wndBEV);
	m_wndBEV.leftTrackers.Add(pBEVRotateTracker);


	return 0;
}

#define BORDER 3

void CSimView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	cx -= BORDER;
	cy -= BORDER;

	m_wndREV.MoveWindow(0 + BORDER, 0 + BORDER, 
		cx / 2 - BORDER, cy - BORDER);	

	m_wndBEV.MoveWindow(cx / 2 + BORDER, 0 + BORDER, 
		cx / 2 - BORDER, cy - BORDER);	

	if (GetDocument()->beams.GetSize() > 0)
	{
		CBeam& beam = *GetDocument()->beams.Get(0);
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
	if (GetDocument()->beams.GetSize() > 0)
	{
		CBeam& beam = *GetDocument()->beams.Get(0);
		pMainBeam = &beam;

		beam.name.Set("AP");

		currentBeam.Set(&beam);

		m_pDRRRenderer = new CDRRRenderer(&m_wndBEV);
		m_pDRRRenderer->forVolume.Set(&GetDocument()->GetSeries()->volume);
		m_pDRRRenderer->volumeTransform.Set(GetDocument()->GetSeries()->volumeTransform.Get());
		m_wndBEV.renderers.Add(m_pDRRRenderer);

		CBeamRenderer *pBEVBeamRenderer = new CBeamRenderer(&m_wndBEV);
		pBEVBeamRenderer->SetBeam(&beam);
		pBEVBeamRenderer->isGraticuleEnabled.Set(TRUE);
		m_wndBEV.renderers.Add(pBEVBeamRenderer);

		beam.AddObserver(this, (ChangeFunction) OnChange);
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
	pSeriesItem->label.Set("Series: " + pSeries->GetFileRoot());
	pPatientItem->children.Add(pSeriesItem);
#else
	pSeriesItem->label.Set("Structures");
	pSeriesItem->Create(pExplorer);
#endif

	for (int nAtSurf = 0; nAtSurf < pSeries->structures.GetSize(); nAtSurf++)
	{
		CSurface *pSurface = GetDocument()->GetSeries()->structures.Get(nAtSurf);

		// add the surface to the object explorer
		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->forObject.Set(pSurface);

		pNewItem->imageResourceID.Set(arrIconResourceIDs[nAtSurf % 13]);
		pNewItem->selectedImageResourceID.Set(arrIconResourceIDs[nAtSurf % 13]);

		pNewItem->isChecked.Set(TRUE);

		pSeriesItem->children.Add(pNewItem);

		CSurfaceRenderer *pSurfaceRenderer = new CSurfaceRenderer(&m_wndREV);
		pSurfaceRenderer->isWireFrame.SyncTo(&isWireFrame);

		pSurfaceRenderer->color.Set(arrColors[nAtSurf]);
		pSurfaceRenderer->SetSurface(pSurface);
		pSurfaceRenderer->tableOffset.SyncTo(&pMainBeam->tableOffset);
		pSurfaceRenderer->couchAngle.SyncTo(&pMainBeam->couchAngle);
		m_wndREV.renderers.Add(pSurfaceRenderer);

		pSurfaceRenderer->isEnabled.SyncTo(&pNewItem->isChecked);

		if (nAtSurf == 0)	// patient surface
		{
			m_pSurfaceRenderer = pSurfaceRenderer;
			m_pSurfaceRenderer->showBoundsSurface.Set(TRUE);

			// compute the center of the patient surface
			CVector<3> vMin = pSurface->GetBoundsMin();
			CVector<3> vMax = pSurface->GetBoundsMax();
			// CSurfaceRenderer::m_vXlate = (vMin + vMax) * -0.5; 

			m_wndREV.camera.SetFieldOfView((float) (8.5 * pSurface->GetMaxSize())); 
			m_wndBEV.camera.SetFieldOfView((float) (2.5 * pSurface->GetMaxSize())); 
		}

		if (nAtSurf > 0)
		{
			CSurfaceRenderer *pBEVSurfaceRenderer = new CSurfaceRenderer(&m_wndBEV);
			pBEVSurfaceRenderer->isWireFrame.SyncTo(&isWireFrame);
			pBEVSurfaceRenderer->isColorWash.SyncTo(&isColorWash);
			pBEVSurfaceRenderer->color.Set(arrColors[nAtSurf]);
			pBEVSurfaceRenderer->SetSurface(pSurface);
			m_wndBEV.renderers.Add(pBEVSurfaceRenderer);
			pBEVSurfaceRenderer->isEnabled.SyncTo(&pNewItem->isChecked);
		}
	}

	CObjectTreeItem *pRefPtItem = new CObjectTreeItem();

#ifdef USE_HIERARCHY
#else
	pRefPtItem->label.Set("Ref Pts");
	pRefPtItem->Create(pExplorer);
#endif

	{
		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->label.Set("pt1");
		pNewItem->imageResourceID.Set(IDB_POINT_BLUE);
		pNewItem->selectedImageResourceID.Set(IDB_POINT_BLUE);

		pRefPtItem->children.Add(pNewItem);

		pNewItem = new CObjectTreeItem();

		pNewItem->label.Set("pt2");
		pNewItem->imageResourceID.Set(IDB_POINT_YELLOW);
		pNewItem->selectedImageResourceID.Set(IDB_POINT_YELLOW);

		pRefPtItem->children.Add(pNewItem);
	}

	if (GetDocument()->beams.GetSize() > 0)
	{
		CBeam& beam = *GetDocument()->beams.Get(0);
		SetBEVPerspective(beam);
	}

	CObjectTreeItem *pPlanItem = new CObjectTreeItem();

#ifdef USE_HIERARCHY
	pPlanItem->label.Set("Plan: " + GetDocument()->GetFileRoot());
	pPatientItem->children.Add(pPlanItem);
#else
	pPlanItem->label.Set("Beams");
	pPlanItem->Create(pExplorer);
#endif

	int nAtBeam;
	for (nAtBeam = 0; nAtBeam < GetDocument()->beams.GetSize(); nAtBeam++)
	{
		CBeam *pBeam = GetDocument()->beams.Get(nAtBeam);

		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->forObject.Set(pBeam);

		pNewItem->imageResourceID.Set(IDB_BEAM_GREEN);
		pNewItem->selectedImageResourceID.Set(IDB_BEAM_MAGENTA);

		pPlanItem->children.Add(pNewItem);

		// create and add the machine renderer to the REV
		CMachineRenderer *pMachineRenderer = new CMachineRenderer(&m_wndREV);
		pMachineRenderer->forBeam.Set(pBeam);
		m_wndREV.renderers.Add(pMachineRenderer);

		// create and add the beam renderer to the REV
		m_pBeamRenderer = new CBeamRenderer(&m_wndREV);
		m_pBeamRenderer->SetBeam(pBeam);
		m_pBeamRenderer->SetREVBeam();

		m_wndREV.renderers.Add(m_pBeamRenderer);
		m_pBeamRenderer->isEnabled.SyncTo(&pNewItem->isChecked);
	}

#ifdef USE_HIERARCHY
	CObjectTreeItem *pAltPatientItem = new CObjectTreeItem();
	pAltPatientItem->label.Set("Patient: Sprat, Jack");
	pAltPatientItem->isChecked.Set(FALSE);
	pAltPatientItem->Create(pExplorer);
#endif

	// rotate the view to a standard position
	pRotateTracker->OnLButtonDown(0, CPoint(111, 45));
	pRotateTracker->OnMouseDrag(0, CPoint(250, 405));

	pRotateTracker->OnLButtonDown(0, CPoint(75, 109));
	pRotateTracker->OnMouseDrag(0, CPoint(123, 154));
}

void CSimView::OnViewBeam() 
{
	m_pBeamRenderer->isEnabled.Set(!m_pBeamRenderer->isEnabled.Get());
}

void CSimView::OnUpdateViewBeam(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pBeamRenderer != NULL);
	if (m_pBeamRenderer != NULL)
	{
		pCmdUI->SetCheck(m_pBeamRenderer->isEnabled.Get() ? 1 : 0);
	}
}

void CSimView::OnViewLightfield() 
{
	if (m_pSurfaceRenderer->GetLightFieldBeam() == NULL)
		m_pSurfaceRenderer->SetLightFieldBeam(GetDocument()->beams.Get(0));
	else
		m_pSurfaceRenderer->SetLightFieldBeam(NULL);
}

void CSimView::OnUpdateViewLightfield(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pSurfaceRenderer != NULL);
	if (m_pSurfaceRenderer != NULL)
	{
		pCmdUI->SetCheck(m_pSurfaceRenderer->GetLightFieldBeam() ? 1 : 0);
	}
}

BOOL CSimView::OnEraseBkgnd(CDC* pDC) 
{
	// over-ride default to prevent flicker
	return TRUE;
}

void CSimView::OnViewWireframe() 
{
	isWireFrame.Set( !isWireFrame.Get() );
}

void CSimView::OnUpdateViewWireframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pSurfaceRenderer != NULL);
	if (m_pSurfaceRenderer != NULL)
	{
		pCmdUI->SetCheck(isWireFrame.Get() ? 1 : 0);
	}
}

CMatrix<4> CSimView::ComputeProjection(CBeam& beam)
{
	CMatrix<4> mProj;

	// translate forward slightly (will not affect projection, since it occurs _after_ the perspective)
	mProj *= CreateTranslate(0.01, CVector<3>(0.0, 0.0, 1.0));

	mProj *= beam.forMachine->projection.Get();

	CVector<2> vMin = beam.collimMin.Get();
	CVector<2> vMax = beam.collimMax.Get();

	double collimHeight = max(fabs(vMin[1]), fabs(vMax[1])) + 10.0;
	double collimWidth = max(fabs(vMin[0]), fabs(vMax[0])) + 10.0;

	// get the window's client rectangle
	CRect rect;
	m_wndBEV.GetClientRect(&rect);
	float aspect = (float) rect.Width() / (float) rect.Height();

	double height = max(collimWidth / aspect, collimHeight);
	double width = max(collimHeight * aspect, collimWidth);

	// scale, including a slight dilation along the z-axis to avoid clipping the collimator plane
	mProj *= CreateScale(CVector<3>(1.0 / width, 1.0 / height, 1.0));

	// now rotate by 180 degrees about Y
	mProj *= CreateRotate(PI, CVector<3>(0.0, 1.0, 0.0));

	return mProj;
}

void CSimView::SetBEVPerspective(CBeam& beam)
{
	// compute the projection
	CMatrix<4> mProj = ComputeProjection(beam);
	m_wndBEV.camera.perspective.Set(mProj);

	// and inverse transform from beam to patient
	CMatrix<4> mB2P = beam.beamToPatientXform.Get();
	mB2P.Invert();

	m_wndBEV.camera.modelXform.RemoveObserver(this, (ChangeFunction) OnChange);
	m_wndBEV.camera.modelXform.Set(mB2P);

	// m_wndBEV.camera.modelXform.Set(beam.beamToPatientXform.Get()); // mProj);
	m_wndBEV.camera.modelXform.AddObserver(this, (ChangeFunction) OnChange); 
}

void CSimView::OnViewColorwash() 
{
	isColorWash.Set( !isColorWash.Get() );
}

void CSimView::OnUpdateViewColorwash(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(isColorWash.Get() ? 1 : 0);	
}
