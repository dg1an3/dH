// PolygonSet3D.h : Declaration of the CPolygonSet3D

#ifndef __POLYGONSET3D_H_
#define __POLYGONSET3D_H_

#include "resource.h"       // main symbols

#include <vector>

#include "Matrix.h"
#include "Polygon3D.h"

/////////////////////////////////////////////////////////////////////////////
// CPolygonSet3D
class ATL_NO_VTABLE CPolygonSet3D : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPolygonSet3D, &CLSID_PolygonSet3D>,
	public IDispatchImpl<IPolygonSet3D, &IID_IPolygonSet3D, &LIBID_FieldCOMLib>,
	public IDispatchImpl<IField, &IID_IField, &LIBID_FieldCOMLib>
{
	// the polygon's label
	CComVariant m_varLabel;

	// the basis matrix
	CComObject<CMatrix> *m_pBasis;

	// vector of polygons
	std::vector<IPolygon3D*> m_arrPolygons;

public:
	CPolygonSet3D()
		: m_pBasis(NULL)
	{
	}

	// FinalConstruct
	HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_POLYGONSET3D)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPolygonSet3D)
	COM_INTERFACE_ENTRY(IPolygonSet3D)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IDispatch, IPolygonSet3D)
	COM_INTERFACE_ENTRY(IField)
END_COM_MAP()

// IPolygonSet3D
public:
	STDMETHOD(get_Label)
		(/*[out,retval]*/ VARIANT *pVal);
	STDMETHOD(put_Label)
		(/*[in]*/ VARIANT newVal);
	STDMETHOD(get__NewEnum)
		(/*[out, retval]*/ IUnknown **ppEnum);
	STDMETHOD(Item)
		(/*[in]*/ long index, /*[out, retval]*/ IPolygon3D **ppItem);
	STDMETHOD(get_Count)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(Union)
		(/*[in]*/ IPolygon3D *pPoly);
	STDMETHOD(Intersect)
		(/*[in]*/ IPolygon3D *pPoly);
	STDMETHOD(Diff)
		(/*[in]*/ IPolygon3D *pPoly);
	STDMETHOD(get_Basis)
		(/*[out, retval]*/ IMatrix * *pVal);
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

#endif //__POLYGONSET3D_H_
