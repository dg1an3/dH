// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.cpp 644 2009-11-05 17:46:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "ConjGradOptimizer.h"

#include <vnl/algo/vnl_brent_minimizer.h>
#include <vnl/algo/vnl_symmetric_eigensystem.h>

#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
class LineProjectionFunction : public vnl_cost_function
{
public:
	LineProjectionFunction(vnl_cost_function& projectedFunction)
		: m_projectedFunction(projectedFunction) { }

	DeclareMember(Point, vnl_vector<REAL>);
	DeclareMember(Direction, vnl_vector<REAL>);

	virtual double f(vnl_vector<double> const& x)
	{
		m_vEvalPoint = GetDirection();
		m_vEvalPoint *= x[0];
		m_vEvalPoint += GetPoint();

		// return evaluate of projected function
		return m_projectedFunction.f(m_vEvalPoint);
	}

private:
	// the function to project
	vnl_cost_function& m_projectedFunction;

	// temporary store of evaluation point
	mutable vnl_vector<REAL> m_vEvalPoint;
};


///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

// maximum iterations
const int ITER_MAX = 500;

// z-epsilon -- small number to protect against fractional accuracy for
//		a minimum that happens to be exactly zero.
const REAL ZEPS = (REAL) 1.0e-10;

///////////////////////////////////////////////////////////////////////////////
static REAL
	GetGradConvergenceTol()
	// Gradient-norm convergence threshold from BRIMSTONE_GRAD_TOL. When > 0, the
	//	optimizer converges on |grad F| <= this value instead of the relative
	//	change in F. This is scale-invariant to an additive offset in the
	//	objective -- e.g. the -w*entropy term in F = KL - w*H, which holds |F|
	//	away from 0 and makes the default relative test declare convergence early
	//	-- so it stops only when the gradient (the KL vs entropy force balance)
	//	is actually small. Read once, cached. 0/unset => original relative test.
{
	static bool s_init = false;
	static REAL s_tol = 0.0;
	if (!s_init)
	{
		s_init = true;
		const char *pEnv = getenv("BRIMSTONE_GRAD_TOL");
		if (pEnv != NULL)
			s_tol = (REAL) atof(pEnv);
	}
	return s_tol;
}

///////////////////////////////////////////////////////////////////////////////
static int
	GetCGRestartInterval()
	// Conjugate-gradient restart interval from BRIMSTONE_CG_RESTART. When > 0,
	//	the direction update uses PR+ (beta clamped to >= 0) AND restarts to
	//	steepest descent every N iterations. Plain Polak-Ribiere (the historical
	//	behavior, N=0/unset) loses conjugacy and zig-zags on the non-quadratic,
	//	ill-conditioned objective produced by the softmax-entropy regularizer,
	//	stalling with a gradient that creeps but never converges. Read once,
	//	cached. 0/unset => original plain-PR behavior.
{
	static bool s_init = false;
	static int s_interval = 0;
	if (!s_init)
	{
		s_init = true;
		const char *pEnv = getenv("BRIMSTONE_CG_RESTART");
		if (pEnv != NULL)
			s_interval = atoi(pEnv);
	}
	return s_interval;
}

///////////////////////////////////////////////////////////////////////////////
DynamicCovarianceOptimizer::DynamicCovarianceOptimizer(DynamicCovarianceCostFunction *pFunc)
	: // COptimizer(pFunc)
	m_pCostFunction(pFunc)
	, m_bCalcVar(false)
	, m_bComputeFreeEnergy(false)
	, m_Entropy(0.0)
	, m_FreeEnergy(0.0)
{
}	// CConjGradOptimizer::CConjGradOptimizer

