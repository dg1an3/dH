// ObjectTreeItem.h: interface for the CObjectTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_)
#define AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_

#include <Value.h>
#include <Association.h>
#include <Collection.h>
#include <ModelObject.h>

class CObjectExplorer;

class CObjectTreeItem : public CCmdTarget, public CObserver
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
	CValue< CString > label;

	// value to indicate that the item should be checked
	CValue< BOOL > isChecked; 

	// access to the object represented by this item
	CAssociation< CObject > forObject;

	// flag to indicate that the children of this item should be auto-created
	CValue< BOOL > autoCreateChildren;

	// access to the parent tree item; NOTE: do not change this by hand,
	//		use the parent's AddChild member function
	CAssociation< CObjectTreeItem > parent;

	// child access -- override GetChildCount and GetChild in order to
	//		create children from model object's contained objects
	CCollection< CObjectTreeItem > children;
	int AddChild(CObjectTreeItem * pChild);

	// resource identifiers for the icons to be used
	CValue<UINT> imageResourceID;
	CValue<UINT> selectedImageResourceID;

	// returns the current popup menu for this item
	virtual CMenu * GetPopupMenu();

	// actually creates the tree item
	virtual BOOL Create(CObjectExplorer *pObjectExplorer);

	// member function to determine of a potential dropped item may be dropped here
	virtual BOOL IsDroppable(CObjectTreeItem *pPotentialDrop, 
		CString *pStrToolTipMessage = NULL);

	// change handler -- manages ensuring the consistency of the object's state
	virtual void OnChange(CObservableObject *pSource);

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
