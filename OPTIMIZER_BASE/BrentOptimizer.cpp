//////////////////////////////////////////////////////////////////////
// BrentOptimizer.cpp: implementation of the CBrentOptimizer
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "BrentOptimizer.h"

///////////////////////////////////////////////////////////////////////////////
// macros used to manipulate parameters
///////////////////////////////////////////////////////////////////////////////

// shifts three parameters
#define SHFT(a,b,c,d) (a)=(b); (b)=(c); (c)=(d);

// moves three parameters at a time
#define MOV3(a, b, c, d, e, f) (a)=(d); (b)=(e); (c)=(f);

// SIGN returns the argument a with the sign of the argument b 
#define SIGN(a,b) ((b) >= (TYPE) 0.0 ? (TYPE) fabs(a) : (TYPE) -fabs(a))  
	

///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

const double GOLD = 1.618034;	// golden mean

const double CGOLD = 0.3819660;	// golden section ratio
								
const double ZEPS = 1.0e-1;		// z-epsilon -- small number to protect against 
								// fractional accuracy for a minimum that
								// happens to be exactly zero;  used in
								//    function FindMinimum


const double TINY = 1.0e-20;	// used in function BracketMinimum 

const double BRACKET = 5.0;		// initial bracket size

const double GLIMIT = 100.0;	// parameter needed by function BracketMinimum  

const int ITER_MAX = 1500;		// maximum iteration


///////////////////////////////////////////////////////////////////////////////
// global definitions of dummy optimizers -- needed to get the compiler
//		to expand the necessary code
///////////////////////////////////////////////////////////////////////////////

CBrentOptimizer<double> g_BrentOptimizerDouble(NULL);
CBrentOptimizer<float> g_BrentOptimizerFloat(NULL);

//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<TYPE>::Optimize
// 
// performs the optimization given the initial value vector
//////////////////////////////////////////////////////////////////////
template<class TYPE>
CVector<1, TYPE> CBrentOptimizer<TYPE>::Optimize(const CVector<1, TYPE>& vInit)
{
	// find three values the bracket a minimum
	TYPE ax = vInit[0];
	TYPE bx = ax + (TYPE) BRACKET;
	TYPE cx;
	BracketMinimum(ax, bx, cx);

	// find the actual minimum
	TYPE finalx;
	
	// which version of FindMinimum we use depends on whether
	//		gradient information is available and should be used
	if (m_pFunc->HasGradientInfo() && UseGradientInfo())
		finalx = FindMinimumGrad(ax, bx, cx);
	else
		finalx = FindMinimum(ax, bx, cx);

	// set the member variable that holds the final value
	finalParam.Set(finalx);

	// and return it
	return finalParam.Get();
}

//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<TYPE>::BracketMinimum
// 
// Given two distinct initial points ax and bx, this routine searches downhill 
// (defined by the function as evaluated at the initial points) and returns new 
// points ax, bx, cx that bracket a minimum of the function.  Also returned are 
// the function values at the three points fa, fb, fc.
//
// GOLD is the default ratio by which successive intervals are magnified;
// GLIMIT is the maximum magnification allowed for a parabolic-fit step
//
// This function was adapted from function mnbrak, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
//
//////////////////////////////////////////////////////////////////////
template<class TYPE>
void CBrentOptimizer<TYPE>::BracketMinimum(TYPE& ax, TYPE& bx, TYPE& cx)
{
	TYPE fa, fb, fc;

	fa = (*m_pFunc)(ax);
	fb = (*m_pFunc)(bx);

	// Switch roles of a and b so that we can go downhill in the direction 
	// from a to b. 
	if (fb > fa)
	{
		TYPE temp;
		SHFT(temp, ax, bx, temp)
		SHFT(temp, fa, fb, temp)
	}

	// First guess for c. 
	cx = (bx) + (TYPE) GOLD * (bx - ax);
	fc = (*m_pFunc)(cx);

	// Keep returning here until we bracket. 
	while (fb > fc)
	{
		// Compute u by parabolic extrapolation from a,b,c.  TINY is used to 
		// prevent any possible division by zero. 
		TYPE r = (bx - ax) * (fb - fc);
		TYPE q = (bx - cx) * (fb - fa);
		TYPE u = (TYPE) (bx - ((bx - cx) * q - (bx - ax) * r)
			 / (2.0 * SIGN(max(fabs(q - r), TINY),q - r)));
		TYPE fu; // function value at u
		TYPE ulim = bx + (TYPE)(GLIMIT * (cx - bx));

		// We won't go farther than this.  Test various possibilities 
		if ((bx - u) * (u - cx) > 0.0)
		{
			// Parabolic u is between b and c: try it. 
			fu = (*m_pFunc)(u);
			if (fu < fc)
			{
				// Got a minimum between b and c. 
				ax = bx;
				bx = u;
				fa = fb;
				fb = fu;
				return;
			}
			else if (fu > fb)
			{
				// Got a minimum between a and u. 
				cx = u;
				fc = fu;
				return;
			}

			// Parabolic fit was no use.  Use default magnification. 
			u = cx + (TYPE) GOLD * (cx - bx);
			fu = (*m_pFunc)(u);
		}
		else if ((cx - u) * (u - ulim) > 0.0)
		{
			// Parabolic fit is between c and its allowed limit. 
			fu = (*m_pFunc)(u);
			if (fu < fc)
			{
				SHFT(bx,cx,u,cx + (TYPE) GOLD * (cx - bx))
				SHFT(fb,fc,fu,(*m_pFunc)(u))
			}
		}
		else if ((u - ulim) * (ulim - cx) >= 0.0)
		{
			// Limit parabolic u to maximum allowed value. 
			u = ulim;
			fu = (*m_pFunc)(u);
		}
		else
		{
			// Reject parabolic u, use default magnification. 
			u=(cx) + (TYPE) GOLD*(cx-bx);
			fu=(*m_pFunc)(u);
		}

		// Eliminate oldest point and continue. 
		SHFT(ax,bx,cx,u)
		SHFT(fa,fb,fc,fu)
	}
}


