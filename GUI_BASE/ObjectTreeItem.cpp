// ObjectTreeItem.cpp: implementation of the CObjectTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_BASE_resource.h"

#include "ObjectTreeItem.h"
#include "ObjectExplorer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CObjectTreeItem::CObjectTreeItem()
    : forObject(NULL),
		m_pObjectExplorer(NULL),
		m_hTreeItem(NULL),
		parent(NULL),
		autoCreateChildren(TRUE),
		imageResourceID(IDB_FOLDER_CLOSED),
		selectedImageResourceID(IDB_FOLDER_OPENED),
		isChecked(TRUE)
{
	forObject.AddObserver(this, (ChangeFunction) OnForObjectChanged);
	// forObject.SetAutoObserver(this, (ChangeFunction) OnChange);
	label.AddObserver(this, (ChangeFunction) OnLabelChanged);
	parent.AddObserver(this, (ChangeFunction) OnParentChanged);
	children.AddObserver(this, (ChangeFunction) OnChildrenChanged);
}

CObjectTreeItem::~CObjectTreeItem()
{
}

BOOL CObjectTreeItem::Create(CObjectExplorer *pExplorer)
{
    // set the local pointer to the tree control
    m_pObjectExplorer = pExplorer;

    // create the tree control
    m_hTreeItem = pExplorer->InsertItem(this);

    // create the child items
    for (int nAtChild = 0; nAtChild < children.GetSize(); nAtChild++)
    	children.Get(nAtChild)->Create(pExplorer);

    // everythings OK
    return (m_hTreeItem != NULL);
}

CObjectTreeItem::operator HTREEITEM() const
{
    return m_hTreeItem;
}

IMPLEMENT_DYNCREATE(CObjectTreeItem, CCmdTarget)

    
void CObjectTreeItem::OnRenameItem()
{
    m_pObjectExplorer->EditLabel(m_hTreeItem);
}
    
void CObjectTreeItem::OnDeleteItem()
{
	// TODO: delete the item
}
    
void CObjectTreeItem::OnForObjectChanged(CObject *pSource, void *pOldValue)
{
	if (forObject->IsKindOf(RUNTIME_CLASS(CModelObject)))
	{
		CModelObject *pModel = (CModelObject *)forObject.Get();

		// set the label for this item to the name of the model object
		label.SyncTo(&pModel->name);

		if (autoCreateChildren.Get())
		{
			// delete existing children
			children.RemoveAll();

			// auto-create the children
			for (int nAt = 0; nAt < pModel->children.GetSize(); nAt++)
			{
				CObjectTreeItem *pChildItem = new CObjectTreeItem();

				// set the initial icons
				pChildItem->imageResourceID.Set(imageResourceID.Get());
				pChildItem->selectedImageResourceID.Set(selectedImageResourceID.Get());

				// set the parent
				pChildItem->parent.Set(this);

				// set the object for the item
				pChildItem->forObject.Set(pModel->children.Get(nAt));
			}
		}
	}
}

void CObjectTreeItem::OnLabelChanged(CObject *pSource, void *pOldValue)
{
	if (m_pObjectExplorer != NULL)
	{
		// if so, change the label for the item
		m_pObjectExplorer->SetItem(m_hTreeItem, TVIF_TEXT, 
  			label.Get(), 0, 0, 0, 0, NULL);
	}
}

void CObjectTreeItem::OnParentChanged(CObject *pSource, void *pOldValue)
{
	if (pOldValue)
	{
		// check that the old value is a pointer to the parent
		ASSERT(((CObject *)pOldValue)->IsKindOf(RUNTIME_CLASS(CObjectTreeItem)));

		// form a pointer to the old parent
		CObjectTreeItem *pOldParent = (CObjectTreeItem *) pOldValue;

		// remove this from its current parent
		pOldParent->children.Release(this);
	}

	// now delete this item from the old tree view
	if (m_pObjectExplorer)
		m_pObjectExplorer->DeleteItem(this);

	// add this to the new parent
	parent->children.Add(this);
}

void CObjectTreeItem::OnChildrenChanged(CObject *pSource, void *pOldValue)
{
	if (pOldValue)
	{
		// check that the old value is a pointer to the parent
		ASSERT(((CObject *)pOldValue)->IsKindOf(RUNTIME_CLASS(CObjectTreeItem)));

		// form a pointer to the new child
		CObjectTreeItem *pNewChild = (CObjectTreeItem *) pOldValue;

		// set the parent parent for the new child
		pNewChild->parent.Set(this);

		// create the child if this (the parent) has been created
		if (m_pObjectExplorer)
    		pNewChild->Create(m_pObjectExplorer);
	}
}
  
void CObjectTreeItem::OnSelected()
{
    TRACE(_T("Selected item\n"));
}

BOOL CObjectTreeItem::OnDrop(CObjectTreeItem *pDroppedItem)
{
	// move the dropped item here by setting this as the parent
	pDroppedItem->parent.Set(this);

	// return TRUE
    return TRUE;
}
    
BOOL CObjectTreeItem::IsDroppable(CObjectTreeItem *pPotentialDrop,
    							  CString *pStrToolTipMessage)
{
    if (GetRuntimeClass() != RUNTIME_CLASS(CObjectTreeItem))
    {
    	return FALSE;
    }
    if (pStrToolTipMessage != NULL)
    {
    	(*pStrToolTipMessage) = "Drop Here";
    }
    return TRUE;
}

CMenu * CObjectTreeItem::GetPopupMenu()
{
    // load the popup menu
    BOOL bResult = FALSE;  
    CMenu *pPopupMenu = new CMenu();
	bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_ITEM_MENU));

	if (!bResult)
	{
  		delete pPopupMenu;
  		return NULL;
	}

    return pPopupMenu;
}

BEGIN_MESSAGE_MAP(CObjectTreeItem, CCmdTarget)
   //{{AFX_MSG_MAP(CObjectTreeItem)
	ON_COMMAND(ID_ITEM_RENAME, OnRenameItem)
	ON_COMMAND(ID_ITEM_DELETE, OnDeleteItem)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()
