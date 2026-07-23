// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrimstoneView.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include "Brimstone.h"

#include <HistogramDataSeries.h>
#ifdef USE_RTOPT
#include <TargetDVHSeries.h>
#include <KLDivTerm.h>
#include <PlanPyramid.h>
#endif

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"
#include "RenderControlDlg.h"
#include "OptGraphHtml.h"
#include "DvhViewHtml.h"

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
	ON_MESSAGE(WM_OPTIMIZER_UPDATE, OnOptimizerThreadUpdate)
	ON_MESSAGE(WM_OPTIMIZER_STOP, OnOptimizerThreadDone)
	ON_MESSAGE(WM_OPTIMIZER_DONE, OnOptimizerThreadDone)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(IDC_BTNOPTIMIZE, &CBrimstoneView::OnOptimize)
	ON_UPDATE_COMMAND_UI(IDC_BTNOPTIMIZE, &CBrimstoneView::OnUpdateOptimize)
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_SCANBEAMLETS, &CBrimstoneView::OnViewScanbeamlets)
	ON_COMMAND(ID_VIEW_RENDERCONTROL, &CBrimstoneView::OnViewRenderControl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView construction/destruction

/////////////////////////////////////////////////////////////////////////////
CBrimstoneView::CBrimstoneView()
	: m_pOptThread(NULL)
	, m_bOptimizerRun(false)
	, m_pRenderCtrlDlg(NULL)
	// , m_pIterDS(NULL)
{
	m_pOptThread = static_cast<COptThread*>(
		AfxBeginThread(RUNTIME_CLASS(COptThread), 
			THREAD_PRIORITY_BELOW_NORMAL,	// priority
			0,								// stack size
			CREATE_SUSPENDED				// flags
		));
	m_pOptThread->m_bAutoDelete = true;
	m_pOptThread->SetMsgTarget(this);
	m_pOptThread->ResumeThread();
}

/////////////////////////////////////////////////////////////////////////////
CBrimstoneView::~CBrimstoneView()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL 
	CBrimstoneView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::AddHistogram(dH::Structure * pStruct)
	// generates a histogram for the specified structure
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, true);
	ASSERT(pHisto != NULL);

	CHistogramDataSeries::Pointer pHistogramSeries = CHistogramDataSeries::New();
	pHistogramSeries->SetHistogram(pHisto);

	pHistogramSeries->m_pGraph = &m_graphDVH;
	pHistogramSeries->SetColor(pStruct->GetColor());

	CDataSeries::Pointer pSeries = pHistogramSeries;
	m_graphDVH.AddDataSeries(pSeries);
	m_graphDVH.AutoScale();
	m_graphDVH.SetAxesMin(MakeVector<2>(0.0f, 0.0f));
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::RemoveHistogram(dH::Structure * pStruct)
	// removes histogram for designated structure
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
	ASSERT(pHisto != NULL);

	for (int nAt = 0; nAt < m_graphDVH.GetDataSeriesCount(); nAt++)
	{
		CHistogramDataSeries *pSeries = 
			static_cast<CHistogramDataSeries *>(m_graphDVH.GetDataSeriesAt(nAt));
		if (pSeries->GetHistogram() == pHisto)
		{
			m_graphDVH.RemoveDataSeries(nAt);
			m_graphDVH.AutoScale();
			m_graphDVH.SetAxesMin(MakeVector<2>(0.0f, 0.0f));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::AddStructTerm(dH::VOITerm * pVOIT)
	// add new structure term
{
#ifdef USE_RTOPT
	// add to the prescription object
	GetDocument()->m_pOptimizer->/*GetPrescription(0)->*/AddStructureTerm(pVOIT);

	// get the struct
	dH::Structure *pStruct = pVOIT->GetVOI();

	// form the data series
	CTargetDVHSeries::Pointer pTargetDvhSeries = CTargetDVHSeries::New();
	pTargetDvhSeries->SetForKLDivTerm((dH::KLDivTerm*)pVOIT);
	pTargetDvhSeries->OnKLDTChanged();

	pTargetDvhSeries->SetColor(pStruct->GetColor());
	pTargetDvhSeries->SetPenStyle(PS_DASHDOT);
	pTargetDvhSeries->SetHasHandles(TRUE);

	CDataSeries::Pointer pSeries = pTargetDvhSeries;
	m_graphDVH.AddDataSeries(pSeries);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// WebView2 DVH view: data feed (C++ -> page) and edit handling (page -> C++)

static CString ColorToHex(COLORREF c)
{
	CString s;
	s.Format(_T("#%02x%02x%02x"), GetRValue(c), GetGValue(c), GetBValue(c));
	return s;
}

void CBrimstoneView::SendStructuresToDvh()
{
#ifdef USE_RTOPT
	CBrimstoneDoc *pDoc = GetDocument();
	if (pDoc == NULL || !pDoc->m_pSeries)
		return;
	dH::PlanOptimizer *pOpt = pDoc->m_pOptimizer.get();
	dH::Prescription *pPresc0 = (pOpt != NULL) ? pOpt->GetPrescription(0) : NULL;

	CString js = _T("setStructures([");
	const int n = pDoc->m_pSeries->GetStructureCount();
	for (int i = 0; i < n; i++)
	{
		dH::Structure *pStruct = pDoc->m_pSeries->GetStructureAt(i);
		double mn = 0.0, mx = 0.0, wt = 0.0;
		dH::VOITerm *pVOIT = (pPresc0 != NULL) ? pPresc0->GetStructureTerm(pStruct) : NULL;
		if (pVOIT != NULL)
		{
			wt = pVOIT->GetWeight();
			dH::KLDivTerm *pKL = dynamic_cast<dH::KLDivTerm*>(pVOIT);
			if (pKL != NULL) { mn = pKL->GetMinDose() * 100.0; mx = pKL->GetMaxDose() * 100.0; }
		}

		CString name(pStruct->GetName().c_str());
		name.Replace(_T("\\"), _T("\\\\"));
		name.Replace(_T("\""), _T("\\\""));

		CString item;
		item.Format(_T("%s{\"id\":%d,\"name\":\"%s\",\"color\":\"%s\",\"type\":%d,\"min\":%.4g,\"max\":%.4g,\"weight\":%.4g,\"priority\":%d}"),
			(i ? _T(",") : _T("")), i, (LPCTSTR)name, (LPCTSTR)ColorToHex(pStruct->GetColor()),
			(int)pStruct->GetType(), mn, mx, wt, pStruct->GetPriority());
		js += item;
	}
	js += _T("])");
	m_webDVH.ExecScript(js);
#endif
}

void CBrimstoneView::SendDvhCurvesToDvh()
{
#ifdef USE_RTOPT
	CString js = _T("setDvh([");
	const int cnt = m_graphDVH.GetDataSeriesCount();
	bool first = true;
	for (int i = 0; i < cnt; i++)
	{
		CDataSeries *pS = m_graphDVH.GetDataSeriesAt(i);
		if (pS == NULL)
			continue;
		const bool bTarget = (dynamic_cast<CTargetDVHSeries*>(pS) != NULL);
		const CMatrixNxM<>& m = pS->GetDataMatrix();
		const int nPts = m.GetCols();

		CString pts;
		int emitted = 0;
		for (int c = 0; c < nPts; c++)
		{
			const double x = m[c][0];	// 100 * dose
			const double y = m[c][1];	// volume %
			if (x < 0.0)
				continue;				// the GDI graph clips X<0 too
			CString p;
			p.Format(_T("%s[%.3g,%.3g]"), (emitted ? _T(",") : _T("")), x, y);
			pts += p;
			emitted++;
		}

		CString item;
		item.Format(_T("%s{\"id\":%d,\"color\":\"%s\",\"target\":%s,\"pts\":[%s]}"),
			(first ? _T("") : _T(",")), i, (LPCTSTR)ColorToHex(pS->GetColor()),
			(bTarget ? _T("true") : _T("false")), (LPCTSTR)pts);
		js += item;
		first = false;
	}
	js += _T("])");
	m_webDVH.ExecScript(js);
#endif
}

void CBrimstoneView::OnDvhMessage(const std::wstring & msg)
{
#ifdef USE_RTOPT
	CBrimstoneDoc *pDoc = GetDocument();
	if (pDoc == NULL || !pDoc->m_pSeries)
		return;
	dH::PlanOptimizer *pOpt = pDoc->m_pOptimizer.get();

	// split "cmd|arg|arg..."
	CString s(msg.c_str());
	CStringArray tok;
	int pos = 0;
	for (CString t = s.Tokenize(_T("|"), pos); !t.IsEmpty(); t = s.Tokenize(_T("|"), pos))
		tok.Add(t);
	if (tok.GetSize() < 1)
		return;
	const CString cmd = tok[0];

	dH::Series *pSeries = pDoc->m_pSeries;
	const int nStructs = pSeries->GetStructureCount();

	if (cmd == _T("weight") && tok.GetSize() >= 3 && pOpt != NULL)
	{
		const int id = _wtoi(tok[1]);
		if (id >= 0 && id < nStructs)
		{
			dH::VOITerm *pV = pOpt->GetPrescription(0)->GetStructureTerm(pSeries->GetStructureAt(id));
			if (pV != NULL)
				pV->SetWeight(_wtof(tok[2]));
		}
	}
	else if (cmd == _T("interval") && tok.GetSize() >= 4 && pOpt != NULL)
	{
		const int id = _wtoi(tok[1]);
		const double mn = _wtof(tok[2]), mx = _wtof(tok[3]);
		if (id >= 0 && id < nStructs && mn < mx)
		{
			dH::VOITerm *pV = pOpt->GetPrescription(0)->GetStructureTerm(pSeries->GetStructureAt(id));
			dH::KLDivTerm *pK = dynamic_cast<dH::KLDivTerm*>(pV);
			if (pK != NULL)
				pK->SetInterval((REAL)(mn / 100.0), (REAL)(mx / 100.0), 1.0, TRUE);
		}
	}
	else if (cmd == _T("color") && tok.GetSize() >= 3)
	{
		const int id = _wtoi(tok[1]);
		const unsigned int rgb = wcstoul(tok[2], NULL, 16);
		if (id >= 0 && id < nStructs)
			pSeries->GetStructureAt(id)->SetColor(RGB((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff));
	}
	else if (cmd == _T("type") && tok.GetSize() >= 3 && pOpt != NULL)
	{
		const int id = _wtoi(tok[1]);
		const int newType = _wtoi(tok[2]);
		if (id >= 0 && id < nStructs)
		{
			dH::Structure *pStruct = pSeries->GetStructureAt(id);
			dH::Prescription *pP0 = pOpt->GetPrescription(0);
			dH::VOITerm *pV = pP0->GetStructureTerm(pStruct);
			pStruct->SetType((dH::Structure::StructType) newType);

			if (newType == dH::Structure::eNONE)
			{
				if (pV != NULL)
					for (int lvl = 0; lvl < dH::PlanPyramid::MAX_SCALES; lvl++)
						pOpt->GetPrescription(lvl)->RemoveStructureTerm(pStruct);
				RemoveHistogram(pStruct);
			}
			else if (pV == NULL)
			{
				// set weight + interval BEFORE AddStructTerm: it clones the term
				// per pyramid level at call time, so later edits wouldn't propagate
				dH::KLDivTerm::Pointer pKL = dH::KLDivTerm::New();
				pKL->SetVOI(pStruct);
				pKL->SetWeight((REAL)2.5);
				const double d1 = (newType == dH::Structure::eTARGET) ? 0.60 : 0.0;
				const double d2 = (newType == dH::Structure::eTARGET) ? 0.70 : 0.30;
				pKL->SetInterval((REAL)d1, (REAL)d2, 1.0, TRUE);
				AddStructTerm(pKL);		// clones across levels + adds the target curve
				AddHistogram(pStruct);
			}
			pP0->SetElementInclude();
		}
	}
	else if (cmd == _T("order") && tok.GetSize() >= 2)
	{
		int p2 = 0, prio = 0;
		for (CString idt = tok[1].Tokenize(_T(","), p2); !idt.IsEmpty(); idt = tok[1].Tokenize(_T(","), p2))
		{
			const int id = _wtoi(idt);
			if (id >= 0 && id < nStructs)
				pSeries->GetStructureAt(id)->SetPriority(prio);
			prio++;
		}
	}

	// reflect the change back to the page and repaint the image view (colors/regions)
	SendStructuresToDvh();
	SendDvhCurvesToDvh();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView drawing

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneView::OnDraw(CDC* pDC)
{
	CBrimstoneDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CView::OnDraw(pDC);
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

/////////////////////////////////////////////////////////////////////////////
int 
	CBrimstoneView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
	m_arrColormap.SetSize(/* size.cx */ size.cy);

	CArray<UCHAR, UCHAR&> arrRaw;
	arrRaw.SetSize(size.cx * size.cy * 3);
	colormap.GetBitmapBits(size.cx * size.cy * 3, arrRaw.GetData());

	int nAtRaw = 0;
	for (int nAt = 0; nAt < m_arrColormap.GetSize(); nAt++)
	{ 
		m_arrColormap[nAt] = RGB(arrRaw[nAtRaw+2], arrRaw[nAtRaw+1], arrRaw[nAtRaw]);
		nAtRaw += (3 * size.cx);
	}

	m_wndPlanarView.SetLUT(m_arrColormap, 1); 

	m_wndPlanarView.SetWindowLevel((REAL) 1.0 / 0.8, 0.4, 1);

	// create the graph window
	m_graphDVH.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 200, 200, 400), this, /* nID */ 113);

	m_graphDVH.SetLegendLUT(m_arrColormap,
		m_wndPlanarView.m_window[1], m_wndPlanarView.m_level[1]);

	// WebView2 DVH view (chart + Target/OAR/None structure editor) in the same
	//	top-right quadrant. Placeholder geometry; real rect set in OnSize. The
	//	legacy GDI m_graphDVH is hidden -- its CHistogramDataSeries stay attached
	//	so GetDataMatrix() still yields the DVH curves we forward to the page.
	m_webDVH.Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
		CRect(0, 200, 200, 400), this, /* nID */ 116);
	m_webDVH.SetMessageHandler([this](const std::wstring& msg) { OnDvhMessage(msg); });
	m_webDVH.NavigateToString(g_dvhViewHtml);
	m_graphDVH.ShowWindow(SW_HIDE);

	// create the graph window
	m_graphIterations.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
		CRect(0, 200, 200, 400), this, /* nID */ 114);

	for (int nD = 0; nD < dH::Structure::MAX_SCALES; nD++)
		m_pIterDS[nD] = CDataSeries::New();
	m_pIterDS[0]->SetColor(RGB(255, 0, 0));
	m_graphIterations.AddDataSeries(m_pIterDS[0]);
	m_pIterDS[1]->SetColor(RGB(0, 255, 0));
	m_graphIterations.AddDataSeries(m_pIterDS[1]);
	m_pIterDS[2]->SetColor(RGB(0, 0, 255));
	m_graphIterations.AddDataSeries(m_pIterDS[2]);
	m_pIterDS[3]->SetColor(RGB(255, 0, 255));
	m_graphIterations.AddDataSeries(m_pIterDS[3]);

	// WebView2 iteration/convergence chart. Placeholder geometry here; the real
	//	rectangle (bottom-right quadrant) is set in OnSize. Phase 1: load a
	//	self-contained test page to verify the WebView2 integration renders inside
	//	the view. It sits over m_graphIterations, which still receives data so the
	//	existing GDI flow is left untouched until the chart takes over.
	m_webChart.Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,
		CRect(0, 400, 200, 600), this, /* nID */ 115);
	m_webChart.NavigateToString(g_optGraphHtml);

	// hide the legacy GDI iteration graph -- the WebView2 chart takes its place.
	//	The data series remain attached to m_graphIterations so the existing
	//	OnOptimizerThreadUpdate flow keeps working (it just paints into a hidden
	//	window) until the chart is wired to receive the data directly.
	m_graphIterations.ShowWindow(SW_HIDE);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnDestroy()
{
	if (m_pRenderCtrlDlg != NULL)
	{
		if (::IsWindow(m_pRenderCtrlDlg->GetSafeHwnd()))
			m_pRenderCtrlDlg->DestroyWindow();
		delete m_pRenderCtrlDlg;
		m_pRenderCtrlDlg = NULL;
	}

	m_pOptThread->PostThreadMessage(WM_QUIT, NULL, NULL);
	CView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
#ifdef USE_RTOPT
	m_pOptThread->SetPlanOpt(GetDocument()->m_pOptimizer.get());
#endif

	if (GetDocument()->m_pSeries)
	{
		m_wndPlanarView.SetSeries(GetDocument()->m_pSeries);
		//m_wndPlanarView.SetVolume(GetDocument()->m_pSeries->GetDensity(), 0);
		//m_wndPlanarView.m_pSeries = GetDocument()->m_pSeries.get();

	}
	if (GetDocument()->m_pPlan)
	{
		m_wndPlanarView.SetVolume(GetDocument()->m_pPlan->GetDoseMatrix(), 1);
	}
	m_wndPlanarView.Invalidate(TRUE);

	// set the initial view center
	m_wndPlanarView.InitZoomCenter();
	if (GetDocument()->m_pPlan
		&& GetDocument()->m_pPlan->GetBeamCount() > 0)
	{
		Vector<REAL> vIsocenter = GetDocument()->m_pPlan->GetBeamAt(0)->GetIsocenter();
		m_wndPlanarView.SetCenter(vIsocenter);
	}

	// delete histograms
	m_graphDVH.RemoveAllDataSeries();

	// generate ADDHISTO events
	for (int nAt = 0; nAt < GetDocument()->m_pSeries->GetStructureCount(); nAt++)
	{
		dH::Structure *pStruct = GetDocument()->m_pSeries->GetStructureAt(nAt);
		CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
		if (NULL != pHisto)
		{
			AddHistogram(pStruct);
		}
	}

	m_graphDVH.Invalidate(TRUE);

	// seed the WebView2 DVH view with the current structures + curves
	SendStructuresToDvh();
	SendDvhCurvesToDvh();
}

/////////////////////////////////////////////////////////////////////////////
void
	CBrimstoneView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	m_wndPlanarView.MoveWindow(0, 0, 5 * cx / 8, cy);

	// reposition the graph window
	m_graphDVH.MoveWindow(5 * cx / 8, 0, 3 * cx / 8, cy / 2);

	// WebView2 DVH view occupies the same top-right quadrant, over the GDI graph
	if (m_webDVH.GetSafeHwnd() != NULL)
		m_webDVH.MoveWindow(5 * cx / 8, 0, 3 * cx / 8, cy / 2);

	// reposition the iterations window
	m_graphIterations.MoveWindow(5 * cx / 8, cy / 2, 3 * cx / 8, cy / 2);

	// WebView2 chart occupies the same bottom-right quadrant, over the GDI graph
	if (m_webChart.GetSafeHwnd() != NULL)
		m_webChart.MoveWindow(5 * cx / 8, cy / 2, 3 * cx / 8, cy / 2);
}


