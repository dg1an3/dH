// TPSTransform.h : Declaration of the CTPSTransform

#ifndef __TPSTRANSFORM_H_
#define __TPSTRANSFORM_H_

#include "resource.h"       // main symbols
#include "FieldCOMCP.h"

/////////////////////////////////////////////////////////////////////////////
// CTPSTransform
class ATL_NO_VTABLE CTPSTransform : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTPSTransform, &CLSID_TPSTransform>,
	public IDispatchImpl<ITPSTransform, &IID_ITPSTransform, &LIBID_FieldCOMLib>,
	public CProxy_IObjectEvents< CTPSTransform >,
	public IConnectionPointContainerImpl<CTPSTransform>
{
public:
	CTPSTransform()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TPSTRANSFORM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTPSTransform)
	COM_INTERFACE_ENTRY(ITPSTransform)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

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

// ITPSTransform
	STDMETHOD(get_Landmarks)(/*[out, retval]*/ IObjects **pVal);
public :

BEGIN_CONNECTION_POINT_MAP(CTPSTransform)
	CONNECTION_POINT_ENTRY(DIID__IObjectEvents)
END_CONNECTION_POINT_MAP()

};

#endif //__TPSTRANSFORM_H_
