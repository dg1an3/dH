// ObjectTreeItem.h: interface for the CObjectTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_)
#define AFX_OBJECTTREEITEM_H__BFB71EC3_C55D_11D4_BE40_005004D16DAA__INCLUDED_

#include <ModelObject.h>

class CObjectExplorer;

class CObjectTreeItem : public CCmdTarget
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
	const CString& GetLabel() const;
	void SetLabel(const CString& strLabel);

	// pointer to the object represented by this item
	CObject *GetObject();
	void SetObject(CObject *pObject);

	// parent accessor
	CObjectTreeItem *GetParent();

	// children accessors
	void AddChild(CObjectTreeItem *pChildItem);

	// value to indicate that the item should be checked
	BOOL m_bChecked; 

	// flag to indicate that the children of this item should be auto-created
	BOOL m_bAutoCreateChildren;

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

	// string holding the name (label) for this tree item
	CString m_strLabel;

	// access to the object represented by this item
	CObject *m_pObject;

	// parent access
	CObjectTreeItem *m_pParent;

	// children
	CObArray m_arrChildren;		// CObjectTreeItem
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
