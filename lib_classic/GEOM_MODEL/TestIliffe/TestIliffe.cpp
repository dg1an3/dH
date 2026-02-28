// TestIliffe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <CastVectorD.h>

#include <ModelObject.h>

#include "BufferND.h"
#include "MeshND.h"
#include "Field.h"


int main(int argc, char* argv[])
{
	CBufferND<2, CVectorD<3, float> > testField(
		CBufferBounds<2>(CVectorD<2, int>(0, 0), 
			CVectorD<2, int>(31, 31)));
	testField.ClearSamples();

	CBufferIndex<2> vNdx(testField.GetExtent());
	while (vNdx.IsInRange())
	{
		testField[vNdx] = CVectorD<3>(1.0, 0.0, 0.0);
		vNdx++;
	}

	// initial point
	testField[15][15][1] = 0.75;
	testField[15][16][1] = 0.5;
	testField[15][17][1] = 0.0;

	testField[16][15][1] = 0.5;
	testField[16][16][1] = 1.0;
	testField[16][17][1] = 0.5;

	testField[17][15][1] = 0.0;
	testField[17][16][1] = 0.5;
	testField[17][17][1] = 0.25;

	CBufferND<2, CVectorD<3, float> > kernel_Lap(
		CBufferBounds<2>(CVectorD<2, int>(-3, -3), 
			CVectorD<2, int>(3, 3)));
	MakeLapKernel(kernel_Lap, 0.5);

	FILE *file = fopen("test_field.txt", "wt");

	fprintf(file, "kernel = \n", 0);
	WriteField(file, kernel_Lap);

	fprintf(file, "time = %lf\n", 0);
	WriteField(file, testField);

	REAL k1 = 0.05; // rate of B -> P
	REAL k2 = 0.03; // rate of P -> B
	REAL k3 = 0.02; // rate of B -> D
	REAL k4 = 0.01; // rate of D -> B
	
	REAL time = 0.0;
	REAL time_step = (REAL) 0.5;
	CVectorD<3> vDiff_coef(0.0, 2e-5, 1e-5); // 1.55);
	while (time < (REAL) 100.0)
	{
		// filter with lap filter
		CBufferND<2, CVectorD<3, float> > testField_Lap;
		testField_Lap.ConformTo(testField);
		testField_Lap.Convolve(testField, kernel_Lap);

		// update based on diffusion coefficient
		testField.Accumulate(testField_Lap, time_step * vDiff_coef);

		CBufferIndex<2> vNdx(testField.GetExtent());
		while (vNdx.IsInRange())
		{
			printf("%lf %lf %lf\n", testField[vNdx][0], testField[vNdx][1], testField[vNdx][2]);

			testField[vNdx][0] += time_step 
				* (-k1 * testField[vNdx][0] * testField[vNdx][1]
					+ k4 * testField[vNdx][2]);

			testField[vNdx][1] += time_step 
				* (k1 * testField[vNdx][0] * testField[vNdx][1]
					- k2 * testField[vNdx][1] * testField[vNdx][2]);

			testField[vNdx][2] += time_step 
				* (k3 * testField[vNdx][0] * testField[vNdx][1]
					- k4 * testField[vNdx][2]);

			vNdx++;
		}

		// next time step
		time += time_step;

		fprintf(file, "time = %lf\n", time);
		WriteField(file, testField);
	}

	{
		CBufferND<2, float> testScalar;
		CBufferND<2, float> kernel;
		CBufferND<2, float> testConv;
		// testConv.Convolve(testScalar, kernel);
	}

	fclose(file);

	return 0;
}

