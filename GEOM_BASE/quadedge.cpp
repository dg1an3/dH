#include "stdafx.h"

#include <stdlib.h>
#include "quadedge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*************** Geometric Predicates for Delaunay Diagrams *****************/

inline double TriArea(const CVector<2>& a, const CVector<2>& b, const CVector<2>& c)
// Returns twice the area of the oriented triangle (a, b, c), i.e., the
// area is positive if the triangle is oriented counterclockwise.
{
	return (b[0] - a[0])*(c[1] - a[1]) - (b[1] - a[1])*(c[0] - a[0]);
}

int InCircle(const CVector<2>& a, const CVector<2>& b,
			 const CVector<2>& c, const CVector<2>& d)
// Returns TRUE if the point d is inside the circle defined by the
// points a, b, c. See Guibas and Stolfi (1985) p.107.
{
	return 	(a[0]*a[0] + a[1]*a[1]) * TriArea(b, c, d) -
			(b[0]*b[0] + b[1]*b[1]) * TriArea(a, c, d) +
			(c[0]*c[0] + c[1]*c[1]) * TriArea(a, b, d) -
			(d[0]*d[0] + d[1]*d[1]) * TriArea(a, b, c) > 0;
}

int ccw(const CVector<2>& a, const CVector<2>& b, const CVector<2>& c)
// Returns TRUE if the points a, b, c are in a counterclockwise order
{
	return (TriArea(a, b, c) > 0);
}

int RightOf(const CVector<2>& x, Edge* e)
{
	return ccw(x, e->Dest2d(), e->Org2d());
}

int LeftOf(const CVector<2>& x, Edge* e)
{
	return ccw(x, e->Org2d(), e->Dest2d());
}

int OnEdge(const CVector<2>& x, Edge* e)
// A predicate that determines if the point x is on the edge e.
// The point is considered on if it is in the EPS-neighborhood
// of the edge.
{
	double t1, t2, t3;
	t1 = (x - e->Org2d()).GetLength(); // norm();
	t2 = (x - e->Dest2d()).GetLength(); // norm();
	if (t1 < EPS || t2 < EPS)
	    return TRUE;
	t3 = (e->Org2d() - e->Dest2d()).GetLength(); // norm();
	if (t1 > t3 || t2 > t3)
	    return FALSE;
	Line line(e->Org2d(), e->Dest2d());
	return (fabs(line.eval(x)) < EPS);
}

/************* Topological Operations for Delaunay Diagrams *****************/

Subdivision::Subdivision(const CVector<2>& a, const CVector<2>& b, const CVector<2>& c)
// Initialize a subdivision to the triangle defined by the points a, b, c.
{
//	CVector<2> *da, *db, *dc;
//	da = new CVector<2>(a), db = new CVector<2>(b), dc = new CVector<2>(c);
	Edge* ea = MakeEdge();
	ea->EndPoints(a, b);

	Edge* eb = MakeEdge();
	Splice(ea->Sym(), eb);
	eb->EndPoints(b, c);

	Edge* ec = MakeEdge();
	Splice(eb->Sym(), ec);
	ec->EndPoints(c, a);
	Splice(ec->Sym(), ea);

	m_pStartingEdge = ea;
}

Subdivision::~Subdivision()
// Initialize a subdivision to the triangle defined by the points a, b, c.
{
	for (int nAt = 0; nAt < m_arrQuadEdges.GetSize(); nAt++)
	{
		for (int nAt2 = nAt+1; nAt2 < m_arrQuadEdges.GetSize(); nAt2++)
			ASSERT(m_arrQuadEdges[nAt2] != m_arrQuadEdges[nAt]);

		delete m_arrQuadEdges[nAt];
	}
}

Edge* Subdivision::MakeEdge()
{
	QuadEdge *ql = new QuadEdge;
	m_arrQuadEdges.Add(ql);
	return ql->m_edges;
}

void Subdivision::DeleteEdge(Edge* e)
{
	Splice(e, e->Oprev());
	Splice(e->Sym(), e->Sym()->Oprev());

	for (int nAt = 0; nAt < m_arrQuadEdges.GetSize(); nAt++)
		if (m_arrQuadEdges[nAt] == e->Qedge())
			m_arrQuadEdges.RemoveAt(nAt);

	delete e->Qedge();
}

