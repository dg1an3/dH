// TCP_NTCP_Optimizer.cpp: implementation of the CTCP_NTCP_Optimizer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TCP_NTCP_Optimizer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// erfcc
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
double erfcc(double x)
{
	double t, z, ans;
	
	z = fabs(x);
	t = 1.0 / (1.0 + 0.5 * z);

	ans = t * exp(-z * z - 1.26551223 
		+ t * (1.00002368 + t * (0.37409196 
			+ t * (0.09678418 + t * (-0.18628806 
				+ t * (0.27886807 + t * (-1.13520398 
					+ t * (1.48851587 + t * (-0.82215223 
						+ t * 0.17087277)))))))));
	return x >= 0.0 ? ans : 2.0 - ans;

}	// erfcc

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::CTCP_NTCP_Optimizer()
// 
// constructs new CTCP_NTCP_Optimizer objects
//////////////////////////////////////////////////////////////////////
CTCP_NTCP_Optimizer::CTCP_NTCP_Optimizer()
	: CObjectiveFunction(FALSE)
{

}	// CTCP_NTCP_Optimizer::CTCP_NTCP_Optimizer()


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::~CTCP_NTCP_Optimizer()
// 
// disposes of CTCP_NTCP_Optimizer objects
//////////////////////////////////////////////////////////////////////
CTCP_NTCP_Optimizer::~CTCP_NTCP_Optimizer()
{

}	// CTCP_NTCP_Optimizer::~CTCP_NTCP_Optimizer()


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetTargetCount
// 
// returns the number of target histograms
//////////////////////////////////////////////////////////////////////
long CTCP_NTCP_Optimizer::GetTargetCount() const
{
	return m_arrTargetHisto.GetSize();

}	// CTCP_NTCP_Optimizer::GetTargetCount


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetTargetHistogram
// 
// returns a particular target histogram
//////////////////////////////////////////////////////////////////////
CHistogram *CTCP_NTCP_Optimizer::GetTargetHistogram(long nAt)
{
	return (CHistogram *) m_arrTargetHisto[nAt];

}	// CTCP_NTCP_Optimizer::GetOARHistogram


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetTargetParams
// 
// returns the target TCP parameters for the target
//////////////////////////////////////////////////////////////////////
void CTCP_NTCP_Optimizer::GetTargetParams(long nAt, 
		double *p_alpha, double *p_CPV)
{
	(*p_alpha) = m_arrTargetParams[nAt].m_alpha;
	(*p_CPV) = m_arrTargetParams[nAt].m_CPV;

}	// CTCP_NTCP_Optimizer::GetTargetParams


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetOARCount
// 
// returns the number of OAR histograms
//////////////////////////////////////////////////////////////////////
long CTCP_NTCP_Optimizer::GetOARCount() const
{
	return m_arrOARHisto.GetSize();

}	// CTCP_NTCP_Optimizer::GetOARCount


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetOARHistogram
// 
// returns the number of OAR histograms
//////////////////////////////////////////////////////////////////////
CHistogram *CTCP_NTCP_Optimizer::GetOARHistogram(long nAt)
{
	return (CHistogram *) m_arrOARHisto[nAt];

}	// CTCP_NTCP_Optimizer::GetOARHistogram


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::GetOARParams
// 
// returns the NTCP parameters for the organ-at-risk
//////////////////////////////////////////////////////////////////////
void CTCP_NTCP_Optimizer::GetOARParams(long nAt,
		double *p_m, double *p_n, double *p_D_max, double *pTD50_1)
{
	(*p_m) = m_arrOARParams[nAt].m_m;
	(*p_n) = m_arrOARParams[nAt].m_n;
	(*p_D_max) = m_arrOARParams[nAt].m_D_max;
	(*pTD50_1) = m_arrOARParams[nAt].m_TD50_1;

}	// CTCP_NTCP_Optimizer::GetOARParams


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::Eval_lnTCP
// 
// evaluates the ln of TCP value
//////////////////////////////////////////////////////////////////////
REAL CTCP_NTCP_Optimizer::Eval_lnTCP(long nAt, CVectorN<> *pGrad)
{
	// retrieve accessors to the histogram and volume
	CHistogram *pHisto = GetTargetHistogram(nAt);

	// get the corresponding TCP parameters
	double alpha, CPV;
	GetTargetParams(nAt, &alpha, &CPV);

	// get the base histogram
	CVectorN<>& arrBins = pHisto->GetGBins();

	// get binning values
	const REAL binWidth = pHisto->GetBinWidth();
	REAL binValue = pHisto->GetBinMinValue();

	// accumulate base TCP summation
	REAL sum = 0.0;
	for (int nAtBin = 0; nAtBin < arrBins.GetDim(); nAtBin++)
	{
		sum += -CPV * arrBins[nAtBin] * exp(alpha * binValue);
		binValue += binWidth;
	}

	return sum;

}	// CTCP_NTCP_Optimizer::Eval_lnTCP


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::Eval_lnNTCP
// 
// objective function = sum of log TCP for all targets + log NTCP for 
//		all OARs
//////////////////////////////////////////////////////////////////////
REAL CTCP_NTCP_Optimizer::Eval_lnNTCP(long nAt, CVectorN<> *pGrad)
{
	// retrieve accessors to the histogram and volume
	CHistogram *pHisto = GetOARHistogram(nAt);

	// get the corresponding NTCP parameters
	double m, n, D_max, TD50_1;
	GetOARParams(nAt, &m, &n, &D_max, &TD50_1);

	// get the base histogram
	const CVectorN<>& arrBins = pHisto->GetGBins();

	// get binning values
	const REAL binWidth = pHisto->GetBinWidth();
	REAL binValue = pHisto->GetBinMinValue();

	// accumulate effective volume for NTCP
	REAL effectiveVolume = 0.0;
	for (int nAtBin = 0; nAtBin < arrBins.GetDim(); nAtBin++)
	{
		REAL adjDose = pow(binValue / D_max, 1.0 / n);
		effectiveVolume += arrBins[nAtBin] * adjDose;
	}

	// compute effective TD50 (TD50 at effective volume)
	REAL TD50_effV = TD50_1 * pow(effectiveVolume, -n);

	// compute t as a function of dose (D_max)
	REAL t_dose = D_max / (m * TD50_effV) - 1.0 / m;

	// return error function of t
	return erfcc(t_dose);

}	// CTCP_NTCP_Optimizer::Eval_lnNTCP


