// ObjectExplorer.cpp : implementation file
//

#include "stdafx.h"
#include "ObjectExplorer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CObjectExplorer

CObjectExplorer::CObjectExplorer()
: m_pDragImageList(NULL),
    m_bDragging(FALSE),
    m_pItemDrag(NULL),
    m_pItemDrop(NULL),
    m_pCommandTarget(NULL)
{
}

CObjectExplorer::~CObjectExplorer()
{
	CObArray arrDeleteList;

    POSITION pos = m_mapItemHandles.GetStartPosition();
    while (pos != NULL)
    {
		// The CObjectTreeItems are automatically deleted by their parent, 
		//		if they have one
    	HTREEITEM hItem;
    	CObjectTreeItem *pItem;
    	m_mapItemHandles.GetNextAssoc(pos, hItem, pItem);
		if (pItem->parent.Get() == NULL)
			arrDeleteList.Add(pItem);
    }

	for (int nAt = 0; nAt < arrDeleteList.GetSize(); nAt++)
    	delete arrDeleteList[nAt];
}

HTREEITEM CObjectExplorer::InsertItem(CObjectTreeItem *pNewItem)
{
    // get the handle for the parent item
    HTREEITEM hParentItem = TVI_ROOT;

    // only if the parent is not NULL,
    if (pNewItem->parent.Get() != NULL)
    {
    	// get the real handle to the parent
    	hParentItem = (HTREEITEM)(*pNewItem->parent.Get());
    }

 	// set up the images for the item

 	// add them to the list, if necessary
 	CBitmap bitmap;
 	bitmap.LoadBitmap(pNewItem->imageResourceID.Get());
	ASSERT(GetImageList(TVSIL_NORMAL) != NULL);
 	int nImageIndex = GetImageList(TVSIL_NORMAL)->Add(&bitmap, (COLORREF)0xFFFFFF);
 	bitmap.DeleteObject();
 
 	bitmap.LoadBitmap(pNewItem->selectedImageResourceID.Get());
 	int nSelImageIndex = GetImageList(TVSIL_NORMAL)->Add(&bitmap, (COLORREF)0xFFFFFF);
 	bitmap.DeleteObject();
 
    // create the tree control
    HTREEITEM hTreeItem = CTreeCtrl::InsertItem(TVIF_IMAGE 
    	| TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT,
    	pNewItem->label.Get(), 
 		nImageIndex, // pNewItem->GetImageIndex(),
 		nSelImageIndex, // pNewItem->GetSelectedImageIndex(),
    	INDEXTOSTATEIMAGEMASK(pNewItem->isChecked.Get() ? 2 : 1) 
    		| TVIS_EXPANDED, // nState, 
    	TVIS_STATEIMAGEMASK | 0xFF, // nStateMask, 
    	NULL, // lParam, 
    	hParentItem, TVI_LAST);

    // add the inserted item to the handle map
    m_mapItemHandles.SetAt(hTreeItem, pNewItem);

    // return the new item handle
    return hTreeItem;
}

void CObjectExplorer::DeleteItem(CObjectTreeItem *pItem)
{
    // check to make sure the window exists
    if (::IsWindow(m_hWnd))
    {
    	// and then delete the item
    	CTreeCtrl::DeleteItem((HTREEITEM)(*pItem));
    }

    // remove the item from the handle map
    m_mapItemHandles.RemoveKey((HTREEITEM)(*pItem));
}

void CObjectExplorer::SetToolTipMessages()
{
    // iterate over all of the tree view items
    POSITION pos = m_mapItemHandles.GetStartPosition();
    while (pos != NULL)
    {
    	// get the CObjectTreeItem object for this item
    	HTREEITEM hItem;
    	CObjectTreeItem *pItem;
    	m_mapItemHandles.GetNextAssoc(pos, hItem, pItem);

    	// delete any tooltips currently set up for the item
    	m_ToolTip.DelTool(this, (UINT) hItem);

    	// check to see if the item is visible, and if it can accept the
    	//		dragged item
    	CRect rect;		// stores the item's rectangle
    	CString strMessage;		// stores the drop tooltip
    	if (GetItemRect(hItem, &rect, FALSE)
    		&& pItem->IsDroppable(m_pItemDrag, &strMessage))
    	{
    		// if so, add the tooltip message for the item
    		m_ToolTip.AddTool(this, strMessage, rect, (UINT) hItem);
    	}
    }

    // assert that the number of tooltips is less than or equal to the
    //		number of displayed items
    // ASSERT(m_ToolTip.GetToolCount() <= (int) GetVisibleCount());
}