///////////////////////////////////////////////////////////////////////////////
// bool 
//	CConjGradOptimizer::minimize(vnl_vector<REAL>& v)
// const CVectorN<>& 
vnl_nonlinear_minimizer::ReturnCodes 
	DynamicCovarianceOptimizer::minimize(vnl_vector<REAL>& vInit)
{
	LineProjectionFunction m_lineFunction(*m_pCostFunction);
	vnl_brent_minimizer m_optimizeBrent(m_lineFunction);
	m_optimizeBrent.set_x_tolerance(GetLineOptimizerTolerance());

	// initialize, if we are calculating adaptive variance?
	InitializeDynamicCovariance(vInit.size());

	// store the initial parameter vector
	// m_vFinalParam.SetDim(vInit.GetDim());
	m_FinalParameter = vInit; // const_cast<CVectorN<>&>(vInit).GetVnlVector();

	// set the dimension of the current direction
	m_vGrad.set_size(vInit.size());		// TODO: is this needed (check logic of compute)

	// evaluate the function at the initial point, storing
	//		the gradient as the current direction
	m_pCostFunction->compute(m_FinalParameter, &m_FinalValue, &m_vGrad);
	m_vGrad *= R(-1.0);

	// if we are too short,
	if (m_vGrad.magnitude() < 1e-8)
	{
		Log("Gradient too small -- adding length");
		// NOTE: must be a step large enough for the line minimizer's initial
		//	bracket to produce a distinguishable function value -- get_x_tolerance()
		//	is a convergence tolerance, not a usable step scale, and using it here
		//	made the bracket's two initial probes tie, tripping vxl's fb < fa assert.
		RandomVector(R(1.0), &m_vGrad[0], m_vGrad.size());
	}

	// set the initial (steepest descent) direction
	m_vDir = m_vGrad;

	BOOL bConvergence = FALSE;
	ReturnCodes retCode = FAILED_TOO_MANY_ITERATIONS;
	for (num_iterations_ = 0; (num_iterations_ < ITER_MAX) && !bConvergence; num_iterations_++)
	{
		///////////////////////////////////////////////////////////////////////////////
		// line minimization

		// set up the direction for the line minimization
		m_lineFunction.SetPoint(m_FinalParameter);

		// vnl_brent_minimizer::minimize(x) always brackets with a FIXED unit
		//	step (x-1, x+1) regardless of the direction vector's magnitude, so
		//	the direction handed to it must itself be normalized to a
		//	consistent O(1) scale -- m_vDir's raw magnitude (steepest descent
		//	initially, Polak-Ribiere updated thereafter) can be arbitrarily
		//	small at any iteration, not just the first, which otherwise
		//	produces an imperceptible step and ties fa/fb/fc, tripping vxl's
		//	fb < fa / fb < fc bracket assertions. m_vDir itself must stay
		//	unnormalized -- the CG recurrence below depends on its true scale.
		vnl_vector<REAL> vLineDir = m_vDir;
		vLineDir.normalize();
		m_lineFunction.SetDirection(vLineDir);

		// now launch a line optimization
		REAL lambda = m_optimizeBrent.minimize(0);
		REAL new_fv = m_optimizeBrent.f_at_last_minimum();

		// guard against a degenerate/tied bracket -- vnl_bracket_minimum can
		//	return a flat bracket when the objective doesn't change over a
		//	unit step (most likely near convergence, where the gradient is
		//	small in every direction), which violates vnl_brent_minimizer's
		//	internal fb<fa/fb<fc preconditions (an assert in debug builds,
		//	silently skipped in release) and can produce a non-finite lambda
		//	from its parabolic interpolation. Applying a non-finite lambda
		//	corrupts m_FinalParameter and cascades into heap corruption
		//	downstream (e.g. a NaN dose value producing a garbage histogram
		//	bin index), so treat this iteration as no improvement instead.
		if (!_finite(lambda) || !_finite(new_fv))
		{
			lambda = 0.0;
			new_fv = m_FinalValue;
		}

		// update the final parameter value
		m_vLambdaScaled = m_lineFunction.GetDirection();
		m_vLambdaScaled *= lambda;
		m_FinalParameter += m_vLambdaScaled;

		// relative-change convergence test (the historical default)
		bool bRelConverged = (2.0 * fabs(m_FinalValue - new_fv)
			<= get_x_tolerance() * (fabs(m_FinalValue) + fabs(new_fv) + ZEPS));

		// store the previous lambda value
		m_FinalValue = new_fv;

		// need to call-back?
		if (m_pCallbackFunc)
		{
			if (!(*m_pCallbackFunc)(this, m_pCallbackParam))
			{
				// request to terminate
				// num_iterations_ = -1;
				//vInit = m_vFinalParam;
				retCode = FAILED_USER_REQUEST;
				break;
				//return retCode; // m_vFinalParam;
			}
		}

		// are we calculating adaptive variance?
		UpdateDynamicCovariance();

		///////////////////////////////////////////////////////////////////////////////
		// Update Direction

		// Compute the gradient at the new point up front: it is needed both for
		//	the Polak-Ribiere direction update below AND for the optional
		//	gradient-norm convergence test. m_vGradPrev holds the previous
		//	gradient g_k (its squared norm gg is the PR denominator).
		m_vGradPrev = m_vGrad;
		REAL gg = dot_product(m_vGradPrev, m_vGradPrev);
		m_pCostFunction->gradf(m_FinalParameter, m_vGrad);
		m_vGrad *= -1.0;								// g_{k+1} = -grad F(x_{k+1})

		const REAL gradNorm = m_vGrad.magnitude();		// |grad F| at the new point
		{
			RTM_TRACE("iter %d: F=%.6g |gradF|=%.6g",
				num_iterations_, (double) m_FinalValue, (double) gradNorm);
		}

		// choose the convergence criterion: gradient-norm if BRIMSTONE_GRAD_TOL
		//	is set (scale-invariant to the objective's additive offset), else the
		//	historical relative-change test. gg==0 always converges.
		const REAL gradTol = GetGradConvergenceTol();
		if (gradTol > 0.0)
			bConvergence = (gradNorm <= gradTol) || (gg == 0.0);
		else
			bConvergence = bRelConverged || (gg == 0.0);

		// must have performed at least one full iteration
		if (!bConvergence)
		{
			// compute numerator for gamma (Polak-Ribiera formula)
			REAL dgg = dot_product(m_vGrad, m_vGrad) - dot_product(m_vGradPrev, m_vGrad);
			REAL beta = dgg / gg;

			// optional CG robustness (BRIMSTONE_CG_RESTART = N > 0): PR+ clamps
			//	beta >= 0 (auto-restart on loss of descent), and every N
			//	iterations force a steepest-descent restart (beta = 0). Restores
			//	conjugacy on the ill-conditioned entropy-regularized objective.
			const int nRestart = GetCGRestartInterval();
			if (nRestart > 0)
			{
				if (beta < 0.0)
					beta = 0.0;
				if ((num_iterations_ % nRestart) == (nRestart - 1))
					beta = 0.0;
			}

			// otherwise, update the direction: d = g + beta*d
			m_vDir *= beta;
			m_vDir += m_vGrad;
		}
		else
		{
			retCode = CONVERGED_XTOL;
		}
	}

	//if (!bConvergence)
	//{
	//	retCode = FAILED_TOO_MANY_ITERATIONS;
	//}

	vInit = m_FinalParameter;

	// return the last parameter vector
	//m_vFinalParamTemp.SetDim(m_vFinalParam.size());
	//m_vFinalParamTemp.GetVnlVector() = m_vFinalParam;
	return retCode; // m_vFinalParamTemp;

}	// CConjGradOptimizer::Optimize


