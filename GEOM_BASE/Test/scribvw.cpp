// ScribVw.cpp : implementation of the CScribbleView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "Scribble.h"

#include "ScribDoc.h"
#include "ScribVw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDC *g_pDC = NULL;
double g_cx = 100.0;
double g_cy = 100.0;

void DrawLine(double *a, double *b)
{
	g_pDC->MoveTo(CPoint((int) (a[0] * g_cx), (int) (a[1] * g_cy)));
	g_pDC->LineTo(CPoint((int) (b[0] * g_cx), (int) (b[1] * g_cy)));
}

/////////////////////////////////////////////////////////////////////////////
// CScribbleView

IMPLEMENT_DYNCREATE(CScribbleView, CView)

BEGIN_MESSAGE_MAP(CScribbleView, CView)
	//{{AFX_MSG_MAP(CScribbleView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScribbleView construction/destruction

CScribbleView::CScribbleView()
{
}

CScribbleView::~CScribbleView()
{
}

BOOL CScribbleView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CScribbleView drawing

void CScribbleView::OnDraw(CDC* pDC)
{
	CScribbleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// The view delegates the drawing of individual strokes to
	// CStroke::DrawStroke().
	CTypedPtrList<CObList,CStroke*>& strokeList = pDoc->m_strokeList;
	POSITION pos = strokeList.GetHeadPosition();
	while (pos != NULL)
	{
		CStroke* pStroke = strokeList.GetNext(pos);
		pStroke->DrawStroke(pDC);
	}

	CRect rect;
	GetClientRect(&rect);

	// now draw the mesh
	g_cx = rect.Width();
	g_cy = rect.Height();

	g_pDC = pDC;

	pDoc->m_mesh.Draw();
}

/////////////////////////////////////////////////////////////////////////////
// CScribbleView printing

BOOL CScribbleView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CScribbleView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CScribbleView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CScribbleView diagnostics

#ifdef _DEBUG
void CScribbleView::AssertValid() const
{
	CView::AssertValid();
}

void CScribbleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CScribbleDoc* CScribbleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CScribbleDoc)));
	return (CScribbleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CScribbleView message handlers

void CScribbleView::OnLButtonDown(UINT, CPoint point)
{

#if defined(DRAW_STROKES)
	// Pressing the mouse button in the view window starts a new stroke
	m_pStrokeCur = GetDocument()->NewStroke();
	// Add first point to the new stroke
	m_pStrokeCur->m_pointArray.Add(point);

#else
	// Adding a point to the mesh
	CRect rect;
	GetClientRect(&rect);

	double x = (double) point.x / (double) rect.Width();
	double y = (double) point.y / (double) rect.Height();

	x = (x < 0.01) ? 0.01 : (x > 0.99) ? 0.99 : x;
	y = (y < 0.01) ? 0.01 : (y > 0.99) ? 0.99 : y;

	GetDocument()->m_mesh.InsertSite(Point2d(x, y));

	RedrawWindow();
#endif

	SetCapture();       // Capture the mouse until button up.
	m_ptPrev = point;   // Serves as the MoveTo() anchor point for the
						// LineTo() the next point, as the user drags the
						// mouse.
	return;
}

void CScribbleView::OnLButtonUp(UINT, CPoint point)
{
	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

#if defined(DRAW_STROKES)
	// Mouse button up is interesting in the Scribble application
	// only if the user is currently drawing a new stroke by dragging
	// the captured mouse.

	CScribbleDoc* pDoc = GetDocument();

	CClientDC dc(this);

	CPen* pOldPen = dc.SelectObject(pDoc->GetCurrentPen());
	dc.MoveTo(m_ptPrev);
	dc.LineTo(point);
	dc.SelectObject(pOldPen);
	m_pStrokeCur->m_pointArray.Add(point);

#endif

	ReleaseCapture();   // Release the mouse capture established at
						// the beginning of the mouse drag.
	return;
}

void CScribbleView::OnMouseMove(UINT, CPoint point)
{
	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

#if defined(DRAW_STROKES)
	// Mouse movement is interesting in the Scribble application
	// only if the user is currently drawing a new stroke by dragging
	// the captured mouse.

	CClientDC dc(this);
	m_pStrokeCur->m_pointArray.Add(point);

	// Draw a line from the previous detected point in the mouse
	// drag to the current point.
	CPen* pOldPen = dc.SelectObject(GetDocument()->GetCurrentPen());
	dc.MoveTo(m_ptPrev);
	dc.LineTo(point);
	dc.SelectObject(pOldPen);
	m_ptPrev = point;
#else
	CPoint ptOffset = point - m_ptPrev;
	if (ptOffset.x * ptOffset.x + ptOffset.y * ptOffset.y > 10 * 10)
	{
		// Adding a point to the mesh
		CRect rect;
		GetClientRect(&rect);

		double x = (double) point.x / (double) rect.Width();
		double y = (double) point.y / (double) rect.Height();

		x = (x < 0.01) ? 0.01 : (x > 0.99) ? 0.99 : x;
		y = (y < 0.01) ? 0.01 : (y > 0.99) ? 0.99 : y;

		GetDocument()->m_mesh.InsertSite(Point2d(x, y));

		m_ptPrev = point;
		RedrawWindow();
	}

#endif
	return;
}
