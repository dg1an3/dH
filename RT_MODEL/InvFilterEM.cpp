// InvFilterEM.cpp: implementation of the CInvFilterEM class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "TestHisto.h"
#include "InvFilterEM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <float.h>

#include <XMLLogging.h>

#include <MatrixBase.inl>

// TODO: get this from MathUtil
REAL Sigmoid(REAL x)
{
	REAL res = 1.0 / (1.0 + exp(-x));
		// exp(m_inputScale * x) / (1.0 + exp(m_inputScale * x));

	if (!_finite(res))
	{
		if (x > 0.0)
			res = 1.0;
		else
			res = 0.0;
	}

	return res;
}

// TODO: move this to GEOM
BOOL IntersectRayPlane(const CVectorN<>& vPoint, const CVectorN<>& vDir, 
			const CVectorN<>& vPlanePoint, const CVectorN<>& vPlaneNormal, REAL *pLambda)
{
	BOOL bRes = FALSE;

	BEGIN_LOG_SECTION(IntersectRayPlane);
	LOG_EXPR_EXT(vPoint);
	LOG_EXPR_EXT(vDir);
	LOG_EXPR_EXT(vPlanePoint);
	LOG_EXPR_EXT(vPlaneNormal);

	REAL num = (vPlanePoint - vPoint) * vPlaneNormal;
	REAL den = vDir * vPlaneNormal;
	LOG_EXPR(num);
	LOG_EXPR(den);

	if (!IsApproxEqual<REAL>(den, 0.0))
	{
		(*pLambda) = num/den;
		LOG_EXPR(*pLambda);

		bRes = TRUE;
	}
	
	END_LOG_SECTION();

	return bRes;
}

