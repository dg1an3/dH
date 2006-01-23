#include "stdafx.h"
#include <DFPOptimizer.h>

CDFPOptimizer::CDFPOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc)
{
}

CDFPOptimizer::~CDFPOptimizer(void)
{
}


#ifdef REWQRITE



bool TestDirTol(const CVectorN<>& vDir, const CVectorN<>& vP, const REAL Tol)
{
	// form max of absolute value of ratio of direction to vP for all 
	REAL test = 0.0;
	for (int nI = 0; nI < vDir.GetDim(); nI++) 
	{
		REAL temp = fabs(vDir[nI]) / MAX(fabs(vP[nI]), 1.0);
		if (temp > test) 
		{
			test = temp;
		}
	}

	// test if max is below tolerance
	return (test < Tol);
}


// solves for vector y = Mx
bool CholeskySolve(const CMatrixNxM<>& mA, const CVectorN<>& vY, CVectorN<>& vX)
{
	return true;
}

bool TestGradTol(const CVectorN<>& vDir, const CVectorN<>& vP, const REAL fret, const REAL tol)
{
	REAL test = 0.0;
	REAL den = MAX(fret, 1.0);
	for (nI = 0; nI < n; nI++) 
	{
		temp = fabs(vG[nI]) * MAX(fabs(vP[nI]), 1.0) / den;
		if (temp > test) 
		{
			test = temp;
		}
	}

	// test if max is below tolerance
	return (test < Tol);
}