//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<TYPE>::FindMinimum
// 
// Given a function f, and given a bracketing triplet of abscissas
// ax, bx, cx, (such that bx is between ax and cx, and f(bx) < f(ax)
// and f(bx) < f(cx)), this routine isolates the minimum to a fractional
// precision of about tol using Brent's method.  The abscissa of the
// minimum is returned as xmin, and the minimum function value is returned
// as brent, the returned function value.
//
// This function was adapted from function brent, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
// 
// ax,..,cx are function values needed to perform the minimization  
// tol is variable specifing the minimum agreement needed between successive 
// intertions before stopping 
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CBrentOptimizer<TYPE>::FindMinimum (TYPE ax, TYPE bx, TYPE cx)
{
	// The following are intermediate computed values. 
	TYPE a,b,d,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
	TYPE e=0.0;            // distance moved on the step before last 

	// a and b must be in ascending order, but input abscissas need not be.
	a=(ax < cx ? ax : cx);
	b=(ax > cx ? ax : cx);
	x=w=v=bx;
	fw=fv=fx=(*m_pFunc)(x);

	// Main function loop. 
	for (iteration.Set(0); iteration.Get() < ITER_MAX; 
			iteration.Set(iteration.Get()+1))
	{
		xm = (TYPE) 0.5*(a+b);
		tol1 = (TYPE) (tolerance.Get() * fabs(x)+ZEPS);
		tol2 = (TYPE) 2.0 * tol1;

		// Test for done here. 
		if (fabs(x - xm) <= (tol2-0.5*(b-a)))
		{
			finalValue.Set(fx);
			return x;
		}

		// Construct a trial parabolic fit. 
		if (fabs(e) > tol1)
		{
			r=(x-w)*(fx-fv);
			q=(x-v)*(fx-fw);
			p=(x-v)*q-(x-w)*r;
			q=(TYPE) 2.0*(q-r);
			if (q > 0.0)
				p = -p;
			q=(TYPE)fabs(q);
			etemp=e;
			e=d;

			// These conditions determine the acceptibility of the parabolic 
			//   fit. 
			if ((fabs(p) >= fabs(0.5*q*etemp)) || (p <= q*(a-x)) || (p >= q*(b-x)))
				// Here we take the golden section step into the larger of the two 
				//   segments 
				d=(TYPE) CGOLD*(e=(x >= xm ? a-x : b-x));
			else
			{
				// Take the parabolic step. 
				d=p/q;
				u=x+d;
				if ((u-a < tol2) || (b-u < tol2))
					d=(TYPE) SIGN(tol1,xm-x);
			}
		}
		else
		{
			d=(TYPE) CGOLD*(e=(x >= xm ? a-x : b-x));
		}

		u=(fabs(d) >= tol1 ? x+d : x+(TYPE)SIGN(tol1,d));

		// This is the one function evaluation per iteration. 
		fu=(*m_pFunc)(u);

		// Now decide what to do with our function evaluation. 
		// Housekeeping follows: 
		if (fu <= fx)
		{
			if (u >= x) a=x; else b=x;
			SHFT(v,w,x,u)
			SHFT(fv,fw,fx,fu)
		}
		else
		{
			if (u < x) a=u; else b=u;
			if (fu <= fw || w == x)
			{
				v=w;
				w=u;
				fv=fw;
				fw=fu;
			}
			else if (fu <= fv || v == x || v == w)
			{
				v=u;
				fv=fu;
			}
		}
		// Done with housekeeping.  Back for another iteration. 
	}

	finalValue.Set(fx);
	return x;
}