BOOL CObjectExplorer::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{	
    if (m_pCommandTarget != NULL)
    {
    	BOOL bResult = m_pCommandTarget->OnCmdMsg(nID, 
    		nCode, pExtra, pHandlerInfo);
    	if (bResult)
    	{
    		return TRUE;
    	}
    }

    return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BEGIN_MESSAGE_MAP(CObjectExplorer, CTreeCtrl)
    //{{AFX_MSG_MAP(CObjectExplorer)
    ON_WM_CREATE()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CLICK, OnCheck)
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnCheck)
    ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
    ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_WM_TIMER()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
    
/////////////////////////////////////////////////////////////////////////////
// CObjectExplorer message handlers

int CObjectExplorer::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
    	return -1;
    
    // intialize the drop and no-drop cursors
    m_hDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    m_hNoDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO));

    // create and initialize the tooltip
    m_ToolTip.Create(this);
    m_ToolTip.SetDelayTime(TTDT_INITIAL, 200);
    
 	// set up the image list
 	m_imageList.Create(16, 16, ILC_MASK, 6, 4);
 	SetImageList(&m_imageList, TVSIL_NORMAL);
 
    return 0;
}

void CObjectExplorer::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

    CPoint      ptAction;
    UINT        nFlags;

    GetToolTips()->Activate(FALSE);

    GetCursorPos(&ptAction);
    ScreenToClient(&ptAction);
    ASSERT(!m_bDragging);
    m_bDragging = TRUE;

    HTREEITEM hDragItem = HitTest(ptAction, &nFlags);
    SelectItem(hDragItem);

    BOOL bResult =
    	m_mapItemHandles.Lookup(hDragItem, m_pItemDrag);
    if (bResult)
    {
    	m_pItemDrop = NULL;

    	ASSERT(m_pDragImageList == NULL);
    	m_pDragImageList = CreateDragImage(*m_pItemDrag);  // get the image list for dragging
    	m_pDragImageList->DragShowNolock(TRUE);
    	m_pDragImageList->SetDragCursorImage(0, CPoint(0, 0));
    	m_pDragImageList->BeginDrag(0, CPoint(0, 0));
    	m_pDragImageList->DragMove(ptAction);
    	m_pDragImageList->DragEnter(this, ptAction);
    	SetCapture();

    	SetToolTipMessages();

    	m_ToolTip.Activate(TRUE); 

    	// set up the timer for auto scrolling
    	m_nScrollTimerID = SetTimer(1, 200, NULL);
    }
    
    *pResult = 0;
}

void CObjectExplorer::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // if we're in the middle of a drag, ignore this message
    if (m_bDragging)
    {
    	return;
    }

    // find the tree item being selected
    HTREEITEM hSelectedItem = HitTest(point);
    SelectItem(hSelectedItem);
    
    	// find the tree item
 	CObjectTreeItem *pItem = NULL;
    	m_mapItemHandles.Lookup(hSelectedItem, pItem);
 	if (pItem == NULL)
 		return;
    
    // get the popup menu for the item
    CMenu *pPopupMenu = pItem->GetPopupMenu();
    if (pPopupMenu != NULL)
    {
    	// set the menu command target
    	m_pCommandTarget = pItem;

    	// process the popup menu
    	ClientToScreen(&point);
    	BOOL bResult = pPopupMenu->GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN
    		| TPM_RIGHTBUTTON, point.x, point.y, this);

    	// destroy the popup menu
    	pPopupMenu->DestroyMenu();
 	delete pPopupMenu;
    }
}

