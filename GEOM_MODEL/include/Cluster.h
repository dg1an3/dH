//////////////////////////////////////////////////////////////////////
// Cluster.h: interface for the CCluster class.
//
// Copyright (C) 2003 Derek G Lane
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLUSTER_H__9B149912_00AC_49D9_ADF7_84B18B474FD4__INCLUDED_)
#define AFX_CLUSTER_H__9B149912_00AC_49D9_ADF7_84B18B474FD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// need general float stuff
#include <float.h>

// vector is default template class
#include <VectorD.h>

// forward define
template<typename TYPE>
class CCluster;


//////////////////////////////////////////////////////////////////////
// class CDistanceFunction
// 
// a distance function must be provided for the cluster analysis
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CDistanceFunction
{
public:
	virtual double operator()(const CCluster<TYPE> *pRight, 
		const CCluster<TYPE> *pLeft) = 0;
};


//////////////////////////////////////////////////////////////////////
// class CCluster
// 
// represents a single cluster within a cluster analysis.  don't 
// construct directly, just call the static function Analyze.
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CCluster : public CObject  
{
	// constructor / destructor
	CCluster();

public:
	virtual ~CCluster();

	// accessors for labels
	const CString& GetLabel() const;

	// accessors for cluster value (mean if an amalgamation)
	const TYPE& GetValue() const;

	// returns descendant count (all children, grandchildren, etc.)
	int GetDescendantCount() const;

	// child accessors
	const CCluster *GetRight() const;
	const CCluster *GetLeft() const;

	// returns the depth (levels of children)
	int GetDepth() const;

	// returns the distance measure between the two children
	double GetDist() const;

	// returns the maximum distance of this and all children
	double MaxDist() const;

	// helper function for graphing
	virtual void Draw(CDC *pDC, CRect *pRect, double maxDist = 0.0, 
		double minDist = 0.0) const;

	// static function to create the analysis tree from an array of TYPE
	static const CCluster *Analyze(TYPE *pValues, int nCount, 
		CDistanceFunction<TYPE>& func, CStringArray *arrLabels = NULL);

protected:
	// sets the children and distance
	void SetChildren(CCluster *pRight, CCluster *pLeft, double dist);

	// helper function to find the closest cluster in an array
	void FindClosest(const CArray<CCluster *, CCluster *>& arrClusters, 
		int nStart, int *pAtClosest, double *pMinDist, 
		CDistanceFunction<TYPE>& func);

private:
	// the cluster label
	CString m_strLabel;

	// stores the cluster's value (mean if an amalgamation)
	mutable TYPE m_value;
	mutable BOOL m_bComputeValue;

	// stores the number of descendants
	int m_nDescendantCount;

	// pointers to children
	CCluster *m_pRight;
	CCluster *m_pLeft;

	// distance measure between children
	double m_dist;

	// pointer to all allocated clusters
	CCluster *m_pAllClusters;
};


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::CCluster
// 
// returns the characteristic value for this cluster
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
CCluster<TYPE>::CCluster()
: m_bComputeValue(FALSE),
	m_pRight(NULL),
	m_pLeft(NULL),
	m_nDescendantCount(0),
	m_dist(0.0),
	m_pAllClusters(NULL)
{	
}	// CCluster<TYPE>::CCluster


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::~CCluster
// 
// returns the characteristic value for this cluster
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
CCluster<TYPE>::~CCluster()
{
	if (m_pAllClusters)
	{
		delete [] m_pAllClusters;
	}

}	// CCluster<TYPE>::~CCluster


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetValue
// 
// returns the characteristic value for this cluster
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
const TYPE& CCluster<TYPE>::GetValue() const
{
	// see if the value (mean) needs to be computed
	if (TRUE // m_bComputeValue 
		&& m_pRight != NULL 
		&& m_pLeft != NULL)
	{
		// determine the weights for right and left child values
		double weightRight = (double) (1 + m_pRight->GetDescendantCount()) 
			/ (double) GetDescendantCount();
		double weightLeft = (double) (1 + m_pLeft->GetDescendantCount())
			/ (double) GetDescendantCount();

		// form the composite value
		m_value = weightRight * m_pRight->GetValue();
		m_value += weightLeft * m_pLeft->GetValue();

		// don't do this again
		m_bComputeValue = FALSE;
	}

	// return the computed or stored value
	return m_value;

}	// CCluster<TYPE>::GetValue


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetDescendantCount
// 
// returns the number of descendant clusters
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
int CCluster<TYPE>::GetDescendantCount() const
{
	return m_nDescendantCount;

}	// CCluster<TYPE>::GetDescendantCount


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetRight
// 
// returns the right child
//////////////////////////////////////////////////////////////////////
template<typename TYPE> 
const CCluster<TYPE> *CCluster<TYPE>::GetRight() const
{
	return m_pRight;

}	// CCluster<TYPE>::GetRight


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetLeft
// 
// returns the left child
//////////////////////////////////////////////////////////////////////
template<typename TYPE> 
const CCluster<TYPE> *CCluster<TYPE>::GetLeft() const
{
	return m_pLeft;

}	// CCluster<TYPE>::GetLeft


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetDepth
// 
// returns the depth (levels of children)
//////////////////////////////////////////////////////////////////////
template<typename TYPE> 
int CCluster<TYPE>::GetDepth() const
{
	if (GetRight() == NULL)
	{
		return 0;
	}

	return __max(GetLeft()->GetDepth(), GetRight()->GetDepth())+1;

}	// CCluster<TYPE>::GetDepth
	

