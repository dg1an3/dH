// MeshND.h: interface for the CMeshND class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESHND_H__94B5F271_44E2_4827_8492_2B01035BF414__INCLUDED_)
#define AFX_MESHND_H__94B5F271_44E2_4827_8492_2B01035BF414__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

/*
class CMeshBase
{
public:
};
*/

template<int DIM, class TYPE>
class CMeshND  : public CModelObject
{
public:
	CMeshND();
	virtual ~CMeshND();

	int GetVertexCount()
	{
		return m_nVertices;
	}

	CVectorD<DIM, TYPE>& GetVertexAt(int nAt)
	{
		return m_pVertices[nAt];
	}

	int GetSimplexCount()
	{
		return m_nSimplices;
	}

	CVectorD<DIM, TYPE>& GetVertexAt(int nSimplex, int nVert)
	{
		return m_pVertices[m_pIndices[nSimplex][nVert]];
	}

	CVectorD<DIM, int>& GetIndices(int nSimplex)
	{
		return m_pIndices[nSimplex];
	}

private:
	int m_nSimplices;
	CVectorD<DIM, int> *m_pIndices;

	int m_nVertexCount;
	CVectorD<DIM, TYPE> *m_pVertices;
};

#endif // !defined(AFX_MESHND_H__94B5F271_44E2_4827_8492_2B01035BF414__INCLUDED_)
