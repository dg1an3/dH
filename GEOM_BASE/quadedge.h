#ifndef QUADEDGE_H
#define QUADEDGE_H

#include "Vector.h"

class Line 
{
public:
	Line(const CVector<2>&, const CVector<2>&);
	double eval(const CVector<2>&) const;
	int classify(const CVector<2>&) const;

private:
	double a, b, c;
};

// Line:

inline Line::Line(const CVector<2>& p, const CVector<2>& q)
// Computes the normalized line equation through the
// points p and q.
{
	CVector<2> t = q - p;
	double len =  t.GetLength();
	a = t[1] / len; 
	b = - t[0] / len; 
	c = - (a * p[0] + b * p[1]);
}

inline double Line::eval(const CVector<2>& p) const
// Plugs point p into the line equation.
{
	return (a * p[0] + b * p[1] + c);
}

inline int Line::classify(const CVector<2>& p) const
// Returns -1, 0, or 1, if p is to the left of, on,
// or right of the line, respectively.
{
	double d = eval(p);
	return (d < -EPS) ? -1 : (d > EPS ? 1 : 0);
}

class QuadEdge;
class Subdivision;

class Edge 
{
	friend QuadEdge;
	friend Subdivision;
public:
	Edge();

	// accessors for the edges end points
	const CVector<2>& Org2d() const;
	const CVector<2>& Dest2d() const;
	void  EndPoints(const CVector<2>&, const CVector<2>&);

	Edge* Rot();
	Edge* invRot();
	Edge* Sym();

	Edge* Onext();
	Edge* Oprev();

	Edge* Dnext();
	Edge* Dprev();

	Edge* Lnext();
	Edge* Lprev();

	Edge* Rnext();
	Edge* Rprev();

	// accessor for the parent QuadEdge
	QuadEdge* Qedge()	{ return (QuadEdge *)(this - m_nIndex); }

private:
	// stores the index of this edge in the QuadEdge
	int m_nIndex;

	// the next edge (contains the destination for this edge)
	Edge *m_pNext;

	// the origin point for this edge
	CVector<2> m_pt;
};

class QuadEdge 
{
	friend Subdivision;
public:
	QuadEdge();
	int TimeStamp(unsigned int);
private:
	Edge m_edges[4];
	unsigned int m_ts;
};

class Subdivision 
{
public:
	Subdivision(const CVector<2>&, const CVector<2>&, const CVector<2>&);
	virtual ~Subdivision();

	void InsertSite(const CVector<2>&);
	void InsertPoints(int nCount);

	Edge *GetStartingEdge()
	{
		return m_pStartingEdge;
	}

protected:
	Edge *MakeEdge();	
	void DeleteEdge(Edge* e);

	Edge *Locate(const CVector<2>& pt);

	Edge* Connect(Edge* a, Edge* b);
	void Swap(Edge* e);
	void Splice(Edge* a, Edge* b);

private:
	// pointer to the starting edge for the subdivision
	Edge *m_pStartingEdge;

	// array of created QuadEdges; used for housekeeping
	CTypedPtrArray<CPtrArray, QuadEdge*> m_arrQuadEdges;
};

inline QuadEdge::QuadEdge()
{
	m_edges[0].m_nIndex = 0;
	m_edges[1].m_nIndex = 1;
	m_edges[2].m_nIndex = 2;
	m_edges[3].m_nIndex = 3;

	m_edges[0].m_pNext = &(m_edges[0]); 
	m_edges[1].m_pNext = &(m_edges[3]);
	m_edges[2].m_pNext = &(m_edges[2]); 
	m_edges[3].m_pNext = &(m_edges[1]);

	m_ts = 0;
}

inline int QuadEdge::TimeStamp(unsigned int stamp)
{
	if (m_ts != stamp) 
	{
		m_ts = stamp;
		return TRUE;
	} else
		return FALSE;
}

/************************* Edge Algebra *************************************/

inline Edge::Edge() 
: m_pNext(NULL) 
{ 
}

inline Edge* Edge::Rot()
// Return the dual of the current edge, directed from its right to its left.
{
	return (m_nIndex < 3) ? this + 1 : this - 3;
}

inline Edge* Edge::invRot()
// Return the dual of the current edge, directed from its left to its right.
{
	return (m_nIndex > 0) ? this - 1 : this + 3;
}

inline Edge* Edge::Sym()
// Return the edge from the destination to the origin of the current edge.
{
	return (m_nIndex < 2) ? this + 2 : this - 2;
}

inline Edge* Edge::Onext()
// Return the next ccw edge around (from) the origin of the current edge.
{
	return m_pNext;
}

inline Edge* Edge::Oprev()
// Return the next cw edge around (from) the origin of the current edge.
{
	return Rot()->Onext()->Rot();
}

inline Edge* Edge::Dnext()
// Return the next ccw edge around (into) the destination of the current edge.
{
	return Sym()->Onext()->Sym();
}

inline Edge* Edge::Dprev()
// Return the next cw edge around (into) the destination of the current edge.
{
	return invRot()->Onext()->invRot();
}

inline Edge* Edge::Lnext()
// Return the ccw edge around the left face following the current edge.
{
	return invRot()->Onext()->Rot();
}

inline Edge* Edge::Lprev()
// Return the ccw edge around the left face before the current edge.
{
	return Onext()->Sym();
}

inline Edge* Edge::Rnext()
// Return the edge around the right face ccw following the current edge.
{
	return Rot()->Onext()->invRot();
}

inline Edge* Edge::Rprev()
// Return the edge around the right face ccw before the current edge.
{
	return Sym()->Onext();
}

/************** Access to data pointers *************************************/

inline const CVector<2>& Edge::Org2d() const
{
	return m_pt;
}

inline const CVector<2>& Edge::Dest2d() const
{
	return (m_nIndex < 2) ? ((this + 2)->m_pt) : ((this - 2)->m_pt);	// Sym()->m_pt; 
}

inline void Edge::EndPoints(const CVector<2>& or, const CVector<2>& de)
{
	m_pt = or;
	Sym()->m_pt = de;
}

#endif /* QUADEDGE_H */
