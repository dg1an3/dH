// XMLConsoleDoc.h : interface of the CXMLConsoleDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLCONSOLEDOC_H__6A6E1586_4D19_4521_BDF7_AAFD18F81B25__INCLUDED_)
#define AFX_XMLCONSOLEDOC_H__6A6E1586_4D19_4521_BDF7_AAFD18F81B25__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CXMLConsoleDoc : public CDocument
{
protected: // create from serialization only
	CXMLConsoleDoc();
	DECLARE_DYNCREATE(CXMLConsoleDoc)

// Attributes
public:
	IXMLDocument *m_pXMLDoc;

	DWORD m_dwMaxUID;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXMLConsoleDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	IXMLElement * GetParent(IXMLElement *pIElem);
	IXMLElement * GetRootElem();
	IXMLElement * GetElementByUID(DWORD dwUID);

	CString GetTagName(IXMLElement *pIElem);
	DWORD GetUID(IXMLElement *pIElem);

	CString GetAttribute(IXMLElement *pIElem, const CString& strAttr);

	CString GetText(IXMLElement *pIElem);

	int GetChildCount(IXMLElement *pIElem);
	IXMLElement * GetChild(IXMLElement *pIElem, int nAt);

	virtual ~CXMLConsoleDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HRESULT MapUID(IXMLElement *pIElem);

	CMapPtrToPtr m_mapUIDtoElem;

// Generated message map functions
protected:
	//{{AFX_MSG(CXMLConsoleDoc)
	afx_msg void OnUpdateViewRefresh(CCmdUI* pCmdUI);
	afx_msg void OnViewRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XMLCONSOLEDOC_H__6A6E1586_4D19_4521_BDF7_AAFD18F81B25__INCLUDED_)
