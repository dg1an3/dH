// XMLConsoleView.cpp : implementation of the CXMLConsoleView class
//

#include "stdafx.h"
#include "XMLConsole.h"

#include "XMLConsoleDoc.h"
#include "XMLConsoleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView

IMPLEMENT_DYNCREATE(CXMLConsoleView, CListView)

BEGIN_MESSAGE_MAP(CXMLConsoleView, CListView)
	//{{AFX_MSG_MAP(CXMLConsoleView)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView construction/destruction

CXMLConsoleView::CXMLConsoleView()
: m_pTreeCtrl(NULL)
{
	// TODO: add construction code here

}

CXMLConsoleView::~CXMLConsoleView()
{
}

BOOL CXMLConsoleView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (LVS_REPORT | LVS_OWNERDATA); // | LVS_NOCOLUMNHEADER);
	
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView drawing

void CXMLConsoleView::OnDraw(CDC* pDC)
{
	CXMLConsoleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CListCtrl& refCtrl = GetListCtrl();
	refCtrl.InsertItem(0, "Item!");
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView printing

BOOL CXMLConsoleView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CXMLConsoleView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CXMLConsoleView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView diagnostics

#ifdef _DEBUG
void CXMLConsoleView::AssertValid() const
{
	CListView::AssertValid();
}

void CXMLConsoleView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CXMLConsoleDoc* CXMLConsoleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXMLConsoleDoc)));
	return (CXMLConsoleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleView message handlers
void CXMLConsoleView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window
}

void CXMLConsoleView::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);
	
	int nCountPerPage = GetListCtrl().GetCountPerPage();
	GetListCtrl().SetItemCountEx(nCountPerPage, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);

	if (m_pTreeCtrl)
	{
		CRect rectItem;
		GetListCtrl().GetItemRect(0, rectItem, LVIR_BOUNDS);
		m_pTreeCtrl->SetItemHeight(rectItem.Height());
	}
}

void CXMLConsoleView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	CRect rectItem;
	GetListCtrl().GetItemRect(pDispInfo->item.iItem, rectItem, LVIR_BOUNDS);

	CPoint pt = rectItem.CenterPoint();
	GetListCtrl().ClientToScreen(&pt);

	m_pTreeCtrl->ScreenToClient(&pt);
	pt.x = 10;

	HTREEITEM hItem = m_pTreeCtrl->HitTest(pt);
	if (hItem != NULL)
	{
		DWORD dwUID = m_pTreeCtrl->GetItemData(hItem);
		IXMLElement *pIElem = GetDocument()->GetElementByUID(dwUID);
		if (pIElem)
		{
			static char pszText[10000];

			switch (pDispInfo->item.iSubItem)
			{
			case 0 :
				strcpy(pszText, GetDocument()->GetText(pIElem));
				pDispInfo->item.pszText = pszText;
				break;

			case 1 :
				strcpy(pszText, GetDocument()->GetAttribute(pIElem, "type"));
				pDispInfo->item.pszText = pszText;
				break;
			}
		}
	}

	*pResult = 0;
}

