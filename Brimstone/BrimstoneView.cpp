// BrimstoneView.cpp : implementation of the CBrimstoneView class
//

#include "stdafx.h"
#include "Brimstone.h"

#include <Dib.h>
#include <HistogramDataSeries.h>

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView

IMPLEMENT_DYNCREATE(CBrimstoneView, CView)

BEGIN_MESSAGE_MAP(CBrimstoneView, CView)
	//{{AFX_MSG_MAP(CBrimstoneView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_SCANBEAMLETS_G0, OnScanbeamletsG0)
	ON_COMMAND(ID_VIEW_STRUCT_COLORWASH, OnViewStructColorwash)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRUCT_COLORWASH, OnUpdateViewStructColorwash)
	ON_COMMAND(ID_SCANBEAMLETS_G1, OnScanbeamletsG1)
	ON_COMMAND(ID_SCANBEAMLETS_G2, OnScanbeamletsG2)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView construction/destruction

CBrimstoneView::CBrimstoneView()
: m_bColorwashStruct(FALSE)
{
	// TODO: add construction code here

}

CBrimstoneView::~CBrimstoneView()
{
}

BOOL CBrimstoneView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}


// generates a histogram for the specified structure
void CBrimstoneView::AddHistogram(CStructure * pStruct)
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
	ASSERT(pHisto != NULL);

	CHistogramDataSeries *pSeries = new CHistogramDataSeries(pHisto);
	pSeries->SetColor(pStruct->GetColor());

	m_graph.AddDataSeries(pSeries);
	m_graph.AutoScale();
	m_graph.SetAxesMin(CVectorD<2>(0.0f, 0.0f));
}

// removes histogram for designated structure
void CBrimstoneView::RemoveHistogram(CStructure * pStruct)
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
	ASSERT(pHisto != NULL);

	for (int nAt = 0; nAt < m_graph.GetDataSeriesCount(); nAt++)
	{
		CHistogramDataSeries *pSeries = 
			static_cast<CHistogramDataSeries *>(m_graph.GetDataSeriesAt(nAt));
		if (pSeries->GetHistogram() == pHisto)
		{
			m_graph.RemoveDataSeries(nAt);
			m_graph.AutoScale();
			m_graph.SetAxesMin(CVectorD<2>(0.0f, 0.0f));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView drawing

void CBrimstoneView::OnDraw(CDC* pDC)
{
	CBrimstoneDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CView::OnDraw(pDC);

	return;

	if (!pDoc->m_pPlan) 
		return;
	if (!pDoc->m_pSeries)
		return;

	CRect rect;
	GetClientRect(&rect);

	CRect rectGraph;
	m_graph.GetWindowRect(&rectGraph);
	ScreenToClient(&rectGraph);

	rect.bottom = rectGraph.top;
	rect.right = rect.left + rect.Height();

	CVolume<VOXEL_REAL>& density = *pDoc->m_pSeries->m_pDens;

	CMatrixD<4> mBasis = density.GetBasis();
	mBasis[0][0] = (REAL) density.GetWidth() / (REAL) rect.Width();
	mBasis[1][1] = (REAL) density.GetHeight() / (REAL) rect.Height();

	static CVolume<VOXEL_REAL> densityResamp;
	if (densityResamp.GetWidth() != rect.Width())
	{
		densityResamp.SetDimensions(rect.Width(), rect.Height(), 1);
		densityResamp.SetBasis(mBasis);
	}
	densityResamp.ClearVoxels();

	Resample(&density, &densityResamp, TRUE);

	CVolume<VOXEL_REAL>& totalDose = *pDoc->m_pPlan->GetDoseMatrix();

	static CVolume<VOXEL_REAL> doseResamp;
	doseResamp.ConformTo(&densityResamp);
	doseResamp.ClearVoxels();

	Resample(&totalDose, &doseResamp, TRUE);

	static CArray<COLORREF, COLORREF> arrPixels;
	arrPixels.SetSize(rect.Width() * rect.Height());

	VOXEL_REAL *pDens = &densityResamp.GetVoxels()[0][0][0];
	VOXEL_REAL *pDose = &doseResamp.GetVoxels()[0][0][0];
	int nRowSize = densityResamp.GetWidth();
	for (int nAt = 0; nAt < nRowSize * nRowSize; nAt++)
	{
		VOXEL_REAL dose = (VOXEL_REAL)(pDose[nAt] / 0.80);
		int colorIndex = (int)(dose * (VOXEL_REAL) (m_arrColormap.GetSize()-1));
		colorIndex = __min(colorIndex, m_arrColormap.GetSize()-1);
		COLORREF color = m_arrColormap[colorIndex];

		VOXEL_REAL intensity = pDens[nAt];
		// intensity /= 1000.0;

		color = RGB(intensity * GetBValue(color), 
			intensity * GetGValue(color),
			intensity * GetRValue(color));

		arrPixels[nAt] = color;
	}

	static CDib dib;
	if (dib.GetSize().cx != rect.Width())
	{
		dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(*pDC, rect.Width(), rect.Height());
		dib.Attach(bm);
	}
	DWORD dwCount = dib.SetBitmapBits(rect.Width() * rect.Height() * sizeof(COLORREF),
		(void *) arrPixels.GetData());

//	PLDrawBitmap(*pDC, &dib, NULL, NULL, SRCCOPY);

	DrawContours(pDC, rect, &density);

//	ValidateRect(NULL);

	CView::OnDraw(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView printing

BOOL CBrimstoneView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBrimstoneView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBrimstoneView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView diagnostics

#ifdef _DEBUG
void CBrimstoneView::AssertValid() const
{
	CView::AssertValid();
}

void CBrimstoneView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBrimstoneDoc* CBrimstoneView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBrimstoneDoc)));
	return (CBrimstoneDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView message handlers

int CBrimstoneView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create the planar window
	m_wndPlanarView.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 0, 200, 200), this, /* nID */ 111);

	// load the colormap for dose display
	CDib colormap;
	BOOL bResult = colormap.Load(GetModuleHandle(NULL), IDB_RAINBOW);	
	CSize size = colormap.GetSize();
	m_arrColormap.SetSize(size.cx * size.cy);

	CArray<UCHAR, UCHAR&> arrRaw;
	arrRaw.SetSize(size.cx * size.cy * 3);
	colormap.GetBitmapBits(size.cx * size.cy * 3, arrRaw.GetData());

	int nAtRaw = 0;
	for (int nAt = 0; nAt < m_arrColormap.GetSize(); nAt++)
	{ 
		m_arrColormap[nAt] = RGB(arrRaw[nAtRaw+2], arrRaw[nAtRaw+1], arrRaw[nAtRaw]);
		nAtRaw += 3;
	}

	m_wndPlanarView.SetLUT(m_arrColormap, 1); 

	// create the graph window
	m_graph.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 200, 200, 400), this, /* nID */ 113);

	m_graph.SetLegendLUT(m_arrColormap, 
		m_wndPlanarView.m_window[1], m_wndPlanarView.m_level[1]);

	// set timer to update plan
	// SetTimer(7, 20, NULL);

	return 0;
}