//////////////////////////////////////////////////////////////////////////////
void
	DynamicCovarianceOptimizer::SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax)
	// used to set up the variance min / max calculation
{
	// set the flag
	m_bCalcVar = bCalcVar;

	// store min / max
	m_varMin = varMin;
	m_varMax = varMax;

	// set up for the objective function
	m_pCostFunction->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);

}	// CConjGradOptimizer::SetAdaptiveVariance


//////////////////////////////////////////////////////////////////////////////
void
	DynamicCovarianceOptimizer::SetComputeFreeEnergy(bool bComputeFreeEnergy)
	// used to enable explicit free energy calculation
{
	m_bComputeFreeEnergy = bComputeFreeEnergy;

}	// DynamicCovarianceOptimizer::SetComputeFreeEnergy


//////////////////////////////////////////////////////////////////////////////
REAL
	DynamicCovarianceOptimizer::ComputeEntropyFromCovariance(const vnl_matrix<REAL>& covar)
	// computes differential entropy from covariance matrix
	// H = 0.5 * log(det(2πe * Σ))
	//   = 0.5 * (n * log(2πe) + log(det(Σ)))
{
	int nDim = covar.rows();

	// compute determinant via sum of log eigenvalues (more numerically stable)
	vnl_matrix<REAL> covarSymmetric = (covar + covar.transpose()) * 0.5;
	vnl_symmetric_eigensystem<REAL> eigenSystem(covarSymmetric);

	REAL logDet = 0.0;
	for (int i = 0; i < nDim; i++)
	{
		REAL eigenvalue = eigenSystem.get_eigenvalue(i);
		// protect against negative or zero eigenvalues
		if (eigenvalue > 1e-10)
		{
			logDet += log(eigenvalue);
		}
		else
		{
			// use minimum eigenvalue for numerical stability
			logDet += log(1e-10);
		}
	}

	// entropy = 0.5 * (n * log(2πe) + log(det(Σ)))
	const REAL log2PiE = log(2.0 * 3.14159265358979323846 * 2.71828182845904523536);
	REAL entropy = 0.5 * (nDim * log2PiE + logDet);

	return entropy;

}	// DynamicCovarianceOptimizer::ComputeEntropyFromCovariance


