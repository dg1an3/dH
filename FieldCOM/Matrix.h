// Matrix.h : Declaration of the CMatrix

#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "resource.h"       // main symbols

#include <VectorN.h>
#include <MatrixNxM.h>

/////////////////////////////////////////////////////////////////////////////
// CMatrix
class ATL_NO_VTABLE CMatrix : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComControl<CMatrix>,
	public IDispatchImpl<IMatrix, &IID_IMatrix, &LIBID_FieldCOMLib>,
	public IMatrixAccess,
	public IDispatchImpl<IField, &IID_IField, &LIBID_FieldCOMLib>,
	public IConnectionPointContainerImpl<CMatrix>,
	public IPropertyNotifySinkCP<CMatrix>,
	// public IConnectionPointImpl<CMatrix, &IID_IPropertyDispatchSink>,
	public IPersistStreamInitImpl<CMatrix>,
	public IPersistStorageImpl<CMatrix>,
	public CComCoClass<CMatrix, &CLSID_Matrix>
{
	// the matrix name
	CComVariant m_varLabel;

	// holds this matrix
	CMatrixNxM<double> m_Matrix;

	// static to hold the initial size of the array
	static const long INIT_SIZE[2];

public:
	CMatrix()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MATRIX)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMatrix)
	COM_INTERFACE_ENTRY(IMatrix)
	COM_INTERFACE_ENTRY2(IDispatch, IMatrix)
	COM_INTERFACE_ENTRY(IMatrixAccess)
	COM_INTERFACE_ENTRY(IField)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistStorage)
END_COM_MAP()

BEGIN_PROP_MAP(CMatrix)
	PROP_ENTRY("Label",		DISPID_LABEL,			CLSID_NULL)
	PROP_ENTRY("Rows",		DISPID_MATRIX_ROWS,		CLSID_NULL)
	PROP_ENTRY("Cols",		DISPID_MATRIX_COLS,		CLSID_NULL)
	PROP_ENTRY("Elements",	DISPID_MATRIX_ELEMENTS,	CLSID_NULL)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CMatrix)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
//	CONNECTION_POINT_ENTRY(IID_IPropertyDispatchSink)
END_CONNECTION_POINT_MAP()

// IMatrix
public:
	STDMETHOD(get_Label)
		(/*[out, retval]*/ VARIANT *pVal);
	STDMETHOD(put_Label)
		(/*[in]*/ VARIANT newVal);
	STDMETHOD(get_Cols)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Cols)
		(/*[in]*/ long newVal);
	STDMETHOD(get_Rows)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Rows)
		(/*[in]*/ long newVal);
	STDMETHOD(Reshape)
		(/*[in]*/ long nCols, /*[in]*/ long nRows);
	STDMETHOD(get_Element)
		(/*[in]*/ long nCols, /*[in]*/ long nRows, /*[out, retval]*/ double *pVal);
	STDMETHOD(put_Element)
		(/*[in]*/ long nCols, /*[in]*/ long nRows, /*[in]*/ double newVal);
	STDMETHOD(get_Elements)
		(/*[out, retval]*/ VARIANT *pElements);
	STDMETHOD(put_Elements)
		(/*[in]*/ VARIANT elements);
	STDMETHOD(Copy)
		(/*[in]*/ IMatrix *pMatrix);
	STDMETHOD(Identity)
		();
	STDMETHOD(IsApproxEqual)
		(/*[in]*/ IMatrix *pMatrix, /*[in]*/ double epsilon, /*[out, retval]*/ BOOL *pVal);
	STDMETHOD(get_Det)
		(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_IsOrthogonal)
		(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(Orthogonalize)
		();
	STDMETHOD(Transpose)
		();
	STDMETHOD(Invert)
		();
	STDMETHOD(Add)
		(/*[in]*/ IMatrix *pMatrix);
	STDMETHOD(Sub)
		(/*[in]*/ IMatrix *pMatrix);
	STDMETHOD(MultScalar)
		(/*[in]*/ double scalar);
	STDMETHOD(Mult)
		(/*[in]*/ IMatrix *pLeft, /*[in]*/ IMatrix *pRight);
	STDMETHOD(Dot)
		(/*[in]*/ IMatrix *pMatrix, /*[out, retval]*/ double *pVal);

// IMatrixAccess
	STDMETHOD(AccessElements)
		(/*[out, retval]*/ double **pElements);
	STDMETHOD(UnaccessElements)
		(/*[in]*/ BOOL bChanged);

// IField
	STDMETHOD(get_DimIn)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_DimIn)
		(/*[in]*/ long newVal);
	STDMETHOD(get_DimOut)
		(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_DimOut)
		(/*[in]*/ long newVal);
	STDMETHOD(Eval)
		(/*[in]*/ IMatrix *pIn, /*[out, retval]*/ IMatrix **pOut);
	STDMETHOD(EvalJac)
		(/*[in]*/ IMatrix *pIn, /*[out]*/ IMatrix **pOut, 
			/*[out, retval]*/ IMatrix **pJac);
};

#endif //__MATRIX_H_
