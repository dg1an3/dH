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
	forObject.AddObserver(this);
	forObject.SetAutoObserver(this);
	label.AddObserver(this);
	parent.AddObserver(this);
	children.AddObserver(this);
}

CObjectTreeItem::~CObjectTreeItem()
{
}

int CObjectTreeItem::AddChild(CObjectTreeItem * pNewChild)
{
    // set the parent-child relationship
    pNewChild->parent.Set(this);
    int nIndex = children.Add(pNewChild);

    // create the child if this (the parent) has been created
    if (m_pObjectExplorer != NULL)
    {
    	pNewChild->Create(m_pObjectExplorer);
    }

    // return the index of the newly created child
    return nIndex;
}

BOOL CObjectTreeItem::Create(CObjectExplorer *pExplorer)
{
    // set the local pointer to the tree control
    m_pObjectExplorer = pExplorer;

    // create the tree control
    m_hTreeItem = pExplorer->InsertItem(this);
    
    // create the child items
    for (int nAtChild = 0; nAtChild < children.GetSize(); nAtChild++)
    {
    	children.Get(nAtChild)->Create(pExplorer);
    }

    // everythings OK
    return (m_hTreeItem != NULL);
}

CObjectTreeItem::operator HTREEITEM() const
{
    return m_hTreeItem;
}

//// returns the string that is the label for the object
//CString CObjectTreeItem::GetLabel()
//{
// 	CString strLabel;
//#ifdef USE_CLASS_LABEL
//	strLabel = GetClassLabel();
//#endif
// 
// 	if (forObject.Get() != NULL 
// 		&& forObject->IsKindOf(RUNTIME_CLASS(CModelObject)))
// 	{
// 		CString strName = ((CModelObject *)forObject.Get())->name.Get();
// 		if (strName != "")
// 		{
// 			if (strLabel != "")
// 			{
// 				strLabel += ": ";
// 			}
// 			strLabel += strName;
// 		}
// 	}
//    
//  	// return the default label
// 	if (strLabel != "")
//		return strLabel;
//	else
//	 	return name.Get();
//}

IMPLEMENT_DYNCREATE(CObjectTreeItem, CCmdTarget)

//CString CObjectTreeItem::GetClassLabel()
//{
//    CString strClassName = "";
//  	if ((forObject.Get() != NULL) && (forObject->GetRuntimeClass() != RUNTIME_CLASS(CModelObject)))
//    {
//    	strClassName = forObject->GetRuntimeClass()->m_lpszClassName;
//    	strClassName = strClassName.Mid(1);
//    }
//
//    return strClassName;
//}
    
void CObjectTreeItem::OnRenameItem()
{
    m_pObjectExplorer->EditLabel(m_hTreeItem);
}
    
void CObjectTreeItem::OnDeleteItem()
{
	// TODO: delete the item
}
    
void CObjectTreeItem::OnChange(CObservableObject *pSource)
{
	if (pSource == &forObject 
			&& forObject->IsKindOf(RUNTIME_CLASS(CModelObject)))
	{
		CModelObject *pModel = (CModelObject *)forObject.Get();

		// set the label for this item to the name of the model object
		label.SyncTo(&pModel->name);

		if (autoCreateChildren.Get())
		{
			// auto-create the children
			for (int nAt = 0; nAt < pModel->children.GetSize(); nAt++)
			{
				TRACE2("Creating item for %s, child of %s\n", 
					pModel->children.Get(nAt)->name.Get(),
					pModel->name.Get());

				CObjectTreeItem *pChildItem = new CObjectTreeItem();
				pChildItem->imageResourceID.SyncTo(&imageResourceID);
				pChildItem->selectedImageResourceID.SyncTo(&selectedImageResourceID);
				AddChild(pChildItem);
				pChildItem->forObject.Set(pModel->children.Get(nAt));
			}
		}
	}
	// see if the target item has changed
	else if (pSource == &label
			&& m_pObjectExplorer != NULL)
	{
		// if so, change the label for the item
		m_pObjectExplorer->SetItem(m_hTreeItem, TVIF_TEXT, 
  			label.Get(), 0, 0, 0, 0, NULL);
	}
	else if (pSource == &parent)
	{
		// TODO: remove this from its current parent
		// add this to the new parent
	}
	else if (pSource == &children)
	{
//		// set the parent-child relationship
//		pNewChild->parent.Set(this);
//
//		// create the child if this (the parent) has been created
//		if (m_pObjectExplorer != NULL)
//		{
//    		pNewChild->Create(m_pObjectExplorer);
//		}
	}
}
  
void CObjectTreeItem::OnSelected()
{
    TRACE(_T("Selected item\n"));
}

BOOL CObjectTreeItem::OnDrop(CObjectTreeItem *pDroppedItem)
{
    return FALSE;
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
    return FALSE;
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