void CObjectExplorer::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_bDragging)
    {
    	// get the current mouse move message
    	MSG msg;
    	memcpy(&msg, GetCurrentMessage(), sizeof(MSG));

    	// and send it to the tooltip for processing
    	m_ToolTip.RelayEvent(&msg);	
    
    	// move the dragged image
    	ASSERT(m_pDragImageList != NULL);
    	m_pDragImageList->DragMove(point);

    	// see if there is a potential drop target
    	HTREEITEM hDropItem;
    	if ((hDropItem = HitTest(point)) != NULL
    			&& m_mapItemHandles.Lookup(hDropItem, m_pItemDrop))
    	{
    		// clear the dragging images
    		m_pDragImageList->DragLeave(this);

    		// unselect the current drop target
    		SelectDropTarget(NULL);

    		// droppable if the drop target was found and it will accept the 
    		//		dragged item
    		BOOL bIsDroppable = m_pItemDrop->IsDroppable(m_pItemDrag);

    		// set the appropriate cursor
    		::SetCursor(bIsDroppable ? m_hDropCursor : m_hNoDropCursor);

    		// select the drop target
    		SelectDropTarget(bIsDroppable ? hDropItem : NULL);

    		// restore the dragging images
    		m_pDragImageList->DragEnter(this, point);
    	}
    	
    }

    CTreeCtrl::OnMouseMove(nFlags, point);
}

void CObjectExplorer::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // are we ending a drag?
    if (m_bDragging)
    {
    	// finish up the drag image
    	ASSERT(m_pDragImageList != NULL);
    	m_pDragImageList->DragLeave(this);
    	m_pDragImageList->EndDrag();
    	delete m_pDragImageList;
    	m_pDragImageList = NULL;

    	// check to make sure a self-drop isn't about to occur and that 
    	//		the dropped-upon item is present and willing to accept
    	if (m_pItemDrag != m_pItemDrop
    		&& m_pItemDrop != NULL
    		&& m_pItemDrop->IsDroppable(m_pItemDrag))
    	{
    		// notify it of the drop
    		m_pItemDrop->OnDrop(m_pItemDrag);
    	}
    	else
    	{
    		// otherwise, beep to indicate that an invalid drop target
    		//		was selected
    		MessageBeep(0);
    	}

    	// not dragging any longer, so release the cursor and de-select
    	//		the target
    	m_bDragging = FALSE;
    	ReleaseCapture();
    	SelectDropTarget(NULL);

    	// de-active the tooltip
    	m_ToolTip.Activate(FALSE);

    	GetToolTips()->Activate(TRUE);

    	// kill the scrolling timer
    	KillTimer(m_nScrollTimerID);
    }
    
    CTreeCtrl::OnLButtonUp(nFlags, point);
}

CString strOldClassLabel;

void CObjectExplorer::OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
    if (m_bDragging)
    {
    	return;
    }

    TV_DISPINFO     *ptvinfo;

    ptvinfo = (TV_DISPINFO *)pnmhdr;
    if (ptvinfo->item.pszText != NULL)
    {
    	CString strLabel = ptvinfo->item.pszText;
    	int nColonAt = strLabel.Find(_T(": "));
    	strOldClassLabel = strLabel.Left(nColonAt+2);
    	strLabel = strLabel.Mid(nColonAt+2);
    	// wcscpy(ptvinfo->item.pszText, strLabel.GetBuffer(100));
    	// ptvinfo->item.mask = TVIF_TEXT;
    	GetEditControl()->SetWindowText(strLabel);

    	// SetItem(&ptvinfo->item);
    	// EditLabel(ptvinfo->item.hItem);
    }
    *pLResult = FALSE;	// do not continue editing
}

void CObjectExplorer::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
    TV_DISPINFO     *ptvinfo;

    ptvinfo = (TV_DISPINFO *)pnmhdr;
    if (ptvinfo->item.pszText != NULL)
    {
 		CObjectTreeItem *pObjectTreeItem;
 		if (m_mapItemHandles.Lookup(ptvinfo->item.hItem, pObjectTreeItem))
 		{
 			CObject *pObject = pObjectTreeItem->forObject.Get();
			if (pObject->IsKindOf(RUNTIME_CLASS(CModelObject)))
			{
				CModelObject *pModelObject = (CModelObject *)pObject;
				pModelObject->name.Set(ptvinfo->item.pszText);
			}
 		}

    	static CString strNewLabel;
    	strNewLabel = strOldClassLabel + // CString("Class: ") + 
    		ptvinfo->item.pszText;
    	ptvinfo->item.pszText = strNewLabel.GetBuffer(100);
    	ptvinfo->item.mask = TVIF_TEXT;
    	SetItem(&ptvinfo->item);
    }
    *pLResult = TRUE;
}