//////////////////////////////////////////////////////////////////////////////
void
	DynamicCovarianceOptimizer::InitializeDynamicCovariance(int nDim)
{
	if (!m_bCalcVar)
		return;

	// initialize orthogonal basis matrix
	m_mOrthoBasis.set_size(nDim, nDim); 
	m_mOrthoBasis.set_identity();
	m_mSearchedDir.set_size(nDim, nDim);
	m_mSearchedDir.set_identity();

	m_vAdaptVariance.SetDim(nDim);
	for (int nN = 0; nN < m_vAdaptVariance.GetDim(); nN++)
	{
		m_vAdaptVariance[nN] = m_varMax;
	}

	m_pCostFunction->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);
}

//////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceOptimizer::UpdateDynamicCovariance()
{
	if (!m_bCalcVar)
		return;

	// add direction to orthogonal basis
	vnl_vector<REAL> vDirNorm = m_vDir;
	vDirNorm.normalize();
	m_mSearchedDir.set_column(num_iterations_, vDirNorm);
	m_mOrthoBasis.set_column(num_iterations_, m_mSearchedDir.get_column(num_iterations_));

	// stores the projection vector
	vnl_vector<REAL> vProj;
	vProj.set_size(m_mOrthoBasis.rows());

	// now use GSO to make sure basis is orthogonal to already searched directions
	for (int nDir = num_iterations_-1; nDir >= 0; nDir--)
	{
		vnl_vector<REAL> vOrtho;
		vOrtho.set_size(m_mSearchedDir.rows());
		vOrtho = m_mSearchedDir.get_column(nDir);
		for (int nDirOrtho = nDir+1; nDirOrtho < num_iterations_; nDirOrtho++)
		{					
			REAL projScale = dot_product(vOrtho, m_mOrthoBasis.get_column(nDirOrtho));
			vOrtho -= projScale * m_mOrthoBasis.get_column(nDirOrtho);
		}
		vOrtho.normalize(); 
		m_mOrthoBasis.set_column(nDir, vOrtho);
	}

	// now use GSO to make sure basis is orthogonal to already searched directions
	for (int nDir = num_iterations_+1; nDir < m_mOrthoBasis.columns(); nDir++)
	{
		vnl_vector<REAL> vOrtho;
		vOrtho.set_size(m_mSearchedDir.rows());
		vOrtho = m_mSearchedDir.get_column(nDir);
		for (int nDirOrtho = nDir-1; nDirOrtho >= 0; nDirOrtho--)
		{
			REAL projScale = dot_product(vOrtho, m_mOrthoBasis.get_column(nDirOrtho));
			vOrtho -= projScale * m_mOrthoBasis.get_column(nDirOrtho);
		}
		vOrtho.normalize();
		m_mOrthoBasis.set_column(nDir, vOrtho);
	}

	vnl_matrix<REAL> mScaling;
	mScaling.set_size(m_mOrthoBasis.rows(), m_mOrthoBasis.columns());
	mScaling.fill(0.0);
	for (int nScale = 0; nScale < m_vDir.size(); nScale++)
	{
		REAL scale = 1.0;
		if (nScale < num_iterations_)
			scale = pow(4.0, nScale) / pow(4.0, (double) num_iterations_);

		mScaling(nScale, nScale) = 1.0 / (scale * (m_varMax - m_varMin) + m_varMin);
	}

	vnl_matrix<REAL> mOrthoBasisT = m_mOrthoBasis.transpose();
	vnl_matrix<REAL> covar = mOrthoBasisT;
	covar *= mScaling;
	covar *= m_mOrthoBasis;
	for (int nDim = 0; nDim < m_vDir.size(); nDim++)
	{
		m_vAdaptVariance[nDim] = 1.0 / covar(nDim, nDim);
	}

	// compute explicit free energy if enabled
	if (m_bComputeFreeEnergy)
	{
		// compute entropy from the covariance matrix
		m_Entropy = ComputeEntropyFromCovariance(covar);

		// free energy = KL divergence (objective value) - Entropy
		// note: m_FinalValue contains the KL divergence sum (expected log likelihood term)
		m_FreeEnergy = m_FinalValue - m_Entropy;

		{
			RTM_TRACE("Iteration %d: KL=%.6f, Entropy=%.6f, FreeEnergy=%.6f",
				(int) num_iterations_, (double) m_FinalValue,
				(double) m_Entropy, (double) m_FreeEnergy);
		}
	}

	// now reset the final value, using the new AV vector
	m_FinalValue = m_pCostFunction->f(m_FinalParameter);
}
