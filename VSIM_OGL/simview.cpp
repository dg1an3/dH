// SimView1.cpp : implementation file
//

#include "stdafx.h"
#include "VSIM_OGL.h"
#include "SimView.h"

#include <math.h>

#include <gl/gl.h>

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
	ON_WM_TIMER()
	ON_COMMAND(ID_EXPORT_FIELDCOM, OnExportFieldcom)
	ON_COMMAND(ID_EXPORT_NUAGES, OnExportNuages)
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

	CRect rectREV;
	m_wndREV.GetWindowRect(&rectREV);
	ScreenToClient(&rectREV);

	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rectREV);
	pDC->SelectClipRgn(&rgn, RGN_DIFF);

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
			CRect(0, 0, 1024, 1024), this, 0))
	{
		return -1;
	}

	// set up the REV camera
	m_wndREV.GetCamera().SetDirection(CVectorD<3>(0.0, 0.2, 1.0));
	m_wndREV.GetCamera().SetDistance(300.0);
	m_wndREV.GetCamera().SetClippingPlanes(1.0, 1000.0);
	m_wndREV.GetCamera().SetViewingAngle(PI / 4);
	// m_wndREV.GetCamera().SetFieldOfView(20.0);
	// m_wndREV.GetCamera().SetDirection(CVectorD<3>(1.0, 0.5, 0.2));

	// set up the trackers
	m_wndREV.AddLeftTracker(new CRotateTracker(&m_wndREV));
	m_wndREV.AddMiddleTracker(new CZoomTracker(&m_wndREV));

	// set a timer event to occur
	SetTimer(7, 10, NULL);

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
	CMatrixD<4> mMV = CMatrixD<4>(
		CreateRotate(m_pCurrentBeam->GetCouchAngle(), CVectorD<3>(0.0, 0.0, 1.0)))
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
	CObjectExplorer *pExplorer = &pFrame->m_wndExplorer;

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
			CMesh *pSurface = 
				(CMesh *) pSeries->GetStructureAt(nAtSurf);

			// create the object tree item
			CObjectTreeItem *pNewItem = new CObjectTreeItem();
			pNewItem->SetObject(pSurface);
			pNewItem->SetImages(arrIconResourceIDs[nAtSurf % 13], 
				arrIconResourceIDs[nAtSurf % 13]);
			pSeriesItem->AddChild(pNewItem);

			// create the renderable for the structure
			CSurfaceRenderable *pSurfaceRenderable = (CSurfaceRenderable *) 
				pNewItem->CreateRenderableForObject(&m_wndREV);

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
				// m_wndREV.GetCamera().SetFieldOfView((float) 
				//	(8.5 * pSurface->GetMaxSize())); 
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

		// create the object tree item
		CObjectTreeItem *pNewItem = new CObjectTreeItem();
		pNewItem->SetObject(pBeam);
		pNewItem->SetImages(IDB_BEAM_GREEN, IDB_BEAM_MAGENTA);
		pPlanItem->AddChild(pNewItem);

		// create the renderable for the structure
		CBeamRenderable *pBeamRenderable = (CBeamRenderable *)
			pNewItem->CreateRenderableForObject(&m_wndREV);

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

	if (GetDocument()->GetBeamCount() > 0)
	{
		// create and add the machine Renderable to the REV
		CMachineRenderable *pMachineRenderable = 
			new CMachineRenderable();
		pMachineRenderable->SetBeam(GetDocument()->GetBeamAt(0));
		pMachineRenderable->SetColor(RGB(255, 255, 128));
		pMachineRenderable->SetAlpha(1.0); // 0.5);
		m_wndREV.AddRenderable(pMachineRenderable);	
	}
}

int nAtStructure = 0;

void CSimView::OnTimer(UINT nIDEvent) 
{
	CView::OnTimer(nIDEvent);

	return;

	if (nAtStructure < GetDocument()->GetSeries()->GetStructureCount())
	{
		// get structure pointer
		CMesh *pStructure = GetDocument()->GetSeries()->GetStructureAt(nAtStructure);

		// orient 50 faces
		for (int nAt = 0; nAt < 50; nAt++)
		{
			pStructure->OrientNextFace();
		}

		if (!pStructure->OrientNextFace())
		{
			nAtStructure++;
		}

		pStructure->GetChangeEvent().Fire();
	}
	
}

#ifdef USE_ExportFieldCOM
#import "..\..\srcAx\FieldCOM\FieldCOM.tlb" no_namespace

