// DelaunayTess.h: interface for the CDelaunayTess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DELAUNAYTESS_H__95792EB1_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
#define AFX_DELAUNAYTESS_H__95792EB1_1715_11D5_9E54_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Observer.h>
#include <Collection.h>

#include "Polygon.h"
#include "Edge.h"
#include "QuadEdge1.h"

class CDelaunayTess : public CObservableObject  
{
public:
	CDelaunayTess(const CVector<2>&, const CVector<2>&, const CVector<2>&);
	virtual ~CDelaunayTess();

	CEdge *GetStartingEdge();

	// inserts a new vertex into the tessellation
	void AddVertex(const CVector<2>&);

	// adds a new polygon to the tessellation
	CCollection< CPolygon > polygons;

	// inserts a number of random vertices into the tessellation
	void InsertRandomPoints(int nCount = 20);

protected:
	CEdge *MakeEdge();	
	void DeleteEdge(CEdge* e);

	CEdge *Locate(const CVector<2>& pt);

	CEdge* Connect(CEdge* a, CEdge* b);
	void Swap(CEdge* e);
	void Splice(CEdge* a, CEdge* b);

private:
	// pointer to the starting edge for the CDelaunayTriangulation
	CEdge *m_pStartingEdge;

	// array of created QuadEdges; used for housekeeping
	CTypedPtrArray<CPtrArray, CQuadEdge*>  m_arrQuadEdges;
};

#endif // !defined(AFX_DELAUNAYTESS_H__95792EB1_1715_11D5_9E54_00B0D0609AB0__INCLUDED_)