//////////////////////////////////////////////////////////////////////
// CTCP_NTCP_Optimizer::operator()
// 
// objective function = sum of log TCP for all targets + log NTCP for 
//		all OARs
//////////////////////////////////////////////////////////////////////
REAL CTCP_NTCP_Optimizer::operator()(const CVectorN<>& vInput, 
	CVectorN<> *pGrad)
{
	if (pGrad)
	{
		pGrad->SetDim(GetTargetHistogram(0)->Get_dVolumeCount());
		pGrad->SetZero();
	}

	REAL sum = 0.0;
	for (int nAtTarget = 0; nAtTarget < this->GetTargetCount(); nAtTarget++)
	{
		CVectorN<> v_dln_TCP_dH;
		v_dln_TCP_dH.SetDim(GetTargetHistogram(nAtTarget)->Get_dVolumeCount());

		sum += Eval_lnTCP(nAtTarget, &v_dln_TCP_dH);
		(*pGrad) += v_dln_TCP_dH;
	}

	for (int nAt_OAR = 0; nAt_OAR < GetOARCount(); nAt_OAR++)
	{
		CVectorN<> v_dln_NTCP_dH;
		v_dln_NTCP_dH.SetDim(GetOARHistogram(nAt_OAR)->Get_dVolumeCount());

		sum += Eval_lnNTCP(nAt_OAR, &v_dln_NTCP_dH);
		(*pGrad) += v_dln_NTCP_dH;
	}

	return sum;

}	// CTCP_NTCP_Optimizer::operator()


