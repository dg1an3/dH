// OPTIMIZER_BASE_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <PowellOptimizer.h>

#include <ConjGradOptimizer.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<int DIM, class TYPE>
class CGradParabola : public CGradObjectiveFunction<DIM, TYPE>
{
public:
	CGradParabola(const CVector<DIM, TYPE>& initOffset)
		: vOffset(initOffset)
	{
	}

	CValue< CVector<DIM, TYPE> > vOffset;

	virtual TYPE operator()(const CVector<DIM, TYPE>& vInput)
	{
		TYPE length = (vInput - vOffset.Get()).GetLength();
		return length * length;
	}

	virtual CVector<DIM, TYPE> Grad(const CVector<DIM, TYPE>& vInput)
	{
		CVector<DIM, TYPE> vGrad;
		for (int nAt = 0; nAt < DIM; nAt++)
			vGrad[nAt] = 2.0 * vInput[nAt] - 2.0 * vOffset.Get()[nAt];

		return vGrad;
	}
};

int main(int argc, char* argv[])
{
	CVector<4> vInit;
	CVector<4> vFinal; 
//	CGradParabola<4, double> parabola4d(CVector<4>(0.0, 0.0, 0.0, 0.0)); // -1.0, -3.0, -7.0, -11.0));
//	CPowellOptimizer<4, double> po(&parabola4d);
//
//	vInit[0] = 10.0;
//	vInit[1] = 20.0;
//	vInit[2] = -3.6;
//	vInit[3] = 11.0;
//
//	vFinal = po.Optimize(vInit);
//
//	printf("Final value from Powell = <%lf, %lf, %lf, %lf>\n", 
//		vFinal[0], vFinal[1], vFinal[2], vFinal[3]);
//

	CGradParabola<1, double> gradParabola1d(CVector<1>(11.0));
	printf("\nGradient at <0> = %lf\n", 
		gradParabola1d.Grad(CVector<1>(0.0)));
	CConjGradOptimizer<1, double> cgo1d(&gradParabola1d);

	CVector<1> vInit1d(-11.0);
	CVector<1> vFinal1d = cgo1d.Optimize(vInit1d);

	printf("Final value from CGO (1d) = <%lf>\n", vFinal1d[0]);


	CGradParabola<2, double> gradParabola2d(CVector<2>(-7.0, 12.0));
	CVector<2> vGrad2d = gradParabola2d.Grad(CVector<2>(0.0, 0.0));
	printf("\nGradient at <0, 0> = <%lf, %lf>\n", vGrad2d[0], vGrad2d[1]);
		
	CConjGradOptimizer<2, double> cgo2d(&gradParabola2d);

	CVector<2> vInit2d(-11.0, 0.0);
	CVector<2> vFinal2d = cgo2d.Optimize(vInit2d);

	printf("Final value from CGO (2d) = <%lf, %lf>\n", vFinal2d[0], vFinal2d[1]);


	CGradParabola<4, double> gradParabola4d(CVector<4>(-1.0, 3.0, -7.0, -11.0));
	CVector<4> vGrad4d = gradParabola4d.Grad(CVector<4>(0.0, 0.0, 0.0, 0.0));
	printf("\nGradient 4d at <0, 0, 0, 0> = <%lf, %lf, %lf, %lf\n", 
		vGrad4d[0], vGrad4d[1], vGrad4d[2], vGrad4d[3]);
	CConjGradOptimizer<4, double> cgo(&gradParabola4d);

	vFinal = cgo.Optimize(vInit);
	printf("Final value from CGO (4d) = <%lf, %lf, %lf, %lf>\n", 
		vFinal[0], vFinal[1], vFinal[2], vFinal[3]);

	return 0;
}