void CSimView::OnExportFieldcom() 
{
	HRESULT hr;

	CSeries *pSeries = GetDocument()->GetSeries();
	for (int nAtStruct = 0; nAtStruct < pSeries->GetStructureCount(); nAtStruct++)
	{
		CMesh *pSurface = pSeries->GetStructureAt(nAtStruct);
		IMeshPtr pMesh;
		hr = pMesh.CreateInstance("FieldCOM.Mesh");

		// set the mesh label
		COleVariant varLabel(pSurface->GetName());
		hr = pMesh->put_Label(varLabel);

		// now populate the mesh from the structures
		CArray< CPackedVectorD<3>, CPackedVectorD<3>& >& arrVertices = pSurface->GetVertexArray();
		CArray< CPackedVectorD<3>, CPackedVectorD<3>& >& arrNormals = pSurface->GetNormalArray();
		long nCount = arrVertices.GetSize();

		IMatrixPtr pMVertices;
		hr = pMesh->get_Vertices(&pMVertices);
		hr = pMVertices->Reshape(nCount, 3);

		IMatrixPtr pMNormals;
		hr = pMesh->get_Normals(&pMNormals);
		hr = pMNormals->Reshape(nCount, 3);

		for (int nAt = 0; nAt < nCount; nAt++)
		{
			hr = pMVertices->put_Element(nAt, 0, arrVertices[nAt][0]);
			hr = pMVertices->put_Element(nAt, 1, arrVertices[nAt][1]);
			hr = pMVertices->put_Element(nAt, 2, arrVertices[nAt][2]);

			hr = pMNormals->put_Element(nAt, 0, arrNormals[nAt][0]);
			hr = pMNormals->put_Element(nAt, 1, arrNormals[nAt][1]);
			hr = pMNormals->put_Element(nAt, 2, arrNormals[nAt][2]);
		}

		CArray< int, int >& arrIndices = pSurface->GetIndexArray();

		// transfer the indices
		VARIANT varIndices;
		::VariantInit(&varIndices);

		// populate a SAFEARRAYBOUND array with the dimensions
		SAFEARRAYBOUND bound[1];
		bound[0].lLbound = 0;
		bound[0].cElements = arrIndices.GetSize();

		// create the array
		varIndices.vt = VT_ARRAY | VT_I4;
		varIndices.parray = ::SafeArrayCreate(VT_I4, 1, &bound[0]);

		// populate the index array
		long *pData = NULL;
		hr = ::SafeArrayAccessData(varIndices.parray, (void **) &pData);
		memcpy(pData, arrIndices.GetData(), arrIndices.GetSize() * sizeof(long));

		// fix for one-based dereferencing
		for (nAt = 0; nAt < arrIndices.GetSize(); nAt++)
		{
			pData[nAt]--;
			ASSERT(pData[nAt] >= 0);
		}

		hr = ::SafeArrayUnaccessData(varIndices.parray);

		// put the index array
		hr = pMesh->put_Indices(varIndices);

		IFileStoragePtr pStorage;
		pStorage.CreateInstance("FieldCOM.FileStorage");
		hr = pStorage->put_Pathname(_bstr_t(pSurface->GetName() + ".dat"));
		hr = pStorage->Save(pMesh);
		pStorage.Release();
	}
}
#else
void CSimView::OnExportFieldcom() 
{
}
#endif

void CSimView::OnExportNuages() 
{
/*
	CSeries *pSeries = GetDocument()->GetSeries();
	for (int nAtStruct = 0; nAtStruct < pSeries->GetStructureCount(); nAtStruct++)
	{
		CMesh *pSurface = pSeries->GetStructureAt(nAtStruct);

		CString strName = pSurface->GetName() + ".cnt";
		FILE *fCnt = fopen(strName, "w");
		fprintf(fCnt, "S %i\n", pSurface->GetContourCount());
		double refDist = -9999.99;
		for (int nAtContour = 0; nAtContour < pSurface->GetContourCount(); nAtContour++)
		{
			CPolygon *pPoly = &pSurface->get_Contour(nAtContour);

			if (refDist != pSurface->GetContourRefDist(nAtContour))
			{
				refDist = pSurface->GetContourRefDist(nAtContour);
				int nVertCount = pPoly->GetVertexCount();
				// find more contours at same ref dist
				for (int nAt2 = nAtContour+1; nAt2 < pSurface->GetContourCount(); nAt2++)
				{
					if (refDist == pSurface->GetContourRefDist(nAt2))
					{
						CPolygon *pPoly2 = &pSurface->GetContour(nAt2);
						nVertCount += pPoly2->GetVertexCount();
					}
				}

				fprintf(fCnt, "V %i z %lf\n", nVertCount, refDist);
			}

			fprintf(fCnt, "{\n");
			for (int nAtVert = 0; nAtVert < pPoly->GetVertexCount(); nAtVert++)
			{
				CVectorD<2> v = pPoly->GetVertexAt(nAtVert);
				fprintf(fCnt, "%lf %lf\n", v[0], v[1]);
			}
			fprintf(fCnt, "}\n");
		}

		fclose(fCnt);
	}
*/
}
