// TestVecMat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <stdlib.h>
#include <assert.h>

#include <iostream>
using namespace std;

#include "VectorN.h"
#include "Vector.h"
#include "MatrixN.h"
#include "Matrix.h"

//////////////////////////////////////////////////////////////////////
// template<class VECTOR_CLASS, int DIM>
// TestVectorClass(REAL scale)
//
// tests the specified vector class, creating random vectors with
//		elements scaled by scale
//////////////////////////////////////////////////////////////////////
template<class VECTOR_CLASS, int DIM>
void TestVectorClass(REAL scale)
{
	// construction
	VECTOR_CLASS v1;

	// set the dimension if it is > -1
	if (DIM > 0)
	{
		static_cast< CVectorN<> >(v1).SetDim(DIM);
	}

	// element accessors
	for (int nAt = 0; nAt < v1.GetDim(); nAt++)
	{
		v1[nAt] = scale - 2.0 * scale * (REAL) rand() 
			/ (REAL) RAND_MAX;
	}
	TRACE_VECTOR("v1 = ", v1);

	// copy construction
	//	-> ensure dimensions are replicated
	VECTOR_CLASS v2(v1);
	TRACE("VECTOR_CLASS v2(v1);\n");
	TRACE_VECTOR("v2 = ", v2);

	// assignment
	//	-> ensure dimensions are replicated
	VECTOR_CLASS v3 = v2;
	TRACE("VECTOR_CLASS v3 = v2;\n");
	TRACE_VECTOR("v3 = ", v3);

	// length
	TRACE("v3 length = %lf\n", v3.GetLength());

	// normalization
	v3.Normalize();
	TRACE_VECTOR("v3 Normalized = ", v3);

	// comparison (== , !=, IsApproxEqual)
	TRACE("ASSERT(v2 == v1);\n");
	ASSERT(v2 == v1);

	TRACE("ASSERT(v3 != v1);\n");
	ASSERT(v3 != v1);

	TRACE("ASSERT(v1.IsApproxEqual(v2));\n");
	ASSERT(v1.IsApproxEqual(v2));

	TRACE("ASSERT(!v1.IsApproxEqual(v3));\n");
	ASSERT(!v1.IsApproxEqual(v3));

	// in-place arithmetic
	v1 += v2;
	TRACE("v1 += v2;\n");
	TRACE_VECTOR("v1 = ", v1);

	v1 -= v2;
	TRACE("v1 -= v2;\n");
	TRACE_VECTOR("v1 = ", v1);

	v1 *= 2.0;
	TRACE("v1 *= 2.0;\n");
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
template<class MATRIX_CLASS, int DIM>
void TestMatrixClass(REAL scale)
{
	// construction
	MATRIX_CLASS m1;

	// set the dimension if it is > 0
	if (DIM > 0)
	{
		static_cast< CMatrixN<> >(m1).SetDim(DIM);
	}

	// element accessors
	for (int nAtRow = 0; nAtRow < m1.GetDim(); nAtRow++)
	{
		for (int nAtCol = 0; nAtCol < m1.GetDim(); nAtCol++)
		{
			m1[nAtRow][nAtCol] = scale - 2.0 * scale * (REAL) rand() 
				/ (REAL) RAND_MAX;
		}
	}
	TRACE_MATRIX("m1 = ", m1);

	// copy construction
	//	-> ensure dimensions are replicated
	MATRIX_CLASS m2(m1);
	TRACE("MATRIX_CLASS m2(m1);\n");
	TRACE_MATRIX("m2 = ", m2);

	// assignment
	//	-> ensure dimensions are replicated
	MATRIX_CLASS m3 = m2;
	TRACE("MATRIX_CLASS m3 = m2;\n");
	TRACE_MATRIX("m3 = ", m3);

	// determinant
	// TRACE("m3 determinant = %lf\n", m3.GetDeterminant());

	// orthogonalization
	m3.Orthogonalize();
	TRACE_MATRIX("m3 Orthogonalize = ", m3);

	// comparison (== , !=, IsApproxEqual)
	TRACE("ASSERT(m2 == m1);\n");
	ASSERT(m2 == m1);

	TRACE("ASSERT(m3 != m1);\n");
	ASSERT(m3 != m1);

	TRACE("ASSERT(m1.IsApproxEqual(m2));\n");
	ASSERT(m1.IsApproxEqual(m2));

	TRACE("ASSERT(!m1.IsApproxEqual(m3));\n");
	ASSERT(!m1.IsApproxEqual(m3));

	// in-place arithmetic
	m1 += m2;
	TRACE("m1 += m2;\n");
	TRACE_MATRIX("m1 = ", m1);

	m1 -= m2;
	TRACE("m1 -= m2;\n");
	TRACE_MATRIX("m1 = ", m1);

	m1 *= 2.0;
	TRACE("m1 *= 2.0;\n");
	TRACE_MATRIX("m1 = ", m1);

	// dyadic arithmetic
	TRACE_MATRIX("m1 + m2 = ", m1 + m2);
	TRACE_MATRIX("m1 - m3 = ", m1 - m3);
	TRACE_MATRIX("m1 * m3 = ", m1 * m3);

	// inverse
	m2 = m1;
	m1.Invert();

	// create an identity matrix for comparison
	MATRIX_CLASS mI;
	mI.SetIdentity();

	// assert approximate equality
	ASSERT(mI.IsApproxEqual(m1 * m2));
}

//////////////////////////////////////////////////////////////////////
// main
//
// runs tests on
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic vectors of dimension 1..9, 
//			with elements scaled to 100.0
//		static vectors of dimension 1..9, 
//			with elements scaled to 0.01
//		static vectors of dimension 1..9, 
//			with elements scaled to 100.0
//
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		dynamic matrices of dimension 1..9, 
//			with elements scaled to 100.0
//		static matrices of dimension 1..9, 
//			with elements scaled to 0.01
//		static matrices of dimension 1..9, 
//			with elements scaled to 100.0
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVectorClass<CVectorN<>, 1>(0.01);
	TestVectorClass<CVectorN<>, 2>(0.01);
	TestVectorClass<CVectorN<>, 3>(0.01);
	TestVectorClass<CVectorN<>, 4>(0.01);
	TestVectorClass<CVectorN<>, 5>(0.01);
	TestVectorClass<CVectorN<>, 6>(0.01);
	TestVectorClass<CVectorN<>, 7>(0.01);
	TestVectorClass<CVectorN<>, 8>(0.01);
	TestVectorClass<CVectorN<>, 9>(0.01);

	//	dynamic vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVectorClass<CVectorN<>, 1>(100.0);
	TestVectorClass<CVectorN<>, 2>(100.0);
	TestVectorClass<CVectorN<>, 3>(100.0);
	TestVectorClass<CVectorN<>, 4>(100.0);
	TestVectorClass<CVectorN<>, 5>(100.0);
	TestVectorClass<CVectorN<>, 6>(100.0);
	TestVectorClass<CVectorN<>, 7>(100.0);
	TestVectorClass<CVectorN<>, 8>(100.0);
	TestVectorClass<CVectorN<>, 9>(100.0);

	//	static vectors of dimension 1..9, 
	//		with elements scaled to 0.01
	TestVectorClass<CVector<1>, 0>(0.01);
	TestVectorClass<CVector<2>, 0>(0.01);
	TestVectorClass<CVector<3>, 0>(0.01);
	TestVectorClass<CVector<4>, 0>(0.01);
	TestVectorClass<CVector<5>, 0>(0.01);
	TestVectorClass<CVector<6>, 0>(0.01);
	TestVectorClass<CVector<7>, 0>(0.01);
	TestVectorClass<CVector<8>, 0>(0.01);
	TestVectorClass<CVector<9>, 0>(0.01);

	//	static vectors of dimension 1..9, 
	//		with elements scaled to 100.0
	TestVectorClass<CVector<1>, 0>(100.0);
	TestVectorClass<CVector<2>, 0>(100.0);
	TestVectorClass<CVector<3>, 0>(100.0);
	TestVectorClass<CVector<4>, 0>(100.0);
	TestVectorClass<CVector<5>, 0>(100.0);
	TestVectorClass<CVector<6>, 0>(100.0);
	TestVectorClass<CVector<7>, 0>(100.0);
	TestVectorClass<CVector<8>, 0>(100.0);
	TestVectorClass<CVector<9>, 0>(100.0);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrixClass<CMatrixN<>, 1>(0.01);
	TestMatrixClass<CMatrixN<>, 2>(0.01);
	TestMatrixClass<CMatrixN<>, 3>(0.01);
	TestMatrixClass<CMatrixN<>, 4>(0.01);
	TestMatrixClass<CMatrixN<>, 5>(0.01);
	TestMatrixClass<CMatrixN<>, 6>(0.01);
	TestMatrixClass<CMatrixN<>, 7>(0.01);
	TestMatrixClass<CMatrixN<>, 8>(0.01);
	TestMatrixClass<CMatrixN<>, 9>(0.01);

	//	dynamic matrices of dimension 1..9, 
	//		with elements scaled to 100.0
	TestMatrixClass<CMatrixN<>, 1>(100.0);
	TestMatrixClass<CMatrixN<>, 2>(100.0);
	TestMatrixClass<CMatrixN<>, 3>(100.0);
	TestMatrixClass<CMatrixN<>, 4>(100.0);
	TestMatrixClass<CMatrixN<>, 5>(100.0);
	TestMatrixClass<CMatrixN<>, 6>(100.0);
	TestMatrixClass<CMatrixN<>, 7>(100.0);
	TestMatrixClass<CMatrixN<>, 8>(100.0);
	TestMatrixClass<CMatrixN<>, 9>(100.0);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 0.01
	TestMatrixClass<CMatrix<1>, 0>(0.01);
	TestMatrixClass<CMatrix<2>, 0>(0.01);
	TestMatrixClass<CMatrix<3>, 0>(0.01);
	TestMatrixClass<CMatrix<4>, 0>(0.01);
	TestMatrixClass<CMatrix<5>, 0>(0.01);
	TestMatrixClass<CMatrix<6>, 0>(0.01);
	TestMatrixClass<CMatrix<7>, 0>(0.01);
	TestMatrixClass<CMatrix<8>, 0>(0.01);
	TestMatrixClass<CMatrix<9>, 0>(0.01);

	//	static matrices of dimension 1..9, 
	//		with elements scaled to 100.0
	TestMatrixClass<CMatrix<1>, 0>(100.0);
	TestMatrixClass<CMatrix<2>, 0>(100.0);
	TestMatrixClass<CMatrix<3>, 0>(100.0);
	TestMatrixClass<CMatrix<4>, 0>(100.0);
	TestMatrixClass<CMatrix<5>, 0>(100.0);
	TestMatrixClass<CMatrix<6>, 0>(100.0);
	TestMatrixClass<CMatrix<7>, 0>(100.0);
	TestMatrixClass<CMatrix<8>, 0>(100.0);
	TestMatrixClass<CMatrix<9>, 0>(100.0);

	return 0;
}
