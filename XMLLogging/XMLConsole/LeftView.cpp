// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "XMLConsole.h"

#include "XMLConsoleDoc.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CView)

BEGIN_MESSAGE_MAP(CLeftView, CView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREECTRL, OnItemexpanded)
	ON_NOTIFY(TVN_GETDISPINFO, IDC_TREECTRL, OnGetdispinfo)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeftView construction/destruction

CLeftView::CLeftView()
: m_pListCtrl(NULL),
	m_dwMaxUID(0)
{
	// TODO: add construction code here

}

CLeftView::~CLeftView()
{
}

HTREEITEM CLeftView::GetNodeForUID(DWORD dwUID)
{
	HTREEITEM hItem = NULL;
	BOOL bRes = m_mapUIDtoNode.Lookup((void *) dwUID, (void *&) hItem);
	ASSERT(bRes);

	return hItem;
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView drawing

void CLeftView::OnDraw(CDC* pDC)
{
	CXMLConsoleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


/////////////////////////////////////////////////////////////////////////////
// CLeftView printing

BOOL CLeftView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLeftView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLeftView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CLeftView::OnInitialUpdate()
{
	GetTreeCtrl().DeleteAllItems();

	m_mapUIDtoNode.RemoveAll();

	m_dwMaxUID = 0;

	CView::OnInitialUpdate();
}

void CLeftView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	IXMLElement *pIElem = NULL;
	for (m_dwMaxUID = 1; m_dwMaxUID < GetDocument()->m_dwMaxUID; m_dwMaxUID++)
	{
		pIElem = GetDocument()->GetElementByUID(m_dwMaxUID);
		if (pIElem != NULL)
		{
			HTREEITEM hParentItem = NULL;
			IXMLElement *pIElemParent = GetDocument()->GetParent(pIElem);
			if (pIElemParent)
			{
				// now locate the node
				hParentItem = 
					GetNodeForUID(GetDocument()->GetUID(pIElemParent));
				ASSERT(hParentItem != NULL);
			}

			CString strNodeLabel = "<" 
				+ GetDocument()->GetTagName(pIElem) + "> "
				+ GetDocument()->GetAttribute(pIElem, "name");
		
			HTREEITEM hItem = GetTreeCtrl().InsertItem(strNodeLabel, 
				I_IMAGECALLBACK, I_IMAGECALLBACK,
				hParentItem);
			ASSERT(hItem != NULL);

			GetTreeCtrl().SetItemData(hItem, m_dwMaxUID);
			m_mapUIDtoNode[(void *) m_dwMaxUID] = hItem;

			// GetTreeCtrl().Expand(hParentItem, TVE_EXPAND);
		}
	}

	m_dwMaxUID = GetDocument()->m_dwMaxUID;
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CXMLConsoleDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXMLConsoleDoc)));
	return (CXMLConsoleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLeftView message handlers

int CLeftView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_TreeCtrl.Create(WS_CHILD | WS_VISIBLE 
			| TVS_HASLINES | TVS_HASBUTTONS | WS_EX_LEFTSCROLLBAR, 
		CRect(0, 0, 100, 100), this, IDC_TREECTRL))
	{
		return -1;
	}

	return 0;
}

void CLeftView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pListCtrl)
	{
		CRect rect;
		m_pListCtrl->GetItemRect(0, rect, LVIR_BOUNDS);
		m_TreeCtrl.MoveWindow(0, rect.top - 1, 
			cx, cy - (rect.top - 1));
	}
}


void CLeftView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
	CRect rectClient;
	GetClientRect(rectClient);

	dc.SelectObject(GetStockObject(GRAY_BRUSH));

	dc.Rectangle(rectClient);

	// Do not call CView::OnPaint() for painting messages
}

void CLeftView::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_pListCtrl->RedrawItems(0, m_pListCtrl->GetCountPerPage());
	
	*pResult = 0;
}

void CLeftView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_pListCtrl->RedrawItems(0, m_pListCtrl->GetCountPerPage());

	*pResult = 0;
}


