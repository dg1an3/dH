#include "stdafx.h"

#include <limits>

#include <VectorN.h>
#include <MatrixNxM.h>

#include <DFPOptimizer.h>


template<class T>
inline const T SQR(const T a) {return a*a;}




bool TestDirTol(const CVectorN<>& vDir, const CVectorN<>& vP, const REAL tol)
{
	// form max of absolute value of ratio of direction to vP for all 
	REAL test = 0.0;
	for (int nI = 0; nI < vDir.GetDim(); nI++) 
	{
		REAL temp = fabs(vDir[nI]) / __max(fabs(vP[nI]), 1.0);
		if (temp > test) 
		{
			test = temp;
		}
	}

	// test if max is below tolerance
	return (test < tol);
}


bool TestGradTol(const CVectorN<>& vG, const CVectorN<>& vP, const REAL fret, const REAL tol)
{
	REAL test = 0.0;
	REAL den = __max(fret, 1.0);
	for (int nI = 0; nI < vG.GetDim(); nI++) 
	{
		REAL temp = fabs(vG[nI]) * __max(fabs(vP[nI]), 1.0) / den;
		if (temp > test) 
		{
			test = temp;
		}
	}

	// test if max is below tolerance
	return (test < tol);
}


CDFPOptimizer::CDFPOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc)
{
}

CDFPOptimizer::~CDFPOptimizer(void)
{
}

#define REWRITE_MTL
#ifdef REWRITE_MTL


const CVectorN<>& CDFPOptimizer::Optimize(const CVectorN<>& vP)
{
	const REAL EPS = numeric_limits<REAL>::epsilon();
	const REAL TOLX = 4*EPS;

	bool check;

	int nN = vP.GetDim();

	CVectorN<>& vP_curr = m_vFinalParam;
	vP_curr.SetDim(nN);
	vP_curr = vP;

	// shape hessian
	m_mHess_inv.Reshape(nN, nN);					
	m_mHess_inv.SetIdentity();				

	m_vHDG_out_vHDG.Reshape(nN, nN);
	m_vDG_out_vDG.Reshape(nN, nN);
	m_vXi_out_vXi.Reshape(nN, nN);

	// shape temp parameters
	m_vG.SetDim(nN);
	m_vXi.SetDim(nN);
	m_vP_next.SetDim(nN);
	m_vG_next.SetDim(nN);
	m_vDeltaG.SetDim(nN);
	m_vHDG.SetDim(nN);

	// initialize convergence max
	const REAL STPMX = 100.0;
	REAL stpmax = STPMX * __max(vP_curr.GetLength(), REAL(nN));

	// initial eval
	REAL f = (*m_pFunc)(vP_curr, &m_vG);

	// initialize direction
	m_vXi = m_vG;
	m_vXi *= -1.0;

	const int ITMAX = 200;
	for (int nIter = 0; nIter < ITMAX; nIter++) 
	{
		// perform line search
		lnsrch(vP_curr, f, m_vG, m_vXi, m_vP_next, m_finalValue, stpmax, check);

		// update P and direction
		m_vXi = m_vP_next;
		m_vXi -= vP_curr;
		vP_curr = m_vP_next;						
		f = m_finalValue;

		// test for direction tolerance
		if (TestDirTol(m_vXi, vP_curr, TOLX))
		{
			return vP_curr;
		}

		// calc new gradient
		(*m_pFunc)(vP_curr, &m_vG_next);

		// test for gradient tolerance
		if (TestGradTol(m_vG_next, vP_curr, m_finalValue, GetTolerance()))
		{
			return vP_curr;
		}

		//
		// update inverse Hessian
		//

		// to compute gradient delta -- next update (delta) gradient
		m_vDeltaG = m_vG_next;
		m_vDeltaG -= m_vG;
		m_vG = m_vG_next;

		REAL vDG_dot_vXi = m_vDeltaG * m_vXi;

		// is denominator sufficiently large (test for matrix conditioning?)
		if (vDG_dot_vXi > sqrt((m_vDeltaG * m_vDeltaG) * (m_vXi * m_vXi) * EPS))
		{
			m_vHDG.SetZero();
			::MultMatrixVectorN(&m_vHDG[0], 
				&m_mHess_inv[0][0], nN, nN,
				&m_vDeltaG[0]);

			REAL vDG_dot_vHDG = m_vDeltaG * m_vHDG;	
			::OuterProdN(&m_vHDG_out_vHDG[0][0], 
				&m_vHDG[0], nN, &m_vHDG[0], nN);	

			m_vDeltaG = m_vXi;
			m_vDeltaG *= 1.0 / vDG_dot_vXi; 
			m_vHDG *= 1.0 / vDG_dot_vHDG;
			m_vDeltaG -=  m_vHDG;	

			::OuterProdN(&m_vDG_out_vDG[0][0], 
				&m_vDeltaG[0], nN, &m_vDeltaG[0], nN);		
			m_vDG_out_vDG *= vDG_dot_vHDG;
			m_mHess_inv += m_vDG_out_vDG;	
			
			::OuterProdN(&m_vXi_out_vXi[0][0], 
				&m_vXi[0], nN, &m_vXi[0], nN);	
			m_vXi_out_vXi *= 1.0 / vDG_dot_vXi;
			m_mHess_inv += m_vXi_out_vXi;	
															
			m_vHDG_out_vHDG *= 1.0 / vDG_dot_vHDG;
			m_mHess_inv -= m_vHDG_out_vHDG;
		}

		// compute next estimate for X (direction)
		m_vXi.SetZero();
		::MultMatrixVectorN(&m_vXi[0], 
			&m_mHess_inv[0][0], nN, nN, 
			&m_vG[0]);
		m_vXi *= -1.0;							
	}

	TRACE("too many iterations in dfpmin");

	return vP_curr;
}