static int m_nTotalIter;
static FILE *m_pOutFile = NULL;

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnOptimize()
{
	if (!m_bOptimizerRun)
	{
		m_nTotalIter = 0;

		// clear iteration data matrix
		CMatrixNxM<> mEmpty;
		for (int nL = 0; nL < dH::Structure::MAX_SCALES; nL++)
			m_pIterDS[nL]->SetDataMatrix(mEmpty);

		// clear the WebView2 convergence chart
		m_webChart.ExecScript(_T("resetChart()"));

		m_pOptThread->PostThreadMessage(WM_OPTIMIZER_START, 0, 0);
		m_bOptimizerRun = true;
	}
	else
	{
		m_pOptThread->PostThreadMessage(WM_OPTIMIZER_STOP, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneView::OnUpdateOptimize(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOptimizerRun ? 1 : 0);
}

/////////////////////////////////////////////////////////////////////////////
LRESULT 
	CBrimstoneView::OnOptimizerThreadUpdate(WPARAM wParam, LPARAM lParam)
{
#ifdef USE_RTOPT
	USES_CONVERSION;
	
	const bool bUpdateDisplay = true;
	if (bUpdateDisplay)
	{
		// check if another WM_OPTIMIZER_UPDATE message is waiting
		MSG msg;
		const bool bPeekMessage = false;
		if (bPeekMessage
			&& ::PeekMessage(&msg, (HWND) INVALID_HANDLE_VALUE, WM_OPTIMIZER_UPDATE, WM_OPTIMIZER_UPDATE, PM_REMOVE))
		{
			// if so, skip processing this one
			return 0;
		}

		COptThread::COptIterData *pOID = (COptThread::COptIterData *) lParam;
		ASSERT(pOID != NULL);

		if (pOID->m_ofvalue > 0.0)
		{
			const double yVal = -log10(pOID->m_ofvalue);
			m_pIterDS[pOID->m_nLevel]->AddDataPoint(MakeVector<2>(m_nTotalIter, yVal));

			// feed the same point to the WebView2 convergence chart
			CString strJs;
			strJs.Format(_T("addPoint(%d,%d,%.6g)"), pOID->m_nLevel, m_nTotalIter, yVal);
			m_webChart.ExecScript(strJs);
		}

		const int nUpdateEvery = 1;
		if (m_nTotalIter % nUpdateEvery == 0)
		{
			GetDocument()->m_pOptimizer->SetStateVectorToPlan(pOID->m_vParam);
			SendDvhCurvesToDvh();
		}
		m_nTotalIter++;

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
LRESULT 
	CBrimstoneView::OnOptimizerThreadDone(WPARAM wParam, LPARAM lParam)
{
#ifdef USE_RTOPT
	COptThread::COptIterData *pOID = (COptThread::COptIterData *) lParam;
	if (pOID != NULL)
	{
		GetDocument()->m_pOptimizer->SetStateVectorToPlan(pOID->m_vParam);
		SendDvhCurvesToDvh();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	m_bOptimizerRun = false;

	// Sweep instrumentation: when BRIMSTONE_OBJECTIVE_FILE is set (e.g. by the
	//	isocenter-sweep driver), write the final level-0 objective value to that
	//	file. The file's appearance also serves as the optimizer-done signal, so
	//	the driver can wait on a result instead of sleeping a fixed interval.
	//	Inert for normal interactive runs where the env var is unset.
	char szObjFile[MAX_PATH] = {0};
	if (GetEnvironmentVariableA("BRIMSTONE_OBJECTIVE_FILE", szObjFile, MAX_PATH) > 0)
	{
		dH::PlanOptimizer *pOpt = GetDocument()->m_pOptimizer.get();
		if (pOpt != NULL)
		{
			// level-0 (finest) values. The optimizer minimizes F = KL - w*H
			//	(softmax entropy), so GetFinalValue() is F; the prescription
			//	caches the KL and entropy split from its last evaluation.
			const REAL freeEnergy = pOpt->GetOptimizer(0)->GetFinalValue();
			dH::Prescription *pPresc0 = pOpt->GetPrescription(0);
			const REAL kl = pPresc0->GetLastKL();
			const REAL entropy = pPresc0->GetLastEntropy();
			FILE *fp = NULL;
			if (fopen_s(&fp, szObjFile, "w") == 0 && fp != NULL)
			{
				fprintf(fp, "free_energy=%.10g kl=%.10g entropy=%.10g\n",
					(double) freeEnergy, (double) kl, (double) entropy);
				fclose(fp);
			}
		}
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::ScanBeamlets(int nLevel)
{
	dH::Plan::Pointer pPlan = GetDocument()->m_pPlan;

	if (nLevel == 0)
	{
		for (int nAt = pPlan->GetBeamAt(0)->GetBeamletCount()-1; 
			nAt >= // 0; // 
				pPlan->GetBeamAt(0)->GetBeamletCount()/2; 
			nAt--)
		{
			CVectorN<> vWeights;
			vWeights.SetDim(pPlan->GetBeamAt(0)->GetBeamletCount());
			vWeights.SetZero();
			vWeights[nAt] = (REAL) 0.8;

			for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
			{
				CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
				pBeam->SetIntensityMap(vWeights);
			}
			//CVolume<VOXEL_REAL> *pDose = 
			pPlan->GetDoseMatrix();		// triggers recalc
			//VOXEL_REAL maxDose = pDose->GetMax();
			//TRACE("maxDose = %f\n", maxDose);

			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}	
	}
	//else
	//{
	//	for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
	//	{
	//		CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
	//		for (int nShift = -pBeam->GetBeamletCount(nLevel) / 2; 
	//				nShift<= pBeam->GetBeamletCount(nLevel) / 2; nShift++)
	//		{
	//			CVolume<VOXEL_REAL> *pBeamlet = pBeam->GetBeamlet(nShift);
	//			m_wndPlanarView.SetVolume(pBeamlet, 1);
	//			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	//		}

	//	}	
	//}
}

////////////////////////////////////////////////////////////////////////////
void
	CBrimstoneView::OnViewScanbeamlets()
{
	ScanBeamlets(0);

}

////////////////////////////////////////////////////////////////////////////
void
	CBrimstoneView::OnViewRenderControl()
	// toggles the modeless render-control dialog used for UIAutomation testing
{
	if (m_pRenderCtrlDlg == NULL)
	{
		m_pRenderCtrlDlg = new CRenderControlDlg(this);
		m_pRenderCtrlDlg->Create(CRenderControlDlg::IDD, this);
	}

	if (m_pRenderCtrlDlg->IsWindowVisible())
	{
		m_pRenderCtrlDlg->ShowWindow(SW_HIDE);
	}
	else
	{
		// re-seed the controls with the current render state before showing
		m_pRenderCtrlDlg->RefreshFromView();
		m_pRenderCtrlDlg->ShowWindow(SW_SHOW);
		m_pRenderCtrlDlg->SetForegroundWindow();
	}
}
