// DelaunayTess.cpp: implementation of the CDelaunayTess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "DelaunayTess.h"
#include "Line.h"
// #include "QuadEdge1.h"

//////////////////////////////////////////////////////////////////////
// Geometric Predicates for Delaunay Diagrams

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

int RightOf(const CVector<2>& x, CEdge* e)
{
	return ccw(x, e->Dest2d(), e->Org2d());
}

int LeftOf(const CVector<2>& x, CEdge* e)
{
	return ccw(x, e->Org2d(), e->Dest2d());
}

int OnEdge(const CVector<2>& x, CEdge* e)
// A predicate that determines if the point x is on the CEdge e.
// The point is considered on if it is in the EPS-neighborhood
// of the CEdge.
{
	double t1, t2, t3;
	t1 = (x - e->Org2d()).GetLength(); // norm();
	t2 = (x - e->Dest2d()).GetLength(); // norm();
	if (t1 < EPS || t2 < EPS)
	    return TRUE;
	t3 = (e->Org2d() - e->Dest2d()).GetLength(); // norm();
	if (t1 > t3 || t2 > t3)
	    return FALSE;
	CLine line(e->Org2d(), e->Dest2d());
	return (fabs(line.Eval(x)) < EPS);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDelaunayTess::CDelaunayTess(const CVector<2>& a, const CVector<2>& b, const CVector<2>& c)
: m_pStartingEdge(NULL)
// Initialize a CDelaunayTriangulation to the triangle defined by the points a, b, c.
{
	CEdge* ea = MakeEdge();
	ea->EndPoints(a, b);

	CEdge* eb = MakeEdge();
	Splice(ea->Sym(), eb);
	eb->EndPoints(b, c);

	CEdge* ec = MakeEdge();
	Splice(eb->Sym(), ec);
	ec->EndPoints(c, a);
	Splice(ec->Sym(), ea);

	m_pStartingEdge = ea;
}

CDelaunayTess::~CDelaunayTess()
{
	for (int nAt = 0; nAt < m_arrQuadEdges.GetSize(); nAt++)
	{
		for (int nAt2 = nAt+1; nAt2 < m_arrQuadEdges.GetSize(); nAt2++)
			ASSERT(m_arrQuadEdges[nAt2] != m_arrQuadEdges[nAt]);

		delete m_arrQuadEdges[nAt];
	}
}


CEdge *CDelaunayTess::GetStartingEdge()
{
	return m_pStartingEdge;
}


void CDelaunayTess::AddVertex(const CVector<2>& x)
// Inserts a new point into a CDelaunayTess representing a Delaunay
// triangulation, and fixes the affected edges so that the result
// is still a Delaunay triangulation. This is based on the
// pseudocode from Guibas and Stolfi (1985) p.120, with slight
// modifications and a bug fix.
{
	CEdge* e = Locate(x);
	if ((x.IsApproxEqual(e->Org2d())) || (x.IsApproxEqual(e->Dest2d())))  // point is already in
	    return;
	else if (OnEdge(x, e)) 
	{
		e = e->Oprev();
		DeleteEdge(e->Onext());
	}

	// Connect the new point to the vertices of the containing
	// triangle (or quadrilateral, if the new point fell on an
	// existing CEdge.)
	CEdge* base = MakeEdge();

	base->EndPoints(e->Org2d(), x);
	Splice(base, e);
	m_pStartingEdge = base;
	do {
		base = Connect(e, base->Sym());

		e = base->Oprev();
	} while (e->Lnext() != m_pStartingEdge);

	// Examine suspect edges to ensure that the Delaunay condition
	// is satisfied.

	// This is the part of the process that may destroy an edge, so we have to
	// check to ensure that the edge we are about to swap is not a persistent edge
	do {
		CEdge* t = e->Oprev();
		if (RightOf(t->Dest2d(), e) &&
			InCircle(e->Org2d(), t->Dest2d(), e->Dest2d(), x)) 
		{
			if (!e->m_bIsSide)
			{
				Swap(e);
			}	
			else 
			{
				// bisect the edge to avoid breaking the side
				CVector<2> ptNew = e->m_pt + e->Sym()->m_pt;
				ptNew *= 0.5;
				AddVertex(ptNew);
				return;
			}

			e = e->Oprev();
		}
		else if (e->Onext() == m_pStartingEdge)  // no more suspect edges
			return;
		else  // pop a suspect CEdge
		    e = e->Onext()->Lprev();
	} while (TRUE);
}

void CDelaunayTess::InsertRandomPoints(int nCount)
{
	for (int i = 0; i < nCount; i++) 
	{
		double u = (double)rand() / (double)RAND_MAX;
		double v = (double)rand() / (double)RAND_MAX;
		AddVertex(CVector<2>(u,v));
	}
}

CEdge* CDelaunayTess::MakeEdge()
{
	CQuadEdge *ql = new CQuadEdge();
	m_arrQuadEdges.Add(ql);
	return ql->m_edges;
}

void CDelaunayTess::DeleteEdge(CEdge* e)
{
	Splice(e, e->Oprev());
	Splice(e->Sym(), e->Sym()->Oprev());

//	CQuadEdge *pQuadEdge;
//	for (int nAt = 0; nAt < m_arrQuadEdges.GetSize(); nAt++)
//		for (int nAtEdge = 0; nAtEdge < 4; nAtEdge++)
//			if (e = &(m_arrQuadEdges[nAt]->GetEdge(nAtEdge)))
//			{
//				pQuadEdge = m_arrQuadEdges[nAt];
//				// delete m_arrQuadEdges[nAt];
//				m_arrQuadEdges.RemoveAt(nAt);
//				// nAt = m_arrQuadEdges.GetSize();
//				break;
//			}

//	for (int nAt = 0; nAt < m_arrQuadEdges.GetSize(); nAt++)
//		for (int nAtEdge = 0; nAtEdge < 4; nAtEdge++)
//			ASSERT(m_arrQuadEdges[nAt]->GetEdge(nAtEdge).m_pNext != e);

	// delete e->Qedge();
}

CEdge* CDelaunayTess::Locate(const CVector<2>& x)
// Returns an CEdge e, s.t. either x is on e, or e is an CEdge of
// a triangle containing x. The search starts from startingEdge
// and proceeds in the general direction of x. Based on the
// pseudocode in Guibas and Stolfi (1985) p.121.
{
	CEdge* e = m_pStartingEdge;

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

CEdge* CDelaunayTess::Connect(CEdge* a, CEdge* b)
// Add a new CEdge e connecting the destination of a to the
// origin of b, in such a way that all three have the same
// left face after the connection is complete.
// Additionally, the data pointers of the new CEdge are set.
{
	CEdge* e = MakeEdge();

	Splice(e, a->Lnext());
	Splice(e->Sym(), b);
	e->EndPoints(a->Dest2d(), b->Org2d());
	return e;
}

void CDelaunayTess::Swap(CEdge* e)
// Essentially turns CEdge e counterclockwise inside its enclosing
// quadrilateral. The data pointers are modified accordingly.
{
	CEdge* a = e->Oprev();
	CEdge* b = e->Sym()->Oprev();
	Splice(e, a);
	Splice(e->Sym(), b);
	Splice(e, a->Lnext());
	Splice(e->Sym(), b->Lnext());
	e->EndPoints(a->Dest2d(), b->Dest2d());
}

void CDelaunayTess::Splice(CEdge* a, CEdge* b)
// This operator affects the two CEdge rings around the origins of a and b,
// and, independently, the two CEdge rings around the left faces of a and b.
// In each case, (i) if the two rings are distinct, Splice will combine
// them into one; (ii) if the two are the same ring, Splice will break it
// into two separate pieces.
// Thus, Splice can be used both to attach the two edges together, and
// to break them apart. See Guibas and Stolfi (1985) p.96 for more details
// and illustrations.
{
	CEdge* alpha = a->Onext()->Rot();
	CEdge* beta  = b->Onext()->Rot();

	CEdge* t1 = b->Onext();
	CEdge* t2 = a->Onext();
	CEdge* t3 = beta->Onext();
	CEdge* t4 = alpha->Onext();

	a->m_pNext = t1;
	b->m_pNext = t2;
	alpha->m_pNext = t3;
	beta->m_pNext = t4;
}

