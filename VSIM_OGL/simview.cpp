// SimView1.cpp : implementation file
//

#include "stdafx.h"
#include "VSIM_OGL.h"
#include "SimView.h"

#include <math.h>

#include "gl/gl.h"
#include "glMatrixVector.h"

#include "RotateTracker.h"
#include "BeamRenderer.h"
#include "SurfaceRenderer.h"
#include "DRRRenderer.h"

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
		isWireFrame(FALSE)
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

double AngleFromSinCos(double sin_angle, double cos_angle)
{
	double angle = 0.0;

	if (sin_angle >= 0.0)
	{
		angle = acos(cos_angle);	// range of [0..PI]	
	}
	else if (cos_angle >= 0.0)
	{
		angle = asin(sin_angle);	// range of [-PI/2..PI/2]
	}
	else
	{
		angle = 2 * PI - acos(cos_angle);
	}

	// ensure the angle is in range [0..2*PI];
	if (angle < 0.0)
		angle += 2 * PI;

	// now check
	ASSERT(fabs(sin(angle) - sin_angle) < 1e-6);
	ASSERT(fabs(cos(angle) - cos_angle) < 1e-6);

	return angle;
}

void CSimView::OnChange(CObservableObject *pFromObject, void *pOldValue)
{
	if (pFromObject == &m_wndBEV.projectionMatrix)
	{
		// determine the new beam parameters from the BEV viewing matrix
		CMatrix<4> mProjMatrix = m_wndBEV.projectionMatrix.Get();

		// first, retrieve the projection matrix for the beam
		CMatrix<4> mPerspProj = ComputeProjection(*currentBeam.Get());

		// now, annihilate the projection matrix
		CMatrix<4> mNewB2P = Invert(Invert(mPerspProj) * mProjMatrix);
		if (mNewB2P[2][2] < -1.0 || mNewB2P[2][2] > 1.0)
			return;

		// now factor the B2P matrix into translation and rotation components
		double gantry = acos(mNewB2P[2][2]);	// gantry in [0..PI]

		double couch = 0.0;
		double coll = 0.0;

		if (gantry > 0.0)
		{
			double cos_couch = mNewB2P[2][0] / sin(gantry);
			// make sure the couch angle will be in [-PI..PI]
			if (cos_couch < 0.0)
			{
				gantry = 2 * PI - gantry;
				cos_couch = mNewB2P[2][0] / sin(gantry);
			}

			double sin_couch = mNewB2P[2][1] / sin(gantry);
			couch = AngleFromSinCos(sin_couch, cos_couch);

			double cos_coll =  mNewB2P[0][2] / -sin(gantry);
			double sin_coll =  mNewB2P[1][2] / sin(gantry);
			coll = AngleFromSinCos(sin_coll, cos_coll);
		}

		currentBeam->couchAngle.Set(couch);
		double actGantry = PI - gantry;
		actGantry = (actGantry < 0.0) ? (2 * PI + actGantry) : actGantry;
		currentBeam->gantryAngle.Set(actGantry);
		currentBeam->collimAngle.Set(coll);

		m_wndREV.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
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

int CSimView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndREV.Create(NULL, NULL, lpCreateStruct->style, CRect(0, 0, 0, 0), this, 0))
		return -1;
	
	m_wndREV.SetClippingPlanes(1.0f, 200.0f);
	m_wndREV.SetMaxObjSize(20.0f);
	m_wndREV.AddTracker(new CRotateTracker(&m_wndREV));

	if (!m_wndBEV.Create(NULL, NULL, lpCreateStruct->style, CRect(0, 0, 0, 0), this, 0))
		return -1;
	
	m_wndBEV.SetClippingPlanes(1.0f, 200.0f);
	m_wndBEV.SetMaxObjSize(20.0f);
	m_wndBEV.AddTracker(new CRotateTracker(&m_wndBEV));

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

	if (GetDocument()->beams.GetSize() > 0)
	{
		CBeam& beam = *GetDocument()->beams.Get(0);

		currentBeam.Set(&beam);

		m_pDRRRenderer = new CDRRRenderer(&m_wndBEV);
		m_pDRRRenderer->forVolume.Set(&GetDocument()->GetSeries()->volume);
		m_pDRRRenderer->volumeTransform.Set(GetDocument()->GetSeries()->volumeTransform.Get());
		m_wndBEV.AddRenderer(m_pDRRRenderer);

		CBeamRenderer *pBEVBeamRenderer = new CBeamRenderer(&m_wndBEV);
		pBEVBeamRenderer->SetBeam(&beam);
		pBEVBeamRenderer->isGraticuleEnabled.Set(TRUE);
		m_wndBEV.AddRenderer(pBEVBeamRenderer);

		beam.AddObserver(this, (ChangeFunction) OnChange);
	}

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CObjectExplorer *pExplorer = &pFrame->m_wndExplorerCtrl.m_ExplorerCtrl;

	CObjectTreeItem *pPatientItem = new CObjectTreeItem();
	pPatientItem->label.Set("Patient: Doe, John");
	pPatientItem->Create(pExplorer);
	if (GetDocument()->GetSeries() == NULL)
		return;

	CSeries *pSeries = GetDocument()->GetSeries();

	CObjectTreeItem *pSeriesItem = new CObjectTreeItem();
	pSeriesItem->label.Set("Series: " + pSeries->GetFileRoot());

	pPatientItem->children.Add(pSeriesItem);

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
		m_wndREV.AddRenderer(pSurfaceRenderer);

		pSurfaceRenderer->isEnabled.SyncTo(&pNewItem->isChecked);

		if (nAtSurf == 0)	// patient surface
		{
			m_pSurfaceRenderer = pSurfaceRenderer;
			m_pSurfaceRenderer->showBoundsSurface.Set(TRUE);

			// compute the center of the patient surface
			CVector<3> vMin = pSurface->GetBoundsMin();
			CVector<3> vMax = pSurface->GetBoundsMax();
			CSurfaceRenderer::m_vXlate = (vMin + vMax) * -0.5; 

			m_wndREV.SetMaxObjSize((float) (2.5 * pSurface->GetMaxSize())); 

			m_wndBEV.SetMaxObjSize((float) (2.5 * pSurface->GetMaxSize())); 
		}

		if (nAtSurf > 0)
		{
			CSurfaceRenderer *pBEVSurfaceRenderer = new CSurfaceRenderer(&m_wndBEV);
			pBEVSurfaceRenderer->isWireFrame.SyncTo(&isWireFrame);
			pBEVSurfaceRenderer->color.Set(arrColors[nAtSurf]);
			pBEVSurfaceRenderer->SetSurface(pSurface);
			m_wndBEV.AddRenderer(pBEVSurfaceRenderer);
			pBEVSurfaceRenderer->isEnabled.SyncTo(&pNewItem->isChecked);
		}
	}

	if (GetDocument()->beams.GetSize() > 0)
	{
		CBeam& beam = *GetDocument()->beams.Get(0);
		SetBEVPerspective(beam);
	}

	CObjectTreeItem *pPlanItem = new CObjectTreeItem();
	pPlanItem->label.Set("Plan: " + GetDocument()->GetFileRoot());
	pPatientItem->children.Add(pPlanItem);

	int nAtBeam;
	for (nAtBeam = 0; nAtBeam < GetDocument()->beams.GetSize(); nAtBeam++)
	{
		CBeam *pBeam = GetDocument()->beams.Get(nAtBeam);

		CObjectTreeItem *pNewItem = new CObjectTreeItem();

		pNewItem->forObject.Set(pBeam);

		pNewItem->imageResourceID.Set(IDB_BEAM_GREEN);
		pNewItem->selectedImageResourceID.Set(IDB_BEAM_MAGENTA);

		pPlanItem->children.Add(pNewItem);

		m_pBeamRenderer = new CBeamRenderer(&m_wndREV);
		m_pBeamRenderer->SetBeam(pBeam);
		m_wndREV.AddRenderer(m_pBeamRenderer);
		m_pBeamRenderer->isEnabled.SyncTo(&pNewItem->isChecked);
	}

	CObjectTreeItem *pAltPatientItem = new CObjectTreeItem();
	pAltPatientItem->label.Set("Patient: Sprat, Jack");
	pAltPatientItem->isChecked.Set(FALSE);
	pAltPatientItem->Create(pExplorer);
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

	// and inverse transform from beam to patient
	CMatrix<4> mB2P = beam.beamToPatientXform.Get();
	mB2P.Invert();
	mProj *= mB2P;

	m_wndBEV.projectionMatrix.RemoveObserver(this, (ChangeFunction) OnChange);
	m_wndBEV.projectionMatrix.Set(mProj);
	m_wndBEV.projectionMatrix.AddObserver(this, (ChangeFunction) OnChange);
}

