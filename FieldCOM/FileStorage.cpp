// CFileStorage.cpp : Implementation of CFieldCOMApp and DLL registration.

#include "stdafx.h"
#include "FieldCOM.h"
#include "FileStorage.h"

/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CFileStorage::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IFileStorage,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CFileStorage::get_Pathname(BSTR *pVal)
{
	(*pVal) = m_strPathname;

	return S_OK;
}

STDMETHODIMP CFileStorage::put_Pathname(BSTR newVal)
{
	m_strPathname = newVal;

	return S_OK;
}

STDMETHODIMP CFileStorage::Load(IDispatch *pMatrix)
{
	// open the storage object
	IStorage *pStorage = NULL;
	HRESULT hr = ::StgOpenStorage(m_strPathname, 
		NULL,		// priority storage
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,	// access flags
		NULL,		// snbExclude: element names to exclude
		0, &pStorage);

	// get the IPersistStorage interface for the matrix
	IPersistStorage *pPersist = NULL;
	hr = pMatrix->QueryInterface(IID_IPersistStorage, (void **) &pPersist);

	// now load the matrix
	hr = pPersist->Load(pStorage);

	// release the storage
	pStorage->Release();

	return S_OK;
}

STDMETHODIMP CFileStorage::Save(IDispatch *pMatrix)
{
	
	// open the storage object
	IStorage *pStorage = NULL;
	HRESULT hr = ::StgCreateDocfile(m_strPathname, 
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,	// access flags
		0, &pStorage);

	// get the IPersistStorage interface for the matrix
	IPersistStorage *pPersist = NULL;
	hr = pMatrix->QueryInterface(IID_IPersistStorage, (void **) &pPersist);

	// now save the matrix
	hr = pPersist->Save(pStorage, TRUE);

	// and commit the save
	hr = pStorage->Commit(STGC_DEFAULT);

	// release the storage
	pStorage->Release();

	return S_OK;
}