//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::GetDist
// 
// returns the distance measure between the two children
//////////////////////////////////////////////////////////////////////
template<typename TYPE> 
double CCluster<TYPE>::GetDist() const
{
	return m_dist;

}	// CCluster<TYPE>::GetDist


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::MaxDist
// 
// returns the maximum distance of this and all children
//////////////////////////////////////////////////////////////////////
template<typename TYPE> 
double CCluster<TYPE>::MaxDist() const
{
	double childMax = -DBL_MAX;
	if (GetRight())
	{
		childMax = __max(GetRight()->MaxDist(), GetLeft()->MaxDist());
	}

	return __max(GetDist(), childMax);

}	// CCluster<TYPE>::MaxDist


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::Draw
// 
// helper function to draw the cluster
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
void CCluster<TYPE>::Draw(CDC *pDC, CRect *pRect, double maxDist, 
						  double minDist) const
{
	double fraction = (maxDist > 0.0) ? GetDist() / maxDist : 1.0;
	ASSERT(fraction >= 0.0 && fraction <= 1.0);

	int nCenterX = pRect->right - ((double) pRect->Width()) * fraction;

	if (GetRight() && GetDist() > minDist)
	{
		CRect rectUpper(*pRect);
		rectUpper.left = nCenterX; // pRect->left + nCenterX;
		GetRight()->Draw(pDC, &rectUpper, GetDist());

		CRect rectLower(*pRect);
		rectLower.left = nCenterX; // pRect->left + nCenterX;
		rectLower.top = rectUpper.bottom;
		GetLeft()->Draw(pDC, &rectLower, GetDist());

		pDC->MoveTo(nCenterX, rectUpper.CenterPoint().y);
		pDC->LineTo(nCenterX, rectLower.CenterPoint().y);

		pRect->bottom = rectLower.bottom;

		int nCenterY = pRect->CenterPoint().y;
		pDC->MoveTo(pRect->left, nCenterY);
		pDC->LineTo(nCenterX, nCenterY);
	}
	else
	{
		pRect->bottom = pRect->top + 20;

		int nCenterY = pRect->CenterPoint().y;
		pDC->MoveTo(pRect->left, nCenterY);
		pDC->LineTo(pRect->right, nCenterY);

		pDC->SetTextAlign(TA_LEFT);

		// otherwise, just output the name of the cluster
		pDC->TextOut(pRect->right, pRect->top, m_strLabel);
	}

}	// CCluster<TYPE>::Draw


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::Analyze
// 
// analyzes the passed values, and returns a pointer to the first
//		cluster resulting from the analysis
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
const CCluster<TYPE> *CCluster<TYPE>::Analyze(TYPE *arrVal, int nCount, 
		CDistanceFunction<TYPE>& func, CStringArray *arrLabels)
{
	// allocate all of the clusters for the analysis
	CCluster *arrClusters = new CCluster[2 * nCount - 2];

	// declare the array of pointers for remaining (unamalgamated) clusters
	CArray<CCluster *, CCluster *> arrRemainClusters;

	// initialize the leaf clusters and add to remaining pointer array
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		arrClusters[nAt].m_value = arrVal[nAt];
		if (arrLabels != NULL)
		{
			arrClusters[nAt].m_strLabel = (*arrLabels)[nAt];
		}
		arrRemainClusters.Add(&arrClusters[nAt]);
	}

	// the next cluster specifies where to place the next
	//		amalgamation
	int nNextCluster = nCount;

	// as long as more than one cluster remains to be amalgamated
	while (arrRemainClusters.GetSize() > 2)
	{
		// these store the indices of the two children
		int nChild1 = 0;
		int nChild2 = 0;

		// initialize the next amalgamation cluster
		arrClusters[nNextCluster].SetChildren(NULL, NULL, DBL_MAX);

		for (int nAt = 0; nAt < arrRemainClusters.GetSize(); nAt++)
		{
			// stores info about closest cluster
			double dist = 0.0;
			int nAtClosest = 0;

			// find the closest cluster to the current one
			arrRemainClusters[nAt]->FindClosest(arrRemainClusters, nAt+1, 
				&nAtClosest, &dist, func);

			// see if the distance is less than minimum found thus far
			if (dist < arrClusters[nNextCluster].GetDist())
			{
				// if so, store the indices of the children
				nChild1 = nAt;
				nChild2 = nAtClosest;

				// and set the children in the current amalgamation
				arrClusters[nNextCluster].SetChildren(
					arrRemainClusters[nChild1],
					arrRemainClusters[nChild2], dist);
			}
		}

		// finally, removing the children from the remaining list
		//		and adding new amalgamation
		arrRemainClusters[nChild1] = &arrClusters[nNextCluster];
		arrRemainClusters.RemoveAt(nChild2);

		// move to the next available amalgamation
		nNextCluster++;
	}

	// compute the distance between two remaining clusters
	double dist = func(arrRemainClusters[0], arrRemainClusters[1]);

	// create the final root cluster
	CCluster *pRootCluster = new CCluster();

	// set two children of root 
	pRootCluster->SetChildren(arrRemainClusters[0], arrRemainClusters[1], dist);

	// set the AllClusters pointer to the top-most amalgation
	pRootCluster->m_pAllClusters = arrClusters;

	// and return the topmost amalgamation
	return pRootCluster;

}	// CCluster<TYPE>::Analyze


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::SetChildren
// 
// sets the children and stored distance between the children
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
void CCluster<TYPE>::SetChildren(CCluster *pRight, CCluster *pLeft, double dist)
{
	// either both must be NULL, or neither
	ASSERT((pRight == NULL && pLeft == NULL) 
		|| (pRight != NULL && pLeft != NULL));

	// set the children
	m_pRight = pRight;
	m_pLeft = pLeft;

	// set the distance
	m_dist = dist;

	// if the children exist,
	if (m_pRight != NULL
		&& m_pLeft != NULL)
	{
		// update the descendant count -- assumes children's count is accurate
		m_nDescendantCount = m_pRight->GetDescendantCount() 
			+ m_pLeft->GetDescendantCount() + 2;

		// flag to re-compute value
		m_bComputeValue = TRUE;
	}
	else 
	{
		// clear descendant count
		m_nDescendantCount = 0;
	}

	// set the label to reflect number of children
	m_strLabel.Format("(%i nodes)", m_nDescendantCount);

}	// CCluster<TYPE>::SetChildren