// TODO: move this to MathUtil
BOOL NextPerm(int *pInd, int nCount, int nMax)
{
	if (nCount == 0)
	{
		return FALSE;
	} 

	if (!NextPerm(&pInd[1], nCount-1, nMax))
	{
		if (pInd[0] >= nMax-nCount+1)
		{
			return FALSE;
		}

		pInd[0]++;
		for (int nNext = 1; nNext < nCount; nNext++)
		{
			pInd[nNext] = pInd[nNext-1]+1;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInvFilterEM::CInvFilterEM()
	: CObjectiveFunction(FALSE)
{
	m_vFilterKernel.SetDim(3);
	m_vFilterKernel[0] = 0.25;
	m_vFilterKernel[1] = 0.5;
	m_vFilterKernel[2] = 0.25;
}

CInvFilterEM::~CInvFilterEM()
{

}

void CInvFilterEM::SetFilterOutput(const CVectorN<>& vFiltOut)
{
	BEGIN_LOG_SECTION(CInvFilterEM::SetFilterOutput);
	LOG_EXPR_EXT(vFiltOut);

	m_vFilterOut.SetDim(vFiltOut.GetDim() * 2 + 1);
	m_vFilterOut.SetZero();

	for (int nAt = 0; nAt < vFiltOut.GetDim(); nAt++)
	{
		m_vFilterOut[nAt * 2 + 1] = vFiltOut[nAt];
	}

	// set up the filter matrix
	m_mFilter.Reshape(m_vFilterOut.GetDim(), m_vFilterOut.GetDim());
	for (nAt = 0; nAt < m_mFilter.GetRows(); nAt++)
	{
		if (nAt > 0)
		{
			m_mFilter[nAt - 1][nAt] = m_vFilterKernel[0];
		}

		m_mFilter[nAt][nAt] = m_vFilterKernel[1];

		if (nAt < m_mFilter.GetRows()-1)
		{
			m_mFilter[nAt + 1][nAt] = m_vFilterKernel[2];
		}
	}
	LOG_EXPR_EXT(m_mFilter);

/*	// now invert the filter
	// TODO: fix Reshape
	m_mInvFilter.Reshape(m_mFilter.GetCols(), m_mFilter.GetRows());		
	m_mInvFilter = m_mFilter;
	m_mInvFilter.Invert();
	LOG_EXPR_EXT(m_mInvFilter);

	// test filters
	LOG_EXPR_EXT(m_mInvFilter * m_vFilterOut);

	CalcPlanesAndCentroid(); */

	END_LOG_SECTION();
}

REAL CInvFilterEM::operator()(const CVectorN<>& vParam, 
	CVectorN<> *pGrad) const
{
	REAL sum = 0.0;

	BEGIN_LOG_SECTION(CInvFilterEM::operator());

	CVectorN<> vSubOutput;
	ParamToSubOutput(vParam, vSubOutput);

	m_vFilterIn = GetFilterInput(vSubOutput);
	LOG_EXPR_EXT_DESC(m_vFilterIn, "vFilterIn at Parameter");

	// normalize filter in
	double filterInSum = 0.0;
	for (int nAt = 0; nAt < m_vFilterIn.GetDim(); nAt++)
	{
		filterInSum += m_vFilterIn[nAt];
	}
	m_vFilterIn *= 1.0 / filterInSum;

	// now compute entropy of prob. vFilterIn
	for (nAt = 0; nAt < m_vFilterIn.GetDim(); nAt++)
	{
		if (m_vFilterIn[nAt] > 0.0)
		{
#ifdef USE_LSQ
			sum += -m_vFilterIn[nAt] * m_vFilterIn[nAt];
#else
			sum += m_vFilterIn[nAt] * log(m_vFilterIn[nAt]);
#endif
		}
		else LOG("**** m_vFilterInput[%i] = %lf", nAt, m_vFilterIn[nAt]);
	}
	LOG_EXPR_DESC(sum, "Entropy");

	END_LOG_SECTION();

	return sum;

}	// CInvFilterEM::operator()

const CVectorN<>& CInvFilterEM::GetFilterInput(const CVectorN<>& vSubOutput) const
{
	for (int nAt = 0; nAt < vSubOutput.GetDim(); nAt ++)
	{
		m_vFilterOut[nAt*2] = vSubOutput[nAt];
	}
	m_vFilterIn = m_mInvFilter * m_vFilterOut;
	return m_vFilterIn;
}

void CInvFilterEM::ParamToSubOutput(const CVectorN<>& vParam, CVectorN<>& vSubOutput) const
{
	BEGIN_LOG_SECTION(CInvFilterEM::ParamToSubOutput);

	// create the direction
	vSubOutput.SetDim(vParam.GetDim());
	vSubOutput.SetZero();
	vSubOutput[0] = 1.0;

	// form the rotation matrix
	for (int nAt = 1; nAt < vParam.GetDim(); nAt++)
	{
		CMatrixNxM<> mRotate(vParam.GetDim(), vParam.GetDim());
		mRotate[0][nAt] = sin(vParam[nAt] / 5.0);
		mRotate[nAt][0] = -mRotate[0][nAt];
		mRotate[0][0] = mRotate[nAt][nAt] = cos(vParam[nAt] / 5.0);
		vSubOutput = mRotate * vSubOutput;
	}
	LOG_EXPR_EXT_DESC(vSubOutput, "vDir");

	// now determine the shift
	REAL offset = Sigmoid(vParam[0] / 5.0);
	LOG("Sigmoid(%lf) =  %lf", vParam[0] / 5.0, Sigmoid(vParam[0] / 5.0));

	if (offset > 0.5)
	{
		offset = 2.0 * (offset - 0.5);
	}
	else
	{
		vSubOutput *= -1.0;
		offset = 2.0 * (0.5 - offset);
	}
	LOG_EXPR_DESC(offset, "Adjusted offset");

	REAL min_lambda = 10000.0;
	BEGIN_LOG_SECTION(Clip to Planes);
	for (int nAtPlane = 0; nAtPlane < m_arrPlanePoints.size(); nAtPlane++)
	{
		BEGIN_LOG_SECTION(Intersection per Plane);

		// find intersection from centroid, along direction
		REAL lambda = 0.0;
		if (IntersectRayPlane(m_vCentroid, vSubOutput, 
			m_arrPlanePoints[nAtPlane], m_arrPlaneNormals[nAtPlane], &lambda))
		{
			LOG_EXPR(lambda);

			if (lambda >= 0.0)
			{
				min_lambda = _cpp_min(lambda, min_lambda);
			}

			BEGIN_LOG_ONLY(Test intersection);

			CVectorN<> vPoint = m_vCentroid;
			vPoint += lambda * vSubOutput;

			LOG_EXPR_EXT_DESC(GetFilterInput(vPoint),
				"vFilterIn at Intersection");
			LOG("Element %i should be zero", nAtPlane);

			END_LOG_SECTION();	// Test intersection
		}

		END_LOG_SECTION(); // Intersection per Plane
	}
	
	END_LOG_SECTION();	// Clip to Planes

	vSubOutput *= (offset * min_lambda);
	vSubOutput += m_vCentroid;
	LOG_EXPR_EXT(vSubOutput);

	END_LOG_SECTION();	// CInvFilterEM::ParamToSubOutput
}

void CInvFilterEM::CalcPlanesAndCentroid()
{
	BEGIN_LOG_SECTION(CInvFilterEM::CalcPlanesAndCentroid);

	// mDecInvFilter = decimated inverse filter
	CMatrixNxM<> mDecInvFilter;
	mDecInvFilter.Reshape(m_mInvFilter.GetCols() / 2 + 1, m_mInvFilter.GetRows());
	for (int nAtCol = 0; nAtCol < mDecInvFilter.GetCols(); nAtCol++)
	{
		mDecInvFilter[nAtCol] = m_mInvFilter[nAtCol*2];
	}
	LOG_EXPR_EXT(mDecInvFilter);

	// vHG contains non-homogeneous terms for system
	CVectorN<> vZeroSubOutput(mDecInvFilter.GetCols());
	vZeroSubOutput.SetZero();	
	CVectorN<> vHG = GetFilterInput(vZeroSubOutput); 
	LOG_EXPR_EXT_DESC(vHG, "vHG = m_mInvFilter * m_vFilterOut(zeroes)");

	// calculate the planes
	CalcPlanes(mDecInvFilter, vHG);

	// calculate the vertices
	// CalcVerts(mDecInvFilter, vHG);
		
	// calculate the centroid
	CalcCentroid(mDecInvFilter, vHG);

	END_LOG_SECTION();	// CInvFilterEM::CalcPlanesAndCentroid

}	// CInvFilterEM::CalcPlanesAndCentroid


void CInvFilterEM::CalcPlanes(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG)
{
	// store planes
	BEGIN_LOG_SECTION(CInvFilterEM::CalcPlanes);

	// stores the plane point
	CVectorN<> vPlanePoint(mDecInvFilter.GetCols());

	// stores the current row
	CVectorN<> vRow(mDecInvFilter.GetCols());
	for (int nAtRow = 0; nAtRow < mDecInvFilter.GetRows(); nAtRow++)
	{
		BEGIN_LOG_SECTION_(FMT("Zero plane for input element %i", nAtRow));

		// fetch the next row
		mDecInvFilter.GetRow(nAtRow, vRow);

		// get the row's length
		REAL len = vRow.GetLength();

		// adjust to form unit normal
		CVectorN<> &vPlaneNormal = vRow;
		vPlaneNormal *= -1.0 / len;
		m_arrPlaneNormals.push_back(vRow);

		// form the point in the plane
		vPlanePoint = vPlaneNormal;
		vPlanePoint *= vHG[nAtRow] / len;
		m_arrPlanePoints.push_back(vPlanePoint);

		LOG_EXPR_EXT(vPlanePoint);
		LOG_EXPR_EXT(vPlaneNormal);

		BEGIN_LOG_ONLY(Test Plane);

		LOG_EXPR_EXT_DESC(GetFilterInput(vPlanePoint),
			"vFilterIn at Plane Point");

		// initialize a random point 
		CVectorN<> vOtherPoint(vPlanePoint.GetDim());
		RandomVector<REAL>(0.5, vOtherPoint);

		// project onto the plane
		vOtherPoint -= (vOtherPoint * vPlaneNormal) 
			* vPlaneNormal;

		// within some distance of the plane
		vOtherPoint += vPlanePoint;

		LOG_EXPR_EXT_DESC(GetFilterInput(vOtherPoint), 
			"vFilterIn at Other Point");

		LOG("(Element at %i should be zero)", nAtRow);

		END_LOG_SECTION();	// Test Plane

		END_LOG_SECTION();	// Iteration for plane
	}
	
	END_LOG_SECTION();	// CInvFilterEM::CalcPlanes

}	// CInvFilterEM::CalcPlanes


// now identify potential simultaneous zeroes
void CInvFilterEM::CalcVerts(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG)
{
	BEGIN_LOG_SECTION(CInvFilterEM::CalcVerts);

	// clear previous vertices
	m_arrVerts.clear();

	// stores the current permutation matrix
	CMatrixNxM<> mPerm(mDecInvFilter.GetCols(), mDecInvFilter.GetCols());

	// stores a row of the decimated inv filter matrix
	CVectorN<> vRow(mDecInvFilter.GetCols());

	// stores the current permutation indices
	int nRowIndices[] = {0, 1, 2, 3, 4, 5, 6, 7};

	// iterate over permutations
	do
	{
		BEGIN_LOG_SECTION(Per Permutation);
		LOG("RowInd [%i, %i, %i]", nRowIndices[0], nRowIndices[1], nRowIndices[2]);

		// extract the next permutation of rows of the dec inv filter matrix
		for (int nAt = 0; nAt < mDecInvFilter.GetCols(); nAt++)
		{
			mDecInvFilter.GetRow(nRowIndices[nAt], vRow);
			mPerm.SetRow(nAt, vRow);
		}
		LOG_EXPR_EXT_DESC(mPerm, "mPerm (one permutation of mDecInvFilter)");

		// check for singularity
		if (fabs(mPerm.Determinant()) > 1e-4)
		{
			// OK, so invert
			CMatrixNxM<>& mPermInv = mPerm;
			mPermInv.Invert();

			// now solve by forming the vector of non-HG terms,
			CVectorN<> vPermOut(mPermInv.GetCols());
			for (int nAt = 0; nAt < vPermOut.GetDim(); nAt++)
			{
				vPermOut[nAt] = vHG[nRowIndices[nAt]];
			}
			// and multiplying by inverse of permuation matrix
			vPermOut = mPermInv * vPermOut;
			vPermOut *= -1.0;
			LOG_EXPR_EXT(vPermOut);

			// now reconstruct the filter input, to check for <-> elements
			CVectorN<> vFilterInRecons = GetFilterInput(vPermOut);
			LOG_EXPR_EXT_DESC(vFilterInRecons,
				"vFilterIn at permutation solution");
			LOG_EXPR_EXT_DESC(m_vFilterOut, 
				"m_vFilterOut at permutation solution");

			// iterate over, checking for <->s
			BOOL bNeg = FALSE;
			for (nAt = 0; nAt < vFilterInRecons.GetDim(); nAt++)
			{
				// if any element is <->
				if (vFilterInRecons[nAt] < -1e-4)
				{
					// then reject
					bNeg = TRUE;
				}
			}

			// no <->s?
			if (!bNeg)
			{
				// so accept it in to the list of solutions
				m_arrVerts.push_back(vPermOut);
				LOG("Permutation accepted");
			}
			else LOG("Permutation rejected due to negative components");
		}
		else LOG("Permutation rejected due to singular matrix");

		END_LOG_SECTION();	// Per permutation

	} while (NextPerm(nRowIndices, mDecInvFilter.GetCols(), 
		mDecInvFilter.GetRows()-1));

	END_LOG_SECTION();	// CInvFilterEM::CalcVerts

}	// CInvFilterEM::CalcVerts


double pinv15x8[8][15] =
{
	{ 0.32843,	-0.093146,	-0.14214,	0.058875,	0.024387,	-0.010101,	-0.0041841,	0.0017331,	0.00071788,	-0.00029735,	-0.00012319,	5.10E-05,	2.12E-05,	-8.50E-06,	-4.25E-06 },
	{-0.14214,	-0.034271,	0.21068,	-0.044373,	-0.12193,	0.050506,	0.02092,	-0.0086655,	-0.0035894,	0.0014867,	0.00061593,	-0.00025487,	-0.00010619,	4.25E-05,	2.12E-05 },
	{ 0.024387,	0.048773,	-0.12193,	-0.042639,	0.20721,	-0.042937,	-0.12134,	0.05026,	0.020818,	-0.008623,	-0.0035724,	0.0014782,	0.00061593,	-0.00024637,	-0.00012319 },
	{-0.0041841,	-0.0083682,	0.02092,	0.050209,	-0.12134,	-0.042886,	0.20711,	-0.042894,	-0.12132,	0.050251,	0.020818,	-0.0086145,	-0.0035894,	0.0014358,	0.00071788 },
	{ 0.00071788,	0.0014358,	-0.0035894,	-0.0086145,	0.020818,	0.050251,	-0.12132,	-0.042894,	0.20711,	-0.042886,	-0.12134,	0.050209,	0.02092,	-0.0083682,	-0.0041841 },
	{-0.00012319,	-0.00024637,	0.00061593,	0.0014782,	-0.0035724,	-0.008623,	0.020818,	0.05026,	-0.12134,	-0.042937,	0.20721,	-0.042639,	-0.12193,	0.048773,	0.024387 },
	{ 2.12E-05,	4.25E-05,	-0.00010619,	-0.00025487,	0.00061593,	0.0014867,	-0.0035894,	-0.0086655,	0.02092,	0.050506,	-0.12193,	-0.044373,	0.21068,	-0.034271,	-0.14214 },
	{-4.25E-06,	-8.50E-06,	2.12E-05,	5.10E-05,	-0.00012319,	-0.00029735,	0.00071788,	0.0017331,	-0.0041841,	-0.010101,	0.024387,	0.058875,	-0.14214,	-0.093146,	0.32843 }
};

void CInvFilterEM::CalcCentroid(const CMatrixNxM<>& mDecInvFilter, const CVectorN<>& vHG)
{
	BEGIN_LOG_SECTION(CInvFilterEM::CalcCentroid);

	// form origin (intersection of all odd-FilterIn zero planes
	for (int nAt = 0; nAt < mDecInvFilter.GetCols(); nAt ++)
	{
		m_vFilterOut[nAt*2] = 0;
	}
	CVectorN<> vOriginFull = m_mFilter * m_vFilterOut;
	vOriginFull *= 2.0;
	LOG_EXPR_EXT(vOriginFull); 

	CVectorN<> vOrigin(mDecInvFilter.GetCols());
	for (nAt = 0; nAt < vOrigin.GetDim(); nAt ++)
	{
		vOrigin[nAt] = vOriginFull[nAt*2];
	}
	LOG_EXPR_EXT(vOrigin); 
	LOG_EXPR_EXT(GetFilterInput(vOrigin));

	// set centroid to average of origin + other points
	m_vCentroid = vOrigin;

	// now for each even-FilterIn sample, move towards its zero plane until 
	//		intersection with another zero-plane
	CVectorN<> vDir(vOrigin.GetDim());
	for (nAt = 0; nAt < m_mInvFilter.GetCols() / 2; nAt++)
	{
		BEGIN_LOG_SECTION_(FMT("Intersecting zero plane for input element %i", nAt*2 + 1));

		// form direction
		vDir.SetZero();
		for (int nAtD = 0; nAtD < vDir.GetDim(); nAtD++)
		{
			vDir[nAtD] = -m_mInvFilter[nAt*2+1][nAtD*2];
		}
		LOG_EXPR_EXT(vDir);

		CVectorN<> vIntersect;
		REAL lambda = 0.0;
		if (IntersectRayPlane(vOrigin, vDir, m_arrPlanePoints[nAt*2 + 1], m_arrPlaneNormals[nAt*2 + 1], &lambda))
		{
			if (lambda < 0.0)
				lambda = 0.0;
			// ASSERT(lambda >= 0);
			LOG_EXPR(lambda);

			// form intersection point
			vIntersect = vOrigin;
			vIntersect += lambda * vDir;
			LOG_EXPR_EXT_DESC(vIntersect, "vIntersect Principal");
			CVectorN<> vIntersectIn = GetFilterInput(vIntersect);
			LOG_EXPR_EXT(vIntersectIn); 

			// see if any negative elements
			for (int nAtElem = 0; nAtElem < vIntersectIn.GetDim(); nAtElem++)
			{
				REAL lambda2;
				if (vIntersectIn[nAtElem] < -1e-6)
				{
					ASSERT((nAtElem % 2) == 1);
					BOOL bRes = IntersectRayPlane(vOrigin, vDir, m_arrPlanePoints[nAtElem], m_arrPlaneNormals[nAtElem], 
						&lambda2);
					ASSERT(bRes);
					if (lambda2 < 0.0)
						lambda2 = 0.0;

					// see if the new intersection is closer
					if (lambda2 < lambda)
					{
						lambda = lambda2;
						LOG_EXPR_DESC(lambda, "New lambda from clipping");

					}	// if
				}	// if
			}	// for
			
			vIntersect = vOrigin;
			vIntersect += lambda * vDir;

			// now log intersection
			LOG_EXPR_EXT_DESC(vIntersect, "vIntersect Clipped");
			LOG_EXPR_EXT_DESC(GetFilterInput(vIntersect), "vIntersectIn Clipped");
			FLUSH_LOG();

			m_vCentroid += vIntersect;

		}	// if
		else LOG("Intersection failed for direction %i", nAt);

		END_LOG_SECTION(); // Intersecting zero plane for input element

	}	// for
/*
	CMatrixNxM<> mDecInvFilter_pinv(mDecInvFilter);
	if (mDecInvFilter.GetRows() == 15)
	{
		mDecInvFilter_pinv.Reshape(mDecInvFilter.GetRows(), mDecInvFilter.GetCols());
		for (int nAtRow = 0; nAtRow < 8; nAtRow++)
			for (int nAtCol = 0; nAtCol < 15; nAtCol++)
			{
				mDecInvFilter_pinv[nAtCol][nAtRow] = pinv15x8[nAtRow][nAtCol];
			}
		LOG_EXPR_EXT(mDecInvFilter_pinv);
	}
	else 
	{
		mDecInvFilter_pinv.Pseudoinvert();
	}
*/
	{
/*		m_vCentroid = mDecInvFilter_pinv * vHG;
		m_vCentroid *= -1.0;
*/
/*	m_vCentroid.SetDim(m_arrVerts[0].GetDim());
	m_vCentroid.SetZero();
	for (int nAt = 0; nAt < m_arrVerts.size(); nAt++)
	{
		m_vCentroid += m_arrVerts[nAt];
	}
	m_vCentroid *= 1.0 / (REAL) m_arrVerts.size(); */

		m_vCentroid *= 1.0 / (REAL)(m_mInvFilter.GetCols() / 2 + 1);
		LOG_EXPR_EXT(m_vCentroid);
		FLUSH_LOG();

		BEGIN_LOG_ONLY(Testing Centroid);

		LOG_EXPR_EXT_DESC(GetFilterInput(m_vCentroid), "Recons Input at Centroid");
		LOG_EXPR_EXT_DESC(m_vFilterOut, "Output at Centroid");

		END_LOG_SECTION();	// Testing Centroid
		FLUSH_LOG();
	}
	// else LOG("Pseudoinverse Failed");

	END_LOG_SECTION();	// CInvFilterEM::CalcCentroid

}	// CInvFilterEM::CalcCentroid

int CInvFilterEM::GetSubOutputDim()
{
	return m_vCentroid.GetDim();
}
