// ObjectTreeItem.h: interface for the CObjectTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_)
#define AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_

#include <ModelObject.h>

class CObjectExplorer;

class CObjectTreeItem : public CCmdTarget //, public CObserver
{
public:
	// constructors/destructore
	CObjectTreeItem();
	virtual ~CObjectTreeItem();

	// allows for run-time type information on the CObjectTreeItem
	DECLARE_DYNCREATE(CObjectTreeItem)

	// conversion to an HTREEITEM handle
	operator HTREEITEM() const;
    
	// string holding the name (label) for this tree item
	CString m_strLabel;

	// value to indicate that the item should be checked
	BOOL m_bChecked; 

	// access to the object represented by this item
	CObject *m_pObject;

	// flag to indicate that the children of this item should be auto-created
	BOOL m_bAutoCreateChildren;

	// access to the parent tree item; NOTE: do not change this by hand,
	//		use the parent's AddChild member function
	CObjectTreeItem *m_pParent;

	// child access -- override GetChildCount and GetChild in order to
	//		create children from model object's contained objects
	CObArray  m_arrChildren;		// CObjectTreeItem

	// resource identifiers for the icons to be used
	UINT m_nImageResourceID;
	UINT m_nSelectedImageResourceID;

	// returns the current popup menu for this item
	virtual CMenu * GetPopupMenu();

	// actually creates the tree item
	virtual BOOL Create(CObjectExplorer *pObjectExplorer);

	// member function to determine of a potential dropped item may be dropped here
	virtual BOOL IsDroppable(CObjectTreeItem *pPotentialDrop, 
		CString *pStrToolTipMessage = NULL);

	// change handlers -- manages ensuring the consistency of the object's state
	void OnForObjectChanged(CObject *pSource, void *pOldValue);
	void OnLabelChanged(CObject *pSource, void *pOldValue);
	void OnParentChanged(CObject *pSource, void *pOldValue);
	void OnChildrenChanged(CObject *pSource, void *pOldValue);

	// notification events for various user interactions
	virtual void OnSelected();
	virtual BOOL OnDrop(CObjectTreeItem *pDroppedItem);
    
protected:
    
    //{{AFX_MSG(CObjectTreeItem)
    afx_msg void OnRenameItem();
    afx_msg void OnDeleteItem();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    // a pointer to the MFC tree control object that contains this item
    CObjectExplorer * m_pObjectExplorer;

private:
    // the handle to the Windows tree item in the tree control
    HTREEITEM m_hTreeItem;
};

// define this archive extraction operator so that we can use the CAssociation with this 
//		class
inline CArchive& operator>>(CArchive& ar, CObjectTreeItem*&pOb) 
{ 
	// no serialization defined for CObjectTreeItem
	pOb = NULL;

	return ar; 
} 

    
#endif // !defined(AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_)
