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
		CVector<DIM, TYPE> vConstant;
		for (int nAt = 0; nAt < DIM; nAt++)
			vConstant[nAt] = 2.0;

		return vInput * vConstant - vOffset.Get() * vConstant;
	}
};

int main(int argc, char* argv[])
{
	CGradParabola<4, double> parabola4d(CVector<4>(-1.0, -3.0, -7.0, -11.0));
	CPowellOptimizer<4, double> po(&parabola4d);

	CVector<4> vInit;
	vInit[0] = 10.0;
	vInit[1] = 20.0;
	vInit[2] = -3.6;
	vInit[3] = 11.0;

	CVector<4> vFinal = po.Optimize(vInit);

	printf("Final value from Powell = <%lf, %lf, %lf, %lf>\n", 
		vFinal[0], vFinal[1], vFinal[2], vFinal[3]);

	CGradParabola<4, double> gradParabola4d(CVector<4>(-1.0, -3.0, -7.0, -11.0));
	CConjGradOptimizer<4, double> cgo(&gradParabola4d);

	vFinal = cgo.Optimize(vInit);

	printf("Final value from CGO = <%lf, %lf, %lf, %lf>\n", 
		vFinal[0], vFinal[1], vFinal[2], vFinal[3]);


	return 0;
}
