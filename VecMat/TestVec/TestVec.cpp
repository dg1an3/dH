// TestVec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <stdlib.h>
#include <assert.h>

#include <iostream>
using namespace std;

#include "VectorN.h"
#include "Vector.h"
#include "MatrixN.h"

int main(int argc, char* argv[])
{
	CVectorN<> vNa(4);
	vNa[0] = 15.0;
	vNa[1] = -7.0;
	vNa[2] = -12.0;
	vNa[3] = 4.0;

	CVector<4> v4a(vNa);

	CVector<4> v4b;
	v4b[0] = -21.3;
	v4b[1] = -1.3;
	v4b[2] = -3.3;
	v4b[3] = -6.3;

	v4a + v4b;

	// CVector<4> v4c = v4a + vNa;		// error
	CVector<4> v4d = CVector<4>(v4a + vNa);

	// v4d += vNa;						// error
	v4d += CVector<4>(vNa);
	vNa += v4d;

	cout << v4a << " + " << v4b << " = " << v4d << endl;

	vNa + CVectorN<>(v4a);
	CVector<4> v4e = CVector<4>(vNa + v4a);

	CVector<4> v4f = CVector<4>(vNa + v4a);

	CVector<2> v2a;
	// v2a += v4e;						// error
	v2a += CVector<2>(v4e);

	CMatrixN<> mNa;

	return 0;
}
