// MeshSet.h : Declaration of the CMeshSet

#ifndef __MESHSET_H_
#define __MESHSET_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMeshSet
class ATL_NO_VTABLE CMeshSet : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMeshSet, &CLSID_MeshSet>,
	public IDispatchImpl<IMeshSet, &IID_IMeshSet, &LIBID_FieldCOMLib>,
	public IDispatchImpl<IField, &IID_IField, &LIBID_FieldCOMLib>
{
	// the meshset's label
	CComVariant m_varLabel;

public:
	CMeshSet()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MESHSET)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMeshSet)
	COM_INTERFACE_ENTRY(IMeshSet)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IDispatch, IMeshSet)
	COM_INTERFACE_ENTRY(IField)
END_COM_MAP()

// IMeshSet
public:
	STDMETHOD(get_Label)
		(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Label)
		(/*[in]*/ VARIANT newVal);
	STDMETHOD(get__NewEnum)
		(/*[out, retval]*/ IUnknown **ppEnum);
	STDMETHOD(Item)
		(/*[in]*/ long index, /*[out, retval]*/ IDispatch **ppItem);
	STDMETHOD(get_Count)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(Union)
		(/*[in]*/ IMesh *pPoly);
	STDMETHOD(Intersect)
		(/*[in]*/ IMesh *pPoly);
	STDMETHOD(Diff)
		(/*[in]*/ IMesh *pPoly);
	STDMETHOD(IntersectLine)
		(/*[in]*/ IMatrix *pLine, /*[out, retval]*/ double *pLambda);
// IField
	STDMETHOD(get_DimIn)(LONG * pVal)
	{
		if (pVal == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
	STDMETHOD(put_DimIn)(LONG pVal)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(get_DimOut)(LONG * pVal)
	{
		if (pVal == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
	STDMETHOD(put_DimOut)(LONG pVal)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Eval)(IMatrix * pIn, IMatrix * * pOut)
	{
		if (pOut == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
	STDMETHOD(EvalJac)(IMatrix * pIn, IMatrix * * pOut, IMatrix * * pJac)
	{
		if (pOut == NULL)
			return E_POINTER;
			
		if (pJac == NULL)
			return E_POINTER;
			
		return E_NOTIMPL;
	}
};

#endif //__MESHSET_H_
