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

#include <Matrix.h>

#include "ScribDoc.h"
#include "ScribVw.h"
// #include <QuadEdge1.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// #define DRAW_STROKES

/////////////////////////////////////////////////////////////////////////////
// CScribbleView

IMPLEMENT_DYNCREATE(CScribbleView, CView)

BEGIN_MESSAGE_MAP(CScribbleView, CView)
	//{{AFX_MSG_MAP(CScribbleView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
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

void CScribbleView::DrawPolygon(CDC *pDC, CPolygon& polygon, COLORREF color)
{
	// don't draw if no vertices
	if (polygon.GetVertexCount() < 2)
		return;

	// create the pen and select it
	CPen penStroke;
	penStroke.CreatePen(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&penStroke);

	// now draw the polygon
	pDC->MoveTo(m_xform * polygon.GetVertex(0));
	for (int i=1; i < polygon.GetVertexCount(); i++)
		pDC->LineTo(m_xform * polygon.GetVertex(i));

	// de-select the pen
	pDC->SelectObject(pOldPen);
}

void CScribbleView::OnDraw(CDC* pDC)
{
	CScribbleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// The view delegates the drawing of individual strokes to
	// CStroke::DrawStroke().
/*	CTypedPtrList<CObList,CStroke*>& strokeList = pDoc->m_strokeList;
	POSITION pos = strokeList.GetHeadPosition();
	while (pos != NULL)
	{
		CStroke* pStroke = strokeList.GetNext(pos);
		pStroke->DrawStroke(pDC);
	}
*/

	DrawPolygon(pDC, pDoc->m_polygon);

	CPolygon convexHull(pDoc->m_polygon);
	convexHull.MakeConvexHull();

	DrawPolygon(pDC, convexHull, RGB(255, 0, 0));
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
	// add the new vertex to the polygon
	GetDocument()->m_polygon.AddVertex(m_invXform * CVector<2>(point));

	// set mouse capture
	SetCapture();       // Capture the mouse until button up.

	// store the previous point
	m_ptPrev = point;   // Serves as the MoveTo() anchor point for the
						// LineTo() the next point, as the user drags the
						// mouse.

	// redraw the window
	RedrawWindow();
}

void CScribbleView::OnLButtonUp(UINT, CPoint point)
{
	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

	ReleaseCapture();   // Release the mouse capture established at
						// the beginning of the mouse drag.
}

void CScribbleView::OnMouseMove(UINT, CPoint point)
{
	if (GetCapture() != this)
		return; // If this window (view) didn't capture the mouse,
				// then the user isn't drawing in this window.

	CPoint ptOffset = point - m_ptPrev;
	if (ptOffset.x * ptOffset.x + ptOffset.y * ptOffset.y > 10 * 10)
	{
		// add the point to the polygon
		GetDocument()->m_polygon.AddVertex(m_invXform * CVector<2>(point));

		// store the previous point
		m_ptPrev = point;

		// redraw the window
		RedrawWindow();
	}
}

void CScribbleView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// close the polygon
	CVector<2> vInitVertex = GetDocument()->m_polygon.GetVertex(0);
	GetDocument()->m_polygon.AddVertex(vInitVertex);

	// redraw the window
	RedrawWindow();

	CView::OnLButtonDblClk(nFlags, point);
}

void CScribbleView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// set up the scale matrix
	m_xform[0][0] = (double) cx;
	m_xform[1][1] = (double) cy;

	// set up the inverse scale matrix
	m_invXform = Invert(m_xform);
}