void CBrimstoneView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	if (GetDocument()->m_pSeries)
	{
		m_wndPlanarView.SetVolume(GetDocument()->m_pSeries->m_pDens, 0);
		m_wndPlanarView.SetBasis(GetDocument()->m_pSeries->m_pDens->GetBasis());
		m_wndPlanarView.m_pSeries = GetDocument()->m_pSeries;

	}
	if (GetDocument()->m_pPlan)
	{
		m_wndPlanarView.SetVolume(GetDocument()->m_pPlan->GetDoseMatrix(), 1);
	}

	m_wndPlanarView.Invalidate(TRUE);

	// delete histograms
	m_graph.RemoveAllDataSeries();

	// generate ADDHISTO events
	for (int nAt = 0; nAt < GetDocument()->m_pSeries->GetStructureCount(); nAt++)
	{
		CStructure *pStruct = GetDocument()->m_pSeries->GetStructureAt(nAt);
		CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
		if (NULL != pHisto)
		{
			AddHistogram(pStruct);
		}
	}

	// TODO: generate events for prescriptions

	m_graph.Invalidate(TRUE);
}

void CBrimstoneView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (IDC_STRUCTSELECT == lHint)
	{
		CStructure *pStruct = (CStructure *) pHint;

		// clear target curve from CGraph

		// set target curve in CGraph
	}
	else if (IDD_ADDHISTO == lHint)
	{
		CStructure *pStruct = (CStructure *) pHint;
		ASSERT(pStruct->IsKindOf(RUNTIME_CLASS(CStructure)));

		AddHistogram(pStruct);
	}
	else if (IDD_REMOVEHISTO == lHint)
	{
		CStructure *pStruct = (CStructure *) pHint;
		ASSERT(pStruct->IsKindOf(RUNTIME_CLASS(CStructure)));

		RemoveHistogram(pStruct);
	}