void CObjectExplorer::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;

    // find the CObjectTreeItem that has just been selected
    CObjectTreeItem *pNewItem;
    BOOL bResult = 
    	this->m_mapItemHandles.Lookup(pNMTreeView->itemNew.hItem, pNewItem);
    if (bResult)
    {
    	// notify this CObjectTreeItem that has just been selected
    	pNewItem->OnSelected();
    }
    
    *pResult = 0;
}

void CObjectExplorer::OnCheck(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;
 
	TVHITTESTINFO ht;
	ht.pt = GetCurrentMessage()->pt;
	ScreenToClient(&ht.pt);
         
	HitTest(&ht);
 
 	if (TVHT_ONITEMSTATEICON & ht.flags)
 	{
 		CObjectTreeItem *pItem;
 		if (m_mapItemHandles.Lookup(ht.hItem, pItem))
 		{
 			TRACE1("Item is %s\n", GetCheck(ht.hItem) ? "checked" : "not checked");
 
 			// NOTE: use the opposite of the GetCheck call because
 			//		the item's check state has not yet been updated at the time
 			//		of the click message
 			// pItem->SetChecked(!GetCheck(ht.hItem)); // 
			pItem->isChecked.Set(!GetCheck(ht.hItem));
 		}
 	}
 	
 	*pResult = 0;
}
 
void CObjectExplorer::OnTimer(UINT nIDEvent) 
{
    // check the ID to see if it is the scrolling timer
    if (nIDEvent != m_nScrollTimerID)
    {
    	CTreeCtrl::OnTimer(nIDEvent);
    	return;
    }

    // get the "current" mouse position
    const MSG* pMessage;
    CPoint ptMouse;
    pMessage = GetCurrentMessage();
    ASSERT ( pMessage );
    ptMouse = pMessage->pt;
    ScreenToClient(&ptMouse);

    // move the drag image based on the current mouse position
    m_pDragImageList->DragMove(ptMouse);

    // holds the new drop target after the scroll
    HTREEITEM hNewDropTarget = NULL;

    // get the tree controls rectangle for comparison with the mouse position
    CRect rect;
    GetClientRect(&rect);

    // see if the mouse is outside the tree control rectangle, indicating
    //		that scrolling is needed
    if (ptMouse.y <= 0) 
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control up
    	SendMessage(WM_VSCROLL, SB_LINEUP);

    	// the new drop target is already selected as the first visible item
    	hNewDropTarget = GetFirstVisibleItem();
    }
    else if (ptMouse.y >= rect.Height())
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control down	
    	SendMessage(WM_VSCROLL, SB_LINEDOWN);

    	// set the new drop target as the last visible item
    	hNewDropTarget = GetFirstVisibleItem();
    	for (int nAtItem = 0; nAtItem < (int) GetVisibleCount() - 1; nAtItem++)
    	{
    		hNewDropTarget = GetNextVisibleItem(hNewDropTarget);
    	}
    }
    else
    {
    	CTreeCtrl::OnTimer(nIDEvent);
    	return;
    }

    // select the new drop target
    SelectDropTarget(hNewDropTarget);

    // find the CObjectTreeItem for the new drop target
    m_mapItemHandles.Lookup(hNewDropTarget, m_pItemDrop);

    // redraw the drag cursor
    m_pDragImageList->DragEnter(this, ptMouse);
    
    // set the tooltips for the new scroll position
    SetToolTipMessages();

    CTreeCtrl::OnTimer(nIDEvent);
}

CObjectTreeItem * CObjectExplorer::GetRootItem()
{
 	HTREEITEM hRoot = GetNextItem(TVI_ROOT, TVGN_ROOT);
 
 	CObjectTreeItem *pRoot = NULL;
 
 	// find the CObjectTreeItem for the new drop target
 	m_mapItemHandles.Lookup(hRoot, pRoot);
 
 	return pRoot;
}
