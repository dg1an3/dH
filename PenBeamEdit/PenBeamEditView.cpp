// PenBeamEditView.cpp : implementation of the CPenBeamEditView class
//

#include "stdafx.h"
#include "PenBeamEdit.h"

// #include "PenBeamEditDoc.h"
#include "PenBeamEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView

IMPLEMENT_DYNCREATE(CPenBeamEditView, CView)

BEGIN_MESSAGE_MAP(CPenBeamEditView, CView)
	//{{AFX_MSG_MAP(CPenBeamEditView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView construction/destruction

CPenBeamEditView::CPenBeamEditView()
{
	// TODO: add construction code here

}

CPenBeamEditView::~CPenBeamEditView()
{
}

BOOL CPenBeamEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView drawing

void CPenBeamEditView::OnDraw(CDC* pDC)
{
	CPlan* pPlan = GetDocument();
	ASSERT_VALID(pPlan);

	CRect rect;
	GetClientRect(&rect);

	if (!pPlan->GetSeries())
		return;

	CVolume<short>& density = pPlan->GetSeries()->volume;
	CVolume<double>& totalDose = *pPlan->GetDoseMatrix();

	int nDrawSize = __min(rect.Width() / 2, rect.Height());
	int nRowSize = density.GetWidth();
	int nPixelSize = nDrawSize / nRowSize;
	if (nPixelSize % 2 == 1)
		nPixelSize++;
	for (int nAtY = 0; nAtY < nRowSize; nAtY++)
		for (int nAtX = 0; nAtX < nRowSize; nAtX++)
		{
			pDC->SelectStockObject(NULL_PEN);

#define SHOW_DOSE
#ifdef SHOW_DOSE
			double dose = totalDose.GetVoxels()[0][nAtY][nAtX];
			int colorIndex = (int)(dose * (double) (m_arrColormap.GetSize()-1));
			colorIndex = __min(colorIndex, m_arrColormap.GetSize()-1);
			COLORREF color = m_arrColormap[colorIndex];

			double intensity = density.GetVoxels()[0][nAtY][nAtX];
			intensity /= 1000.0;

			color = RGB(intensity * GetRValue(color), 
				intensity * GetGValue(color),
				intensity * GetBValue(color));

			CBrush brush(color);
#else
			int nDensity = (int)(density.GetVoxels()[0][nAtY][nAtX] * 255.0);
			CBrush brush(RGB(nDensity, nDensity, nDensity));
#endif
			CBrush *pOldBrush = pDC->SelectObject(&brush);

			pDC->Rectangle(nAtX * nPixelSize - nPixelSize / 2, 
				nAtY * nPixelSize - nPixelSize / 2,
				nAtX * nPixelSize + nPixelSize / 2 + 1, 
				nAtY * nPixelSize + nPixelSize / 2 + 1);

			pDC->SelectObject(pOldBrush);
		}
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView printing

BOOL CPenBeamEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPenBeamEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPenBeamEditView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView diagnostics

#ifdef _DEBUG
void CPenBeamEditView::AssertValid() const
{
	CView::AssertValid();
}

void CPenBeamEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPlan* CPenBeamEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPlan)));
	return (CPlan*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditView message handlers

int CPenBeamEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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

	return 0;
}

void CPenBeamEditView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	// reposition the graph window
	m_graph.MoveWindow(cx / 2, 0, cx / 2, cy);	
}

void CPenBeamEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CPlan* pPlan = GetDocument();
	ASSERT_VALID(pPlan);

	if (pPlan->GetSeries() == NULL)
		return;

	// set the histogram's volume
	m_histogram.SetVolume(pPlan->GetDoseMatrix());

	// set the histogram's region
	CVolume<double> *pRegion = pPlan->GetSeries()->GetStructureAt(0)->GetRegion();
		// ((CMesh *) pPlan->GetSeries()->m_arrStructures.GetAt(0))->m_pRegion;
	m_histogram.SetRegion(pRegion);

	// now draw the histogram
	const CVectorN<>& arrBins = m_histogram.GetCumBins();
	CDataSeries *pSeries = new CDataSeries();
	m_graph.RemoveAllDataSeries();
	// m_graph.m_arrDataSeries.RemoveAll();
	m_graph.AddDataSeries(pSeries);
	// m_graph.m_arrDataSeries.Add(pSeries);
	for (int nAt = 0; nAt < arrBins.GetDim(); nAt++)
	{
		pSeries->AddDataPoint(CVectorD<2>
			// pSeries->m_arrData.push_back(CVector<2>
			(1000 * nAt / 256, arrBins[nAt] * 100.0));
	}	
}
