// BufferField.h : Declaration of the CBufferField

#ifndef __BUFFERFIELD_H_
#define __BUFFERFIELD_H_

#include "resource.h"       // main symbols
#include "FieldCOMCP.h"

/////////////////////////////////////////////////////////////////////////////
// CBufferField
class ATL_NO_VTABLE CBufferField : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CBufferField, &CLSID_BufferField>,
	public IDispatchImpl<IBufferField, &IID_IBufferField, &LIBID_FieldCOMLib>,
	public CProxy_IObjectEvents< CBufferField >,
	public IConnectionPointContainerImpl<CBufferField>
{
public:
	CBufferField()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BUFFERFIELD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBufferField)
	COM_INTERFACE_ENTRY(IBufferField)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CBufferField)
	CONNECTION_POINT_ENTRY(DIID__IObjectEvents)
END_CONNECTION_POINT_MAP()

public:
// IField
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pName);
	STDMETHOD(put_Name)(/*[in]*/ BSTR pName);
	STDMETHOD(get_DimIn)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_DimIn)(/*[in]*/ long newVal);
	STDMETHOD(get_DimOut)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_DimOut)(/*[in]*/ long newVal);
	STDMETHOD(Evaluate)(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut);
	STDMETHOD(Jacobian)(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut);

// IBufferField
	STDMETHOD(get_Extent)(/*[in]*/ long nDim, /*[out, retval]*/ long *pVal);
	STDMETHOD(put_Extent)(/*[in]*/ long nDim, /*[in]*/ long newVal);
	STDMETHOD(get_Basis)(/*[out, retval]*/ IMatrix * *pVal);
	STDMETHOD(put_Basis)(/*[in]*/ IMatrix * newVal);
	STDMETHOD(get_Scale)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_Scale)(/*[in]*/ double newVal);
	STDMETHOD(get_BufferShort1D)(/*[out, retval]*/ short * *pBuffer);
	STDMETHOD(get_BufferShort2D)(/*[out, retval]*/ short * **ppBuffer);
	STDMETHOD(get_BufferShort3D)(/*[out, retval]*/ short * ***pppBuffer);
	STDMETHOD(get_BufferLong1D)(/*[out, retval]*/ long * *pBuffer);
	STDMETHOD(get_BufferLong2D)(/*[out, retval]*/ long * **ppBuffer);
	STDMETHOD(get_BufferLong3D)(/*[out, retval]*/ long * ***pppBuffer);
	STDMETHOD(get_BytesPerSample)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_BytesPerSample)(/*[in]*/ long newVal);
};

#endif //__BUFFERFIELD_H_
