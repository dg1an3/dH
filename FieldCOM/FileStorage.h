// FileStorage.h: Definition of the FileStorage class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FileSTORAGE_H__71217CEF_02C1_4E9E_B836_72DD09966F3F__INCLUDED_)
#define AFX_FileSTORAGE_H__71217CEF_02C1_4E9E_B836_72DD09966F3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// FileStorage

class CFileStorage : 
	public IDispatchImpl<IFileStorage, &IID_IFileStorage, &LIBID_FieldCOMLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CFileStorage,&CLSID_FileStorage>
{
	// stores
	CComBSTR m_strPathname;

public:
	CFileStorage() {}
BEGIN_COM_MAP(CFileStorage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IFileStorage)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(CFileStorage) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_FILESTORAGE)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IFileStorage
public:
	STDMETHOD(Save)(IDispatch *);
	STDMETHOD(Load)(IDispatch *);
	STDMETHOD(get_Pathname)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Pathname)(/*[in]*/ BSTR newVal);
};

#endif // !defined(AFX_FileSTORAGE_H__71217CEF_02C1_4E9E_B836_72DD09966F3F__INCLUDED_)
