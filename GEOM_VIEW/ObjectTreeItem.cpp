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
    : m_pObject(NULL),
		m_pObjectExplorer(NULL),
		m_hTreeItem(NULL),
		m_pParent(NULL),
		m_bAutoCreateChildren(TRUE),
		m_nImageResourceID(IDB_FOLDER_CLOSED),
		m_nSelectedImageResourceID(IDB_FOLDER_OPENED),
		m_bChecked(TRUE)
{
}

CObjectTreeItem::~CObjectTreeItem()
{
	// don't delete children -- the CObjectExplorer
	//		deletes all object tree items
}

    
// string holding the name (label) for this tree item
const CString& CObjectTreeItem::GetLabel() const
{
	return m_strLabel;
}

// pointer to the object represented by this item
CObject *CObjectTreeItem::GetObject()
{
	return m_pObject;
}

// parent accessor
CObjectTreeItem *CObjectTreeItem::GetParent()
{
	return m_pParent;
}

void CObjectTreeItem::SetImages(UINT nImageID, UINT nSelectedImageID)
{
	// resource identifiers for the icons to be used
	m_nImageResourceID = nImageID;
	m_nSelectedImageResourceID = nSelectedImageID;
}


BOOL CObjectTreeItem::IsChecked() const
{
	return m_bChecked;
}

// sets the check state for the item
void CObjectTreeItem::SetChecked(BOOL bChecked)
{
	m_bChecked = bChecked;
}


BOOL CObjectTreeItem::Create(CObjectExplorer *pExplorer)
{
    // set the local pointer to the tree control
    m_pObjectExplorer = pExplorer;

    // create the tree control
    m_hTreeItem = pExplorer->InsertItem(this);

    // create the child items
    for (int nAtChild = 0; nAtChild < m_arrChildren.GetSize(); nAtChild++)
    	((CObjectTreeItem *) m_arrChildren.GetAt(nAtChild))->Create(pExplorer);

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
    
void CObjectTreeItem::SetObject(CObject *pObject)
{
	m_pObject = pObject;

	if (m_pObject->IsKindOf(RUNTIME_CLASS(CModelObject)))
	{
		CModelObject *pModel = (CModelObject *)m_pObject;

		// set the label for this item to the name of the model object
		m_strLabel = pModel->GetName();

		if (m_bAutoCreateChildren)
		{
			// delete existing children
			m_arrChildren.RemoveAll();

			// auto-create the children
			for (int nAt = 0; nAt < pModel->GetChildCount(); nAt++)
			{
				CObjectTreeItem *pChildItem = new CObjectTreeItem();

				// set the initial icons
				pChildItem->m_nImageResourceID = m_nImageResourceID;
				pChildItem->m_nSelectedImageResourceID = m_nSelectedImageResourceID;

				// set the parent
				pChildItem->m_pParent = this;

				// set the object for the item
				pChildItem->m_pObject = pModel->GetChildAt(nAt);
			}
		}
	}
}

void CObjectTreeItem::SetLabel(const CString& strLabel)
{
	m_strLabel = strLabel;

	if (m_pObjectExplorer != NULL)
	{
		// if so, change the label for the item
		m_pObjectExplorer->SetItem(m_hTreeItem, TVIF_TEXT, 
  			m_strLabel, 0, 0, 0, 0, NULL);
	}
}

/*
void CObjectTreeItem::OnParentChanged(CObject *pSource, void *pOldValue)
{
	if (pOldValue)
	{
		// check that the old value is a pointer to the parent
		ASSERT(((CObject *)pOldValue)->IsKindOf(RUNTIME_CLASS(CObjectTreeItem)));

		// form a pointer to the old parent
		CObjectTreeItem *pOldParent = (CObjectTreeItem *) pOldValue;

		// remove this from its current parent
		// pOldParent->m_arrChildren.Release(this);
	}

	// now delete this item from the old tree view
	if (m_pObjectExplorer)
		m_pObjectExplorer->DeleteItem(this);

	// add this to the new parent
	m_pParent->m_arrChildren.Add(this);
}
*/

void CObjectTreeItem::AddChild(CObjectTreeItem *pChildItem)
{
	// set the parent parent for the new child
	pChildItem->m_pParent = this;

	// create the child if this (the parent) has been created
	if (m_pObjectExplorer)
	{
    	pChildItem->Create(m_pObjectExplorer);
	}
}
  
void CObjectTreeItem::OnSelected()
{
    TRACE(_T("Selected item\n"));
}

BOOL CObjectTreeItem::OnDrop(CObjectTreeItem *pDroppedItem)
{
	// move the dropped item here by setting this as the parent
	pDroppedItem->m_pParent = this;

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
	bResult = 
		pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_ITEM_DISP_MENU));

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
