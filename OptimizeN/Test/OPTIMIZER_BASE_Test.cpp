// OPTIMIZER_BASE_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define XMLLOGGING_ON
#include <XMLLogging.h>

#define USE_XMLLOGGING
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

	virtual REAL operator()(const CVectorN<>& vInput, CVectorN<> *pGrad) const
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


class CQuadratic : public CObjectiveFunction // <DIM, TYPE>
{
public:
	CQuadratic(const CMatrixNxM<>& mA, const CVectorN<>& vB)
		: CObjectiveFunction(TRUE),
			m_mA(mA),
			m_vB(vB)
	{
		BEGIN_LOG_SECTION(CQuadratic::CQuadratic);
		LOG_EXPR_EXT(m_mA);
		LOG_EXPR_EXT(m_vB);
		END_LOG_SECTION();
	}

	virtual REAL operator()(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const
	{
		REAL res = 0.0;
		BEGIN_LOG_SECTION(CQuadratic::operator());
		LOG_EXPR_EXT(vInput);

		res = vInput * (m_mA * vInput);
		LOG_EXPR_EXT(m_mA * vInput);
		LOG_EXPR_DESC(res, "vInput * (m_mA * vInput)");

		res += m_vB * vInput;
		res /= 2.0;
		LOG_EXPR_DESC(res, "res = 0.5 * vInput * (m_mA * vInput) + m_vB * vInput");

		if (NULL != pGrad)
		{
			Gradient(vInput, (*pGrad));
		}

		END_LOG_SECTION();	// CQuadratic::operator()
		return res;
	}

	virtual void Gradient(const CVectorN<>& vInput, CVectorN<>& vGrad_out) const
	{
		BEGIN_LOG_SECTION(CQuadratic::Gradient);

		vGrad_out = m_mA * vInput;
		vGrad_out += m_vB;
		LOG_EXPR_EXT(m_mA * vInput);
		LOG_EXPR_EXT_DESC(vGrad_out, "Gradient = m_mA * vInput + m_vB");

		END_LOG_SECTION();	// CQuadratic::Gradient
	}

	virtual void Hessian(const CVectorN<>& vIn, CMatrixNxM<>& mHess) const
	{
		mHess.Reshape(m_mA.GetCols(), m_mA.GetRows());
		mHess = m_mA;
	}

private:
	CMatrixNxM<> m_mA;
	CVectorN<> m_vB;
};

class CRosenbrock : public CObjectiveFunction
{
public:
	CRosenbrock()
		: CObjectiveFunction(TRUE)
	{
	}

	virtual REAL operator()(const CVectorN<>& vInput, CVectorN<> *pGrad = NULL) const
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

	virtual REAL operator()(const CVectorN<>& v, CVectorN<> *pGrad = NULL) const
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

	CVectorN<> opt = cubOpt.Optimize(CVectorD<1>(1.9));
}

void TestRosenbrock()
{
	CRosenbrock rosen;
	CVectorN<> vInit = CVectorD<2>(-1.9, 2.0);

	TRACE1("Rosenbrock at <-1.9, 2.0> = %lf\n", rosen(CVectorD<2>(-1.9, 2.0)));
	TRACE1("Rosenbrock at <-1.9, 2.0> = %lf\n", rosen(CVectorD<2>(-0.5, 0.5)));
	TRACE1("Rosenbrock at <1.0, 1.0> = %lf\n", rosen(CVectorD<2>(1.0, 1.0)));

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
	CXMLLogFile::GetLogFile()->SetFormat((double) 0, " %le ");

	CMatrixNxM<> m1(1, 1);
	m1[0][0] = 4.0;

	CVectorN<> v1(1);
	v1[0] = -0.5;

	CQuadratic quad(m1, v1);

	CVectorN<> vInit(1);
	vInit[0] = 3.0;
	REAL res = quad(vInit);

	CVectorN<> vGrad;
	quad.Gradient(vInit, vGrad);
	quad.TestGradient(vInit, 1e-6);

	CMatrixNxM<> mHess;
	quad.Hessian(vInit, mHess);
	quad.TestHessian(vInit, 1e-6);

	//////////////////////////////////////////// 

	CMatrixNxM<> m2(2, 2);
	m2[0][0] = 4.0;
	m2[0][1] = -1.0;
	m2[1][1] = 4.0;
	m2[1][0] = -1.0;

	CVectorN<> v2(2);
	v2[0] = -0.5;
	v2[1] = 0.3;

	CQuadratic quad2(m2, v2);

	CVectorN<> vInit2(2);
	vInit2[0] = 3.0;
	vInit2[1] = -2.0;
	REAL res2 = quad2(vInit2);

	CVectorN<> vGrad2;
	quad2.Gradient(vInit2, vGrad2);
	quad2.TestGradient(vInit2, 1e-6);

	CMatrixNxM<> mHess2;
	quad2.Hessian(vInit2, mHess2);
	quad2.TestHessian(vInit2, 1e-4);

/*	TestCubicInterp();

	TestParabola();

	TestRosenbrock(); */
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
//	CGradParabola<4, double> parabola4d(CVectorD<4>(0.0, 0.0, 0.0, 0.0)); // -1.0, -3.0, -7.0, -11.0));
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
