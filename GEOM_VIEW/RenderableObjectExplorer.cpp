// RenderableObjectExplorer.cpp : implementation file
//

#include "stdafx.h"
// #nclude "GUI_BASE_resource.h"
#include "RenderableObjectExplorer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderableObjectExplorer

CRenderableObjectExplorer::CRenderableObjectExplorer()
{
}

CRenderableObjectExplorer::~CRenderableObjectExplorer()
{
}

void CRenderableObjectExplorer::RegisterClass(CRuntimeClass *pObjectClass, 
										CRuntimeClass *pRenderableClass)
{
	// make sure the renderable class is really a renderable class
	ASSERT(pRenderableClass->IsDerivedFrom(RUNTIME_CLASS(CRenderable)));

	// add to the map
	m_mapClasses.SetAt(pObjectClass, pRenderableClass);
}

CRenderable *CRenderableObjectExplorer::CreateItemForObject(CObject *pObject, 
		CSceneView *pSceneView, CObjectTreeItem *pParent,
		UINT nImageID, UINT nSelectedImageID)
{
	CRuntimeClass *pRenderableClass;
	if (m_mapClasses.Lookup(pObject->GetRuntimeClass(), 
			(void *&) pRenderableClass))
	{
		// create the renderable
		CRenderable *pNewRenderable = 
			(CRenderable *) pRenderableClass->CreateObject();

		// set up the renderable
		pNewRenderable->SetObject(pObject);
		pSceneView->AddRenderable(pNewRenderable);

		CObjectTreeItem *pNewItem = new CObjectTreeItem();
		pNewItem->SetObject(pObject);
		pNewItem->SetImages(nImageID, nSelectedImageID);
		pParent->AddChild(pNewItem);

		m_mapRenderables.SetAt(pNewItem, pNewRenderable);

		return pNewRenderable;
	}

	return NULL;
}


BEGIN_MESSAGE_MAP(CRenderableObjectExplorer, CObjectExplorer)
	//{{AFX_MSG_MAP(CRenderableObjectExplorer)
	ON_NOTIFY_REFLECT(NM_CLICK, OnCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CRenderableObjectExplorer::OnCheck(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;
 
	TVHITTESTINFO ht;
	ht.pt = GetCurrentMessage()->pt;
	ScreenToClient(&ht.pt);
         
	HitTest(&ht);
 
 	if (TVHT_ONITEMSTATEICON & ht.flags)
 	{
 		CObjectTreeItem *pItem;
 		if (m_mapItemHandles.Lookup(ht.hItem, pItem))
 		{
 			// NOTE: use the opposite of the GetCheck call because
 			//		the item's check state has not yet been updated at the time
 			//		of the click message
			pItem->SetChecked(!GetCheck(ht.hItem));

			// now look up the associated renderable
			CRenderable *pRenderable;
			m_mapRenderables.Lookup(pItem, (void*&) pRenderable);
 
			// now invalidate the renderable
			pRenderable->SetEnabled(pItem->IsChecked());
			pRenderable->Invalidate();
 		}
 	}
 	
 	*pResult = 0;
}
 

/////////////////////////////////////////////////////////////////////////////
// CRenderableObjectExplorer message handlers
