// BrimstoneView.cpp : implementation of the CBrimstoneView class
//

#include "stdafx.h"
#include "Brimstone.h"

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"

#include <Dib.h>

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
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView construction/destruction

CBrimstoneView::CBrimstoneView()
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

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView drawing

void CBrimstoneView::OnDraw(CDC* pDC)
{
	CBrimstoneDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

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

	CVolume<REAL>& density = *pDoc->m_pSeries->m_pDens;

	CMatrixD<4> mBasis = density.GetBasis();
	mBasis[0][0] = (REAL) density.GetWidth() / (REAL) rect.Width();
	mBasis[1][1] = (REAL) density.GetHeight() / (REAL) rect.Height();

	static CVolume<REAL> densityResamp;
	if (densityResamp.GetWidth() != rect.Width())
	{
		densityResamp.SetDimensions(rect.Width(), rect.Height(), 1);
		densityResamp.SetBasis(mBasis);
	}
	densityResamp.ClearVoxels();

	Resample(&density, &densityResamp, TRUE);

	CVolume<REAL>& totalDose = *pDoc->m_pPlan->GetDoseMatrix();

	static CVolume<REAL> doseResamp;
	doseResamp.ConformTo(&densityResamp);
	doseResamp.ClearVoxels();

	Resample(&totalDose, &doseResamp, TRUE);

	static CArray<COLORREF, COLORREF> arrPixels;
	arrPixels.SetSize(rect.Width() * rect.Height());

	REAL *pDens = &densityResamp.GetVoxels()[0][0][0];
	REAL *pDose = &doseResamp.GetVoxels()[0][0][0];
	int nRowSize = densityResamp.GetWidth();
	for (int nAt = 0; nAt < nRowSize * nRowSize; nAt++)
	{
		REAL dose = pDose[nAt] / 0.80;
		int colorIndex = (int)(dose * (REAL) (m_arrColormap.GetSize()-1));
		colorIndex = __min(colorIndex, m_arrColormap.GetSize()-1);
		COLORREF color = m_arrColormap[colorIndex];

		REAL intensity = pDens[nAt];
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

	PLDrawBitmap(*pDC, &dib, NULL, NULL, SRCCOPY);

	DrawContours(pDC, rect, &density);

	ValidateRect(NULL);

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
	
	// create the graph window
	m_graph.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 0, 200, 200), this, /* nID */ 111);

	// load the colormap for dose display
	CDib colormap;
	BOOL bResult = colormap.Load(GetModuleHandle(NULL), IDB_RAINBOW);	
	CSize size = colormap.GetSize();
	m_arrColormap.SetSize(size.cx * size.cy);

	unsigned char *pRaw = new unsigned char[size.cx * size.cy * 3];
	colormap.GetBitmapBits(size.cx * size.cy * 3, pRaw);

	int nAtRaw = 0;
	for (int nAt = 0; nAt < m_arrColormap.GetSize(); nAt++)
	{ 
		m_arrColormap[nAt] = RGB(pRaw[nAtRaw+2], pRaw[nAtRaw+1], pRaw[nAtRaw]);
		nAtRaw += 3;
	}

	// set timer to update plan
	SetTimer(7, 50, NULL);

	return 0;
}


void CBrimstoneView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	if (GetDocument()->m_pSeries)
	{
		for (int nAt = 0; nAt < GetDocument()->m_pSeries->GetStructureCount(); nAt++)
		{
			CStructure *pStruct = GetDocument()->m_pSeries->GetStructureAt(nAt);
			CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct);
			CHistogramDataSeries *pSeries = new CHistogramDataSeries(pHisto);
			m_graph.AddDataSeries(pSeries);
		}
	}
}

void CBrimstoneView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (lHint == IDC_STRUCTSELECT)
	{
		CStructure *pStruct = (CStructure *) pHint;

		// clear target curve from CGraph

		// set target curve in CGraph
	}

	
	Invalidate(FALSE);

	// DON'T CALL because it will erase
	// CView::OnUpdate(pSender, lHint, pHint);
}

void CBrimstoneView::OnDVHChanged()
{
	// set DVH values

	// set legend
}

void CBrimstoneView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	// reposition the graph window
	m_graph.MoveWindow(0, 2 * cy / 3, cx, cy / 3);	
}

void CBrimstoneView::OnHistogramChange(CObservableEvent *, void *)
{
	// locate the histo
	CHistogram *pHisto = NULL;

	// now draw the histogram
	const CVectorN<>& arrBins = pHisto->GetCumBins();
	CDataSeries *pSeries = new CDataSeries();
	m_graph.RemoveAllDataSeries();
	m_graph.AddDataSeries(pSeries);
	for (int nAt = 0; nAt < arrBins.GetDim(); nAt++)
	{
		pSeries->AddDataPoint(CVectorD<2>
			(1000 * nAt / 256, arrBins[nAt] * 100.0));
	}	
}


void CBrimstoneView::OnTimer(UINT nIDEvent) 
{
	GetDocument()->UpdateFromOptimizer();

	m_graph.Invalidate(TRUE);

	CView::OnTimer(nIDEvent);
}

CPoint ToDC(const CVectorD<2>& vVert, const CRect& rect, CVolume<REAL> *pDens)
{
	CPoint pt(
		vVert[0] / (REAL) pDens->GetWidth() * (REAL) rect.Width(),
		vVert[1] / (REAL) pDens->GetHeight() * (REAL) rect.Height());
	pt += rect.TopLeft();
	return pt;
		
}
void CBrimstoneView::DrawContours(CDC *pDC, const CRect& rect, CVolume<REAL> *pDens)
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
