// Mesh.h : Declaration of the CMesh

#ifndef __MESH_H_
#define __MESH_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMesh
class ATL_NO_VTABLE CMesh : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComControl<CMesh>,
	public IDispatchImpl<IMesh, &IID_IMesh, &LIBID_FieldCOMLib>,
	public IDispatchImpl<IField, &IID_IField, &LIBID_FieldCOMLib>,
	public IConnectionPointContainerImpl<CMesh>,
	public IPropertyNotifySinkCP<CMesh>,
	public IPropertyNotifySink,
	public IPersistStreamInitImpl<CMesh>,
	public IPersistStorageImpl<CMesh>,
	public CComCoClass<CMesh, &CLSID_Mesh>
{
	// the mesh's label
	CComVariant m_varLabel;

	// the index array
	CSafeArray<long> m_arrIndices;

	// the matrix of vertices
	CComPtr<IMatrix> m_pVertices;

	// the matrix of normals
	CComPtr<IMatrix> m_pNormals;

	// the bounding box
	CComPtr<IMatrix> m_pBoundingBox;

	// static to store the size to be passed to CSafeArray
	static long m_nSize;

public:
	CMesh()
		: m_arrIndices(1, &m_nSize),
			m_pVertices(NULL),
			m_pNormals(NULL)
	{
	}

	// FinalConstruct
	HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_MESH)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMesh)
	COM_INTERFACE_ENTRY(IMesh)
	COM_INTERFACE_ENTRY2(IDispatch, IMesh)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPropertyNotifySink)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistStorage)
END_COM_MAP()

BEGIN_PROP_MAP(CMesh)
	PROP_ENTRY("Label",		DISPID_LABEL,				CLSID_NULL)
	PROP_ENTRY("Indices",	DISPID_MESH_INDICES,		CLSID_NULL)
	PROP_ENTRY("Vertices",	DISPID_MESH_VERTICES,		CLSID_NULL)
	PROP_ENTRY("Normals",	DISPID_MESH_NORMALS,		CLSID_NULL)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CMesh)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

public:

// IMesh

	STDMETHOD(get_Label)
		(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Label)
		(/*[in]*/ VARIANT newVal);

	STDMETHOD(get_Indices)
		(/*[out, retval]*/ VARIANT *pIndices);
	STDMETHOD(put_Indices)
		(/*[in]*/ VARIANT indices);

	STDMETHOD(get_Vertices)
		(/*[out, retval]*/ IMatrix **pVertices);
	STDMETHOD(put_Vertices)
		(/*[in]*/ IMatrix *pVertices);

	STDMETHOD(get_Normals)
		(/*[out, retval]*/ IMatrix **pNormals);
	STDMETHOD(put_Normals)
		(/*[out, retval]*/ IMatrix *pNormals);

	STDMETHOD(get_BoundingBox)
		(/*[out, retval]*/ IMatrix **pBoundingBox);

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

// IPropertyNotifySink

	STDMETHOD(OnChanged)
		(/*[in]*/ DISPID dispID);

	STDMETHOD(OnRequestEdit)
		(/*[in]*/ DISPID dispID);
};

#endif //__MESH_H_
