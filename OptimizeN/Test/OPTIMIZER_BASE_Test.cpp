// OPTIMIZER_BASE_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <VectorN.h>

#include <PowellOptimizer.h>

#include <ConjGradOptimizer.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class CParabola : public CObjectiveFunction // <DIM, TYPE>
{
public:
	CParabola(const CVectorN<>& initOffset)
		: CObjectiveFunction(TRUE),
			m_vOffset(initOffset)
	{
	}

	const CVectorN<>& GetOffset() const
	{
		return m_vOffset;
	}

	void SetOffset(const CVectorN<>& vOffset)
	{
		m_vOffset = vOffset;
	}

	virtual REAL operator()(const CVectorN<>& vInput, CVectorN<> *pGrad)
	{
		if (NULL != pGrad)
		{
			pGrad->SetDim(vInput.GetDim());
			pGrad->SetZero();

			for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
			{
				(*pGrad)[nAt] = 2.0f * vInput[nAt] - 2.0f * m_vOffset[nAt];
			}
		}

		REAL length = (vInput - m_vOffset).GetLength();
		return length * length;
	}

private:
	CVectorN<> m_vOffset;
};

class CRosenbrock : public CObjectiveFunction
{
public:
	CRosenbrock()
		: CObjectiveFunction(TRUE)
	{
	}

	virtual REAL operator()(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL)
	{
		ASSERT(vInput.GetDim() == 2);

		REAL term1 = vInput[1] - vInput[0] * vInput[0];
		REAL dterm1_d0 = 2.0 * vInput[0];
		const REAL dterm1_d1 = 1.0;

		REAL term2 = 1.0 - vInput[0];
		const REAL dterm2_d0 = -1.0;
		const REAL dterm2_d1 = 0.0;

		if (NULL != pGrad)
		{
			ASSERT(pGrad->GetDim() == 2);
			(*pGrad)[0] = 200.0 * term1 * dterm1_d0 + 2.0 * term2 * dterm2_d0;
			(*pGrad)[1] = 200.0 * term1 * dterm1_d1 + 2.0 * term2 * dterm2_d1;
		}

		return 100.0 * term1 * term1 + term2 * term2;
	}
};


class CCubic : public CObjectiveFunction
{
public:
	CCubic(REAL a, REAL b, REAL c, REAL d)
		: CObjectiveFunction(TRUE),
			m_a(a),
			m_b(b),
			m_c(c),
			m_d(d)
	{
	}

	virtual REAL operator()(const CVectorN<>& v, CVectorN<> *pGrad = NULL)
	{
		ASSERT(v.GetDim() == 1);

		if (NULL != pGrad)
		{
			pGrad->SetDim(1);

			(*pGrad)[0] = 3.0 * m_a * v[0] * v[0]
				+ 2.0 * m_b * v[0]
				+ m_c;
		}

		return m_a * v[0] * v[0] * v[0] 
			+ m_b * v[0] * v[0]
			+ m_c * v[0]
			+ m_d;
	}

	REAL m_a;
	REAL m_b;
	REAL m_c;
	REAL m_d;
};


void TestCubicInterp()
{
	CCubic cub(1.0, -2.0, 1.0, 0.0);

	CCubicInterpOptimizer cubOpt(&cub);

	CVectorN<> opt = cubOpt.Optimize(CVector<1>(1.9));
}

void TestRosenbrock()
{
	CRosenbrock rosen;
	CVectorN<> vInit = CVector<2>(-1.9, 2.0);

	TRACE1("Rosenbrock at <-1.9, 2.0> = %lf\n", rosen(CVector<2>(-1.9, 2.0)));
	TRACE1("Rosenbrock at <-1.9, 2.0> = %lf\n", rosen(CVector<2>(-0.5, 0.5)));
	TRACE1("Rosenbrock at <1.0, 1.0> = %lf\n", rosen(CVector<2>(1.0, 1.0)));

	CPowellOptimizer pow(&rosen);
	pow.SetTolerance(0.1);

	CVectorN<> vFinal = pow.Optimize(vInit);
	TraceVector(vFinal);
}

void TestParabola()
{
	CVectorN<> vInitN;
	vInitN.SetDim(21);
	vInitN[0] = 12.0f;
	vInitN[12] = 25.0f;
	TraceVector(vInitN);

	CVectorN<REAL> vFinalN;
	vFinalN.SetDim(21);

	CVectorN<> vCenter(4);
	vCenter[0] = 11.0;
	vCenter[1] = -19.438;
	vCenter[2] = 22.1293;
	vCenter[3] = -0.0349;

	CVectorN<> vInit(4);

	CParabola gradParabola(vCenter);
	// printf("\nGradient at <0> = %lf\n", 
	//	gradParabola1d.Grad(CVectorN<>(0.0))[0]);
	CConjGradOptimizer cgo(&gradParabola);
	CVectorN<> vFinal = cgo.Optimize(vInit);

	TraceVector(vFinal);
	// cgo1d.SetUseGradientInfo(TRUE);
}

int main(int argc, char* argv[])
{
	TestCubicInterp();

	TestParabola();

	TestRosenbrock();
/*
	CVectorN<> vInit1d(1);
	vInit1d[0] = -11.0;
	CVectorN<> vFinal1d = cgo1d.Optimize(vInit1d);

	printf("Final value from CGO (1d) = <%lf>\n", vFinal1d[0]);


	CParabola<> gradParabola2d(CVectorN<>(-7.0, 12.0));
	CVectorN<> vGrad2d = gradParabola2d.Grad(CVectorN<>(0.0, 0.0));
	printf("\nGradient at <0, 0> = <%lf, %lf>\n", vGrad2d[0], vGrad2d[1]);
		
	CConjGradOptimizer<2, REAL> cgo2d(&gradParabola2d);

	CVectorN<> vInit2d(-11.0, 0.0);
	CVectorN<> vFinal2d = cgo2d.Optimize(vInit2d);

	printf("Final value from CGO (2d) = <%lf, %lf>\n", vFinal2d[0], vFinal2d[1]);


	CParabola<4> gradParabola4d(CVectorN<>(-1.0, 3.0, -7.0, -11.0));
	CVectorN<> vGrad4d = gradParabola4d.Grad(CVectorN<>(0.0, 0.0, 0.0, 0.0));
	printf("\nGradient 4d at <0, 0, 0, 0> = <%lf, %lf, %lf, %lf\n", 
		vGrad4d[0], vGrad4d[1], vGrad4d[2], vGrad4d[3]);
	CConjGradOptimizer<4, REAL> cgo(&gradParabola4d);

	CVectorN<> vInit;
	vInit.SetDim(4);
	CVectorN<> vFinal; 
	vFinal.SetDim(4);
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
	vFinal = cgo.Optimize(vInit);
	printf("Final value from CGO (4d) = <%lf, %lf, %lf, %lf>\n", 
		vFinal[0], vFinal[1], vFinal[2], vFinal[3]);
*/
	return 0;
}
