// AffineTransform.h : Declaration of the CAffineTransform

#ifndef __AFFINETRANSFORM_H_
#define __AFFINETRANSFORM_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAffineTransform
class ATL_NO_VTABLE CAffineTransform : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAffineTransform, &CLSID_AffineTransform>,
	public IDispatchImpl<IAffineTransform, &IID_IAffineTransform, &LIBID_FIELDCOMLib>
{
public:
	CAffineTransform()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_AFFINETRANSFORM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAffineTransform)
	COM_INTERFACE_ENTRY(IAffineTransform)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IAffineTransform
public:
	STDMETHOD(Identity)();
	STDMETHOD(Translate)(/*[in]*/ VARIANT varAmount);
	STDMETHOD(Rotate)(/*[in]*/ double angle, /*[in]*/ VARIANT varAxis);
	STDMETHOD(Scale)(/*[in]*/ VARIANT varScale);
	STDMETHOD(get_MatrixHG)(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_MatrixHG)(/*[in]*/ VARIANT newVal);
};

#endif //__AFFINETRANSFORM_H_