/*	else if (IDD_ADDPRESC == lHint)
	{
		CStructure *pStruct = (CStructure *) pHint;
		CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct);
		CHistogramDataSeries *pSeries = new CHistogramDataSeries(pHisto);
		pSeries->SetColor(pStruct->GetColor());
		m_graph.AddDataSeries(pSeries);
		m_graph.AutoScale();
		m_graph.SetAxesMin(CVectorD<2>(0.0f, 0.0f));
	}
*/	
	Invalidate(FALSE);
	m_graph.Invalidate(TRUE);
	m_wndPlanarView.Invalidate(TRUE);

	// DON'T CALL because it will erase
	// CView::OnUpdate(pSender, lHint, pHint);
}

void CBrimstoneView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	m_wndPlanarView.MoveWindow(0, 0, cx, 2 * cy / 3);

	// reposition the graph window
	m_graph.MoveWindow(0, 2 * cy / 3, cx, cy / 3);	
}


void CBrimstoneView::OnTimer(UINT nIDEvent) 
{
//	GetDocument()->UpdateFromOptimizer();

	// m_graph.Invalidate(TRUE);

	CView::OnTimer(nIDEvent);
}

CPoint ToDC(const CVectorD<2>& vVert, const CRect& rect, CVolume<VOXEL_REAL> *pDens)
{
	CPoint pt(Round<int>(vVert[0] / (REAL) pDens->GetWidth() * (REAL) rect.Width()),
		Round<int>(vVert[1] / (REAL) pDens->GetHeight() * (REAL) rect.Height()));

	pt += rect.TopLeft();
	return pt;
		
}
void CBrimstoneView::DrawContours(CDC *pDC, const CRect& rect, CVolume<VOXEL_REAL> *pDens)
{
	CSeries *pSeries = GetDocument()->m_pSeries;
	for (int nAtStruct = 0; nAtStruct < pSeries->GetStructureCount(); nAtStruct++)
	{
		CStructure *pStruct = pSeries->GetStructureAt(nAtStruct);
		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			CPolygon *pPoly = pStruct->GetContour(nAtContour);
			pDC->MoveTo(ToDC(pPoly->GetVertexAt(0), rect, pDens));
			for (int nAtVert = 1; nAtVert < pPoly->GetVertexCount(); nAtVert++)
			{
				pDC->LineTo(ToDC(pPoly->GetVertexAt(nAtVert), rect, pDens));
			}
		}
	}
}

void CBrimstoneView::OnScanbeamletsG0() 
{
	ScanBeamlets(0);
}

void CBrimstoneView::OnViewStructColorwash() 
{
	m_bColorwashStruct = !m_bColorwashStruct;

	if (m_bColorwashStruct)
	{
		CStructure *pStruct = GetDocument()->m_pSelectedStruct;
		if (pStruct != NULL)
		{
			CVolume<VOXEL_REAL> *pRegion = pStruct->GetRegion(2);
			m_wndPlanarView.SetVolume(pRegion, 1);
		}
	}
	else
	{
		m_wndPlanarView.SetVolume(NULL, 1);
	}


	m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CBrimstoneView::OnUpdateViewStructColorwash(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bColorwashStruct ? 1 : 0);	
}

void CBrimstoneView::ScanBeamlets(int nLevel)
{
	CPlan *pPlan = GetDocument()->m_pPlan;

	if (nLevel == 0)
	{
		for (int nAt = pPlan->GetBeamAt(0)->GetBeamletCount()-1; nAt >= 0; nAt--)
		{
			CVectorN<> vWeights;
			vWeights.SetDim(pPlan->GetBeamAt(0)->GetBeamletCount());
			vWeights.SetZero();
	/*		vWeights[nAt] = 0.0;

			for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
			{
				CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
				pBeam->SetIntensityMap(vWeights);
			}
			pPlan->GetDoseMatrix();

			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	*/		vWeights[nAt] = (REAL) 0.8;

			for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
			{
				CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
				pBeam->SetIntensityMap(vWeights);
			}
			pPlan->GetDoseMatrix();

			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}	
	}
	else
	{
		for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
		{
			CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
			for (int nShift = -pBeam->GetBeamletCount(nLevel) / 2; 
					nShift<= pBeam->GetBeamletCount(nLevel) / 2; nShift++)
			{
				CVolume<VOXEL_REAL> *pBeamlet = pBeam->GetBeamlet(nShift, nLevel);
				m_wndPlanarView.SetVolume(pBeamlet, 1);
				m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			}

		}	
	}
}

void CBrimstoneView::OnScanbeamletsG1() 
{
	ScanBeamlets(1);	
}

void CBrimstoneView::OnScanbeamletsG2() 
{
	ScanBeamlets(2);
}
