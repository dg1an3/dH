// AffineTransform.h : Declaration of the CAffineTransform

#ifndef __AFFINETRANSFORM_H_
#define __AFFINETRANSFORM_H_

#include "resource.h"       // main symbols
#include "FieldCOMCP.h"

/////////////////////////////////////////////////////////////////////////////
// CAffineTransform
class ATL_NO_VTABLE CAffineTransform : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAffineTransform, &CLSID_AffineTransform>,
	public IDispatchImpl<IAffineTransform, &IID_IAffineTransform, &LIBID_FieldCOMLib>,
	public CProxy_IObjectEvents< CAffineTransform >,
	public IConnectionPointContainerImpl<CAffineTransform>
{
public:
	CAffineTransform()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_AFFINETRANSFORM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAffineTransform)
	COM_INTERFACE_ENTRY(IAffineTransform)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CAffineTransform)
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

// IAffineTransform
	STDMETHOD(get_Matrix)(/*[out, retval]*/ IMatrix * *pVal);
	STDMETHOD(put_Matrix)(/*[in]*/ IMatrix * newVal);
	STDMETHOD(get_IsRigid)(/*[out, retval]*/ BOOL *pVal);
};

#endif //__AFFINETRANSFORM_H_
