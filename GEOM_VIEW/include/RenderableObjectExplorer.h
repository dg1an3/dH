#if !defined(AFX_RENDERABLEOBJECTEXPLORER_H__FBA45CE6_1E15_4D45_9375_EF1CC884FA86__INCLUDED_)
#define AFX_RENDERABLEOBJECTEXPLORER_H__FBA45CE6_1E15_4D45_9375_EF1CC884FA86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenderableObjectExplorer.h : header file
//

#include "ObjectExplorer.h"

#include "SceneView.h"

/////////////////////////////////////////////////////////////////////////////
// CRenderableObjectExplorer window

class CRenderableObjectExplorer : public CObjectExplorer
{
// Construction
public:
	CRenderableObjectExplorer();

// Attributes
public:

// Operations
public:
	// creates a new object tree item for the object
	CRenderable *CreateItemForObject(CObject *pObject, 
		CSceneView *pSceneView,	CObjectTreeItem *pParent = NULL,
		UINT nImageID = 0, UINT nSelectedImageID = 1);

	// registers the renderable for an object class
	void RegisterClass(CRuntimeClass *pObjectClass, 
		CRuntimeClass *pRenderableClass);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderableObjectExplorer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRenderableObjectExplorer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenderableObjectExplorer)
	afx_msg void OnCheck(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItem25percent();
	afx_msg void OnItem50percent();
	afx_msg void OnItem75percent();
	afx_msg void OnItem100percent();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	// map from object classes to renderable classes
	CMapPtrToPtr m_mapClasses;

	// map from object classes to renderable classes
	CMapPtrToPtr m_mapRenderables;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERABLEOBJECTEXPLORER_H__FBA45CE6_1E15_4D45_9375_EF1CC884FA86__INCLUDED_)
