// VSIM_MODEL_TEST.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "Value.h"
//#include "Function.h"
#include "MatrixFunction.h"

int main(int argc, char* argv[])
{
	CValue< double > a = 2.0;
	CValue< double > b = 2.0;

	const CValue< double >& sum = a + b;

	printf("a = %lf\n", a.Get());
	printf("b = %lf\n", b.Get());
	printf("sum = %lf\n", sum.Get());

	a.Set(3.0);

	printf("a = %lf\n", a.Get());
	printf("b = %lf\n", b.Get());
	printf("sum = %lf\n", sum.Get());

	return 0;
}