#ifdef ORIGINAL_REWRITE

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
#endif 

#else

const CVectorN<>& CDFPOptimizer::Optimize(const CVectorN<>& vInit)
{
	m_vFinalParam.SetDim(vInit.GetDim());
	m_vFinalParam = vInit;

	const int ITMAX=200;
	const REAL EPS=numeric_limits<REAL>::epsilon();
	const REAL TOLX=4*EPS,STPMX=100.0;
	bool check;
	int i,its,j;
	REAL den,fac,fad,fae,fp,stpmax,sum=0.0,sumdg,sumxi,temp,test;

	int n=m_vFinalParam.GetDim();
	CVectorN<> dg(n),g(n),hdg(n),pnew(n),xi(n);
	CMatrixNxM<> hessin(n,n);
	fp=(*m_pFunc)(m_vFinalParam,&g); 
	for (i=0;i<n;i++) {
		for (j=0;j<n;j++) hessin[i][j]=0.0;
		hessin[i][i]=1.0;
		xi[i] = -g[i];
		sum += m_vFinalParam[i]*m_vFinalParam[i];
	}
	stpmax=STPMX*__max(sqrt(sum),REAL(n));
	for (its=0;its<ITMAX;its++) {
		m_nIteration=its;
		lnsrch(m_vFinalParam,fp,g,xi,pnew,m_finalValue,stpmax,check);
		fp=m_finalValue;
		for (i=0;i<n;i++) {
			xi[i]=pnew[i]-m_vFinalParam[i];
			m_vFinalParam[i]=pnew[i];
		}
		test=0.0;
		for (i=0;i<n;i++) {
			temp=fabs(xi[i])/__max(fabs(m_vFinalParam[i]),1.0);
			if (temp > test) test=temp;
		}
		if (test < TOLX)
			return m_vFinalParam;
		for (i=0;i<n;i++) dg[i]=g[i];
		(*m_pFunc)(m_vFinalParam,&g); 
		test=0.0;
		den=__max(m_finalValue,1.0);
		for (i=0;i<n;i++) {
			temp=fabs(g[i])*__max(fabs(m_vFinalParam[i]),1.0)/den;
			if (temp > test) test=temp;
		}
		if (test < GetTolerance())
			return m_vFinalParam;
		for (i=0;i<n;i++) dg[i]=g[i]-dg[i];
		for (i=0;i<n;i++) {
			hdg[i]=0.0;
			for (j=0;j<n;j++) hdg[i] += hessin[i][j]*dg[j];
		}
		fac=fae=sumdg=sumxi=0.0;
		for (i=0;i<n;i++) {
			fac += dg[i]*xi[i];
			fae += dg[i]*hdg[i];
			sumdg += SQR(dg[i]);
			sumxi += SQR(xi[i]);
		}
		if (fac > sqrt(EPS*sumdg*sumxi)) {
			fac=1.0/fac;
			fad=1.0/fae;
			for (i=0;i<n;i++) dg[i]=fac*xi[i]-fad*hdg[i];
			for (i=0;i<n;i++) {
				for (j=i;j<n;j++) {
					hessin[i][j] += fac*xi[i]*xi[j]
						-fad*hdg[i]*hdg[j]+fae*dg[i]*dg[j];
					hessin[j][i]=hessin[i][j];
				}
			}
		}
		for (i=0;i<n;i++) {
			xi[i]=0.0;
			for (j=0;j<n;j++) xi[i] -= hessin[i][j]*g[j];
		}
	}
	ASSERT(FALSE);	
	TRACE("too many iterations in dfpmin\n");

	return m_vFinalParam;
}

