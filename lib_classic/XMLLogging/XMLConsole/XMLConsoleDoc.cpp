// XMLConsoleDoc.cpp : implementation of the CXMLConsoleDoc class
//

#include "stdafx.h"
#include "XMLConsole.h"

#include "XMLConsoleDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleDoc

IMPLEMENT_DYNCREATE(CXMLConsoleDoc, CDocument)

BEGIN_MESSAGE_MAP(CXMLConsoleDoc, CDocument)
	//{{AFX_MSG_MAP(CXMLConsoleDoc)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, OnUpdateViewRefresh)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleDoc construction/destruction

CXMLConsoleDoc::CXMLConsoleDoc()
: m_pXMLDoc(NULL),
	m_dwMaxUID(0)
{
	// TODO: add one-time construction code here

}

CXMLConsoleDoc::~CXMLConsoleDoc()
{
}

BOOL CXMLConsoleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleDoc serialization

void CXMLConsoleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleDoc diagnostics

#ifdef _DEBUG
void CXMLConsoleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CXMLConsoleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXMLConsoleDoc commands

BOOL CXMLConsoleDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	DeleteContents();
	
    IStream *pStm = NULL;
    IPersistStreamInit *pPSI = NULL;

	IXMLElement *pIElemRoot = NULL;

	CR_ON(CoCreateInstance(CLSID_XMLDocument, 
			NULL, CLSCTX_INPROC_SERVER,
			IID_IXMLDocument, (void**)&m_pXMLDoc), goto done);

    //
    // Synchronously create a stream on a URL.
    //
    CR_ON(URLOpenBlockingStreamA(0, lpszPathName, &pStm, 0,0), goto done);    
    //
    // Get the IPersistStreamInit interface to the XML doc.
    //
    CR_ON(m_pXMLDoc->QueryInterface(IID_IPersistStreamInit, (void **)&pPSI), goto done);

    //
    // Init the XML doc from the stream.
    //
    if(SUCCEEDED(pPSI->Load(pStm)))
    {
        TRACE("%s : XML File is well formed \r\n", lpszPathName);

    }
    else
    {
        // Print error information !
        IXMLError *pXMLError = NULL ;
        XML_ERROR xmle;

        CR(pPSI->QueryInterface(IID_IXMLError, (void **)&pXMLError));

        ASSERT(pXMLError);

        CR(pXMLError->GetErrorInfo(&xmle));
        SAFERELEASE(pXMLError);

        TRACE("%s :", lpszPathName);
        TRACE(TEXT(" Error on line %d. Found %s while expecting %s\r\n"),
                            xmle._nLine,
                            xmle._pszFound,
                            xmle._pszExpected); 
        
        SysFreeString(xmle._pszFound);
        SysFreeString(xmle._pszExpected);
        SysFreeString(xmle._pchBuf);
    }

	
	pIElemRoot = GetRootElem();
	if (pIElemRoot)
	{
		CR_ON(MapUID(pIElemRoot), goto done);
	}

done:
    //
    // Release any used interfaces.
    //
    SAFERELEASE(pPSI);
    SAFERELEASE(pStm);
	
	return TRUE;
}

IXMLElement * CXMLConsoleDoc::GetRootElem()
{
	// retrieve root XML element
	IXMLElement *pIElemRoot = NULL;
	if (m_pXMLDoc)
	{
		CR_ON(m_pXMLDoc->get_root(&pIElemRoot), return NULL);
	}
	return pIElemRoot;
}

int CXMLConsoleDoc::GetChildCount(IXMLElement *pIElem)
{
	long nCount = 0;

	IXMLElementCollection *pIElemsChildren = NULL;
	CR_ON(pIElem->get_children(&pIElemsChildren), goto done);
	if (pIElemsChildren)
	{
		CR_ON(pIElemsChildren->get_length(&nCount), goto done);
	}

done:
	return nCount;
}

