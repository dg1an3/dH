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
		m_pParent(NULL),
		m_hTreeItem(NULL),
		m_pObjectExplorer(NULL),
		imageResourceID(IDB_FOLDER_CLOSED),
		selectedImageResourceID(IDB_FOLDER_OPENED),
		isChecked(TRUE)
{
}
    
CObjectTreeItem::~CObjectTreeItem()
{
}

CObject * CObjectTreeItem::GetObject()
{
    return m_pObject;
}

void CObjectTreeItem::SetObject(CObject *pObject)
{
    m_pObject = pObject;
if (pObject->IsKindOf(RUNTIME_CLASS(CObservable)))
 	((CObservable *)m_pObject)->AddObserver(this);
}

int CObjectTreeItem::GetChildCount()
{
    return m_arrChildren.GetSize();
}

CObjectTreeItem * CObjectTreeItem::GetChild(int nIndex)
{
    return m_arrChildren.ElementAt(nIndex);
}

int CObjectTreeItem::AddChild(CObjectTreeItem * pNewChild)
{
    // set the parent-child relationship
    pNewChild->SetParent(this);
    int nIndex = m_arrChildren.Add(pNewChild);

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
    for (int nAtChild = 0; nAtChild < GetChildCount(); nAtChild++)
    {
    	GetChild(nAtChild)->Create(pExplorer);
    }

    // everythings OK
    return (m_hTreeItem != NULL);
}

CObjectTreeItem::operator HTREEITEM() const
{
    return m_hTreeItem;
}

// returns the string that is the label for the object
CString CObjectTreeItem::GetLabel()
{
 	CString strLabel = GetClassLabel();
 
 	if (GetObject() != NULL 
 		&& GetObject()->IsKindOf(RUNTIME_CLASS(CModelObject)))
 	{
 		CString strName = ((CModelObject *)GetObject())->name.Get();
 		if (strName != "")
 		{
 			if (strLabel != "")
 			{
 				strLabel += ": ";
 			}
 			strLabel += strName;
 		}
 	}
    
    	// return the default label
 	if (strLabel != "")
		return strLabel;
	else
	 	return name.Get();
}

IMPLEMENT_DYNCREATE(CObjectTreeItem, CCmdTarget)

void CObjectTreeItem::SetParent(CObjectTreeItem *pParent)
{
    // set the pointer to the parent
    m_pParent = pParent;
}

CString CObjectTreeItem::GetClassLabel()
{
    CString strClassName = "";
  	if ((GetObject() != NULL) && (GetObject()->GetRuntimeClass() != RUNTIME_CLASS(CModelObject)))
    {
    	strClassName = GetObject()->GetRuntimeClass()->m_lpszClassName;
    	strClassName = strClassName.Mid(1);
    }

    return strClassName;
}
    
BOOL CObjectTreeItem::IsChecked()
{
	return isChecked.Get();
}

void CObjectTreeItem::SetChecked(BOOL bChecked)
{
	isChecked.Set(bChecked);

	if (m_pObjectExplorer->GetCheck(m_hTreeItem))
  		m_pObjectExplorer->SetCheck(m_hTreeItem, isChecked.Get());
}
    
void CObjectTreeItem::OnRenameItem()
{
    m_pObjectExplorer->EditLabel(m_hTreeItem);
}
    
void CObjectTreeItem::OnDeleteItem()
{
	// TODO: delete the item
}
    
void CObjectTreeItem::OnChange(CObservable *pSource)
{
	if (pSource == m_pObject && m_pObjectExplorer != NULL)
	{
		m_pObjectExplorer->SetItem(m_hTreeItem, TVIF_TEXT, 
  			GetLabel(), // GetClassLabel() + ": " + m_pObject->name.Get(), 
  			0, 0, 0, 0, NULL);
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

BOOL CObjectTreeItem::IsChildOf(CObjectTreeItem *pSuspectedParent)
{
    if (GetParent() == NULL)
    {
    	return FALSE;
    }

    return (pSuspectedParent == GetParent())
    	|| (GetParent()->IsChildOf(pSuspectedParent));
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


CObjectTreeItem * CObjectTreeItem::CopyItem(CObjectTreeItem *pParent)
{
    CObjectTreeItem * pCopy = new CObjectTreeItem();
    pCopy->SetParent(pParent);

    // copy the children
    for (int nIndex = 0; nIndex < GetChildCount(); nIndex++)
    {
    	pCopy->AddChild(GetChild(nIndex)->CopyItem(pCopy));
    }

    // now create this
    pCopy->Create(m_pObjectExplorer);

    return NULL;
}
