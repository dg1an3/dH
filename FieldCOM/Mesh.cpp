// Mesh.cpp : Implementation of CMesh
#include "stdafx.h"
#include "FieldCOM.h"

#include <float.h>

#include <VectorD.h>
#include <MatrixWrapper.h>

#include "Mesh.h"

long CMesh::m_nSize = 1;

/////////////////////////////////////////////////////////////////////////////
// CMesh

HRESULT CMesh::FinalConstruct()
{
	::VariantInit(&m_varLabel);

	// create and initialize the basis matrix
	CHECK_RESULT(m_pVertices.CoCreateInstance(CLSID_Matrix));
	CHECK_RESULT(m_pVertices->Reshape(0, 3));

	// create and initialize the matrix of vertices
	CHECK_RESULT(m_pNormals.CoCreateInstance(CLSID_Matrix));
	CHECK_RESULT(m_pNormals->Reshape(0, 3));

	// get the property notify sink interface for this object
	CComQIPtr<IPropertyNotifySink, &IID_IPropertyNotifySink> pSink(this);

	// holds the cookire returned by the Advise call
	DWORD dwCookie;

	// advise on property notify for vertices
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> 
		pVerticesCPC(m_pVertices);
	CHECK_RESULT(pVerticesCPC.Advise(pSink, IID_IPropertyNotifySink, &dwCookie));

	// advise on property notify for basis
	CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> 
		pNormalsCPC(m_pNormals);
	CHECK_RESULT(pNormalsCPC.Advise(pSink, IID_IPropertyNotifySink, &dwCookie));

	return S_OK;
}

STDMETHODIMP CMesh::get_Label(VARIANT *pVal)
{
	// assign the receiving variant
	return ::VariantCopy(pVal, &m_varLabel);
}

STDMETHODIMP CMesh::put_Label(VARIANT newVal)
{
	return m_varLabel.Copy(&newVal);
}

STDMETHODIMP CMesh::get_Indices(VARIANT *pIndices)
{
	CHECK_RESULT(::VariantClear(pIndices));
	pIndices->vt = VT_ARRAY | VT_I4;
	pIndices->parray = NULL;

	CSafeArray<long> arrOtherIndices(*pIndices);
	arrOtherIndices.Copy(m_arrIndices);

	return S_OK;
}

STDMETHODIMP CMesh::put_Indices(VARIANT indices)
{
	CHECK_CONDITION(indices.vt == (VT_ARRAY | VT_I4));

	CSafeArray<long> arrOtherIndices(indices);
	m_arrIndices.Copy(arrOtherIndices);

	return S_OK;
}

STDMETHODIMP CMesh::get_Vertices(IMatrix **pVertex)
{
	return m_pVertices->QueryInterface(IID_IMatrix, (void **) pVertex);
}

STDMETHODIMP CMesh::put_Vertices(IMatrix *pVertex)
{
	return m_pVertices->Copy(pVertex);
}

STDMETHODIMP CMesh::get_Normals(IMatrix **pNormal)
{
	return m_pNormals->QueryInterface(IID_IMatrix, (void **) pNormal);
}

STDMETHODIMP CMesh::put_Normals(IMatrix *pNormal)
{
	return m_pNormals->Copy(pNormal);
}

STDMETHODIMP CMesh::get_BoundingBox(IMatrix **pBoundingBox)
{
	// see if the bounding box exists
	if (m_pBoundingBox == NULL)
	{
		// create the bounding box
		CHECK_RESULT(m_pBoundingBox.CoCreateInstance(CLSID_Matrix));

		// set to 2 columns by 3 rows, for the two opposed corners
		CHECK_RESULT(m_pBoundingBox->Reshape(8, 3));

		// compute the bounding box
		// TODO: update this when the mesh changes

		{	// ensure release of wrappers

			// get a matrix wrapper for the vertices
			CMatrixWrapper mVert(m_pVertices);
			CMatrixWrapper mBB(m_pBoundingBox);

			// set to max originally
			mBB[0] = CVectorD<3>(FLT_MAX, FLT_MAX, FLT_MAX);

			// accumulate the minimum
			int nVertex;
			for (nVertex = 0; nVertex < mVert.GetCols(); nVertex++) 
			{
				mBB[0][0] = __min(mVert[nVertex][0], mBB[0][0]);
				mBB[0][1] = __min(mVert[nVertex][1], mBB[0][1]);
				mBB[0][2] = __min(mVert[nVertex][2], mBB[0][2]);
			}

			// set to max originally
			mBB[7] = CVectorD<3>(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			// accumulate the minimum
			for (nVertex = 0; nVertex < mVert.GetCols(); nVertex++) 
			{
				mBB[7][0] = __max(mVert[nVertex][0], mBB[7][0]);
				mBB[7][1] = __max(mVert[nVertex][1], mBB[7][1]);
				mBB[7][2] = __max(mVert[nVertex][2], mBB[7][2]);
			}

			// form the other matrix column vectors

			mBB[1] = mBB[0];
			mBB[1][0] = mBB[7][0];

			mBB[2] = mBB[0];
			mBB[2][1] = mBB[7][1];

			mBB[3] = mBB[0];
			mBB[3][2] = mBB[7][2];

			mBB[4] = mBB[7];
			mBB[4][0] = mBB[1][0];

			mBB[5] = mBB[7];
			mBB[5][1] = mBB[1][1];

			mBB[6] = mBB[7];
			mBB[6][2] = mBB[1][2];

			// notify wrapper of change in elements
			mBB.SetDirty();

			// trace out the matrix
			TRACE_MATRIX("Bounding Box", mBB);

		}	// release wrappers
	}

	return m_pBoundingBox->QueryInterface(IID_IMatrix, (void **) pBoundingBox);
}

STDMETHODIMP CMesh::IntersectLine(IMatrix *pLine, double *pLambda)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMesh::OnChanged(DISPID dispID)
{
	return S_OK;
}

STDMETHODIMP CMesh::OnRequestEdit(DISPID dispID)
{
	// see if an attempt is made to change the matrix rows
	if (DISPID_MATRIX_ROWS == dispID)
	{
		return S_FALSE;
	}

	return S_OK;
}