#endif

#ifdef REWRITE_MTL


void CDFPOptimizer::lnsrch(const CVectorN<>& xold, 
						   const REAL fold, 
						   const CVectorN<>& vG, 
						   CVectorN<>& vP,
						   CVectorN<>& x, 
						   REAL &f, 
						   const REAL stpmax, 
						   bool &check)
{
	const REAL ALF=1.0e-4, TOLX=numeric_limits<REAL>::epsilon();
	int i;
	REAL a,alam,alam2=0.0,alamin,b,disc,f2=0.0;
	REAL rhs1,rhs2,slope,sum,temp,test,tmplam;

	int n=xold.GetDim();
	check=false;
	sum = vP * vP;
	sum = sqrt(sum);
	if (sum > stpmax)
	{
		vP *= stpmax/sum;
	}
	slope = vG * vP;
	if (slope >= 0.0) 
	{
		// TRACE("Roundoff problem in lnsrch.");
	}

	test=0.0;
	for (i=0;i<n;i++) 
	{
		temp=fabs(vP[i])/__max(fabs(xold[i]),1.0);
		if (temp > test) 
			test=temp;
	}
	alamin=TOLX/test;
	alam=1.0;
	for (;;) 
	{
		x = vP;				// for (i=0;i<n;i++) x[i]=xold[i]+alam*vP[i];
		x *= alam;
		x += xold;

		f=(*m_pFunc)(x); 
		if (alam < alamin) 
		{
			x = xold;		//			for (i=0;i<n;i++) 
							//					x[i]=xold[i];

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
		alam=__max(tmplam,0.1*alam);
	}
}


#ifdef REFORMATTED

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
		vP_next = vXi;
		vP_next *= alam;
		vP_next += vP;

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

#else

void CDFPOptimizer::lnsrch(const CVectorN<>& xold, 
			const REAL fold, 
			const CVectorN<>& g, 
			CVectorN<>& p,
			CVectorN<>& x, 
			REAL &f, 
			const REAL stpmax, 
			bool &check)
{
	const REAL ALF=1.0e-4, TOLX=numeric_limits<REAL>::epsilon();
	int i;
	REAL a,alam,alam2=0.0,alamin,b,disc,f2=0.0;
	REAL rhs1,rhs2,slope,sum,temp,test,tmplam;

	int n=xold.GetDim();
	check=false;
	sum=0.0;
	for (i=0;i<n;i++) sum += p[i]*p[i];
	sum=sqrt(sum);
	if (sum > stpmax)
		for (i=0;i<n;i++) p[i] *= stpmax/sum;
	slope=0.0;
	for (i=0;i<n;i++)
		slope += g[i]*p[i];
	if (slope >= 0.0) TRACE("Roundoff problem in lnsrch.");
	test=0.0;
	for (i=0;i<n;i++) {
		temp=fabs(p[i])/__max(fabs(xold[i]),1.0);
		if (temp > test) test=temp;
	}
	alamin=TOLX/test;
	alam=1.0;
	for (;;) {
		for (i=0;i<n;i++) x[i]=xold[i]+alam*p[i];
		f=(*m_pFunc)(x); 
		if (alam < alamin) {
			for (i=0;i<n;i++) x[i]=xold[i];
			check=true;
			return;
		} else if (f <= fold+ALF*alam*slope) return;
		else {
			if (alam == 1.0)
				tmplam = -slope/(2.0*(f-fold-slope));
			else {
				rhs1=f-fold-alam*slope;
				rhs2=f2-fold-alam2*slope;
				a=(rhs1/(alam*alam)-rhs2/(alam2*alam2))/(alam-alam2);
				b=(-alam2*rhs1/(alam*alam)+alam*rhs2/(alam2*alam2))/(alam-alam2);
				if (a == 0.0) tmplam = -slope/(2.0*b);
				else {
					disc=b*b-3.0*a*slope;
					if (disc < 0.0) tmplam=0.5*alam;
					else if (b <= 0.0) tmplam=(-b+sqrt(disc))/(3.0*a);
					else tmplam=-slope/(b+sqrt(disc));
				}
				if (tmplam>0.5*alam)
					tmplam=0.5*alam;
			}
		}
		alam2=alam;
		f2 = f;
		alam=__max(tmplam,0.1*alam);
	}
}

#endif