#if defined(USE_FUNCTIONS_FOR_BEV)

CVector<2> ComputeContainingCenteredRect(const CVector<2>& vMin, 
										 const CVector<2>& vMax)
{
	return CVector<2>( max(fabs(vMin[1]), fabs(vMax[1])) + 1.0,
		max(fabs(vMin[0]), fabs(vMax[0])) + 1.0);
}

CVector<2> AdjustAspectRatio(const CVector<2>& vRect, double aspect)
{
	return CVector<2>( max(vRect[1] / aspect, vRect[0]),
		max(vRect[0] * aspect, vRect[1]));
}

void CSimView::SetBEVPerspective(CBeam& beam)
{
//	CValue< double >& collimHeight = 
//		max(fabs(vMin[1]), fabs(vMax[1])) + 1.0;
//	CValue< double >& collimWidth = 
//		max(fabs(vMin[0]), fabs(vMax[0])) + 1.0;

	// get the window's client rectangle
//	CRect rect;
//	m_wndBEV.GetClientRect(&rect);
//	float aspect = (float) rect.Width() / (float) rect.Height();
//
//	double height = max(collimWidth / aspect, collimHeight);
//	double width = max(collimHeight * aspect, collimWidth);

//	// scale, including a slight dilation along the z-axis to avoid clipping the collimator plane
//	mProj *= CreateScale(CVector<3>(1.0 / width, 1.0 / height, 1.0));

	// now rotate by 180 degrees about Y
//	mProj *= CreateRotate(...

	// and inverse transform from beam to patient
//	CMatrix<4> mB2P = beam.myBeamToPatientXform.Get();
//	mB2P.Invert();
//	mProj *= mB2P;

	CValue< CVector<2> >& vWidthHeight =
		ComputeCenteredRect(beam.collimMin, beam.collimMax);
	CValue< CVector<2> >& vAspWidthHeight =
		AdjustAspect(vWidthHeight, m_wndBEV.myAspect);

	CValue< CMatrix<4> >& mProj =
		CreateTranslate(0.01, CVector<3>(0.0, 0.0, 1.0))	// constant
		* beam.forMachine->myProjection;					// value
		* Invert(CreateScale(vAspWidthHeight))				// value
		* CreateRotate(PI, CVector<3>(0.0, 1.0, 0.0))		// constant
		* Invert(beam.myBeamToPatientXform);

	m_wndBEV.myProjectionMatrix.SyncTo(&mProj);
}
#endif