#if !defined(AFX_OBJECTEXPLORER_H__208E94E1_C575_11D4_BE40_005004D16DAA__INCLUDED_)
#define AFX_OBJECTEXPLORER_H__208E94E1_C575_11D4_BE40_005004D16DAA__INCLUDED_

#include "ObjectTreeItem.h"

/////////////////////////////////////////////////////////////////////////////
// CObjectExplorer window

class CObjectExplorer : public CTreeCtrl
{
public:
	// Constructor/destructor
	CObjectExplorer();
	virtual ~CObjectExplorer();

public:
	// accessor to create/destroy tree items
	HTREEITEM InsertItem(CObjectTreeItem *pNewItem);
	void DeleteItem(CObjectTreeItem *pItem);

	// helper function to set up the tooltip messages
	void SetToolTipMessages();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectExplorer)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CObjectExplorer)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheck(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CImageList m_imageList;
	// the map of handles to CObjectTreeItem objects
	CMap<HTREEITEM, HTREEITEM, CObjectTreeItem *, CObjectTreeItem *>
		m_mapItemHandles;

	// target for right-click menu commands
	CCmdTarget * m_pCommandTarget;

	// member variables for managing drag-drop
	BOOL m_bDragging;
	CObjectTreeItem *m_pItemDrag;
	CObjectTreeItem *m_pItemDrop;

	// handles the dragging image
	CImageList * m_pDragImageList;

	// handles for the drop cursor and the no-drop cursor
	HCURSOR m_hDropCursor;
	HCURSOR m_hNoDropCursor;

	// handles auto-scrolling when the user drags off the end of the control
	UINT m_nScrollTimerID;

	// handles the drop tooltips
	CToolTipCtrl m_ToolTip;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTEXPLORER_H__208E94E1_C575_11D4_BE40_005004D16DAA__INCLUDED_)
