// Polygon3D.h : Declaration of the CPolygon3D

#ifndef __POLYGON_H_
#define __POLYGON_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPolygon3D

class ATL_NO_VTABLE CPolygon3D : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComControl<CPolygon3D>,
	public IDispatchImpl<IPolygon3D, &IID_IPolygon3D, &LIBID_FieldCOMLib>,
	public IDispatchImpl<IField, &IID_IField, &LIBID_FieldCOMLib>,
	public IConnectionPointContainerImpl<CPolygon3D>,
	public IPropertyNotifySinkCP<CPolygon3D>,
//	public IDispEventImpl<DISPID_POLYGON3D_VERTICES, CPolygon3D, &IID_IPropertyDispatchSink>,
//	public IDispEventImpl<DISPID_POLYGON3D_BASIS, CPolygon3D, &IID_IPropertyDispatchSink>,
	public IPropertyNotifySink,
	public IPersistStreamInitImpl<CPolygon3D>,
	public IPersistStorageImpl<CPolygon3D>,
	public CComCoClass<CPolygon3D, &CLSID_Polygon3D>
{
	// the polygon's label
	CComVariant m_varLabel;

	// the basis matrix
	CComPtr<IMatrix> m_pBasis;

	// the matrix of vertices
	CComPtr<IMatrix> m_pVertices;

	// flag to indicate that the vertices may change
	BOOL m_bShapeLocked;

public:
	CPolygon3D()
		: m_pVertices(NULL),
			m_pBasis(NULL),
			m_bShapeLocked(FALSE)
	{
	}

	// FinalConstruct
	HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_POLYGON3D)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPolygon3D)
	COM_INTERFACE_ENTRY(IPolygon3D)
	COM_INTERFACE_ENTRY2(IDispatch, IPolygon3D)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPropertyNotifySink)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistStorage)
END_COM_MAP()

BEGIN_PROP_MAP(CPolygon3D)
	PROP_ENTRY("Label",		DISPID_LABEL,				CLSID_NULL)
	PROP_ENTRY("Vertices",	DISPID_POLYGON3D_VERTICES,	CLSID_NULL)
	PROP_ENTRY("Basis",		DISPID_POLYGON3D_BASIS,		CLSID_NULL)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CPolygon3D)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_SINK_MAP(CPolygon3D)
//	SINK_ENTRY(DISPID_POLYGON3D_VERTICES, DISPID_ONREQUESTEDIT, OnVerticesRequestEdit)
//	SINK_ENTRY(DISPID_POLYGON3D_BASIS, DISPID_ONREQUESTEDIT, OnBasisRequestEdit)
END_SINK_MAP()

public:

// IPolygon3D

	STDMETHOD(get_Label)
		(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Label)
		(/*[in]*/ VARIANT newVal);

	STDMETHOD(get_Vertices)
		(/*[out, retval]*/ IMatrix **pVertices);
	STDMETHOD(put_Vertices)
		(/*[in]*/ IMatrix *pVertices);

	STDMETHOD(get_Basis)
		(/*[out, retval]*/ IMatrix **pBasis);
	STDMETHOD(put_Basis)
		(/*[in]*/ IMatrix *pBasis);

	STDMETHOD(AddVertex)
		(/*[in]*/ double x, /*[in]*/ double y);

	STDMETHOD(SignedArea)
		(/*[out, retval]*/ double *pVal);

	STDMETHOD(IntersectLine)
		(/*[in]*/ IMatrix *pLine, /*[out, retval]*/ double *pLambda);

// IField

	STDMETHOD(get_DimIn)
		(/*[out, retval]*/ LONG * pVal);
	STDMETHOD(put_DimIn)
		(/*[in]*/ LONG pVal);

	STDMETHOD(get_DimOut)
		(/*[out, retval]*/ LONG * pVal);
	STDMETHOD(put_DimOut)
		(/*[in]*/ LONG pVal);

	STDMETHOD(Eval)
		(/*[in]*/ IMatrix * pIn, /*[out, retval]*/ IMatrix * * pOut);

	STDMETHOD(EvalJac)
		(/*[in]*/ IMatrix * pIn, /*[out]*/ IMatrix * * pOut, /*[out, retval]*/ IMatrix * * pJac);

// IPropertyNotifySink

	STDMETHOD(OnChanged)
		(/*[in]*/ DISPID dispID);

	STDMETHOD(OnRequestEdit)
		(/*[in]*/ DISPID dispID);
};

#endif //__POLYGON_H_
