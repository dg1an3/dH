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

	// conversion to an HTREEITEM handle
	operator HTREEITEM() const;
    
	// string holding the name (label) for this tree item
	CValue< CString > name;

	// value to indicate that the item should be checked
	CValue< BOOL > isChecked; 

	// access to the object represented by this item
	CAssociation< CObject > forObject;
	void SetObject(CObject *pObject);
	CObject * GetObject();

	// access to the parent tree item
//	CAssociation< CObjectTreeItem > parent;
	CObjectTreeItem *GetParent() { return m_pParent; }
	void SetParent(CObjectTreeItem *pParent);

	// child access -- override GetChildCount and GetChild in order to
	//		create children from model object's contained objects
	CCollection< CObjectTreeItem > children;
	virtual int GetChildCount();
	virtual CObjectTreeItem * GetChild(int nIndex);
	int AddChild(CObjectTreeItem * pChild);

	BOOL IsChildOf(CObjectTreeItem *pSuspectedParent);

//	CObjectTreeItem * CopyItem(CObjectTreeItem *pParent = NULL);

	// returns the string that is the label for the object; override to
	//		provide custom label processing
	virtual CString GetLabel();
	virtual CString GetClassLabel();

	CValue<UINT> imageResourceID;
	CValue<UINT> selectedImageResourceID;

	virtual CMenu * GetPopupMenu();

	// actually creates the tree item
	virtual BOOL Create(CObjectExplorer *pObjectExplorer);

	virtual BOOL IsDroppable(CObjectTreeItem *pPotentialDrop, 
		CString *pStrToolTipMessage = NULL);

	DECLARE_DYNCREATE(CObjectTreeItem)

	virtual void OnChange(CObservableObject *pSource);

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

    // the parent tree item of this tree item
    CObjectTreeItem * m_pParent;

    // the array of children of this tree item
    CArray<CObjectTreeItem *, CObjectTreeItem *> m_arrChildren;

    // a pointer to the object that this item manages
//	CObject * m_pObject;
};
    
#endif // !defined(AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_)
