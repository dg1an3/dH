// TestMTL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <stdlib.h>
#include <assert.h>

#include <iostream>
using namespace std;


#include <VectorD.h>
#include <VectorN.h>

#include <MatrixD.h>
#include <MatrixNxM.h>


template<class TYPE>
TYPE GenRand(TYPE range)
{
	return range - 2.0 * range * (REAL) rand() 
			/ (REAL) RAND_MAX;
}

#ifdef USE_COMPLEX
template<>
complex<double> GenRand(complex<double> range)
{
	return complex<double>(
		range.real() - 2.0 * range.real() * (REAL) rand() 
			/ (REAL) RAND_MAX,
		range.imag() - 2.0 * range.imag() * (REAL) rand() 
			/ (REAL) RAND_MAX );
}
#endif

#define TRACE_STMT(x) \
	TRACE("%4i: %s;\n", __LINE__, #x);	\
	x;


//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS, int DIM>
// TestVector(REAL scale)
//
// tests the specified vector class, creating random vectors with
//		elements scaled by scale
//////////////////////////////////////////////////////////////////////
template<class VECTOR_TYPE>
void TestVector(VECTOR_TYPE v1, VECTOR_TYPE::ELEM_TYPE range)
{
	// element accessors
	for (int nAt = 0; nAt < v1.GetDim(); nAt++)
	{
		v1[nAt] = GenRand<VECTOR_TYPE::ELEM_TYPE>(range);
	}
	TRACE_VECTOR("v1 = ", v1);

	// copy construction
	//	-> ensure dimensions are replicated
	TRACE_STMT(VECTOR_TYPE v2(v1));
	TRACE_VECTOR("v2 = ", v2);

	// assignment
	//	-> ensure dimensions are replicated
	TRACE_STMT(VECTOR_TYPE v3 = v2);
	TRACE_VECTOR("v3 = ", v3);

	// length
	TRACE("v3 length = %lf\n", v3.GetLength());

	// normalization
	v3.Normalize();
	TRACE_VECTOR("v3 Normalized = ", v3);

	// comparison (== , !=, IsApproxEqual)
	TRACE_STMT(ASSERT(v2 == v1));
	TRACE_STMT(ASSERT(v3 != v1));
	TRACE_STMT(ASSERT(v1.IsApproxEqual(v2)));
	TRACE_STMT(ASSERT(!v1.IsApproxEqual(v3)));

	// in-place arithmetic
	TRACE_STMT(v1 += v2);
	TRACE_VECTOR("v1 = ", v1);

	TRACE_STMT(v1 -= v2);
	TRACE_VECTOR("v1 = ", v1);

	TRACE_STMT(v1 *= 2.0);
	TRACE_VECTOR("v1 = ", v1);

	// dyadic arithmetic
	TRACE_VECTOR("v1 + v2 = ", v1 + v2);

	TRACE_VECTOR("v1 - v3 = ", v1 - v3);

	TRACE("v1 * v3 = %lf\n", v1 * v3);
}


//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS, int DIM>
// TestMatrixClass(REAL scale)
//
// tests the specified matrix class, creating random matrices with
//		elements scaled by scale
//////////////////////////////////////////////////////////////////////
template<class MATRIX_TYPE>
void TestMatrix(MATRIX_TYPE& m1, MATRIX_TYPE::ELEM_TYPE scale)
{
	// element accessors
	for (int nAtCol = 0; nAtCol < m1.GetCols(); nAtCol++)
	{
		for (int nAtRow = 0; nAtRow < m1.GetRows(); nAtRow++)
		{
			m1[nAtCol][nAtRow] = scale - 2.0 * scale * (REAL) rand() 
				/ (REAL) RAND_MAX;
		}
	}
	TRACE_MATRIX("m1", m1);

	// copy construction
	//	-> ensure dimensions are replicated
	TRACE_STMT(MATRIX_TYPE m2(m1));
	TRACE_MATRIX("m2", m2);

	// assignment
	//	-> ensure dimensions are replicated
	TRACE_STMT(MATRIX_TYPE m3 = m2);
	TRACE_MATRIX("m3", m3);

	// determinant
	double det = Determinant(m3);
	TRACE("m3 determinant = %lf\n", det);

	// orthogonalization
	TRACE_STMT(Orthogonalize(m3));
	TRACE_MATRIX("m3 = ", m3);
	TRACE_STMT(ASSERT(IsOrthogonal(m3)));

	// comparison (== , !=, IsApproxEqual)
	TRACE_STMT(ASSERT(m2 == m1));
	TRACE_STMT(ASSERT(m3 != m1));
	TRACE_STMT(ASSERT(m1.IsApproxEqual(m2)));
	TRACE_STMT(ASSERT(!m1.IsApproxEqual(m3)));

	// in-place arithmetic
	TRACE_STMT(m1 += m2);
	TRACE_MATRIX("m1", m1);

	TRACE_STMT(m1 -= m2);
	TRACE_MATRIX("m1", m1);

	TRACE_STMT(m1 *= 2.0);
	TRACE_MATRIX("m1", m1);

	// dyadic arithmetic
	TRACE_MATRIX("m1", m1);
	TRACE_MATRIX("m2", m2);
	TRACE_MATRIX("m3", m3);
	TRACE_MATRIX("m1 + m2 = ", m1 + m2);
	TRACE_MATRIX("m1 - m3 = ", m1 - m3);
	TRACE_MATRIX("m1 * m3 = ", m1 * m3);

	// transpose
	m1.Transpose();
	TRACE_MATRIX("m1.Transpose()", m1);
	m1.Transpose();

	if (TRUE) // det > DEFAULT_EPSILON)
	{
		// inverse
		m2 = m1;
		if (m1.GetCols() > 1)
		{
			BOOL bRes = m1.Invert();
			TRACE_MATRIX("m1.Invert()", m1);

			if (bRes)
			{
				// create an identity matrix for comparison
				MATRIX_TYPE mI = m1;
				mI.SetIdentity();

				// assert approximate equality
				TRACE_STMT(ASSERT(mI.IsApproxEqual(m1 * m2)));
			}
		}
	}
	else
	{
		TRACE("Determinant too small to invert!\n");
		cout << "Determinant too small to invert! : " << m1.GetRows() << "\n";
	}
}

/*
//////////////////////////////////////////////////////////////////////
// template<class TYPE>
// TestSVD(REAL scale)
//
// tests the singular-valued decomposition
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void TestSVD(int nCols, int nRows, BOOL bHomogeneous)
{
	// generate a matrix for SVD test
	CMatrixNxM<TYPE> m(nCols, nRows);
	for (int nAtRow = 0; nAtRow < nRows; nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < nCols; nAtCol++)
		{
			m[nAtCol][nAtRow] = 
				10.0 - 20.0 * (double) rand() / (double) RAND_MAX;

			if (bHomogeneous && nCols > nRows)
				m[nAtCol][nRows-1] = 1.0;
		}
		if (bHomogeneous && nCols <= nRows)
			m[nCols-1][nAtRow] = 1.0;
	}

	// store the original
	CMatrixNxM<TYPE> mOrig = m;

	// trace out the original
	TRACE_MATRIX("M_orig", m);

	// create the singular value vector
	CVectorN<TYPE> w(nCols);

	// create the orthogonal matrix
	CMatrixNxM<TYPE> v(nCols,nCols);

	// perform SVD
	m.SVD(w, v);

	// output the U matrix (stored in m)
	TRACE_MATRIX("U", m);

	// check orthogonality of U
	m.Transpose();
	CMatrixNxM<TYPE> uIdent = m;
	m.Transpose();
	uIdent *= m;
	TRACE_MATRIX("U * U^T", uIdent);

	CMatrixNxM<TYPE> mI(nCols,nCols);
	mI.SetIdentity();

	if (nCols < nRows)
	{
		TRACE("ASSERT(mI.IsApproxEqual(U * U^T));\n");
		ASSERT(mI.IsApproxEqual(uIdent));
	}

	// form the S matrix
	CMatrixNxM<TYPE> s(w.GetDim(), w.GetDim());
	for (int nAt = 0; nAt < w.GetDim(); nAt++)
	{
		s[nAt][nAt] = w[nAt];
	}
	TRACE_MATRIX("S", s);

	// output V matrix
	TRACE("\n");
	TRACE_MATRIX("V", v);

	// check orthogonality of V
	CMatrixNxM<TYPE> vIdent = v;
	v.Transpose();
	vIdent *= v;
	TRACE_MATRIX("V * V^T", vIdent);

	mI.Reshape(nCols,nCols);
	mI.SetIdentity();
	TRACE("ASSERT(mI.IsApproxEqual(V * V^T));\n");
	ASSERT(mI.IsApproxEqual(vIdent));

	// reform M
	CMatrixNxM<TYPE> mNew = m * s * v;
	TRACE_MATRIX("M_new", mNew);

	// assert equality with original
	TRACE("ASSERT(mNew.IsApproxEqual(mOrig));\n");
	ASSERT(mNew.IsApproxEqual(mOrig));

	if (nRows >= nCols)
	{
		// test pseudo-inverse
		CMatrixNxM<> mPsinv = mOrig;
		mPsinv.Pseudoinvert();
		ASSERT(mOrig.IsApproxEqual(mOrig * mPsinv * mOrig));
		ASSERT(mPsinv.IsApproxEqual(mPsinv * mOrig * mPsinv));
	}
}
*/

//////////////////////////////////////////////////////////////////////
// main
//
// runs tests on
//		static vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		static vectors of dimension 1..9, 
//			with elements scaled to 100.0
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 100.0
//
//		static matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		static matrices of dimension 1..9, 
//			with elements scaled to 100.0
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 100.0
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	//	static vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVector(CVectorD<1, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<2, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<3, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<4, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<5, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<6, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<7, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<8, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<9, REAL>(), (REAL) 0.01);
	TestVector(CVectorD<10, REAL>(), (REAL) 0.01);

	//	static vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVector(CVectorD<1, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<2, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<3, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<4, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<5, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<6, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<7, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<8, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<9, REAL>(), (REAL) 100.0);
	TestVector(CVectorD<10, REAL>(), (REAL) 100.0);

	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVector(CVectorN<REAL>(1), (REAL) 0.01);
	TestVector(CVectorN<REAL>(2), (REAL) 0.01);
	TestVector(CVectorN<REAL>(3), (REAL) 0.01);
	TestVector(CVectorN<REAL>(4), (REAL) 0.01);
	TestVector(CVectorN<REAL>(5), (REAL) 0.01);
	TestVector(CVectorN<REAL>(6), (REAL) 0.01);
	TestVector(CVectorN<REAL>(7), (REAL) 0.01);
	TestVector(CVectorN<REAL>(8), (REAL) 0.01);
	TestVector(CVectorN<REAL>(9), (REAL) 0.01);
	TestVector(CVectorN<REAL>(10), (REAL) 0.01);

	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVector(CVectorN<REAL>(1), (REAL) 100.0);
	TestVector(CVectorN<REAL>(2), (REAL) 100.0);
	TestVector(CVectorN<REAL>(3), (REAL) 100.0);
	TestVector(CVectorN<REAL>(4), (REAL) 100.0);
	TestVector(CVectorN<REAL>(5), (REAL) 100.0);
	TestVector(CVectorN<REAL>(6), (REAL) 100.0);
	TestVector(CVectorN<REAL>(7), (REAL) 100.0);
	TestVector(CVectorN<REAL>(8), (REAL) 100.0);
	TestVector(CVectorN<REAL>(9), (REAL) 100.0);
	TestVector(CVectorN<REAL>(10), (REAL) 100.0);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrix(CMatrixD<1, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<2, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<3, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<4, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<5, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<6, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<7, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<8, REAL>(), (REAL) 0.01);
	TestMatrix(CMatrixD<9, REAL>(), (REAL) 0.01);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 100.0
	TestMatrix(CMatrixD<1, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<2, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<3, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<4, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<5, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<6, REAL>(), (REAL) 10.0);
	TestMatrix(CMatrixD<7, REAL>(), (REAL) 10.0);
//	TestMatrix(CMatrixD<8, REAL>(), (REAL) 10.0);
//	TestMatrix(CMatrixD<9, REAL>(), (REAL) 10.0);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrix(CMatrixNxM<REAL>(1, 1), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(2, 2), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(3, 3), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(4, 4), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(5, 5), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(6, 6), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(7, 7), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(8, 8), (REAL) 0.01);
	TestMatrix(CMatrixNxM<REAL>(9, 9), (REAL) 0.01);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 100.0
	TestMatrix(CMatrixNxM<REAL>(1, 1), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(2, 2), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(3, 3), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(4, 4), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(5, 5), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(6, 6), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(7, 7), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(8, 8), (REAL) 10.0);
	TestMatrix(CMatrixNxM<REAL>(9, 9), (REAL) 10.0);

	/*
	// test the SVD
	for (int nRows = 2; nRows < 20; nRows++)
	{
		for (int nCols = 2; nCols < 20; nCols++)
		{
			TestSVD<REAL>(nCols, nRows, FALSE);
			TestSVD<REAL>(nCols, nRows, TRUE);
		}
	} */

	return 0;
}