IXMLElement * CXMLConsoleDoc::GetChild(IXMLElement *pIElem, int nAt)
{
	IXMLElementCollection *pIElemsChildren = NULL;
	IDispatch *pIDispChild = NULL;
	IXMLElement *pIElemChild = NULL;

	CR_ON(pIElem->get_children(&pIElemsChildren), goto done);
	CR_ON(pIElemsChildren->item(COleVariant((long) nAt), COleVariant((long) NULL), &pIDispChild), goto done);
	CR_ON(pIDispChild->QueryInterface(&pIElemChild), goto done);

done:
	return pIElemChild;
}

CString CXMLConsoleDoc::GetTagName(IXMLElement *pIElem)
{
	CString strTagName;

	// create root tree view
	BSTR bstrTagName;
	CR_ON(pIElem->get_tagName(&bstrTagName), goto done);
	strTagName = bstrTagName;

done:
	::SysFreeString(bstrTagName);

	return strTagName;
}

CString CXMLConsoleDoc::GetAttribute(IXMLElement *pIElem, const CString &strAttr)
{
	USES_CONVERSION;

	CString strValue;

	VARIANT varValue;
	if (S_OK == pIElem->getAttribute(A2W(strAttr), &varValue))
	{
		strValue = varValue.bstrVal;
	}

	return strValue;
}

HRESULT CXMLConsoleDoc::MapUID(IXMLElement *pIElem)
{
	DWORD dwUID = GetUID(pIElem);
	if (dwUID != 0)
	{
		m_mapUIDtoElem[(void *)dwUID] = (void *)pIElem;

		long nCount = GetChildCount(pIElem);
		for (long nAt = 0; nAt < nCount; nAt++)
		{
			IXMLElement *pIElemChild = GetChild(pIElem, nAt);

			long nType = 0;
			CR(pIElemChild->get_type(&nType));
			if (XMLELEMTYPE_ELEMENT == nType)
			{
				CR(MapUID(pIElemChild));
			}
		}
		
		m_dwMaxUID = __max(m_dwMaxUID, dwUID);
	}

	return S_OK;
}

DWORD CXMLConsoleDoc::GetUID(IXMLElement *pIElem)
{
	DWORD dwUID = 0;

	CString strUID = GetAttribute(pIElem, "uid");
	if (strUID.Compare("") != 0)
	{
		sscanf(strUID.GetBuffer(10), "%x", &dwUID);
	}
	else TRACE("No UID found for element\n");


	return dwUID;
}

IXMLElement * CXMLConsoleDoc::GetElementByUID(DWORD dwUID)
{	
	IXMLElement *pIElem = NULL;
	BOOL bResult = m_mapUIDtoElem.Lookup((void *) dwUID, (void *&)pIElem);
	return pIElem;
}

CString CXMLConsoleDoc::GetText(IXMLElement *pIElem)
{
	CString strText;

	long nCount = GetChildCount(pIElem);
	for (long nAt = 0; nAt < nCount; nAt++)
	{
		IXMLElement *pIElemChild = GetChild(pIElem, nAt);

		long nType = 0;
		CR_ON(pIElemChild->get_type(&nType), goto done);

		if (XMLELEMTYPE_TEXT == nType)
		{
			BSTR bstrText;
			CR_ON(pIElemChild->get_text(&bstrText), goto done);
			strText = bstrText;
			::SysFreeString(bstrText);
		}
	}

done:
	return strText;
}

void CXMLConsoleDoc::OnUpdateViewRefresh(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pXMLDoc != NULL);	
}

void CXMLConsoleDoc::OnViewRefresh() 
{
	// store path
	CString strPath = GetPathName();

	// refresh contents of document
	OnOpenDocument(strPath);

	// update views
	UpdateAllViews(NULL);
}

void CXMLConsoleDoc::DeleteContents() 
{
	SAFERELEASE(m_pXMLDoc);
	m_mapUIDtoElem.RemoveAll();
	m_dwMaxUID = 0;
	
	CDocument::DeleteContents();
}

IXMLElement * CXMLConsoleDoc::GetParent(IXMLElement *pIElem)
{
	IXMLElement *pIElemParent = NULL;
	CR_ON(pIElem->get_parent(&pIElemParent), goto done);

done:
	return pIElemParent;
}
