// TCP_NTCP_Optimizer.h: interface for the CTCP_NTCP_Optimizer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCP_NTCP_OPTIMIZER_H__B5F519D8_C124_48FA_9BDA_6BCEF60B62BC__INCLUDED_)
#define AFX_TCP_NTCP_OPTIMIZER_H__B5F519D8_C124_48FA_9BDA_6BCEF60B62BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ObjectiveFunction.h>

#include "Histogram.h"

class CTCP_NTCP_Optimizer : public CObjectiveFunction  
{
public:
	CTCP_NTCP_Optimizer();
	virtual ~CTCP_NTCP_Optimizer();

	long GetTargetCount() const;
	CHistogram *GetTargetHistogram(long nAt);
	void GetTargetParams(long nAt, 
		double *p_alpha, double *p_CPV);
	long AddTarget(CHistogram *pHisto, double alpha, double CPV);

	long GetOARCount() const;
	CHistogram *GetOARHistogram(long nAt);
	void GetOARParams(long nAt,
		double *p_m, double *p_n, double *p_D_max, double *TD50_17);
	long AddOAR(CHistogram *pHisto, double D, double n);

	// evaluates the ln of TCP / NTCP (components of objective function)
	REAL Eval_lnTCP(long nAt, CVectorN<> *pGrad = NULL);
	REAL Eval_lnNTCP(long nAt, CVectorN<> *pGrad = NULL);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);

private:
	CObArray m_arrTargetHisto;
	class CTCP_Params
	{
	public:
		double m_alpha;
		double m_CPV;
	};
	CArray<CTCP_Params, CTCP_Params&> m_arrTargetParams;

	CObArray m_arrOARHisto;
	class CNTCP_Params
	{
	public:
		double m_m;
		double m_n;
		double m_D_max;
		double m_TD50_1;
	};
	CArray<CNTCP_Params, CNTCP_Params&> m_arrOARParams;
};

#endif // !defined(AFX_TCP_NTCP_OPTIMIZER_H__B5F519D8_C124_48FA_9BDA_6BCEF60B62BC__INCLUDED_)
