// ObjectTreeItem.cpp: implementation of the CObjectTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "resource.h"
#include "..\VSIM_OGL\resource.h"
#include "ObjectTreeItem.h"
#include "ObjectExplorer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CStatic *CObjectTreeItem::m_pStatic;
CBitmap CObjectTreeItem::m_bitmapContour;
CBitmap CObjectTreeItem::m_bitmapBeam;
CWnd *CObjectTreeItem::m_pWndContourControl;
CWnd *CObjectTreeItem::m_pWndBeamControl;


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
//	CString strLabel = GetClassLabel();
//
//	if (GetObject() != NULL) // && GetObject()->GetName() != "")
//	{
//		if (strLabel != "")
//		{
//			strLabel += ": ";
//		}
//		strLabel += "Name"; // GetObject()->GetName();
//	}

	// return the default label
	return m_strName;
}

IMPLEMENT_DYNCREATE(CObjectTreeItem, CCmdTarget)

void CObjectTreeItem::SetParent(CObjectTreeItem *pParent)
{
	// set the pointer to the parent
	m_pParent = pParent;
}

//UINT CObjectTreeItem::GetImageResourceID()
//{
//	return IDB_FOLDER_CLOSED; // 0;
//}
//
//UINT CObjectTreeItem::GetSelectedImageResourceID()
//{
//	return IDB_FOLDER_OPENED; // 1;
//}

CString CObjectTreeItem::GetClassLabel()
{
	CString strClassName = "";
	if ((GetObject() != NULL) && (GetObject()->GetRuntimeClass() != RUNTIME_CLASS(CObject)))
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
//	if (GetLabel().Left(3) == "Pat")
//		bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_PATIENTPOPUP));
//	else if (GetLabel().Left(3) == "Ser")
//		bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_SERIESPOPUP));
//	else if (GetLabel().Left(3) == "Pla")
//		bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_PLANPOPUP));
//	else if (GetLabel().Left(3) == "Str")
//		bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_STRUCTUREPOPUP));
//	else if (GetLabel().Left(3) == "Bea")
//		bResult = pPopupMenu->LoadMenu(MAKEINTRESOURCE(IDR_BEAMPOPUP));
//
//	if (!bResult)
//	{
//		return NULL;
//	}

	return pPopupMenu;
}

BEGIN_MESSAGE_MAP(CObjectTreeItem, CCmdTarget)
   //{{AFX_MSG_MAP(CObjectTreeItem)
//	ON_COMMAND(ID_ITEM_RENAME, OnRenameItem)
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