void dfpmin(CVectorN<>& vP, 
			const REAL gtol, 
			REAL (*pFunc)(const CVectorN<>&, CVectorN<> *))
{
	const REAL EPS = numeric_limits<REAL>::epsilon();
	const REAL TOLX = 4*EPS;

	bool check;

	int nN = vP.GetDim();

	CMatrixNxM<> mHess_inv; 		
	mHess_inv.Reshape(nN, nN);					
	mHess_inv.SetIdentity();				

	// initialize convergence max
	const REAL STPMX = 100.0;
	REAL stpmax = STPMX * MAX(vP.GetLength(), REAL(nN));

	// initial eval
	CVectorN<> vG(nN);
	REAL f = (*pFunc)(vP, &vG);

	// initialize direction
	CVectorN<> vXi = -vG;

	const int ITMAX = 200;
	for (int nIter = 0; nIter < ITMAX; nIter++) 
	{
		//
		// perform line search
		//

		CVectorN<> vP_next(nN);
		REAL f_next = 0.0;
		lnsrch(vP, fp, vG, vXi, vP_next, fret, stpmax, check, pFunc);

		// update P and direction
		vXi = vP_next - vP;
		vP = vP_next;						
		f = f_next;

		// test for direction tolerance
		if (TestDirTol(vXi, vP, TOLX))
		{
			return;
		}

		// calc new gradient
		CVectorN<> vG_next(nN);
		(*pFunc)(vP, vG_next);

		// test for gradient tolerance
		if (TestGradTol(vG_next, vP, fret, gtol))
		{
			return;
		}

		//
		// update inverse Hessian
		//

		// to compute gradient delta -- next update (delta) gradient
		CVectorN<> vDeltaG = vG_next - vG;
		vG = vG_next;

		REAL vDG_dot_vXi = vDeltaG * vXi;

		// is denominator sufficiently large (test for matrix conditioning?)
		if (vDG_dot_vXi > sqrt((vDeltaG * vDeltaG) * (vXi * vXi) * EPS)
		{
			CVectorN<> vHDG = mHess_inv * vDG;				
			REAL vDG_dot_vHDG = vDG * vHDG;	

			vDG = vXi / vDG_dot_vXi				
				-  vHDG / vDG_dot_vHDG;	

			CMatrixNxM<> m_vDG_out_vDG;			
			m_vDG_out_vDG.Outer(vDeltaG, vDeltaG);		
			mHess_inv += vDG_dot_vHDG * m_vDG_out_vDG;	
												
			CMatrixNxM<> m_vXi_out_vXi;			
			m_vXi_out_vXi.Outer(vXi, vXi);		
			mHess_inv += m_vXi_out_vXi / vDG_dot_vXi;	
															
			CMatrixNxM<> m_vHDG_out_vHDG;		
			m_vHDG_out_vHDG.Outer(vHDG, vHDG);	
			mHess_inv -= m_vHDG_out_vHDG / vDG_dot_vHDG;
		}

		// compute next estimate for X (direction)
		vXi = mHess_inv * vG;					
		vXi *= -1.0;							
	}

	nrerror("too many iterations in dfpmin");
}






void bfgsmin(CVectorN<>& vP, 
			const REAL gtol, 
			REAL (*pFunc)(const CVectorN<>&, CVectorN<> *))
{
	const REAL EPS = numeric_limits<REAL>::epsilon();
	const REAL TOLX = 4*EPS;

	bool check;

	int nN = vP.GetDim();

	CMatrixNxM<> mHess; 
	mHess.Reshape(nN, nN);					
	mHess.SetIdentity();					

	// initialize convergence max
	const REAL STPMX = 100.0;
	REAL stpmax = STPMX * MAX(vP.GetLength(), REAL(nN));

	// initial eval
	CVectorN<> vGrad(nN);
	REAL f = (*pFunc)(vP, &vGrad);

	// initialize direction
	CVectorN<> vDir = -k * vGrad;

	const int ITMAX = 200;
	for (int nIter = 0; nIter < ITMAX; nIter++) 
	{
		// perform line search
		CVectorN<> vP_next;
		REAL f_next = 0.0;
		lnsrch(vP, f, vGrad, vDir, vP_next, f_next, stpmax, check, pFunc);

		// update gradient
		CVectorN<> vGrad_next;
		(*pFunc)(vP_next, &vGrad_next);

		// test for gradient tolerance
		if (TestGradTol(vG, vP, fret, gtol))
		{
			return;
		}

		// bump function value and parameters
		f = f_next;
		vP = vP_next;						

		// calculate gamma = gradient delta
		CVectorN<> vGamma = vGrad_next - vGrad;
		vGrad = vGrad_next;

		// compute H*vGamma
		CVectorN<> vHG = mHess * vGamma;

		// compute denominator
		REAL vGamma_dot_vDir = vGamma * vDir;

		CMatrixNxM<> m_vDir_out_vHG;
		m_vDir_out_vHG.Outer(vDir, vHG);
		m_vDir_out_vHG *= 1.0 / vGamma_dot_vDir;
		mHess -= m_vDir_out_vHG;

		// TODO: check this
		m_vDir_out_vHG.Transpose();
		mHess -= m_vDir_out_vHG;	

		CMatrixNxM<> m_vDir_out_vDir;
		m_vDir_out_vDir.Outer(vDir, vDir);
		REAL Q = 1.0 + vGamma * vHG / vGamma_dot_vDir;
		m_vDir_out_vDir *= Q / vGamma_dot_vDir;
		mHess += m_vDir_out_vDir;

		// solve for new vGrad
		CholeskySolve(mHess, vP, vGrad);

		// update direction
		vDir = -k * mHess * vGrad;

		// test for direction tolerance
		if (TestDirTol(vDir, vP, TOLX))
		{
			return;
		}
	}

	nrerror("too many iterations in dfpmin");
}






void lnsrch(const CVectorN<>& vP, 
			const REAL f_prev, 
			const CVectorN<>& vG, 
			CVectorN<>& vXi,
			CVectorN<>& vP_next, 
			REAL &f_next, 
			const REAL stpmax, 
			bool &check, 
			REAL (*pFunc)(const CVectorN<>&, CVectorN<> *))
{

	const REAL ALF = 1.0e-4;
	const REAL TOLX = numeric_limits<REAL>::epsilon();

	int nN = vP.size();

	check = false;

	REAL sum = vXi * vXi;
	sum = sqrt(sum);
	if (sum > stpmax)
	{
		vXi *= stpmax / sum;
	}

	REAL slope = vG * vXi;
	if (slope >= 0.0) 
	{
		nrerror("Roundoff problem in lnsrch.");
	}

	REAL test = 0.0;
	for (nI = 0; nI < nN; nI++) 
	{
		REAL temp = fabs(vXi[nI]) 
			/ MAX(fabs(vP[nI]), 1.0);
		if (temp > test) 
		{
			test = temp;
		}
	}

	REAL alamin = TOLX / test;
	REAL alam = 1.0;
	REAL alam2 = 0.0;
	REAL f2 = 0.0;

	for (;;) 
	{
		vP_next = vP + alam * vXi;

		f_next = (*pFunc)(vP_next, NULL);

		// stores temporary ???
		REAL tmplam = 0.0;

		if (alam < alamin) 
		{
			vP_next = vP;

			check = true;

			return;
		} 
		else if (f_next <= f_prev + ALF*alam*slope)
		{
			return;
		}
		else 
		{
			if (alam == 1.0)
			{
				tmplam = -slope/(2.0*(f_next-f_prev-slope));
			}
			else 
			{
				REAL rhs1 = f_next - f_prev - alam * slope;
				REAL rhs2 = f2 - f_prev - alam2 * slope;

				REAL a = (rhs1 / (alam*alam) - rhs2 / (alam2*alam2))
						/ (alam - alam2);

				REAL b = (-alam2 * rhs1 / (alam*alam) + alam * rhs2 / (alam2*alam2))
						/ (alam - alam2);

				if (a == 0.0) 
				{
					tmplam = -slope / (2.0 * b);
				}
				else 
				{
					REAL disc = b * b - 3.0 * a * slope;
					if (disc < 0.0) 
					{
						tmplam = 0.5 * alam;
					}
					else if (b <= 0.0) 
					{
						tmplam = (-b + sqrt(disc)) / (3.0 * a);
					}
					else 
					{
						tmplam = -slope / (b + sqrt(disc));
					}
				}
				if (tmplam > 0.5 * alam)
				{
					tmplam = 0.5 * alam;
				}
			}
		}
		alam2 = alam;
		f2 = f_next;
		alam = MAX(tmplam, 0.1 * alam);
	}
}
#endif

#ifdef ORIGINAL

void dfpmin(CVectorN<>& vP, const REAL gtol, int &iter, REAL &fret,
				REAL func(const CVectorN<>& ), void dfunc(const CVectorN<>& , CVectorN<>& ))
{
	const REAL EPS = numeric_limits<REAL>::epsilon();
	const REAL TOLX = 4*EPS;

	bool check;

	int nN = vP.GetDim();

	CMatrixNxM<> mHess_inv; // (n,n);			// for (i=0;i<n;i++)  {
	mHess_inv.Reshape(nN, nN);					//		for (j=0;j<n;j++) mHess_inv[i][j] = 0.0;
	mHess_inv.SetIdentity();					//		mHess_inv[i][i] = 1.0;
												//		vXi[i] = -vG[i];
												//		sum += vP[i]*vP[i];
												//	}

	// initialize convergence max
	const REAL STPMX = 100.0;
	REAL stpmax = STPMX 
		* MAX(vP.GetLength() /* sqrt(sum) */, REAL(nN));

	// initial eval
	REAL fp = func(vP);

	CVectorN<> vG(nN);
	dfunc(vP, vG);

	// initialize direction
	CVectorN<> vXi(nN);
	vXi = -vG;

	const int ITMAX = 200;
	for (int nIter = 0; nIter < ITMAX; nIter++) 
	{
		//
		// perform line search
		//

		CVectorN<> vP_next(nN);
		REAL fret = 0.0;
		lnsrch(vP, fp, vG, vXi, vP_next, fret, stpmax, check, func);

		fp = fret;

		//
		// update P and direction
		//

		vXi = vP_next - vP;						// for (i=0;i<n;i++) {
		vP = vP_next;							//		vXi[i]=pnew[i]-vP[i];
												//		vP[i]=pnew[i];
												//	}


		//
		// test for direction tolerance
		// 

		// form max of absolute value of ratio of direction to vP for all 
		REAL test = 0.0;
		for (int nI = 0; nI < nN; nI++) 
		{
			REAL temp = fabs(vXi[nI]) / MAX(fabs(vP[nI]), 1.0);
			if (temp > test) 
			{
				test = temp;
			}
		}

		// test if max is below tolerance
		if (test < TOLX)
		{
			return;
		}

		// to compute gradient delta -- first store current gradient
		CVectorN<> vDeltaG = vG;				// 	for (i=0;i<n;i++) vDG[i]=vG[i];

		// calc new gradient
		dfunc(vP, vG);

		// test for gradient tolerance

		test = 0.0;
		REAL den = MAX(fret, 1.0);
		for (nI = 0; nI < n; nI++) 
		{
			temp = fabs(vG[nI]) * MAX(fabs(vP[nI]), 1.0) / den;
			if (temp > test) 
			{
				test = temp;
			}
		}

		if (test < gtol)
		{
			return;
		}

		//
		// form conjugate direction
		//

		// to compute gradient delta -- next update (delta) gradient
		vDeltaG = vG - vDeltaG;					// for (i=0;i<n;i++) vDG[i]=vG[i]-vDG[i];

		CVectorN<> vHDG(nN);
		vHDG = mHess_inv * vDG;					// for (i=0;i<n;i++) {
												//		vHDG[i]=0.0;
												//		for (j=0;j<n;j++) vHDG[i] += mHess_inv[i][j]*vDG[j];
												// }

		REAL // fac 
			vDG_dot_vXi = vDG * vXi;					// for (i=0;i<n;i++) fac += vDG[i] * vXi[i];
		REAL // fae 
			vDG_dot_vHDG = vDG * vHDG;					// for (i=0;i<n;i++) fae += vDG[i] * vHDG[i];

		REAL sumdg = 0.0;
		REAL sumxi = 0.0;
		for (nI=0; nI < n; nI++) 
		{
			sumdg += SQR(vDG[nI]);
			sumxi += SQR(vXi[nI]);
		}

		// is denominator sufficiently large (test for matrix conditioning?)
		if (fac > sqrt(EPS * sumdg * sumxi)) 
		{
			// fac = 1.0 / vDG_dot_vXi; // fac;
			// REAL fad = 1.0 / vDG_dot_vHDG; // fae;

			vDG = vXi * /* fac */ (1.0 / vDG_dot_vXi) 
				-  vHDG * /* fad */ (1.0 / vDG_dot_vHDG);	// for (i=0;i<n;i++) vDG[i]=fac*vXi[i]-fad*vHDG[i];

			CMatrixNxM<> m_vDG_out_vDG;
			m_vDG_out_vDG.Outer(vDG, vDG);
			mHess_inv += /* fae */ vDG_dot_vHDG * m_vDG_out_vDG;

			CMatrixNxM<> m_vXi_out_vXi;			// for (i=0;i<n;i++) {
			m_vXi_out_vXi.Outer(vXi, vXi);		//		for (j=i;j<n;j++) {
			mHess_inv += 
				m_vXi_out_vXi 
					* /* fac */ (1.0 / vDG_dot_vXi);	
												//			mHess_inv[i][j] += 
												//				fac * vXi[i] * vXi[j]
			CMatrixNxM<> m_vHDG_out_vHDG;		//					-fad * vHDG[i] * vHDG[j] 
			m_vHDG_out_vHDG.Outer(vHDG, vHDG);	//					+ fae * vDG[i] * vDG[j];
			mHess_inv -= 
				m_vHDG_out_vHDG 
					* /* fad */ (1.0 / vDG_dot_vHDG);	
												//			mHess_inv[j][i] = mHess_inv[i][j];
												//		}
												//	}
		}

		// compute next estimate for X (direction)
		vXi = mHess_inv * vG;					// for (i=0;i<n;i++) {
		vXi *= -1.0;							//		vXi[i]=0.0;
												//		for (j=0;j<n;j++) 
												//			vXi[i] -= mHess_inv[i][j]*vG[j];
												//	}
	}

	nrerror("too many iterations in dfpmin");
}




void NR::lnsrch(const CVectorN<>& xold, const REAL fold, const CVectorN<>& vG, CVectorN<>& vP,
	CVectorN<>& x, REAL &f, const REAL stpmax, bool &check, REAL func(const CVectorN<>& ))
{
	const REAL ALF=1.0e-4, TOLX=numeric_limits<REAL>::epsilon();
	int i;
	REAL a,alam,alam2=0.0,alamin,b,disc,f2=0.0;
	REAL rhs1,rhs2,slope,sum,temp,test,tmplam;

	int n=xold.size();
	check=false;
	sum=0.0;
	for (i=0;i<n;i++) sum += vP[i]*vP[i];
	sum=sqrt(sum);
	if (sum > stpmax)
	{
		for (i=0;i<n;i++) 
		{
			vP[i] *= stpmax/sum;
		}
	}
	slope=0.0;
	for (i=0;i<n;i++)
	{
		slope += vG[i]*vP[i];
	}
	if (slope >= 0.0) 
	{
		nrerror("Roundoff problem in lnsrch.");
	}
	test=0.0;
	for (i=0;i<n;i++) 
	{
		temp=fabs(vP[i])/MAX(fabs(xold[i]),1.0);
		if (temp > test) 
			test=temp;
	}
	alamin=TOLX/test;
	alam=1.0;
	for (;;) 
	{
		for (i=0;i<n;i++) x[i]=xold[i]+alam*vP[i];
		f=func(x);
		if (alam < alamin) 
		{
			for (i=0;i<n;i++) 
			{
				x[i]=xold[i];
			}
			check=true;
			return;
		} 
		else if (f <= fold+ALF*alam*slope)
		{
			return;
		}
		else 
		{
			if (alam == 1.0)
			{
				tmplam = -slope/(2.0*(f-fold-slope));
			}
			else 
			{
				rhs1 = f-fold-alam*slope;
				rhs2 = f2-fold-alam2*slope;
				a = (rhs1/(alam*alam)-rhs2/(alam2*alam2))/(alam-alam2);
				b = (-alam2*rhs1/(alam*alam)+alam*rhs2/(alam2*alam2))/(alam-alam2);
				if (a == 0.0) 
				{
					tmplam = -slope/(2.0*b);
				}
				else 
				{
					disc = b*b-3.0*a*slope;
					if (disc < 0.0) 
					{
						tmplam = 0.5*alam;
					}
					else if (b <= 0.0) 
					{
						tmplam = (-b+sqrt(disc))/(3.0*a);
					}
					else 
					{
						tmplam = -slope/(b+sqrt(disc));
					}
				}
				if (tmplam>0.5*alam)
				{
					tmplam=0.5*alam;
				}
			}
		}
		alam2=alam;
		f2 = f;
		alam=MAX(tmplam,0.1*alam);
	}
}

#endif 