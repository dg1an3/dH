// QuadEdge1.h: interface for the CQuadEdge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUADEDGE1_H__95792EB4_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
#define AFX_QUADEDGE1_H__95792EB4_1715_11D5_9E54_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Edge.h"

class CQuadEdge : public CObject  
{
	friend CDelaunayTess;
public:
	CQuadEdge();
	virtual ~CQuadEdge();

	CEdge& GetEdge(int nIndex);

private:
	CEdge m_edges[4];
};

inline CQuadEdge::CQuadEdge()
{
//	m_edges[0].m_pQuadEdge = this;
//	m_edges[1].m_pQuadEdge = this;
//	m_edges[2].m_pQuadEdge = this;
//	m_edges[3].m_pQuadEdge = this;
//
	m_edges[0].m_nIndex = 0;
	m_edges[1].m_nIndex = 1;
	m_edges[2].m_nIndex = 2;
	m_edges[3].m_nIndex = 3;

	m_edges[0].m_pNext = &(m_edges[0]); 
	m_edges[1].m_pNext = &(m_edges[3]);
	m_edges[2].m_pNext = &(m_edges[2]); 
	m_edges[3].m_pNext = &(m_edges[1]);
}

inline CEdge& CQuadEdge::GetEdge(int nIndex)
{
	return m_edges[nIndex];
}

#endif // !defined(AFX_QUADEDGE1_H__95792EB4_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