Edge* Subdivision::Connect(Edge* a, Edge* b)
// Add a new edge e connecting the destination of a to the
// origin of b, in such a way that all three have the same
// left face after the connection is complete.
// Additionally, the data pointers of the new edge are set.
{
	Edge* e = MakeEdge();

	Splice(e, a->Lnext());
	Splice(e->Sym(), b);
	e->EndPoints(a->Dest2d(), b->Org2d());
	return e;
}

void Subdivision::Swap(Edge* e)
// Essentially turns edge e counterclockwise inside its enclosing
// quadrilateral. The data pointers are modified accordingly.
{
	Edge* a = e->Oprev();
	Edge* b = e->Sym()->Oprev();
	Splice(e, a);
	Splice(e->Sym(), b);
	Splice(e, a->Lnext());
	Splice(e->Sym(), b->Lnext());
	e->EndPoints(a->Dest2d(), b->Dest2d());
}

void Subdivision::Splice(Edge* a, Edge* b)
// This operator affects the two edge rings around the origins of a and b,
// and, independently, the two edge rings around the left faces of a and b.
// In each case, (i) if the two rings are distinct, Splice will combine
// them into one; (ii) if the two are the same ring, Splice will break it
// into two separate pieces.
// Thus, Splice can be used both to attach the two edges together, and
// to break them apart. See Guibas and Stolfi (1985) p.96 for more details
// and illustrations.
{
	Edge* alpha = a->Onext()->Rot();
	Edge* beta  = b->Onext()->Rot();

	Edge* t1 = b->Onext();
	Edge* t2 = a->Onext();
	Edge* t3 = beta->Onext();
	Edge* t4 = alpha->Onext();

	a->m_pNext = t1;
	b->m_pNext = t2;
	alpha->m_pNext = t3;
	beta->m_pNext = t4;
}

/************* An Incremental Algorithm for the Construction of *************/
/************************ Delaunay Diagrams *********************************/

Edge* Subdivision::Locate(const CVector<2>& x)
// Returns an edge e, s.t. either x is on e, or e is an edge of
// a triangle containing x. The search starts from startingEdge
// and proceeds in the general direction of x. Based on the
// pseudocode in Guibas and Stolfi (1985) p.121.
{
	Edge* e = m_pStartingEdge;

	while (TRUE) {
		if (x.IsApproxEqual(e->Org2d()) || x.IsApproxEqual(e->Dest2d()))
		    return e;
		else if (RightOf(x, e))
			 e = e->Sym();
		else if (!RightOf(x, e->Onext()))
			 e = e->Onext();
		else if (!RightOf(x, e->Dprev()))
			 e = e->Dprev();
		else
		    return e;
	}
}

void Subdivision::InsertSite(const CVector<2>& x)
// Inserts a new point into a subdivision representing a Delaunay
// triangulation, and fixes the affected edges so that the result
// is still a Delaunay triangulation. This is based on the
// pseudocode from Guibas and Stolfi (1985) p.120, with slight
// modifications and a bug fix.
{
	Edge* e = Locate(x);
	if ((x.IsApproxEqual(e->Org2d())) || (x.IsApproxEqual(e->Dest2d())))  // point is already in
	    return;
	else if (OnEdge(x, e)) {
		e = e->Oprev();
		DeleteEdge(e->Onext());
	}

	// Connect the new point to the vertices of the containing
	// triangle (or quadrilateral, if the new point fell on an
	// existing edge.)
	Edge* base = MakeEdge();

	base->EndPoints(e->Org2d(), x);
	Splice(base, e);
	m_pStartingEdge = base;
	do {
		base = Connect(e, base->Sym());

		e = base->Oprev();
	} while (e->Lnext() != m_pStartingEdge);

	// Examine suspect edges to ensure that the Delaunay condition
	// is satisfied.
	do {
		Edge* t = e->Oprev();
		if (RightOf(t->Dest2d(), e) &&
			InCircle(e->Org2d(), t->Dest2d(), e->Dest2d(), x)) {
				Swap(e);
				e = e->Oprev();
		}
		else if (e->Onext() == m_pStartingEdge)  // no more suspect edges
			return;
		else  // pop a suspect edge
		    e = e->Onext()->Lprev();
	} while (TRUE);
}

/*****************************************************************************/

void Subdivision::InsertPoints(int nCount)
{
	for (int i = 0; i < nCount; i++) 
	{
		double u = (double)rand() / (double)RAND_MAX;
		double v = (double)rand() / (double)RAND_MAX;
		InsertSite(CVector<2>(u,v));
	}
}
