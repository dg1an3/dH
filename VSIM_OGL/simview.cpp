// SimView1.cpp : implementation file
//

#include "stdafx.h"
#include "VSIM_OGL.h"
#include "SimView.h"

#include <math.h>

#include <gl/gl.h>
#include <glMatrixVector.h>

#include <RotateTracker.h>
#include <ZoomTracker.h>
#include "BeamRenderable.h"
#include "SurfaceRenderable.h"
#include "MachineRenderable.h"

#include <LightfieldTexture.h>

#include "BeamParamPosCtrl.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

/////////////////////////////////////////////////////////////////////////////
// CSimView

IMPLEMENT_DYNCREATE(CSimView, CView)

CSimView::CSimView()
	: m_pCurrentBeam(NULL),
		m_pBeamRenderable(NULL),
		m_pSurfaceRenderable(NULL),
		m_bPatientEnabled(TRUE),
		m_bWireFrame(FALSE),
		m_bColorWash(FALSE)
{
}

CSimView::~CSimView()
{
}

CBeam *CSimView::GetCurrentBeam()
{
	return m_pCurrentBeam;
}

void CSimView::SetCurrentBeam(CBeam *pBeam)
{
	if (NULL != GetCurrentBeam())
	{
		// register as change listener for current beam
		::RemoveObserver<CSimView>(&GetCurrentBeam()->GetChangeEvent(),
			this, OnBeamChanged);
	}

	// set current beam
	m_pCurrentBeam = pBeam;

	if (NULL != GetCurrentBeam())
	{
		// register as change listener for current beam
		::AddObserver<CSimView>(&GetCurrentBeam()->GetChangeEvent(),
			this, OnBeamChanged);
	}
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
	{
		return -1;
	}

	if (!m_wndREV.Create(NULL, NULL, lpCreateStruct->style, 
			CRect(0, 0, 0, 0), this, 0))
	{
		return -1;
	}

	// set up the REV camera
	m_wndREV.GetCamera().SetClippingPlanes(1.0, 200.0);
	m_wndREV.GetCamera().SetFieldOfView(20.0);
	m_wndREV.GetCamera().SetPhi(-PI / 4.0);
	m_wndREV.GetCamera().SetTheta(3.0 * PI / 8.0);

	// set up the trackers
	m_wndREV.AddLeftTracker(new CRotateTracker(&m_wndREV));
	m_wndREV.AddMiddleTracker(new CZoomTracker(&m_wndREV));

	return 0;
}

#define BORDER 3

void CSimView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	cx -= BORDER;
	cy -= BORDER;

	m_wndREV.MoveWindow(0 + BORDER, 0 + BORDER, 
		cx - BORDER, cy - BORDER);	
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
	if (m_pSurfaceRenderable->GetTexture() == NULL)
	{

		CLightfieldTexture *pTexture =
			new CLightfieldTexture();

		pTexture->SetBeam(GetDocument()->GetBeamAt(0));

		m_pSurfaceRenderable->SetTexture(pTexture);
	}
	else
	{
		delete m_pSurfaceRenderable->GetTexture();

		m_pSurfaceRenderable->SetTexture(NULL);
	}
}

void CSimView::OnUpdateViewLightfield(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pSurfaceRenderable != NULL);
	if (m_pSurfaceRenderable != NULL)
	{
		pCmdUI->SetCheck(m_pSurfaceRenderable->GetTexture() ? 1 : 0);
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

void CSimView::OnViewColorwash() 
{
	m_bColorWash = !m_bColorWash;
}

void CSimView::OnUpdateViewColorwash(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bColorWash ? 1 : 0);	
}

void CSimView::OnBeamChanged(CObservableEvent *pEvent, void *pOldValue)
{
	// compute the new modelview matrix for the surfaces
	CMatrix<4> mMV = CMatrix<4>(
		CreateRotate(m_pCurrentBeam->GetCouchAngle(), CVector<3>(0.0, 0.0, 1.0)))
			* CreateTranslate(-1.0 * m_pCurrentBeam->GetTableOffset());

	// iterate over the surface renderables
	for (int nAt = 0; nAt < m_arrSurfaceRenderables.GetSize(); nAt++)
	{
		CSurfaceRenderable *pSR = 
			(CSurfaceRenderable *)m_arrSurfaceRenderables.GetAt(nAt);

		// update the modelview matrix
		pSR->SetModelviewMatrix(mMV);
	}
}

void CSimView::OnInitialUpdate() 
{
	// base class update
	CView::OnInitialUpdate();
	
	// only process if there is a loaded plan
	if (GetDocument() == NULL)
	{
		return;
	}

	// pointer to the object explorer
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	CRenderableObjectExplorer *pExplorer = &pFrame->m_wndExplorer;

	///////////////////////////////////////////////////////////////////////////
	// structures

	// get a pointer to the seris
	CSeries *pSeries = GetDocument()->GetSeries();
	if (NULL != pSeries)
	{
		// create the item to hold the structures
		CObjectTreeItem *pSeriesItem = new CObjectTreeItem();
		pSeriesItem->SetLabel("Structures");
		pSeriesItem->Create(pExplorer);

		// add renderables and items for each structure
		for (int nAtSurf = 0; nAtSurf < pSeries->GetStructureCount(); nAtSurf++)
		{
			// get a pointer to the next structure
			CSurface *pSurface = 
				(CSurface *) pSeries->GetStructureAt(nAtSurf);

			// create the renderable, adding an item in the process
			CSurfaceRenderable *pSurfaceRenderable = (CSurfaceRenderable *)
				pExplorer->CreateItemForObject(pSurface, &m_wndREV, 
					pSeriesItem, 
					arrIconResourceIDs[nAtSurf % 13], 
					arrIconResourceIDs[nAtSurf % 13]);

			// set the renderable's color
			pSurfaceRenderable->SetColor(arrColors[nAtSurf]);

			// add to the array for management purposes
			m_arrSurfaceRenderables.Add(pSurfaceRenderable);

			// if patient surface
			if (nAtSurf == 0)	
			{
				// store a special pointer
				m_pSurfaceRenderable = pSurfaceRenderable;

				// and set the camera FOV
				m_wndREV.GetCamera().SetFieldOfView((float) 
					(8.5 * pSurface->GetMaxSize())); 
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// beams

	// create the plan item
	CObjectTreeItem *pPlanItem = new CObjectTreeItem();
	pPlanItem->SetLabel("Beams");
	pPlanItem->Create(pExplorer);


	// create a renderable and item for each beam
	for (int nAtBeam = 0; nAtBeam < GetDocument()->GetBeamCount(); nAtBeam++)
	{
		// get a pointer to the current beam
		CBeam *pBeam = GetDocument()->GetBeamAt(nAtBeam);

		// and create the renderable
		CBeamRenderable *pBeamRenderable = (CBeamRenderable *)
			pExplorer->CreateItemForObject(pBeam, &m_wndREV, pPlanItem, 
				IDB_BEAM_GREEN, IDB_BEAM_MAGENTA);

		// set the current beam
		if (nAtBeam == 0)
		{
			// TODO: why is this not loaded?
			pBeam->SetName("AP");

			// set the current beam
			SetCurrentBeam(pBeam);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// treatment machine

	// create and add the machine Renderable to the REV
	CMachineRenderable *pMachineRenderable = 
		new CMachineRenderable();
	pMachineRenderable->SetBeam(GetDocument()->GetBeamAt(0));
	pMachineRenderable->SetColor(RGB(128, 128, 255));
	m_wndREV.AddRenderable(pMachineRenderable);	
}