//////////////////////////////////////////////////////////////////////
// CBrentOptimizer<TYPE>::FindMinimumGrad
// 
// Given a function f, and given a bracketing triplet of abscissas
// ax, bx, cx, (such that bx is between ax and cx, and f(bx) < f(ax)
// and f(bx) < f(cx)), this routine isolates the minimum to a fractional
// precision of about tol using Brent's method.  The abscissa of the
// minimum is returned as xmin, and the minimum function value is returned
// as brent, the returned function value.
//
// This function was adapted from function brent, Press et al., Numerical
// Recipes in C, 2nd Ed. 1992.
// 
// ax,..,cx are function values needed to perform the minimization  
// tol is variable specifing the minimum agreement needed between successive 
// intertions before stopping 
//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE CBrentOptimizer<TYPE>::FindMinimumGrad(TYPE ax, TYPE bx, TYPE cx)
{
	// a and b must be in ascending order, but input abscissas need not be.
	TYPE a=(ax < cx ? ax : cx);
	TYPE b=(ax > cx ? ax : cx);

	TYPE x,w,v,u;
	x=w=v=bx;

	TYPE fx,fw,fv,fu;
	fw=fv=fx=(*m_pFunc)(x);

	TYPE dx,dw,dv, du;
	dw=dv=dx=m_pFunc->Grad(x)[0];

	
	TYPE d = 0.0;			// holds a temporary value for e
	TYPE e = 0.0;           // distance moved on the step before last 

	// Main function loop. 
	for (iteration.Set(0); iteration.Get() < ITER_MAX; 
			iteration.Set(iteration.Get()+1))
	{
		// computing mean x-value
		TYPE xm = (TYPE) 0.5 * (a + b);

		// computing bounding tolerances
		TYPE tol1 = tolerance.Get() * (TYPE) fabs(x) + (TYPE) ZEPS;
		TYPE tol2 = (TYPE) 2.0 * tol1;

		// test for convergence
		if ((TYPE) fabs(x - xm) <= (tol2 - (TYPE) 0.5 * (b-a)))
		{
			finalValue.Set(fx);
			return x;
		}

		if ((TYPE) fabs(e) > tol1)
		{
			// compute d1 with secant method
			TYPE d1;	
			if (dw != dx) 
				d1 = (w - x) * dx / (dx - dw);
			else					// otherwise,
				d1 = (TYPE) 2.0 * (b - a);	// initialize with out-of-bracket value

			TYPE d2;	
			if (dv != dx) 
				d2 = (v - x) * dx / (dx - dv);
			else
				d2 = (TYPE) 2.0 * (b - a);	// initialize with out-of-bracket value

			// which estimate?  must be within the bracket, and on the side 
			//		pointed to be the derivative at x
			TYPE u1 = x + d1;
			TYPE u2 = x + d2;

			BOOL bOK1 = (a - u1) * (u1 - b) > 0.0 && dx * d1 <= 0.0;
			BOOL bOK2 = (a - u2) * (u2 - b) > 0.0 && dx * d2 <= 0.0;

			TYPE olde = e;
			e = d;

			if (bOK1 || bOK2)
			{
				if (bOK1 && bOK2)
					d = (fabs(d1) < fabs(d2) ? d1 : d2);
				else if (bOK1)
					d = d1;
				else
					d = d2;

				if (fabs(d) <= fabs(0.5*olde))
				{
					u = x + d;
					if (u - a < tol2 || b - u < tol2)
						d = SIGN(tol1, xm - x);
				}
				else
				{
					// decide which segment by the sign of the derivative
					d = (TYPE) 0.5 * (e = (dx >= 0.0 ? a-x : b-x));
				}
			}
			else
			{
				// decide which segment by the sign of the derivative
				d = (TYPE) 0.5 * (e = (dx >= 0.0 ? a-x : b-x));
			}
		}
		else
		{
			// decide which segment by the sign of the derivative
			d = (TYPE) 0.5 * (e = (dx >= 0.0 ? a-x : b-x));
		}

		if (fabs(d) >= tol1)
		{
			u = x + d;
			fu = (*m_pFunc)(u);
		}
		else
		{
			u = x + SIGN(tol1, d);
			fu = (*m_pFunc)(u);
			if (fu > fx)
			{
				finalValue.Set(fx);
				return x;
			}
		}

		du = m_pFunc->Grad(u)[0];
		if (fu <= fx)
		{
			if (u >= x) 
				a = x;
			else
				b = x;
			MOV3(v, fv, dv, w, fw, dw);
			MOV3(w, fw, dw, x, fx, dx);
			MOV3(x, fx, fx, u, fu, du);
		}
		else
		{
			if (u < x) 
				a = u;
			else
				b = u;
			if (fu <= fw || w == x)
			{
				MOV3(v, fv, dv, w, fw, dw);
				MOV3(w, fw, dw, u, fu, du);
			}
			else if (fu < fv || v == x || v == w)
			{
				MOV3(v, fv, dv, u, fu, du);
			}
		}
	}

	ASSERT("Too many iterations\n");
	return 0.0;
}
