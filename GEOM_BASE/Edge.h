// Edge.h: interface for the CEdge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDGE_H__95792EB3_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
#define AFX_EDGE_H__95792EB3_1715_11D5_9E54_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Vector.h"

class CQuadEdge;
class CDelaunayTess;

class CEdge : public CObject  
{
	friend CQuadEdge;
	friend CDelaunayTess;
public:
	CEdge();
	virtual ~CEdge();

	// accessors for the edges end points
	const CVector<2>& Org2d() const;
	const CVector<2>& Dest2d() const;
	void  EndPoints(const CVector<2>&, const CVector<2>&);

	CEdge* Rot();
	CEdge* invRot();
	CEdge* Sym();

	CEdge* Onext();
	CEdge* Oprev();

	CEdge* Dnext();
	CEdge* Dprev();

	CEdge* Lnext();
	CEdge* Lprev();

//	CEdge* Rnext();
//	CEdge* Rprev();

	// accessor for the parent QuadEdge
	int TimeStamp(unsigned int);

private:
	// stores the index of this CEdge in the QuadEdge
	int m_nIndex;

	// the next CEdge (contains the destination for this CEdge)
	CEdge *m_pNext;

	// the origin point for this CEdge
	CVector<2> m_pt;

	// flag to indicate that the edge should not be broken
	BOOL m_bIsSide;

	// the time-stamp for the edge
	unsigned int m_ts;
};

inline CEdge::CEdge() 
: m_pNext(NULL),
	m_bIsSide(FALSE),
	m_ts(0)
{ 
}

inline CEdge* CEdge::Rot()
// Return the dual of the current CEdge, directed from its right to its left.
{
	return (m_nIndex < 3) ? this + 1 : this - 3;
}

inline CEdge* CEdge::invRot()
// Return the dual of the current CEdge, directed from its left to its right.
{
	return (m_nIndex > 0) ? this - 1 : this + 3;
}

inline CEdge* CEdge::Sym()
// Return the CEdge from the destination to the origin of the current CEdge.
{
	return (m_nIndex < 2) ? this + 2 : this - 2;
}

inline CEdge* CEdge::Onext()
// Return the next ccw CEdge around (from) the origin of the current CEdge.
{
	return m_pNext;
}

inline CEdge* CEdge::Oprev()
// Return the next cw CEdge around (from) the origin of the current CEdge.
{
	return Rot()->Onext()->Rot();
}

inline CEdge* CEdge::Dnext()
// Return the next ccw CEdge around (into) the destination of the current CEdge.
{
	return Sym()->Onext()->Sym();
}

inline CEdge* CEdge::Dprev()
// Return the next cw CEdge around (into) the destination of the current CEdge.
{
	return invRot()->Onext()->invRot();
}

inline CEdge* CEdge::Lnext()
// Return the ccw CEdge around the left face following the current CEdge.
{
	return invRot()->Onext()->Rot();
}

inline CEdge* CEdge::Lprev()
// Return the ccw CEdge around the left face before the current CEdge.
{
	return Onext()->Sym();
}

//inline CEdge* CEdge::Rnext()
//// Return the CEdge around the right face ccw following the current CEdge.
//{
//	return Rot()->Onext()->invRot();
//}
//
//inline CEdge* CEdge::Rprev()
//// Return the CEdge around the right face ccw before the current CEdge.
//{
//	return Sym()->Onext();
//}

/************** Access to data pointers *************************************/

inline const CVector<2>& CEdge::Org2d() const
{
	return m_pt;
}

inline const CVector<2>& CEdge::Dest2d() const
{
	return (m_nIndex < 2) ? ((this + 2)->m_pt) : ((this - 2)->m_pt);	// Sym()->m_pt; 
}

inline void CEdge::EndPoints(const CVector<2>& or, const CVector<2>& de)
{
	m_pt = or;
	Sym()->m_pt = de;
}

inline int CEdge::TimeStamp(unsigned int stamp)
{
	if (m_ts != stamp) 
	{
		m_ts = stamp;
		return TRUE;
	} else
		return FALSE;
}

#endif // !defined(AFX_EDGE_H__95792EB3_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