//////////////////////////////////////////////////////////////////////
// CCluster<TYPE>::FindClosest
// 
// searches through the remaining elements in the array, attempting
//		to locate the closest as defined by the distance function
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
void CCluster<TYPE>::FindClosest(const CArray<CCluster *, CCluster *>& arrClusters, 
		int nStart, int *pAtClosest, double *pMinDist, 
		CDistanceFunction<TYPE>& func)
{
	// initialize the index of the closest cluster
	(*pAtClosest) = 0;

	// initialize the minimum distance to max value
	(*pMinDist) = DBL_MAX;

	// loop through the other clusters
	for (int nAt = nStart; nAt < arrClusters.GetSize(); nAt++)
	{
		// compute the distance
		double dist = func(this, (CCluster<TYPE> *) arrClusters[nAt]);

		// check to see if this cluster is closer than the current one
		if (dist < (*pMinDist))
		{
			// store the distance
			(*pMinDist) = dist;

			// and set the new closest cluster
			(*pAtClosest) = nAt;
		}
	}

}	// CCluster<TYPE>::FindClosest


//////////////////////////////////////////////////////////////////////
// class CEuclideanVectorDistance
// 
// class to perform cluster analysis with euclidean distance
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CEuclideanVectorDistance : public CDistanceFunction<TYPE>
{
public:
	// computes the euclidean distance between the vector values of 
	//		the input clusters
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// form vector difference
		TYPE vec = pRight->GetValue();
		vec -= pLeft->GetValue();

		if (vec.GetLength() == 0.0)
		{ 
			TRACE_VECTOR("vec", pRight->GetValue());
			TRACE_VECTOR("other", pLeft->GetValue());
		}

		// returns length of vector difference
		return vec.GetLength();

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CInverseSquareDistance
// 
// distance function for inverse square distance of two vectors/scalars
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CInverseProductDistance : public CDistanceFunction<TYPE>
{
	// flag for squaring
	BOOL m_bSquare;

	// epsilon for inverse
	double m_epsilon;

public:
	// constructor -- pass parameters
	CInverseProductDistance(double epsilon = EPSILON, BOOL bSquare = TRUE) 
		: m_epsilon(epsilon),
			m_bSquare(bSquare) 
	{ 
	}	// CInverseProductDistance

	// computes the inverse product distance
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// form dot product
		double product = pRight->GetValue() * pLeft->GetValue();

		if (m_bSquare)
		{
			// returns inverse product squared
			return 1.0 / (product * product + m_epsilon);
		}
		else
		{
			// returns inverse absolute value
			return 1.0 / (abs(product) + m_epsilon);
		}

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CNearestNeighborDistance
// 
// class to perform cluster analysis with a nearest neighbor 
//		comparison
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CNearestNeighborDistance : public CDistanceFunction<TYPE>
{
	// helper to perform regular euclidean distance
	CDistanceFunction<TYPE> *m_pDist;

public:
	// constructor -- pass the base distance function
	CNearestNeighborDistance(CDistanceFunction<TYPE> *pDist) 
		: m_pDist(pDist) 
	{ 
	}	// CNearestNeighborDistance


	// computes the NN distance, traversing children if present
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// are children present?
		if (pRight->GetRight() != NULL)
		{
			if (pLeft->GetRight() != NULL)
			{
				// get NN distance for children combos
				double distRR = (*this)(pRight->GetRight(), pLeft->GetRight());
				double distRL = (*this)(pRight->GetRight(), pLeft->GetLeft());
				double distLR = (*this)(pRight->GetLeft(), pLeft->GetRight());
				double distLL = (*this)(pRight->GetLeft(), pLeft->GetLeft());

				// and return the minimum
				return __min(__min(distRR, distRL), __min(distLR, distLL));
			}
			else
			{
				// and return the minimum
				return __min(
					(*this)(pRight->GetRight(), pLeft), 
					(*this)(pRight->GetLeft(), pLeft));
			}
		} 
		else if (pLeft->GetRight() != NULL)
		{
			// and return the minimum
			return __min(
				(*this)(pRight, pLeft->GetRight()), 
				(*this)(pRight, pLeft->GetLeft()));
		}


		// otherwise, use other distance function
		return (*m_pDist)(pRight, pLeft);

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CFurthestNeighborDistance
// 
// class to perform cluster analysis with a nearest neighbor 
//		comparison
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CFurthestNeighborDistance : public CDistanceFunction<TYPE>
{
	// helper to perform regular euclidean distance
	CDistanceFunction<TYPE> *m_pDist;

public:
	// constructor -- pass the base distance function
	CFurthestNeighborDistance(CDistanceFunction<TYPE> *pDist) 
		: m_pDist(pDist) { }

	// computes the FN distance, traversing children if present
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// are children present?
		if (pRight->GetRight() != NULL)
		{
			if (pLeft->GetRight() != NULL)
			{
				// get NN distance for children combos
				double distRR = (*this)(pRight->GetRight(), pLeft->GetRight());
				double distRL = (*this)(pRight->GetRight(), pLeft->GetLeft());
				double distLR = (*this)(pRight->GetLeft(), pLeft->GetRight());
				double distLL = (*this)(pRight->GetLeft(), pLeft->GetLeft());

				// and return the maximum
				return __max(__max(distRR, distRL), __max(distLR, distLL));
			}
			else
			{
				// return the maximum
				return __max(
					(*this)(pRight->GetRight(), pLeft), 
					(*this)(pRight->GetLeft(), pLeft));
			}
		} 
		else if (pLeft->GetRight() != NULL)
		{
			// return the maximum
			return __max(
				(*this)(pRight, pLeft->GetRight()), 
				(*this)(pRight, pLeft->GetLeft()));
		}

		// otherwise, use other distance function
		return (*m_pDist)(pRight, pLeft);

	}	// operator()
};

#endif // !defined(AFX_CLUSTER_H__9B149912_00AC_49D9_ADF7_84B18B474FD4__INCLUDED_)
